#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "config_store.h"
#include "radio_channel.h"
#include "rotctld_server.h"
#include "rotor_controller.h"
#include "pot_manager.h"
#include "touch_ui.h"
#include "web_config.h"

static AppConfig           g_cfg;
static ConfigStore         g_store;
static RadioChannelManager g_radios;
static HamlibTcpServer     g_rotctld;
static RotorController     g_rotor;
static PotManager          g_pots;
static TouchUI             g_ui;
static WebConfig           g_web;

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
    Serial.printf("\n=== ESP32 CAT Remote Panel (%d Funk) ===\n", RADIO_COUNT);

    g_store.begin();
    g_store.load(g_cfg);

    setupWiFi();

    g_radios.cat(0).onStateChange(onStateChange);
    g_radios.begin(g_cfg);

    g_rotor.begin(g_cfg);
    g_rotctld.begin(g_cfg.rotctldPort, "rotctld");
    g_rotctld.setHandler([](const String& cmd) -> String {
        return dispatchRotctl(cmd, g_rotor, &g_cfg);
    });

    g_pots.begin(g_cfg.pots);

    g_ui.setCatController(&g_radios.cat(0));
    g_ui.begin();
    g_ui.updateState(g_radios.cat(0).state());

    g_web.begin(&g_cfg, onConfigSaved);
    g_web.attachRadioAudio(&g_radios);

    IPAddress ip = WiFi.localIP();
    if (!ip) ip = WiFi.softAPIP();

    for (int i = 0; i < RADIO_COUNT; i++) {
        if (!g_cfg.radios[i].enabled) continue;
        Serial.printf("[rigctld-%s] rigctl -m 2 -r %s:%u\n",
                      g_cfg.radios[i].label, ip.toString().c_str(),
                      g_cfg.radios[i].rigctldPort);
    }
    if (g_cfg.rotorEnabled) {
        Serial.printf("[rotctld] rotctl -m 2 -r %s:%d\n",
                      ip.toString().c_str(), g_cfg.rotctldPort);
    }
    Serial.println("[web] config UI: http://" + ip.toString());
    if (g_cfg.radios[0].audioEnabled)
        Serial.println("[audio-A] http://" + ip.toString() + "/audio");
#if RADIO_COUNT > 1
    if (g_cfg.radios[1].audioEnabled)
        Serial.println("[audio-B] http://" + ip.toString() + "/audio_b");
#endif
#if RADIO_COUNT > 2
    if (g_cfg.radios[2].audioEnabled)
        Serial.println("[audio-C] http://" + ip.toString() + "/audio_c");
#endif
}

void loop() {
    g_radios.loop();
    g_rotctld.loop();
    g_rotor.loop();
    g_pots.loop(g_radios.cat(0), g_radios.cat(0).state());
    g_ui.loop();

    if (millis() - g_lastPoll > 1000) {
        g_lastPoll = millis();
        uint64_t hz = 0;
        if (g_radios.cat(0).getFrequency(hz)) {
            RadioState st = g_radios.cat(0).state();
            g_ui.updateState(st);
        }
    }
}
