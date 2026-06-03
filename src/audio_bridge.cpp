#include "audio_bridge.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <driver/i2s.h>
#include <AsyncWebSocket.h>
#include <cstring>

AudioBridge* AudioBridge::_instance = nullptr;

static WiFiUDP s_udpIn;
static WiFiUDP s_udpOut;
static AsyncWebSocket* s_ws = nullptr;

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
const SR=16000,FRAME=320;
let ws,ctx,playNode,proc,micStream,seq=0;
const st=document.getElementById('st');
function log(m){st.textContent=m;}
document.getElementById('start').onclick=async()=>{
  ctx=new AudioContext({sampleRate:SR});
  const proto=location.protocol==='https:'?'wss:':'ws:';
  ws=new WebSocket(proto+'//'+location.host+'/ws/audio');
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

bool AudioBridge::begin(const AppConfig* cfg) {
    _instance = this;
    _cfg = cfg;
    if (!cfg->audioEnabled) {
        Serial.println("[audio] disabled in config");
        return true;
    }

    if (!initI2s()) {
        Serial.println("[audio] I2S init failed");
        return false;
    }

    if (!s_udpIn.begin(cfg->audioPortIn)) {
        Serial.println("[audio] UDP in bind failed");
        return false;
    }

    _running = true;
    xTaskCreatePinnedToCore(taskEntry, "audio", 8192, this, 1, nullptr, 0);

    Serial.printf("[audio] UDP out :%u (radio→client) in :%u (client→radio)\n",
                  cfg->audioPortOut, cfg->audioPortIn);
    Serial.printf("[audio] %u Hz mono, Web UI /audio\n", cfg->audioSampleRate);
    return true;
}

void AudioBridge::attachToServer(AsyncWebServer* server) {
    if (!server || !_cfg || !_cfg->audioEnabled) return;

    server->on("/audio", HTTP_GET, [](AsyncWebServerRequest* req) {
        req->send_P(200, "text/html", AUDIO_PAGE);
    });

    s_ws = new AsyncWebSocket(AUDIO_WS_PATH);
    s_ws->onEvent([](AsyncWebSocket* ws, AsyncWebSocketClient* client,
                     AwsEventType type, void* arg, uint8_t* data, size_t len) {
        if (!_instance || !_instance->_running) return;
        if (type == WS_EVT_CONNECT) {
            Serial.printf("[audio] WS client %s\n",
                          client->remoteIP().toString().c_str());
        } else if (type == WS_EVT_DISCONNECT) {
            (void)ws;
        } else if (type == WS_EVT_DATA && arg) {
            AwsFrameInfo* info = (AwsFrameInfo*)arg;
            if (info->opcode == WS_BINARY && info->index == 0 && info->final) {
                _instance->onClientAudio(data, len, true);
            }
        }
    });
    server->addHandler(s_ws);
}

bool AudioBridge::initI2s() {
    const uint32_t rate = _cfg->audioSampleRate;

    i2s_config_t i2s_cfg = {};
    i2s_cfg.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX);
    i2s_cfg.sample_rate = rate;
    i2s_cfg.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
    i2s_cfg.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
    i2s_cfg.communication_format = I2S_COMM_FORMAT_STAND_I2S;
    i2s_cfg.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
    i2s_cfg.dma_buf_count = 4;
    i2s_cfg.dma_buf_len = AUDIO_FRAME_SAMPLES;
    i2s_cfg.use_apll = false;
    i2s_cfg.tx_desc_auto_clear = true;

    if (i2s_driver_install(I2S_NUM_0, &i2s_cfg, 0, nullptr) != ESP_OK)
        return false;

    i2s_pin_config_t pins = {};
    pins.bck_io_num = I2S_BCLK_PIN;
    pins.ws_io_num = I2S_LRCK_PIN;
    pins.data_out_num = I2S_DOUT_PIN;
    pins.data_in_num = I2S_DIN_PIN;
    pins.mck_io_num = I2S_PIN_NO_CHANGE;

    if (i2s_set_pin(I2S_NUM_0, &pins) != ESP_OK) return false;
    i2s_set_clk(I2S_NUM_0, rate, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
    return true;
}

void AudioBridge::taskEntry(void* arg) {
    auto* self = static_cast<AudioBridge*>(arg);
    while (self->_running) {
        self->loopOnce();
    }
    vTaskDelete(nullptr);
}

void AudioBridge::loopOnce() {
    int16_t buf[AUDIO_FRAME_SAMPLES];
    size_t bytesRead = 0;
    i2s_read(I2S_NUM_0, buf, sizeof(buf), &bytesRead, pdMS_TO_TICKS(50));
    size_t n = bytesRead / sizeof(int16_t);
    if (n > 0) sendToClient(buf, n);

    int packetSize = s_udpIn.parsePacket();
    if (packetSize > 0) {
        uint8_t rxBuf[sizeof(AudioPacketHdr) + AUDIO_FRAME_SAMPLES * sizeof(int16_t)];
        int len = s_udpIn.read(rxBuf, sizeof(rxBuf));
        if (!_udpActive) {
            _udpClientIp = s_udpIn.remoteIP();
            _udpClientPortOut = _cfg->audioPortOut;
            _udpActive = true;
            Serial.printf("[audio] UDP client %s\n", _udpClientIp.toString().c_str());
        }
        onClientAudio(rxBuf, len, false);
    }
}

void AudioBridge::sendToClient(const int16_t* samples, size_t n) {
    if (n == 0) return;

    uint8_t pkt[sizeof(AudioPacketHdr) + AUDIO_FRAME_SAMPLES * sizeof(int16_t)];
    AudioPacketHdr* hdr = reinterpret_cast<AudioPacketHdr*>(pkt);
    hdr->magic = AUDIO_MAGIC;
    hdr->seq = _seqOut++;
    hdr->nSamples = (uint16_t)n;
    memcpy(pkt + sizeof(AudioPacketHdr), samples, n * sizeof(int16_t));
    size_t pktLen = sizeof(AudioPacketHdr) + n * sizeof(int16_t);

    if (s_ws) {
        for (auto& c : s_ws->getClients()) {
            if (c.status() == WS_CONNECTED)
                c.binary(pkt, pktLen);
        }
    }

    if (_udpActive && _udpClientIp) {
        s_udpOut.beginPacket(_udpClientIp, _udpClientPortOut);
        s_udpOut.write(pkt, pktLen);
        s_udpOut.endPacket();
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
    i2s_write(I2S_NUM_0, pcm, pcmBytes, &written, pdMS_TO_TICKS(50));
}
