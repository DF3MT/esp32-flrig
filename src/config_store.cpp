#include "config_store.h"
#include "radio_profiles.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

const char* ConfigStore::PATH = "/config.json";

void ConfigStore::setDefaults(AppConfig& cfg) {
    radioProfileApply(cfg, "IC-7300");
    cfg.connMode = ConnectionMode::DIRECT_CAT;
    cfg.wifiSsid[0] = '\0';
    cfg.wifiPass[0] = '\0';
    cfg.remoteHost[0] = '\0';
    cfg.remotePort = RIGCTLD_PORT;
    cfg.audioEnabled = false;
    cfg.audioPortOut = AUDIO_PORT_OUT;
    cfg.audioPortIn = AUDIO_PORT_IN;
    cfg.audioSampleRate = AUDIO_SAMPLE_RATE;  // 48 kHz für FT8 / WSJT-X
    cfg.rotorEnabled = true;
    cfg.rotorBtnCcw = ROTOR_BTN_CCW_DEFAULT;
    cfg.rotorBtnCw = ROTOR_BTN_CW_DEFAULT;
    cfg.rotorOcCcw = ROTOR_OC_CCW_DEFAULT;
    cfg.rotorOcCw = ROTOR_OC_CW_DEFAULT;
    cfg.rotctldPort = ROTCTLD_PORT;
    cfg.rotorSpeed = ROTOR_SPEED_DEFAULT;
    cfg.rotorDebounceMs = ROTOR_DEBOUNCE_MS_DEFAULT;

    auto setPot = [](PotConfig& p, PotAction action, float minV, float maxV, float step) {
        p.action = action;
        p.minValue = minV;
        p.maxValue = maxV;
        p.step = step;
        p.customCmd[0] = '\0';
        p.invert = false;
    };
    setPot(cfg.pots[0], PotAction::FREQ_COARSE, -100000, 100000, 10000);
    setPot(cfg.pots[1], PotAction::FREQ_FINE,   -5000,    5000,   10);
    setPot(cfg.pots[2], PotAction::AF_GAIN,      0.0f,     1.0f,   0.01f);
    setPot(cfg.pots[3], PotAction::RF_POWER,     0.0f,     1.0f,   0.01f);
    setPot(cfg.pots[4], PotAction::SQUELCH,      0.0f,     1.0f,   0.01f);
}

bool ConfigStore::begin() {
    if (!LittleFS.begin(true)) {
        Serial.println("[config] LittleFS mount failed");
        return false;
    }
    return true;
}

static const char* actionToStr(PotAction a) {
    switch (a) {
        case PotAction::FREQ_FINE:   return "FREQ_FINE";
        case PotAction::FREQ_COARSE: return "FREQ_COARSE";
        case PotAction::AF_GAIN:     return "AF_GAIN";
        case PotAction::RF_POWER:    return "RF_POWER";
        case PotAction::RF_GAIN:     return "RF_GAIN";
        case PotAction::SQUELCH:     return "SQUELCH";
        case PotAction::MIC_GAIN:    return "MIC_GAIN";
        case PotAction::RIT_OFFSET:  return "RIT_OFFSET";
        case PotAction::CUSTOM_RIGCTL: return "CUSTOM_RIGCTL";
        default: return "NONE";
    }
}

static PotAction strToAction(const char* s) {
    if (!s) return PotAction::NONE;
    if (strcmp(s, "FREQ_FINE") == 0)   return PotAction::FREQ_FINE;
    if (strcmp(s, "FREQ_COARSE") == 0) return PotAction::FREQ_COARSE;
    if (strcmp(s, "AF_GAIN") == 0)     return PotAction::AF_GAIN;
    if (strcmp(s, "RF_POWER") == 0)    return PotAction::RF_POWER;
    if (strcmp(s, "RF_GAIN") == 0)     return PotAction::RF_GAIN;
    if (strcmp(s, "SQUELCH") == 0)     return PotAction::SQUELCH;
    if (strcmp(s, "MIC_GAIN") == 0)    return PotAction::MIC_GAIN;
    if (strcmp(s, "RIT_OFFSET") == 0)  return PotAction::RIT_OFFSET;
    if (strcmp(s, "CUSTOM_RIGCTL") == 0) return PotAction::CUSTOM_RIGCTL;
    return PotAction::NONE;
}

