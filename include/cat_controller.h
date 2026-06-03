#pragma once

#include "config.h"
#include <functional>

struct RadioState {
    uint64_t freqHz   = 0;
    char     mode[16] = "USB";
    bool     ptt      = false;
    float    afLevel  = 0.5f;
    float    rfPower  = 0.5f;
    int      ritHz    = 0;
    bool     connected = false;
};

class CatProtocol {
public:
    virtual ~CatProtocol() = default;
    virtual bool begin(HardwareSerial& serial, uint32_t baud) = 0;
    virtual bool setFrequency(uint64_t hz) = 0;
    virtual bool getFrequency(uint64_t& hz) = 0;
    virtual bool setMode(const char* mode) = 0;
    virtual bool getMode(char* mode, size_t len) = 0;
    virtual bool setPtt(bool on) = 0;
    virtual bool setLevel(const char* level, float value) = 0;
    virtual bool sendRaw(const uint8_t* data, size_t len) = 0;
    virtual void poll() = 0;  // process transceive frames
};

class CatController {
public:
    bool begin(RadioVendor vendor, uint32_t baud, uint8_t icomAddr = 0x94);
    void setVendor(RadioVendor v);
    void poll();

    bool setFrequency(uint64_t hz);
    bool getFrequency(uint64_t& hz);
    bool setMode(const char* mode);
    bool getMode(char* mode, size_t len);
    bool setPtt(bool on);
    bool setLevel(const char* level, float value);
    bool sendRawHex(const char* hex);

    const RadioState& state() const { return _state; }
    RadioState&       state()       { return _state; }

    using StateCallback = std::function<void(const RadioState&)>;
    void onStateChange(StateCallback cb) { _callback = cb; }

private:
    HardwareSerial _serial{2};
    CatProtocol*   _proto = nullptr;
    RadioState     _state;
    StateCallback  _callback;
    RadioVendor    _vendor = RadioVendor::ICOM;
    uint8_t        _icomAddr = 0x94;
};
