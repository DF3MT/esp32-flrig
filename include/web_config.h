#pragma once

#include "config.h"
#include <functional>

class AudioBridge;

class WebConfig {
public:
    using SaveCallback = std::function<void(const AppConfig&)>;

    void setAudioBridge(AudioBridge* audio) { _audio = audio; }
    bool begin(AppConfig* cfg, SaveCallback onSave);
    void loop();

private:
    AppConfig* _cfg = nullptr;
    SaveCallback _onSave;
    AudioBridge* _audio = nullptr;
};
