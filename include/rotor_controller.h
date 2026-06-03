#pragma once

#include "config.h"

/** Hamlib rotctld move() direction flags (see rotctld.1). */
namespace RotorDir {
    constexpr int UP    = 2;
    constexpr int DOWN  = 4;
    constexpr int LEFT  = 8;
    constexpr int RIGHT = 16;
}

struct RotorState {
    float azimuth   = 0.0f;
    float elevation = 0.0f;
    int   moving    = 0;      // 0 = stop, else RotorDir bit
    int   speed     = 50;     // 1–100
};

class RotorController {
public:
    bool begin(const AppConfig& cfg);
    void loop();

    bool move(int direction, int speed);
    void stop();
    bool park();

    bool setPosition(float az, float el);
    void getPosition(float& az, float& el) const;

    const RotorState& state() const { return _state; }

private:
    RotorState _state;
    bool       _enabled = false;
    uint8_t    _btnCcw = 0;
    uint8_t    _btnCw  = 0;
    uint8_t    _ocCcw  = 0;
    uint8_t    _ocCw   = 0;
    uint32_t   _debounceMs = 40;

    bool _btnCcwLast = true;
    bool _btnCwLast  = true;
    uint32_t _lastBtnMs = 0;

    void applyOutputs();
    void setOc(uint8_t pin, bool active);
};
