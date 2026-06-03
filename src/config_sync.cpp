#include "config_sync.h"
#include "radio_profiles.h"
#include <cstring>

void configSyncLegacyFromRadios(AppConfig& cfg) {
    const RadioChannelConfig& ch = cfg.radios[0];
    strlcpy(cfg.radioModel, ch.radioModel, sizeof(cfg.radioModel));
    cfg.vendor = ch.vendor;
    cfg.catBaud = ch.catBaud;
    cfg.icomAddress = ch.icomAddress;
    cfg.audioEnabled = ch.audioEnabled;
    cfg.audioPortOut = ch.audioPortOut;
    cfg.audioPortIn = ch.audioPortIn;
    cfg.audioSampleRate = ch.audioSampleRate;
}

void configSyncRadiosFromLegacy(AppConfig& cfg) {
    for (int i = 0; i < RADIO_COUNT; i++)
        radioChannelDefaults(cfg.radios[i], i);

    RadioChannelConfig& ch = cfg.radios[0];
    if (cfg.radioModel[0])
        strlcpy(ch.radioModel, cfg.radioModel, sizeof(ch.radioModel));
    ch.vendor = cfg.vendor;
    ch.catBaud = cfg.catBaud;
    ch.icomAddress = (uint8_t)cfg.icomAddress;
    ch.audioEnabled = cfg.audioEnabled;
    ch.audioPortOut = cfg.audioPortOut;
    ch.audioPortIn = cfg.audioPortIn;
    ch.audioSampleRate = cfg.audioSampleRate;
    ch.enabled = true;
    if (cfg.radioModel[0])
        radioProfileApplyChannel(ch, cfg.radioModel);
}
