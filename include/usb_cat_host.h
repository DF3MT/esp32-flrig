#pragma once

#include <Arduino.h>

/** USB-Host (ESP32-S3): bis zu 2× CDC-Serial (CAT) über Powered Hub. */
class UsbCatHost {
public:
    static bool begin();
    static void loop();
    /** nullptr wenn Gerät nicht bereit */
    static Stream* cdcStream(uint8_t index);
    static bool deviceReady(uint8_t index);
};
