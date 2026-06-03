#include "usb_cat_host.h"

/*
 * USB-Host CAT (ESP32-S3): Funkgeräte als CDC am Powered USB-Hub.
 * Vollständiger Host-Stack: env esp32-s3-dual-usb + passende Arduino-USB-Host-Lib.
 * Bis dahin: Architektur + rigctld/Audio pro Kanal; CAT-USB meldet „nicht bereit“.
 */
bool UsbCatHost::begin() {
#if defined(BOARD_S3_USB_HOST)
    Serial.println("[usb-host] S3: Hub + 2× USB-CAT – Treiber folgt (CDC attach)");
#endif
    return true;
}

void UsbCatHost::loop() {}

Stream* UsbCatHost::cdcStream(uint8_t) { return nullptr; }

bool UsbCatHost::deviceReady(uint8_t) { return false; }
