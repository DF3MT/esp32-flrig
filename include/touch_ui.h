#pragma once

#include "cat_controller.h"

class TouchUI {
public:
    bool begin();
    void loop();
    void updateState(const RadioState& state);
    void setCatController(CatController* cat) { _cat = cat; }

    using FreqChangeCallback = void (*)(uint64_t hz, void* user);
    void onFreqChange(FreqChangeCallback cb, void* user) {
        _freqCb = cb; _freqUser = user;
    }

private:
    CatController* _cat = nullptr;
    FreqChangeCallback _freqCb = nullptr;
    void* _freqUser = nullptr;
    uint64_t _displayFreq = 0;
    bool _initialized = false;

    void createMainScreen();
};
