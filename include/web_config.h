#pragma once

#include "config.h"
#include <functional>

class WebConfig {
public:
    using SaveCallback = std::function<void(const AppConfig&)>;

    bool begin(AppConfig* cfg, SaveCallback onSave);
    void loop();

private:
    AppConfig* _cfg = nullptr;
    SaveCallback _onSave;
};
