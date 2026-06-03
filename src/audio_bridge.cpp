#include "audio_bridge.h"
#include <WiFi.h>
#include <driver/i2s.h>
#include <AsyncWebSocket.h>
#include <cstring>

static const char AUDIO_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32 Audio Bridge</title>
<style>
body{font-family:sans-serif;background:#1a1a2e;color:#eee;margin:1em;max-width:480px}
h1{color:#00ff88}button{background:#00ff88;color:#111;border:none;padding:.6em 1.2em;cursor:pointer;margin:.5em .5em 0 0}
#st{margin-top:1em;color:#aaa}
</style>
</head>
<body>
<h1>Audio Bridge</h1>
<p>Empfang vom Funk → Browser · Mikrofon → Funk-Mic/Line-In</p>
<button id="start">Verbinden</button>
<button id="stop" disabled>Trennen</button>
<p id="st">Bereit</p>
<script>
const SR=48000,FRAME=960;
let ws,ctx,playNode,proc,micStream,seq=0;
const st=document.getElementById('st');
function log(m){st.textContent=m;}
document.getElementById('start').onclick=async()=>{
  ctx=new AudioContext({sampleRate:SR});
  const proto=location.protocol==='https:'?'wss:':'ws:';
  const p=location.pathname;
  const wspath=p.indexOf('_c')>=0?'/ws/audio_c':(p.indexOf('_b')>=0?'/ws/audio_b':'/ws/audio');
  ws=new WebSocket(proto+'//'+location.host+wspath);
  ws.binaryType='arraybuffer';
  ws.onopen=async()=>{
    log('WebSocket verbunden');
    micStream=await navigator.mediaDevices.getUserMedia({audio:{sampleRate:SR,channelCount:1,echoCancellation:true},video:false});
    const src=ctx.createMediaStreamSource(micStream);
    proc=ctx.createScriptProcessor(FRAME,1,1);
    proc.onaudioprocess=e=>{
      if(ws.readyState!==1)return;
      const f=e.inputBuffer.getChannelData(0);
      const pcm=new Int16Array(FRAME);
      for(let i=0;i<FRAME;i++)pcm[i]=Math.max(-32768,Math.min(32767,f[i]*32767|0));
      const hdr=new ArrayBuffer(10);
      const dv=new DataView(hdr);
      dv.setUint32(0,0x45535041,true);
      dv.setUint32(4,seq++,true);
      dv.setUint16(8,FRAME,true);
      const out=new Uint8Array(10+FRAME*2);
      out.set(new Uint8Array(hdr),0);
      out.set(new Uint8Array(pcm.buffer),10);
      ws.send(out.buffer);
    };
    src.connect(proc);proc.connect(ctx.destination);
    playNode=ctx.createScriptProcessor(FRAME,1,1);
    playNode.onaudioprocess=e=>{
      const o=e.outputBuffer.getChannelData(0);
      if(!playNode._q||!playNode._q.length){o.fill(0);return;}
      const pcm=playNode._q.shift();
      for(let i=0;i<FRAME;i++)o[i]=pcm[i]/32768;
    };
    playNode._q=[];
    playNode.connect(ctx.destination);
    ws.onmessage=ev=>{
      const u=new Uint8Array(ev.data);
      if(u.length<10)return;
      const dv=new DataView(u.buffer,u.byteOffset,u.byteLength);
      if(dv.getUint32(0,true)!==0x45535041)return;
      const n=dv.getUint16(8,true);
      const pcm=new Float32Array(n);
      for(let i=0;i<n;i++)pcm[i]=dv.getInt16(10+i*2,true)/32768;
      if(playNode._q.length<8)playNode._q.push(pcm);
    };
    document.getElementById('start').disabled=true;
    document.getElementById('stop').disabled=false;
  };
  ws.onclose=()=>log('Getrennt');
  ws.onerror=()=>log('WebSocket-Fehler');
};
document.getElementById('stop').onclick=()=>{
  if(ws)ws.close();
  if(micStream)micStream.getTracks().forEach(t=>t.stop());
  if(ctx)ctx.close();
  document.getElementById('start').disabled=false;
  document.getElementById('stop').disabled=true;
  log('Bereit');
};
</script>
</body>
</html>
)rawliteral";

bool AudioBridge::begin(const RadioChannelConfig* ch, uint8_t channelIndex) {
    _ch = ch;
    _channelIndex = channelIndex;
    if (!ch || !ch->audioEnabled) {
        Serial.printf("[audio-%s] disabled\n", ch ? ch->label : "?");
        return true;
    }

    if (ch->audioLink == AudioLinkType::USB_UAC) {
        Serial.printf("[audio-%s] USB-UAC: S3 Host + Hub (Treiber folgt)\n", ch->label);
        return true;
    }

    _frameSamples = audioFrameSamples(ch->audioSampleRate);
    if (_frameSamples == 0 || _frameSamples > AUDIO_FRAME_SAMPLES_MAX)
        _frameSamples = audioFrameSamples(AUDIO_SAMPLE_RATE);

    if (!initI2s()) {
        Serial.printf("[audio-%s] I2S init failed\n", ch->label);
        return false;
    }

    if (!_udpIn.begin(ch->audioPortIn)) {
        Serial.printf("[audio-%s] UDP in :%u bind failed\n", ch->label, ch->audioPortIn);
        return false;
    }

    _running = true;
    char taskName[12];
    snprintf(taskName, sizeof(taskName), "audio%c", ch->label[0]);
    xTaskCreatePinnedToCore(taskEntry, taskName, 12288, this, 1, nullptr, 0);

    Serial.printf("[audio-%s] UDP out:%u in:%u %lu Hz I2S%d\n",
                  ch->label, ch->audioPortOut, ch->audioPortIn,
                  (unsigned long)ch->audioSampleRate, (int)_i2sPort);
    return true;
}

void AudioBridge::attachToServer(AsyncWebServer* server) {
    if (!server || !_ch || !_ch->audioEnabled || !_running) return;

    const char* pagePath = "/audio";
    const char* wsPath = AUDIO_WS_PATH;
    if (_channelIndex == 1) {
        pagePath = "/audio_b";
        wsPath = "/ws/audio_b";
    } else if (_channelIndex == 2) {
        pagePath = "/audio_c";
        wsPath = "/ws/audio_c";
    }

    server->on(pagePath, HTTP_GET, [](AsyncWebServerRequest* req) {
        req->send_P(200, "text/html", AUDIO_PAGE);
    });

    _ws = new AsyncWebSocket(wsPath);
    AudioBridge* self = this;
    _ws->onEvent([self](AsyncWebSocket* ws, AsyncWebSocketClient* client,
                        AwsEventType type, void* arg, uint8_t* data, size_t len) {
        if (!self->_running) return;
        if (type == WS_EVT_CONNECT) {
            Serial.printf("[audio-%s] WS %s\n", self->_ch->label,
                          client->remoteIP().toString().c_str());
        } else if (type == WS_EVT_DISCONNECT) {
            (void)ws;
        } else if (type == WS_EVT_DATA && arg) {
            AwsFrameInfo* info = (AwsFrameInfo*)arg;
            if (info->opcode == WS_BINARY && info->index == 0 && info->final)
                self->onClientAudio(data, len, true);
        }
    });
    server->addHandler(_ws);
}

bool AudioBridge::initI2s() {
    const uint32_t rate = _ch->audioSampleRate;
    int bclk = I2S_BCLK_PIN, lrck = I2S_LRCK_PIN, dout = I2S_DOUT_PIN, din = I2S_DIN_PIN;

    _i2sPort = (_channelIndex == 0) ? (int)I2S_NUM_0 : (int)I2S_NUM_1;
    if (_channelIndex > 0) {
#if defined(I2S1_BCLK_PIN)
        bclk = I2S1_BCLK_PIN;
        lrck = I2S1_LRCK_PIN;
        dout = I2S1_DOUT_PIN;
        din = I2S1_DIN_PIN;
#else
        Serial.printf("[audio-%s] kein 2. I2S-Bus definiert\n", _ch->label);
        return false;
#endif
    }

    i2s_config_t i2s_cfg = {};
    i2s_cfg.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX);
    i2s_cfg.sample_rate = rate;
    i2s_cfg.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
    i2s_cfg.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
    i2s_cfg.communication_format = I2S_COMM_FORMAT_STAND_I2S;
    i2s_cfg.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
    i2s_cfg.dma_buf_count = 4;
    i2s_cfg.dma_buf_len = _frameSamples;
    i2s_cfg.use_apll = false;
    i2s_cfg.tx_desc_auto_clear = true;

    if (i2s_driver_install((i2s_port_t)_i2sPort, &i2s_cfg, 0, nullptr) != ESP_OK)
        return false;

    i2s_pin_config_t pins = {};
    pins.bck_io_num = bclk;
    pins.ws_io_num = lrck;
    pins.data_out_num = dout;
    pins.data_in_num = din;
    pins.mck_io_num = I2S_PIN_NO_CHANGE;

    if (i2s_set_pin((i2s_port_t)_i2sPort, &pins) != ESP_OK) return false;
    i2s_set_clk((i2s_port_t)_i2sPort, rate, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
    return true;
}

void AudioBridge::taskEntry(void* arg) {
    auto* self = static_cast<AudioBridge*>(arg);
    while (self->_running)
        self->loopOnce();
    vTaskDelete(nullptr);
}

void AudioBridge::loopOnce() {
    if (!_running || _ch->audioLink != AudioLinkType::I2S) {
        vTaskDelay(pdMS_TO_TICKS(100));
        return;
    }

    int16_t buf[AUDIO_FRAME_SAMPLES_MAX];
    size_t bytesRead = 0;
    size_t wantBytes = _frameSamples * sizeof(int16_t);
    i2s_read((i2s_port_t)_i2sPort, buf, wantBytes, &bytesRead, pdMS_TO_TICKS(50));
    size_t n = bytesRead / sizeof(int16_t);
    if (n > 0) sendToClient(buf, n);

    int packetSize = _udpIn.parsePacket();
    if (packetSize > 0) {
        uint8_t rxBuf[sizeof(AudioPacketHdr) + AUDIO_FRAME_SAMPLES_MAX * sizeof(int16_t)];
        int len = _udpIn.read(rxBuf, sizeof(rxBuf));
        if (!_udpActive) {
            _udpClientIp = _udpIn.remoteIP();
            _udpClientPortOut = _ch->audioPortOut;
            _udpActive = true;
            Serial.printf("[audio-%s] UDP client %s\n", _ch->label,
                          _udpClientIp.toString().c_str());
        }
        onClientAudio(rxBuf, len, false);
    }
}

void AudioBridge::sendToClient(const int16_t* samples, size_t n) {
    if (n == 0) return;
    if (n > AUDIO_FRAME_SAMPLES_MAX) n = AUDIO_FRAME_SAMPLES_MAX;

    uint8_t pkt[sizeof(AudioPacketHdr) + AUDIO_FRAME_SAMPLES_MAX * sizeof(int16_t)];
    AudioPacketHdr* hdr = reinterpret_cast<AudioPacketHdr*>(pkt);
    hdr->magic = AUDIO_MAGIC;
    hdr->seq = _seqOut++;
    hdr->nSamples = (uint16_t)n;
    memcpy(pkt + sizeof(AudioPacketHdr), samples, n * sizeof(int16_t));
    size_t pktLen = sizeof(AudioPacketHdr) + n * sizeof(int16_t);

    if (_ws) {
        for (auto& c : _ws->getClients()) {
            if (c.status() == WS_CONNECTED)
                c.binary(pkt, pktLen);
        }
    }

    if (_udpActive && _udpClientIp) {
        _udpOut.beginPacket(_udpClientIp, _udpClientPortOut);
        _udpOut.write(pkt, pktLen);
        _udpOut.endPacket();
    }
}

void AudioBridge::onClientAudio(const uint8_t* data, size_t len, bool fromWs) {
    (void)fromWs;
    if (len < sizeof(AudioPacketHdr)) return;
    const auto* hdr = reinterpret_cast<const AudioPacketHdr*>(data);
    if (hdr->magic != AUDIO_MAGIC) return;
    size_t pcmBytes = hdr->nSamples * sizeof(int16_t);
    if (len < sizeof(AudioPacketHdr) + pcmBytes) return;
    const int16_t* pcm = reinterpret_cast<const int16_t*>(data + sizeof(AudioPacketHdr));
    size_t written = 0;
    i2s_write((i2s_port_t)_i2sPort, pcm, pcmBytes, &written, pdMS_TO_TICKS(50));
}