bool ConfigStore::load(AppConfig& cfg) {
    setDefaults(cfg);
    if (!LittleFS.exists(PATH)) {
        save(cfg);
        return true;
    }

    File f = LittleFS.open(PATH, "r");
    if (!f) return false;

    JsonDocument doc;
    if (deserializeJson(doc, f)) {
        f.close();
        return false;
    }
    f.close();

    const char* model = doc["radio_model"] | "";
    if (model[0]) {
        strlcpy(cfg.radioModel, model, sizeof(cfg.radioModel));
        radioProfileApply(cfg, cfg.radioModel);
    } else {
        const char* vendor = doc["vendor"] | "ICOM";
        cfg.vendor = (strcmp(vendor, "YAESU") == 0) ? RadioVendor::YAESU : RadioVendor::ICOM;
        cfg.icomAddress = doc["icom_address"] | 0x94;
        cfg.catBaud = doc["cat_baud"] | CAT_DEFAULT_BAUD;
        cfg.radioModel[0] = '\0';
    }
    if (doc["cat_baud"].is<uint32_t>()) cfg.catBaud = doc["cat_baud"];
    if (doc["icom_address"].is<uint32_t>()) cfg.icomAddress = doc["icom_address"];
    const char* vendor = doc["vendor"] | nullptr;
    if (vendor)
        cfg.vendor = (strcmp(vendor, "YAESU") == 0) ? RadioVendor::YAESU : RadioVendor::ICOM;

    strlcpy(cfg.wifiSsid, doc["wifi_ssid"] | "", sizeof(cfg.wifiSsid));
    strlcpy(cfg.wifiPass, doc["wifi_pass"] | "", sizeof(cfg.wifiPass));
    strlcpy(cfg.remoteHost, doc["remote_host"] | "", sizeof(cfg.remoteHost));
    cfg.remotePort = doc["remote_port"] | RIGCTLD_PORT;
    cfg.audioEnabled = doc["audio_enabled"] | false;
    cfg.audioPortOut = doc["audio_port_out"] | AUDIO_PORT_OUT;
    cfg.audioPortIn = doc["audio_port_in"] | AUDIO_PORT_IN;
    cfg.audioSampleRate = doc["audio_sample_rate"] | AUDIO_SAMPLE_RATE;
    cfg.rotorEnabled = doc["rotor_enabled"] | true;
    cfg.rotorBtnCcw = doc["rotor_btn_ccw"] | ROTOR_BTN_CCW_DEFAULT;
    cfg.rotorBtnCw = doc["rotor_btn_cw"] | ROTOR_BTN_CW_DEFAULT;
    cfg.rotorOcCcw = doc["rotor_oc_ccw"] | ROTOR_OC_CCW_DEFAULT;
    cfg.rotorOcCw = doc["rotor_oc_cw"] | ROTOR_OC_CW_DEFAULT;
    cfg.rotctldPort = doc["rotctld_port"] | ROTCTLD_PORT;
    cfg.rotorSpeed = doc["rotor_speed"] | ROTOR_SPEED_DEFAULT;
    cfg.rotorDebounceMs = doc["rotor_debounce_ms"] | ROTOR_DEBOUNCE_MS_DEFAULT;

    JsonArray pots = doc["pots"].as<JsonArray>();
    for (size_t i = 0; i < POT_COUNT && i < pots.size(); i++) {
        JsonObject p = pots[i];
        cfg.pots[i].action = strToAction(p["action"] | "NONE");
        cfg.pots[i].minValue = p["min"] | 0.0f;
        cfg.pots[i].maxValue = p["max"] | 1.0f;
        cfg.pots[i].step = p["step"] | 0.01f;
        cfg.pots[i].invert = p["invert"] | false;
        strlcpy(cfg.pots[i].customCmd, p["custom_cmd"] | "", sizeof(cfg.pots[i].customCmd));
    }
    return true;
}

bool ConfigStore::save(const AppConfig& cfg) {
    JsonDocument doc;
    doc["radio_model"] = cfg.radioModel;
    doc["vendor"] = (cfg.vendor == RadioVendor::YAESU) ? "YAESU" : "ICOM";
    doc["icom_address"] = cfg.icomAddress;
    doc["cat_baud"] = cfg.catBaud;
    doc["wifi_ssid"] = cfg.wifiSsid;
    doc["wifi_pass"] = cfg.wifiPass;
    doc["remote_host"] = cfg.remoteHost;
    doc["remote_port"] = cfg.remotePort;
    doc["audio_enabled"] = cfg.audioEnabled;
    doc["audio_port_out"] = cfg.audioPortOut;
    doc["audio_port_in"] = cfg.audioPortIn;
    doc["audio_sample_rate"] = cfg.audioSampleRate;
    doc["rotor_enabled"] = cfg.rotorEnabled;
    doc["rotor_btn_ccw"] = cfg.rotorBtnCcw;
    doc["rotor_btn_cw"] = cfg.rotorBtnCw;
    doc["rotor_oc_ccw"] = cfg.rotorOcCcw;
    doc["rotor_oc_cw"] = cfg.rotorOcCw;
    doc["rotctld_port"] = cfg.rotctldPort;
    doc["rotor_speed"] = cfg.rotorSpeed;
    doc["rotor_debounce_ms"] = cfg.rotorDebounceMs;

    JsonArray pots = doc["pots"].to<JsonArray>();
    for (int i = 0; i < POT_COUNT; i++) {
        JsonObject p = pots.add<JsonObject>();
        p["action"] = actionToStr(cfg.pots[i].action);
        p["min"] = cfg.pots[i].minValue;
        p["max"] = cfg.pots[i].maxValue;
        p["step"] = cfg.pots[i].step;
        p["invert"] = cfg.pots[i].invert;
        p["custom_cmd"] = cfg.pots[i].customCmd;
    }

    File f = LittleFS.open(PATH, "w");
    if (!f) return false;
    serializeJsonPretty(doc, f);
    f.close();
    return true;
}
