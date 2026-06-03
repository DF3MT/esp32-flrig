#pragma once

#include "config.h"
#include "cat_controller.h"
#include <functional>

class PotManager {
public:
    void begin(const PotConfig configs[POT_COUNT]);
    void setConfigs(const PotConfig configs[POT_COUNT]);
    void loop(CatController& cat, RadioState& state);

    int  rawValue(int idx) const;
    float normalized(int idx) const;

    using ConfigCallback = std::function<void(int idx, const PotConfig&)>;
    void onConfigChange(ConfigCallback cb) { _cfgCb = cb; }

private:
    PotConfig _configs[POT_COUNT];
    int       _raw[POT_COUNT] = {};
    int       _smooth[POT_COUNT][POT_SMOOTH_SAMPLES] = {};
    int       _smoothIdx[POT_COUNT] = {};
    int       _lastSent[POT_COUNT] = {};
    uint32_t  _lastPoll = 0;
    ConfigCallback _cfgCb;

    int  readSmoothed(int idx);
    void applyAction(int idx, CatController& cat, RadioState& state, float norm);
    float mapNorm(float norm, const PotConfig& cfg) const;
};
