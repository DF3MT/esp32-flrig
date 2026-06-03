#pragma once

#include "config.h"
#include "cat_controller.h"
#include "hamlib_tcp_server.h"
#include "audio_bridge.h"

/** Initialisiert alle Funk-Kanäle (UART und/oder USB-CDC). */
class RadioChannelManager {
public:
    bool begin(AppConfig& cfg);
    void loop();

    static constexpr int kMax = RADIO_COUNT;

    CatController& cat(int i) { return _cat[i]; }
    HamlibTcpServer& rigctld(int i) { return _rigctld[i]; }
    AudioBridge& audio(int i) { return _audio[i]; }

private:
    CatController   _cat[kMax];
    HamlibTcpServer _rigctld[kMax];
    AudioBridge     _audio[kMax];
    HardwareSerial* _serials[kMax] = {};
};

bool radioChannelBeginCat(int index, RadioChannelConfig& ch, CatController& cat,
                          HardwareSerial** outSerial);
