#pragma once

#include "cat_controller.h"

// Yaesu CAT – ASCII commands terminated with ';'
class YaesuCat : public CatProtocol {
public:
    bool begin(Stream& serial, uint32_t baud) override;
    bool setFrequency(uint64_t hz) override;
    bool getFrequency(uint64_t& hz) override;
    bool setMode(const char* mode) override;
    bool getMode(char* mode, size_t len) override;
    bool setPtt(bool on) override;
    bool setLevel(const char* level, float value) override;
    bool sendRaw(const uint8_t* data, size_t len) override;
    void poll() override;

private:
    Stream* _serial = nullptr;
    String _rxBuf;

    bool sendCommand(const char* cmd);
    bool queryCommand(const char* cmd, String& response, uint32_t timeoutMs = 500);
    static char yaesuModeFromStr(const char* mode);
    static void strFromYaesuMode(char ymode, char* out, size_t len);
};
