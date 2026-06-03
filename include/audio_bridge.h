#pragma once

#include "config.h"
#include <ESPAsyncWebServer.h>

class AudioBridge {
public:
    bool begin(const AppConfig* cfg);
    void attachToServer(AsyncWebServer* server);
    static void taskEntry(void* arg);

private:
    void loopOnce();
    bool initI2s();
    void sendToClient(const int16_t* samples, size_t n);
    void onClientAudio(const uint8_t* data, size_t len, bool fromWs);

    const AppConfig* _cfg = nullptr;
    bool _running = false;
    uint32_t _seqOut = 0;
    IPAddress _udpClientIp;
    uint16_t _udpClientPortOut = 0;
    bool _udpActive = false;
    uint16_t _frameSamples = 960;

    static AudioBridge* _instance;
};
