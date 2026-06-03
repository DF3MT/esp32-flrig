#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "config_store.h"
#include "cat_controller.h"
#include "rigctld_server.h"
#include "pot_manager.h"
#include "touch_ui.h"
#include "web_config.h"
#include "audio_bridge.h"

static AppConfig      g_cfg;
static ConfigStore    g_store;
static CatController  g_cat;
static RigctldServer  g_rigctld;
static PotManager     g_pots;
static TouchUI        g_ui;
static WebConfig      g_web;
static AudioBridge    g_audio;

static uint32_t g_lastPoll = 0;

static void setupWiFi() {
    if (g_cfg.wifiSsid[0] != '\0') {
        WiFi.mode(WIFI_STA);
        WiFi.begin(g_cfg.wifiSsid, g_cfg.wifiPass);
        Serial.printf("[wifi] connecting to %s", g_cfg.wifiSsid);
        for (int i = 0; i < 30 && WiFi.status() != WL_CONNECTED; i++) {
            delay(500);
            Serial.print('.');
        }
        Serial.println();
        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("[wifi] IP: %s\n", WiFi.localIP().toString().c_str());
            return;
        }
    }

    WiFi.mode(WIFI_AP);
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS);
    Serial.printf("[wifi] AP mode: %s / %s → %s\n",
                  WIFI_AP_SSID, WIFI_AP_PASS, WiFi.softAPIP().toString().c_str());
}

static void onConfigSaved(const AppConfig& cfg) {
    g_store.save(cfg);
}

static void onStateChange(const RadioState& state) {
    g_ui.updateState(state);
}

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== ESP32 CAT Remote Panel ===");

    g_store.begin();
    g_store.load(g_cfg);

    setupWiFi();

    // CAT direct to radio
    if (g_cfg.connMode == ConnectionMode::DIRECT_CAT) {
        if (g_cat.begin(g_cfg.vendor, g_cfg.catBaud, g_cfg.icomAddress)) {
            Serial.println("[cat] radio connected");
        } else {
            Serial.println("[cat] radio init failed – check wiring/baud");
        }
    }

    g_cat.onStateChange(onStateChange);

    // rigctld server – flrig/hamlib can connect here
    g_rigctld.begin(RIGCTLD_PORT);
    g_rigctld.setHandler([](const String& cmd) -> String {
        return dispatchRigctl(cmd, g_cat, g_cat.state());
    });

    g_pots.begin(g_cfg.pots);

    g_ui.setCatController(&g_cat);
    g_ui.begin();
    g_ui.updateState(g_cat.state());

    g_web.setAudioBridge(&g_audio);
    g_web.begin(&g_cfg, onConfigSaved);
    g_audio.begin(&g_cfg);

    Serial.printf("[rigctld] flrig/hamlib: rigctl -m 2 -r %s:%d\n",
                  WiFi.localIP().toString().c_str(), RIGCTLD_PORT);
    Serial.println("[web] config UI: http://" + WiFi.localIP().toString());
    if (g_cfg.audioEnabled) {
        Serial.println("[audio] monitor: http://" + WiFi.localIP().toString() + "/audio");
    }
}

void loop() {
    g_rigctld.loop();
    g_cat.poll();
    g_pots.loop(g_cat, g_cat.state());
    g_ui.loop();

    // periodic radio poll (freq/mode sync)
    if (millis() - g_lastPoll > 1000) {
        g_lastPoll = millis();
        uint64_t hz = 0;
        if (g_cat.getFrequency(hz)) {
            RadioState st = g_cat.state();
            g_ui.updateState(st);
        }
    }
}
