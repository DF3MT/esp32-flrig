#pragma once

#include "cat_controller.h"

// ICOM CI-V protocol (FE FE addr ctrl cmd [sub] data FD)
class IcomCiv : public CatProtocol {
public:
    explicit IcomCiv(uint8_t radioAddr = 0x94, uint8_t ctrlAddr = 0xE0);

    bool begin(Stream& serial, uint32_t baud) override;
    bool setFrequency(uint64_t hz) override;
    bool getFrequency(uint64_t& hz) override;
    bool setMode(const char* mode) override;
    bool getMode(char* mode, size_t len) override;
    bool setPtt(bool on) override;
    bool setLevel(const char* level, float value) override;
    bool sendRaw(const uint8_t* data, size_t len) override;
    void poll() override;

    void setRadioAddress(uint8_t addr) { _radioAddr = addr; }

private:
    Stream* _serial = nullptr;
    uint8_t _radioAddr;
    uint8_t _ctrlAddr;
    uint8_t _rxBuf[128];
    size_t  _rxLen = 0;

    bool sendFrame(const uint8_t* data, size_t len);
    bool transact(const uint8_t* cmd, size_t cmdLen, uint8_t* resp, size_t& respLen, uint32_t timeoutMs = 500);
    static void freqToBcd(uint64_t hz, uint8_t out[5]);
    static uint64_t bcdToFreq(const uint8_t bcd[5]);
    static uint8_t modeToIcom(const char* mode);
    static void icomToMode(uint8_t icomMode, uint8_t filter, char* out, size_t len);
    void parseIncoming();
};
