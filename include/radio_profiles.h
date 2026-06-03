#pragma once

#include "config.h"

struct RadioProfileInfo {
    const char* id;
    const char* label;
    RadioVendor vendor;
    uint32_t    catBaud;
    uint8_t     icomAddress;
    const char* hamlibModel;
    const char* catPort;
    const char* audioRx;
    const char* audioTx;
    const char* notes;
};

const RadioProfileInfo* radioProfileFind(const char* id);
const RadioProfileInfo* radioProfileByIndex(size_t index);
size_t radioProfileCount();
bool radioProfileApply(AppConfig& cfg, const char* id);
const char* radioProfileLabel(const AppConfig& cfg);
