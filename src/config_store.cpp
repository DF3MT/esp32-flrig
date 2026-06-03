#include "config_store.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

const char* ConfigStore::PATH = "/config.json";

void ConfigStore::setDefaults(AppConfig& cfg) {
    cfg.vendor = RadioVendor::ICOM;
    cfg.connMode = ConnectionMode::DIRECT_CAT;
    cfg.icomAddress = 0x94;
    cfg.catBaud = CAT_DEFAULT_BAUD;
    cfg.wifiSsid[0] = '\0';
    cfg.wifiPass[0] = '\0';
    cfg.remoteHost[0] = '\0';
    cfg.remotePort = RIGCTLD_PORT;

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

    const char* vendor = doc["vendor"] | "ICOM";
    cfg.vendor = (strcmp(vendor, "YAESU") == 0) ? RadioVendor::YAESU : RadioVendor::ICOM;

    cfg.icomAddress = doc["icom_address"] | 0x94;
    cfg.catBaud = doc["cat_baud"] | CAT_DEFAULT_BAUD;

    strlcpy(cfg.wifiSsid, doc["wifi_ssid"] | "", sizeof(cfg.wifiSsid));
    strlcpy(cfg.wifiPass, doc["wifi_pass"] | "", sizeof(cfg.wifiPass));
    strlcpy(cfg.remoteHost, doc["remote_host"] | "", sizeof(cfg.remoteHost));
    cfg.remotePort = doc["remote_port"] | RIGCTLD_PORT;

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
    doc["vendor"] = (cfg.vendor == RadioVendor::YAESU) ? "YAESU" : "ICOM";
    doc["icom_address"] = cfg.icomAddress;
    doc["cat_baud"] = cfg.catBaud;
    doc["wifi_ssid"] = cfg.wifiSsid;
    doc["wifi_pass"] = cfg.wifiPass;
    doc["remote_host"] = cfg.remoteHost;
    doc["remote_port"] = cfg.remotePort;

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
