#include "radio_profiles.h"
#include "config_sync.h"
#include <cstring>

static const RadioProfileInfo PROFILES[] = {
    // ── Xiegu (CI-V-kompatibel, 19200, Adresse A4) ───────────────────────────
    {
        "XIEGU_G90", "Xiegu G90",
        RadioVendor::ICOM, 19200, 0xA4,
        "Xiegu G90 / IC-7100 (CI-V)",
        "Display-Unit IO (3,5 mm) / USB Serial-B",
        "DATA OUT / USB-Sound RX", "USB-Sound TX / Mic",
        "CI-V 19200, Addr A4. CAT nur am Display-IO, nicht am Basis-IO."
    },
    {
        "XIEGU_X6100", "Xiegu X6100",
        RadioVendor::ICOM, 19200, 0xA4,
        "Xiegu X6100 (CI-V)",
        "USB-C DEV, Serial-B",
        "USB Audio RX", "USB Audio TX",
        "CI-V 19200 A4. flrig/WSJT-X: Modell Xiegu X6100 oder IC-7100."
    },
    {
        "XIEGU_X6200", "Xiegu X6200",
        RadioVendor::ICOM, 19200, 0xA4,
        "Xiegu X6200 (CI-V)",
        "USB-C DEV",
        "USB Audio RX", "USB Audio TX",
        "CI-V 19200 A4. In Software „XIEGU X6200“ wählen."
    },
    // ── Yaesu (CAT ASCII) ────────────────────────────────────────────────────
    {
        "YAESU_FT991A", "Yaesu FT-991A",
        RadioVendor::YAESU, 38400, 0,
        "FT-991 / FT-991A",
        "CAT / ACC (Mini-DIN)",
        "1394 LINE OUT / USB Audio", "USB Audio TX / LINE IN",
        "Menü CAT 38400. FT8: USB oder DATA-USB."
    },
    {
        "YAESU_FT910", "Yaesu FT-910",
        RadioVendor::YAESU, 38400, 0,
        "FT-910",
        "CAT (USB)",
        "USB Audio OUT", "USB Audio IN",
        "CAT 38400 bps."
    },
    {
        "YAESU_FTDX10", "Yaesu FT-DX10",
        RadioVendor::YAESU, 38400, 0,
        "FT-DX10",
        "CAT (USB)",
        "USB Audio RX", "USB Audio TX / LINE IN",
        "CAT RATE 38400, RTS optional."
    },
    {
        "YAESU_FTDX101D", "Yaesu FT-DX101D",
        RadioVendor::YAESU, 38400, 0,
        "FT-DX101D",
        "CAT (USB)",
        "USB Audio RX", "USB Audio TX",
        "CAT 38400. Zwei CAT-Ports: CAT-1 für Steuerung."
    },
    {
        "YAESU_FTDX101MP", "Yaesu FT-DX101MP",
        RadioVendor::YAESU, 38400, 0,
        "FT-DX101MP",
        "CAT (USB)",
        "USB Audio RX", "USB Audio TX",
        "Wie FT-DX101D."
    },
    {
        "YAESU_FT891", "Yaesu FT-891",
        RadioVendor::YAESU, 38400, 0,
        "FT-891",
        "CAT (Mini-DIN)",
        "LINE OUT", "LINE IN / DATA",
        "CAT 38400 (Menü einstellbar)."
    },
    {
        "YAESU_FT897", "Yaesu FT-897",
        RadioVendor::YAESU, 4800, 0,
        "FT-897",
        "CAT (ACC Jack)",
        "LINE OUT", "PACKET/SOUND IN",
        "CAT oft 4800 – im Menü prüfen, ggf. 38400."
    },
    {
        "YAESU_FT857", "Yaesu FT-857",
        RadioVendor::YAESU, 4800, 0,
        "FT-857",
        "CAT (ACC Jack)",
        "LINE OUT", "PACKET IN",
        "CAT Standard 4800 bps."
    },
    // Legacy / Referenz
    {
        "IC-7300", "ICOM IC-7300",
        RadioVendor::ICOM, 19200, 0x94,
        "IC-7300",
        "REMOTE (CI-V)",
        "ACC VARI OUT", "ACC MODU IN",
        "CI-V 19200, Addr 94h."
    },
};

static const size_t PROFILE_COUNT = sizeof(PROFILES) / sizeof(PROFILES[0]);

const RadioProfileInfo* radioProfileFind(const char* id) {
    if (!id || !id[0]) return nullptr;
    for (size_t i = 0; i < PROFILE_COUNT; i++) {
        if (strcmp(PROFILES[i].id, id) == 0) return &PROFILES[i];
    }
    return nullptr;
}

const RadioProfileInfo* radioProfileByIndex(size_t index) {
    if (index >= PROFILE_COUNT) return nullptr;
    return &PROFILES[index];
}

size_t radioProfileCount() { return PROFILE_COUNT; }

bool radioProfileApplyChannel(RadioChannelConfig& ch, const char* id) {
    const RadioProfileInfo* p = radioProfileFind(id);
    if (!p) return false;
    strlcpy(ch.radioModel, p->id, sizeof(ch.radioModel));
    ch.vendor = p->vendor;
    ch.catBaud = p->catBaud;
    if (p->vendor == RadioVendor::ICOM)
        ch.icomAddress = p->icomAddress;
    return true;
}

bool radioProfileApply(AppConfig& cfg, const char* id) {
    if (!radioProfileApplyChannel(cfg.radios[0], id)) return false;
    configSyncLegacyFromRadios(cfg);
    return true;
}

const char* radioProfileLabel(const AppConfig& cfg) {
    const RadioProfileInfo* p = radioProfileFind(cfg.radioModel);
    return p ? p->label : cfg.radioModel;
}
