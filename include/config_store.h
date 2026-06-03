#pragma once

#include "config.h"

class ConfigStore {
public:
    bool begin();
    bool load(AppConfig& cfg);
    bool save(const AppConfig& cfg);
    void setDefaults(AppConfig& cfg);

private:
    static const char* PATH;
};
