#include "radio_channel.h"
#include "usb_cat_host.h"
#include "rigctld_server.h"
#include "config_sync.h"
#include "radio_profiles.h"

static HardwareSerial s_serialA(2);
#if RADIO_COUNT > 1
static HardwareSerial s_serialB(1);
#endif

static HardwareSerial* serialForIndex(int index) {
    if (index == 0) return &s_serialA;
#if RADIO_COUNT > 1
    if (index == 1) return &s_serialB;
#endif
    return nullptr;
}

bool radioChannelBeginCat(int index, RadioChannelConfig& ch, CatController& cat,
                          HardwareSerial** outSerial) {
    if (!ch.enabled) return false;

    Stream* stream = nullptr;
    if (ch.catLink == CatLinkType::USB_CDC) {
        stream = UsbCatHost::cdcStream(ch.usbHostIndex);
        if (!stream) {
            Serial.printf("[cat-%s] USB CDC #%u nicht bereit (Hub/Kabel?)\n",
                          ch.label, ch.usbHostIndex);
            return false;
        }
        if (outSerial) *outSerial = nullptr;
    } else {
        HardwareSerial* ser = serialForIndex(index);
        if (!ser) return false;
        int rx = ch.uartRxPin >= 0 ? ch.uartRxPin : CAT_RX_PIN;
        int tx = ch.uartTxPin >= 0 ? ch.uartTxPin : CAT_TX_PIN;
        if (index == 1) {
            rx = ch.uartRxPin >= 0 ? ch.uartRxPin : CAT_B_RX_PIN;
            tx = ch.uartTxPin >= 0 ? ch.uartTxPin : CAT_B_TX_PIN;
        }
        ser->begin(ch.catBaud, SERIAL_8N1, rx, tx);
        stream = ser;
        if (outSerial) *outSerial = ser;
    }

    if (!cat.begin(*stream, ch.vendor, ch.catBaud, ch.icomAddress)) return false;
    Serial.printf("[cat-%s] %s @ %lu (%s)\n", ch.label,
                  ch.vendor == RadioVendor::YAESU ? "Yaesu" : "ICOM",
                  (unsigned long)ch.catBaud,
                  ch.catLink == CatLinkType::USB_CDC ? "USB" : "UART");
    return true;
}

bool RadioChannelManager::begin(AppConfig& cfg) {
    configSyncRadiosFromLegacy(cfg);

    bool needUsb = false;
    for (int i = 0; i < kMax; i++) {
        if (cfg.radios[i].enabled && cfg.radios[i].catLink == CatLinkType::USB_CDC)
            needUsb = true;
        if (cfg.radios[i].enabled && cfg.radios[i].audioLink == AudioLinkType::USB_UAC)
            needUsb = true;
    }
    if (needUsb) UsbCatHost::begin();

    bool anyCat = false;
    for (int i = 0; i < kMax; i++) {
        RadioChannelConfig& ch = cfg.radios[i];
        if (!ch.enabled) continue;

        if (ch.radioModel[0])
            radioProfileApplyChannel(ch, ch.radioModel);

        if (cfg.connMode == ConnectionMode::DIRECT_CAT) {
            if (radioChannelBeginCat(i, ch, _cat[i], &_serials[i]))
                anyCat = true;
        }

        _rigctld[i].begin(ch.rigctldPort, i == 0 ? "rigctld" : "rigctld-B");
        const RadioChannelConfig* chPtr = &ch;
        _rigctld[i].setHandler([&, i, chPtr](const String& cmd) -> String {
            return dispatchRigctl(cmd, _cat[i], _cat[i].state(), &cfg, chPtr);
        });

        if (ch.audioEnabled)
            _audio[i].begin(&ch, (uint8_t)i);

        Serial.printf("[radio-%s] rigctld :%u audio %s\n", ch.label, ch.rigctldPort,
                      ch.audioEnabled ? "on" : "off");
    }

    if (kMax > 0)
        configSyncLegacyFromRadios(cfg);

    return anyCat || cfg.connMode != ConnectionMode::DIRECT_CAT;
}

void RadioChannelManager::loop() {
    UsbCatHost::loop();
    for (int i = 0; i < kMax; i++) {
        if (_serials[i])
            _cat[i].poll();
        _rigctld[i].loop();
    }
}
