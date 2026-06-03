#include "pot_manager.h"
#include <math.h>

void PotManager::begin(const PotConfig configs[POT_COUNT]) {
    setConfigs(configs);
    for (int i = 0; i < POT_COUNT; i++) {
        pinMode(POT_PINS[i], INPUT);
        _lastSent[i] = -9999;
        _smoothIdx[i] = 0;
        for (int s = 0; s < POT_SMOOTH_SAMPLES; s++)
            _smooth[i][s] = analogRead(POT_PINS[i]);
    }
}

void PotManager::setConfigs(const PotConfig configs[POT_COUNT]) {
    memcpy(_configs, configs, sizeof(_configs));
}

int PotManager::readSmoothed(int idx) {
    _smooth[idx][_smoothIdx[idx]] = analogRead(POT_PINS[idx]);
    _smoothIdx[idx] = (_smoothIdx[idx] + 1) % POT_SMOOTH_SAMPLES;

    long sum = 0;
    for (int s = 0; s < POT_SMOOTH_SAMPLES; s++) sum += _smooth[idx][s];
    _raw[idx] = static_cast<int>(sum / POT_SMOOTH_SAMPLES);
    return _raw[idx];
}

float PotManager::normalized(int idx) const {
    if (idx < 0 || idx >= POT_COUNT) return 0.0f;
    float n = _raw[idx] / 4095.0f;
    if (_configs[idx].invert) n = 1.0f - n;
    return constrain(n, 0.0f, 1.0f);
}

int PotManager::rawValue(int idx) const {
    return (idx >= 0 && idx < POT_COUNT) ? _raw[idx] : 0;
}

float PotManager::mapNorm(float norm, const PotConfig& cfg) const {
    return cfg.minValue + norm * (cfg.maxValue - cfg.minValue);
}

void PotManager::applyAction(int idx, CatController& cat, RadioState& state, float norm) {
    const PotConfig& cfg = _configs[idx];
    float mapped = mapNorm(norm, cfg);

    switch (cfg.action) {
        case PotAction::NONE:
            break;

        case PotAction::FREQ_FINE: {
            int steps = static_cast<int>((norm - 0.5f) * 200.0f);
            if (abs(steps) < 1) break;
            int64_t delta = static_cast<int64_t>(steps) * static_cast<int64_t>(cfg.step);
            uint64_t nf = static_cast<uint64_t>(static_cast<int64_t>(state.freqHz) + delta);
            cat.setFrequency(nf);
            break;
        }

        case PotAction::FREQ_COARSE: {
            int steps = static_cast<int>((norm - 0.5f) * 40.0f);
            if (abs(steps) < 1) break;
            int64_t delta = static_cast<int64_t>(steps) * static_cast<int64_t>(cfg.step);
            uint64_t nf = static_cast<uint64_t>(static_cast<int64_t>(state.freqHz) + delta);
            cat.setFrequency(nf);
            break;
        }

        case PotAction::AF_GAIN:
            cat.setLevel("AF", mapped);
            break;

        case PotAction::RF_POWER:
            cat.setLevel("RFPOWER", mapped);
            break;

        case PotAction::RF_GAIN:
            cat.setLevel("RF", mapped);
            break;

        case PotAction::SQUELCH:
            cat.setLevel("SQL", mapped);
            break;

        case PotAction::MIC_GAIN:
            cat.setLevel("MIC", mapped);
            break;

        case PotAction::RIT_OFFSET:
            // RIT via raw or future extension
            state.ritHz = static_cast<int>(mapped);
            break;

        case PotAction::CUSTOM_RIGCTL:
            if (cfg.customCmd[0] != '\0' && strlen(cfg.customCmd) > 0) {
                // customCmd supports placeholder {val} for mapped value
                char cmd[96];
                snprintf(cmd, sizeof(cmd), "%s", cfg.customCmd);
                char valStr[16];
                snprintf(valStr, sizeof(valStr), "%.3f", mapped);
                char* pos = strstr(cmd, "{val}");
                if (pos) {
                    String s(cmd);
                    s.replace("{val}", valStr);
                    strncpy(cmd, s.c_str(), sizeof(cmd) - 1);
                }
                // For direct CAT: send as Yaesu command or ICOM raw
                cat.sendRawHex(cmd);  // if hex, else extend for ASCII
            }
            break;
    }
}

void PotManager::loop(CatController& cat, RadioState& state) {
    if (millis() - _lastPoll < POT_POLL_MS) return;
    _lastPoll = millis();

    for (int i = 0; i < POT_COUNT; i++) {
        if (_configs[i].action == PotAction::NONE) continue;

        int raw = readSmoothed(i);
        if (abs(raw - _lastSent[i]) < POT_DEADZONE) continue;
        _lastSent[i] = raw;

        float norm = normalized(i);
        applyAction(i, cat, state, norm);
    }
}
