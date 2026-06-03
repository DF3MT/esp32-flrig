#include "web_config.h"
#include "audio_bridge.h"
#include "radio_profiles.h"
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <ArduinoJson.h>

static AsyncWebServer s_server(WEB_CONFIG_PORT);

static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32 CAT Panel</title>
<style>
body{font-family:sans-serif;background:#1a1a2e;color:#eee;margin:1em;max-width:640px}
h1{color:#00ff88}label{display:block;margin:.5em 0 .2em}
input,select{width:100%;padding:.4em;margin-bottom:.5em;background:#16213e;color:#eee;border:1px solid #444}
button{background:#00ff88;color:#111;border:none;padding:.6em 1.2em;cursor:pointer;margin-top:1em}
.pot{border:1px solid #333;padding:.8em;margin:.5em 0;border-radius:4px}
#profile_info{font-size:.9em;color:#aaa;border-left:3px solid #00ff88;padding:.5em .8em;margin:.5em 0}
.hidden{display:none}
</style>
</head>
<body>
<h1>ESP32 CAT Remote Panel</h1>
<form id="cfg">
<label>Funkgerät</label>
<select id="radio_model"></select>
<div id="profile_info"></div>
<div id="icom_row">
<label>ICOM CI-V Adresse (hex)</label>
<input id="icom_address" type="text" value="94">
</div>
<label>CAT Baudrate</label>
<input id="cat_baud" type="number" value="38400">
<label>WiFi SSID (leer = AP-Modus)</label>
<input id="wifi_ssid">
<label>WiFi Passwort</label>
<input id="wifi_pass" type="password">
<label>Remote Host (flrig/rigctld IP, optional)</label>
<input id="remote_host">
<label>Remote Port</label>
<input id="remote_port" type="number" value="4532">
<h2>Audio über WiFi</h2>
<label><input id="audio_enabled" type="checkbox"> Audio-Bridge aktiv (I2S + UDP/WebSocket)</label>
<label>UDP Port Funk→Client</label>
<input id="audio_port_out" type="number" value="4533">
<label>UDP Port Client→Funk</label>
<input id="audio_port_in" type="number" value="4534">
<label>Sample Rate (Hz)</label>
<input id="audio_sample_rate" type="number" value="48000">
<p><a href="/audio">Audio-Monitor im Browser</a> (nur wenn Audio aktiv)</p>
<h2>Rotor (Hamlib rotctld)</h2>
<label><input id="rotor_enabled" type="checkbox" checked> Rotorsteuerung aktiv</label>
<label>rotctld Port (Standard 4535)</label>
<input id="rotctld_port" type="number" value="4535">
<label>GPIO Taster CCW / CW</label>
<div style="display:flex;gap:.5em">
<input id="rotor_btn_ccw" type="number" min="0" max="39" value="27" style="flex:1">
<input id="rotor_btn_cw" type="number" min="0" max="39" value="5" style="flex:1">
</div>
<label>GPIO Open-Collector CCW / CW (Relais)</label>
<div style="display:flex;gap:.5em">
<input id="rotor_oc_ccw" type="number" min="0" max="39" value="18" style="flex:1">
<input id="rotor_oc_cw" type="number" min="0" max="39" value="19" style="flex:1">
</div>
<label>Move-Speed (1–100, Hamlib M)</label>
<input id="rotor_speed" type="number" min="1" max="100" value="50">
<div id="pots"></div>
<button type="submit">Speichern &amp; Neustart</button>
</form>
<script>
const actions=['NONE','FREQ_FINE','FREQ_COARSE','AF_GAIN','RF_POWER','RF_GAIN','SQUELCH','MIC_GAIN','RIT_OFFSET','CUSTOM_RIGCTL'];
let profiles=[];
function showProfile(id){
  const p=profiles.find(x=>x.id===id);
  const row=document.getElementById('icom_row');
  if(!p){profile_info.textContent='';row.classList.add('hidden');return;}
  profile_info.innerHTML='<b>'+p.label+'</b><br>CAT: '+p.cat_port+' · '+p.cat_baud+' bps · Hamlib: '+p.hamlib_model+
    '<br>Audio RX: '+p.audio_rx+'<br>Audio TX: '+p.audio_tx+'<br><small>'+p.notes+'</small>';
  row.classList.toggle('hidden',p.vendor!=='ICOM');
  cat_baud.value=p.cat_baud;
  if(p.vendor==='ICOM') icom_address.value=p.icom_address.toString(16);
}
fetch('/api/radio_profiles').then(r=>r.json()).then(list=>{
  profiles=list;
  radio_model.innerHTML=list.map(p=>'<option value="'+p.id+'">'+p.label+'</option>').join('');
  return fetch('/api/config');
}).then(r=>r.json()).then(c=>{
  if(c.radio_model) radio_model.value=c.radio_model;
  showProfile(radio_model.value);
  icom_address.value=(c.icom_address||0x94).toString(16);
  cat_baud.value=c.cat_baud; wifi_ssid.value=c.wifi_ssid||'';
  wifi_pass.value=c.wifi_pass||''; remote_host.value=c.remote_host||'';
  remote_port.value=c.remote_port;
  audio_enabled.checked=c.audio_enabled;
  audio_port_out.value=c.audio_port_out;
  audio_port_in.value=c.audio_port_in;
  audio_sample_rate.value=c.audio_sample_rate;
  rotor_enabled.checked=c.rotor_enabled;
  rotctld_port.value=c.rotctld_port;
  rotor_btn_ccw.value=c.rotor_btn_ccw;
  rotor_btn_cw.value=c.rotor_btn_cw;
  rotor_oc_ccw.value=c.rotor_oc_ccw;
  rotor_oc_cw.value=c.rotor_oc_cw;
  rotor_speed.value=c.rotor_speed;
  pots.innerHTML=c.pots.map((p,i)=>`
    <div class="pot"><b>Poti ${i+1}</b>
    <label>Aktion</label><select id="pot${i}_action">${actions.map(a=>`<option ${a===p.action?'selected':''}>${a}</option>`).join('')}</select>
    <label>Min</label><input id="pot${i}_min" type="number" step="any" value="${p.min}">
    <label>Max</label><input id="pot${i}_max" type="number" step="any" value="${p.max}">
    <label>Step</label><input id="pot${i}_step" type="number" step="any" value="${p.step}">
    <label>Custom CMD</label><input id="pot${i}_custom" value="${p.custom_cmd||''}">
    <label><input id="pot${i}_inv" type="checkbox" ${p.invert?'checked':''}> Invertiert</label>
    </div>`).join('');
});
radio_model.onchange=()=>showProfile(radio_model.value);
cfg.onsubmit=e=>{e.preventDefault();
  const body={radio_model:radio_model.value,
    icom_address:parseInt(icom_address.value,16),
    cat_baud:+cat_baud.value,wifi_ssid:wifi_ssid.value,wifi_pass:wifi_pass.value,
    remote_host:remote_host.value,remote_port:+remote_port.value,
    audio_enabled:audio_enabled.checked,audio_port_out:+audio_port_out.value,
    audio_port_in:+audio_port_in.value,audio_sample_rate:+audio_sample_rate.value,
    rotor_enabled:rotor_enabled.checked,rotctld_port:+rotctld_port.value,
    rotor_btn_ccw:+rotor_btn_ccw.value,rotor_btn_cw:+rotor_btn_cw.value,
    rotor_oc_ccw:+rotor_oc_ccw.value,rotor_oc_cw:+rotor_oc_cw.value,
    rotor_speed:+rotor_speed.value,
    pots:[...Array(5)].map((_,i)=>({action:document.getElementById('pot'+i+'_action').value,
      min:+document.getElementById('pot'+i+'_min').value,
      max:+document.getElementById('pot'+i+'_max').value,
      step:+document.getElementById('pot'+i+'_step').value,
      custom_cmd:document.getElementById('pot'+i+'_custom').value,
      invert:document.getElementById('pot'+i+'_inv').checked}))};
  fetch('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)})
    .then(()=>alert('Gespeichert – Neustart...'));
};
</script>
</body>
</html>
)rawliteral";

bool WebConfig::begin(AppConfig* cfg, SaveCallback onSave) {
    _cfg = cfg;
    _onSave = onSave;

    s_server.on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
        req->send_P(200, "text/html", INDEX_HTML);
    });

    s_server.on("/api/radio_profiles", HTTP_GET, [](AsyncWebServerRequest* req) {
        JsonDocument doc;
        JsonArray arr = doc.to<JsonArray>();
        for (size_t i = 0; i < radioProfileCount(); i++) {
            const RadioProfileInfo* p = radioProfileByIndex(i);
            if (!p) continue;
            JsonObject o = arr.add<JsonObject>();
            o["id"] = p->id;
            o["label"] = p->label;
            o["vendor"] = (p->vendor == RadioVendor::YAESU) ? "YAESU" : "ICOM";
            o["cat_baud"] = p->catBaud;
            o["icom_address"] = p->icomAddress;
            o["hamlib_model"] = p->hamlibModel;
            o["cat_port"] = p->catPort;
            o["audio_rx"] = p->audioRx;
            o["audio_tx"] = p->audioTx;
            o["notes"] = p->notes;
        }
        String out;
        serializeJson(doc, out);
        req->send(200, "application/json", out);
    });

    s_server.on("/api/config", HTTP_GET, [this](AsyncWebServerRequest* req) {
        JsonDocument doc;
        doc["radio_model"] = _cfg->radioModel;
        doc["radio_label"] = radioProfileLabel(*_cfg);
        doc["vendor"] = (_cfg->vendor == RadioVendor::YAESU) ? "YAESU" : "ICOM";
        doc["icom_address"] = _cfg->icomAddress;
        doc["cat_baud"] = _cfg->catBaud;
        doc["wifi_ssid"] = _cfg->wifiSsid;
        doc["wifi_pass"] = _cfg->wifiPass;
        doc["remote_host"] = _cfg->remoteHost;
        doc["remote_port"] = _cfg->remotePort;
        doc["audio_enabled"] = _cfg->audioEnabled;
        doc["audio_port_out"] = _cfg->audioPortOut;
        doc["audio_port_in"] = _cfg->audioPortIn;
        doc["audio_sample_rate"] = _cfg->audioSampleRate;
        doc["rotor_enabled"] = _cfg->rotorEnabled;
        doc["rotctld_port"] = _cfg->rotctldPort;
        doc["rotor_btn_ccw"] = _cfg->rotorBtnCcw;
        doc["rotor_btn_cw"] = _cfg->rotorBtnCw;
        doc["rotor_oc_ccw"] = _cfg->rotorOcCcw;
        doc["rotor_oc_cw"] = _cfg->rotorOcCw;
        doc["rotor_speed"] = _cfg->rotorSpeed;
        doc["rotor_debounce_ms"] = _cfg->rotorDebounceMs;
        JsonArray pots = doc["pots"].to<JsonArray>();
        const char* names[] = {"NONE","FREQ_FINE","FREQ_COARSE","AF_GAIN","RF_POWER",
                               "RF_GAIN","SQUELCH","MIC_GAIN","RIT_OFFSET","CUSTOM_RIGCTL"};
        for (int i = 0; i < POT_COUNT; i++) {
            JsonObject p = pots.add<JsonObject>();
            p["action"] = names[static_cast<int>(_cfg->pots[i].action)];
            p["min"] = _cfg->pots[i].minValue;
            p["max"] = _cfg->pots[i].maxValue;
            p["step"] = _cfg->pots[i].step;
            p["custom_cmd"] = _cfg->pots[i].customCmd;
            p["invert"] = _cfg->pots[i].invert;
        }
        String out;
        serializeJson(doc, out);
        req->send(200, "application/json", out);
    });

    s_server.on("/api/config", HTTP_POST, [this](AsyncWebServerRequest* req) {},
        nullptr,
        [this](AsyncWebServerRequest* req, uint8_t* data, size_t len, size_t, size_t) {
            JsonDocument doc;
            if (deserializeJson(doc, data, len)) {
                req->send(400, "text/plain", "JSON error");
                return;
            }
            const char* model = doc["radio_model"] | "";
            if (model[0]) {
                radioProfileApply(*_cfg, model);
            } else {
                const char* vendor = doc["vendor"] | "ICOM";
                _cfg->vendor = (strcmp(vendor, "YAESU") == 0) ? RadioVendor::YAESU : RadioVendor::ICOM;
            }
            _cfg->icomAddress = doc["icom_address"] | _cfg->icomAddress;
            _cfg->catBaud = doc["cat_baud"] | _cfg->catBaud;
            strlcpy(_cfg->wifiSsid, doc["wifi_ssid"] | "", sizeof(_cfg->wifiSsid));
            strlcpy(_cfg->wifiPass, doc["wifi_pass"] | "", sizeof(_cfg->wifiPass));
            strlcpy(_cfg->remoteHost, doc["remote_host"] | "", sizeof(_cfg->remoteHost));
            _cfg->remotePort = doc["remote_port"] | RIGCTLD_PORT;
            _cfg->audioEnabled = doc["audio_enabled"] | false;
            _cfg->audioPortOut = doc["audio_port_out"] | AUDIO_PORT_OUT;
            _cfg->audioPortIn = doc["audio_port_in"] | AUDIO_PORT_IN;
            _cfg->audioSampleRate = doc["audio_sample_rate"] | AUDIO_SAMPLE_RATE;
            _cfg->rotorEnabled = doc["rotor_enabled"] | true;
            _cfg->rotctldPort = doc["rotctld_port"] | ROTCTLD_PORT;
            _cfg->rotorBtnCcw = doc["rotor_btn_ccw"] | ROTOR_BTN_CCW_DEFAULT;
            _cfg->rotorBtnCw = doc["rotor_btn_cw"] | ROTOR_BTN_CW_DEFAULT;
            _cfg->rotorOcCcw = doc["rotor_oc_ccw"] | ROTOR_OC_CCW_DEFAULT;
            _cfg->rotorOcCw = doc["rotor_oc_cw"] | ROTOR_OC_CW_DEFAULT;
            _cfg->rotorSpeed = doc["rotor_speed"] | ROTOR_SPEED_DEFAULT;
            _cfg->rotorDebounceMs = doc["rotor_debounce_ms"] | ROTOR_DEBOUNCE_MS_DEFAULT;

            JsonArray pots = doc["pots"].as<JsonArray>();
            for (size_t i = 0; i < POT_COUNT && i < pots.size(); i++) {
                JsonObject p = pots[i];
                const char* act = p["action"] | "NONE";
                if (strcmp(act, "FREQ_FINE") == 0) _cfg->pots[i].action = PotAction::FREQ_FINE;
                else if (strcmp(act, "FREQ_COARSE") == 0) _cfg->pots[i].action = PotAction::FREQ_COARSE;
                else if (strcmp(act, "AF_GAIN") == 0) _cfg->pots[i].action = PotAction::AF_GAIN;
                else if (strcmp(act, "RF_POWER") == 0) _cfg->pots[i].action = PotAction::RF_POWER;
                else if (strcmp(act, "RF_GAIN") == 0) _cfg->pots[i].action = PotAction::RF_GAIN;
                else if (strcmp(act, "SQUELCH") == 0) _cfg->pots[i].action = PotAction::SQUELCH;
                else if (strcmp(act, "MIC_GAIN") == 0) _cfg->pots[i].action = PotAction::MIC_GAIN;
                else if (strcmp(act, "RIT_OFFSET") == 0) _cfg->pots[i].action = PotAction::RIT_OFFSET;
                else if (strcmp(act, "CUSTOM_RIGCTL") == 0) _cfg->pots[i].action = PotAction::CUSTOM_RIGCTL;
                else _cfg->pots[i].action = PotAction::NONE;
                _cfg->pots[i].minValue = p["min"] | 0.0f;
                _cfg->pots[i].maxValue = p["max"] | 1.0f;
                _cfg->pots[i].step = p["step"] | 0.01f;
                _cfg->pots[i].invert = p["invert"] | false;
                strlcpy(_cfg->pots[i].customCmd, p["custom_cmd"] | "", sizeof(_cfg->pots[i].customCmd));
            }
            if (_onSave) _onSave(*_cfg);
            req->send(200, "text/plain", "OK");
            delay(500);
            ESP.restart();
        });

    s_server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest* req) {
        JsonDocument doc;
        doc["ip"] = WiFi.localIP().toString();
        doc["rigctld_port"] = RIGCTLD_PORT;
        doc["rotctld_port"] = _cfg->rotctldPort;
        doc["rotor_enabled"] = _cfg->rotorEnabled;
        doc["radio_model"] = _cfg->radioModel;
        doc["radio_label"] = radioProfileLabel(*_cfg);
        doc["cat_baud"] = _cfg->catBaud;
        doc["audio_enabled"] = _cfg->audioEnabled;
        doc["audio_port_out"] = _cfg->audioPortOut;
        doc["audio_port_in"] = _cfg->audioPortIn;
        doc["audio_ws"] = AUDIO_WS_PATH;
        String out;
        serializeJson(doc, out);
        req->send(200, "application/json", out);
    });

    if (_audio) _audio->attachToServer(&s_server);

    s_server.begin();
    Serial.println("[web] config server started on :80");
    return true;
}

void WebConfig::loop() {}
