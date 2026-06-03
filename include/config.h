#pragma once

#include <Arduino.h>

// ── WiFi / rigctld ──────────────────────────────────────────────────────────
#define WIFI_AP_SSID        "ESP32-CAT-Panel"
#define WIFI_AP_PASS        "hamradio123"
#define RIGCTLD_PORT        4532
#define WEB_CONFIG_PORT     80
#define FLRIG_XMLRPC_PORT   12345

// ── WiFi-Audio (I2S ↔ UDP/WebSocket) ───────────────────────────────────────
#define AUDIO_PORT_OUT      4533   // ESP → Client (Empfang vom Funk)
#define AUDIO_PORT_IN       4534   // Client → ESP (Mikro zum Funk)
#define AUDIO_WS_PATH       "/ws/audio"
#define AUDIO_SAMPLE_RATE   16000
#define AUDIO_FRAME_SAMPLES 320    // 20 ms @ 16 kHz
#define AUDIO_MAGIC         0x45535041u  // 'ESPA'

#ifdef BOARD_CYD
  #define I2S_BCLK_PIN        26
  #define I2S_LRCK_PIN        25
  #define I2S_DOUT_PIN        22   // ESP → DAC → Funk-Mic/Line-In
  #define I2S_DIN_PIN         4    // ADC → ESP ← Funk-Line-Out/SPKR
#elif defined(BOARD_TDISPLAY)
  #define I2S_BCLK_PIN        12
  #define I2S_LRCK_PIN        13
  #define I2S_DOUT_PIN        14
  #define I2S_DIN_PIN         15
#else
  #define I2S_BCLK_PIN        26
  #define I2S_LRCK_PIN        25
  #define I2S_DOUT_PIN        22
  #define I2S_DIN_PIN         4
#endif

// ── Radio UART (CAT) ────────────────────────────────────────────────────────
#ifdef BOARD_TDISPLAY
  // GPIO16 = TFT_DC on TTGO T-Display – CAT auf freie Header-Pins
  #define CAT_RX_PIN          27
  #define CAT_TX_PIN          17
#else
  #define CAT_RX_PIN          16
  #define CAT_TX_PIN          17
#endif
#define CAT_BAUD_ICOM       19200
#define CAT_BAUD_YAESU      38400
#define CAT_DEFAULT_BAUD    38400

// ── Potentiometer ADC pins (GPIO, 12-bit) ─────────────────────────────────
#define POT_COUNT           5
#ifdef BOARD_CYD
  // GPIO33 = Touch CS on CYD – nicht für Potis verwenden
  static const uint8_t POT_PINS[POT_COUNT] = {32, 35, 34, 39, 36};
#elif defined(BOARD_TDISPLAY)
  static const uint8_t POT_PINS[POT_COUNT] = {32, 33, 25, 26, 34};
#else
  static const uint8_t POT_PINS[POT_COUNT] = {32, 33, 34, 35, 36};
#endif

#define POT_DEADZONE        8      // raw ADC steps
#define POT_POLL_MS         50
#define POT_SMOOTH_SAMPLES  4

// ── Display ─────────────────────────────────────────────────────────────────
#ifdef BOARD_CYD
  #define TFT_MOSI          13
  #define TFT_SCLK          14
  #define TFT_CS            15
  #define TFT_DC            2
  #define TFT_RST           -1
  #define TFT_BL            21
  #define TOUCH_CS          33
  #define TFT_WIDTH         320
  #define TFT_HEIGHT        240
#elif defined(BOARD_TDISPLAY)
  // TTGO T-Display v1.1 – ST7789 135x240, keine Touch, 2 Tasten
  #define TFT_MOSI          19
  #define TFT_SCLK          18
  #define TFT_CS            5
  #define TFT_DC            16
  #define TFT_RST           23
  #define TFT_BL            4
  #define BTN_UP_PIN        35   // rechte Taste
  #define BTN_DOWN_PIN      0    // linke Taste (BOOT)
  #define TFT_WIDTH         240  // Landscape
  #define TFT_HEIGHT        135
#else
  #define TFT_MOSI          23
  #define TFT_SCLK          18
  #define TFT_CS            5
  #define TFT_DC            2
  #define TFT_RST           4
  #define TFT_BL            15
  #define TOUCH_CS          22
  #define TFT_WIDTH         320
  #define TFT_HEIGHT        240
#endif

// ── Radio types ─────────────────────────────────────────────────────────────
enum class RadioVendor : uint8_t {
    ICOM = 0,
    YAESU = 1,
};

enum class ConnectionMode : uint8_t {
    DIRECT_CAT = 0,     // ESP32 → Radio UART
    FLRIG_CLIENT = 1,   // ESP32 → PC flrig XML-RPC
    HAMLIB_CLIENT = 2,  // ESP32 → PC rigctld
};

// ── Pot action types (frei programmierbar) ────────────────────────────────────
enum class PotAction : uint8_t {
    NONE = 0,
    FREQ_FINE,          // ± Hz per step
    FREQ_COARSE,        // ± kHz per step
    AF_GAIN,            // Audio level 0.0–1.0
    RF_POWER,           // TX power 0.0–1.0
    RF_GAIN,            // RF gain / AGC
    SQUELCH,            // Squelch level
    MIC_GAIN,           // Mic gain
    RIT_OFFSET,         // RIT ± Hz
    CUSTOM_RIGCTL,      // Send custom rigctl command at thresholds
};

struct PotConfig {
    PotAction action = PotAction::NONE;
    float     minValue = 0.0f;
    float     maxValue = 1.0f;
    float     step     = 0.01f;
    char      customCmd[64] = "";   // for CUSTOM_RIGCTL: e.g. "L AF 0.5"
    bool      invert   = false;
};

struct AppConfig {
    RadioVendor     vendor       = RadioVendor::ICOM;
    ConnectionMode  connMode     = ConnectionMode::DIRECT_CAT;
    uint32_t        icomAddress  = 0x94;    // IC-7300 default
    uint32_t        catBaud      = CAT_DEFAULT_BAUD;
    char            wifiSsid[32]   = "";
    char            wifiPass[64]   = "";
    char            remoteHost[64] = "";    // flrig/rigctld IP
    uint16_t        remotePort   = RIGCTLD_PORT;
    bool            audioEnabled = false;
    uint16_t        audioPortOut = AUDIO_PORT_OUT;
    uint16_t        audioPortIn  = AUDIO_PORT_IN;
    uint32_t        audioSampleRate = AUDIO_SAMPLE_RATE;
    PotConfig       pots[POT_COUNT];
};

#pragma pack(push, 1)
struct AudioPacketHdr {
    uint32_t magic;
    uint32_t seq;
    uint16_t nSamples;
};
#pragma pack(pop)
