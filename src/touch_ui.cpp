#include "touch_ui.h"
#include <lvgl.h>

#if defined(BOARD_CYD) || defined(BOARD_TDISPLAY)
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#endif

#if defined(BOARD_CYD)

class LGFX_CYD : public lgfx::LGFX_Device {
    lgfx::Panel_ILI9341 _panel;
    lgfx::Bus_SPI       _bus;
    lgfx::Light_PWM     _light;
    lgfx::Touch_XPT2046 _touch;

public:
    LGFX_CYD() {
        {
            auto cfg = _bus.config();
            cfg.spi_host = VSPI_HOST;
            cfg.spi_mode = 0;
            cfg.freq_write = 40000000;
            cfg.freq_read  = 16000000;
            cfg.pin_sclk = TFT_SCLK;
            cfg.pin_mosi = TFT_MOSI;
            cfg.pin_miso = -1;
            cfg.pin_dc   = TFT_DC;
            _bus.config(cfg);
            _panel.setBus(&_bus);
        }
        {
            auto cfg = _panel.config();
            cfg.pin_cs   = TFT_CS;
            cfg.pin_rst  = TFT_RST;
            cfg.pin_busy = -1;
            cfg.memory_width  = 240;
            cfg.memory_height = 320;
            cfg.panel_width   = 240;
            cfg.panel_height  = 320;
            cfg.offset_x = 0;
            cfg.offset_y = 0;
            cfg.offset_rotation = 0;
            cfg.readable = true;
            cfg.invert = false;
            cfg.rgb_order = false;
            cfg.dlen_16bit = false;
            cfg.bus_shared = true;
            _panel.config(cfg);
        }
        {
            auto cfg = _light.config();
            cfg.pin_bl = TFT_BL;
            cfg.invert = false;
            cfg.freq   = 44100;
            cfg.pwm_channel = 7;
            _light.config(cfg);
            _panel.setLight(&_light);
        }
        {
            auto cfg = _touch.config();
            cfg.x_min = 300;
            cfg.x_max = 3800;
            cfg.y_min = 300;
            cfg.y_max = 3800;
            cfg.pin_int = -1;
            cfg.bus_shared = true;
            cfg.offset_rotation = 0;
            cfg.spi_host = VSPI_HOST;
            cfg.freq = 2500000;
            cfg.pin_sclk = TFT_SCLK;
            cfg.pin_mosi = TFT_MOSI;
            cfg.pin_miso = 19;
            cfg.pin_cs   = TOUCH_CS;
            _touch.config(cfg);
            _panel.setTouch(&_touch);
        }
        setPanel(&_panel);
    }
};

static LGFX_CYD s_lcd;

#elif defined(BOARD_TDISPLAY)

class LGFX_TDisplay : public lgfx::LGFX_Device {
    lgfx::Panel_ST7789 _panel;
    lgfx::Bus_SPI      _bus;
    lgfx::Light_PWM    _light;

public:
    LGFX_TDisplay() {
        {
            auto cfg = _bus.config();
            cfg.spi_host = HSPI_HOST;
            cfg.spi_mode = 0;
            cfg.freq_write = 40000000;
            cfg.freq_read  = 16000000;
            cfg.pin_sclk = TFT_SCLK;
            cfg.pin_mosi = TFT_MOSI;
            cfg.pin_miso = -1;
            cfg.pin_dc   = TFT_DC;
            _bus.config(cfg);
            _panel.setBus(&_bus);
        }
        {
            auto cfg = _panel.config();
            cfg.pin_cs   = TFT_CS;
            cfg.pin_rst  = TFT_RST;
            cfg.pin_busy = -1;
            cfg.panel_width  = 135;
            cfg.panel_height = 240;
            cfg.offset_x     = 52;
            cfg.offset_y     = 40;
            cfg.offset_rotation = 0;
            cfg.readable = false;
            cfg.invert   = true;
            cfg.rgb_order = false;
            _panel.config(cfg);
        }
        {
            auto cfg = _light.config();
            cfg.pin_bl = TFT_BL;
            cfg.invert = false;
            cfg.freq   = 44100;
            cfg.pwm_channel = 7;
            _light.config(cfg);
            _panel.setLight(&_light);
        }
        setPanel(&_panel);
    }
};

static LGFX_TDisplay s_lcd;

#endif

#if defined(BOARD_CYD) || defined(BOARD_TDISPLAY)

static lv_disp_draw_buf_t s_drawBuf;
static lv_color_t s_buf1[TFT_WIDTH * 20];
static lv_disp_drv_t s_dispDrv;
static lv_indev_drv_t s_indevDrv;

static lv_obj_t* s_lblFreq = nullptr;
static lv_obj_t* s_lblMode = nullptr;
static lv_obj_t* s_lblPtt  = nullptr;
static lv_obj_t* s_btnUp   = nullptr;
static lv_obj_t* s_btnDown = nullptr;

static CatController* s_catPtr = nullptr;
static uint64_t* s_freqPtr = nullptr;

#ifdef BOARD_TDISPLAY
static bool s_btnUpLast = true;
static bool s_btnDownLast = true;
static uint32_t s_btnDebounce = 0;
#endif

static void dispFlush(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_p) {
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;
    s_lcd.startWrite();
    s_lcd.setAddrWindow(area->x1, area->y1, w, h);
    s_lcd.writePixels((lgfx::rgb565_t*)&color_p->full, w * h);
    s_lcd.endWrite();
    lv_disp_flush_ready(drv);
}

#ifdef BOARD_CYD
static void touchRead(lv_indev_drv_t* drv, lv_indev_data_t* data) {
    uint16_t x, y;
    if (s_lcd.getTouch(&x, &y)) {
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = x;
        data->point.y = y;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}
#endif

static void freqStep(int64_t deltaHz) {
    if (!s_catPtr || !s_freqPtr) return;
    int64_t nf = static_cast<int64_t>(*s_freqPtr) + deltaHz;
    if (nf < 0) nf = 0;
    s_catPtr->setFrequency(static_cast<uint64_t>(nf));
}

static void btnUpCb(lv_event_t* e) {
    (void)e;
    freqStep(1000);
}

static void btnDownCb(lv_event_t* e) {
    (void)e;
    freqStep(-1000);
}

bool TouchUI::begin() {
    s_lcd.init();
    s_lcd.setRotation(1);
    s_lcd.fillScreen(TFT_BLACK);
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

#ifdef BOARD_TDISPLAY
    pinMode(BTN_UP_PIN, INPUT);
    pinMode(BTN_DOWN_PIN, INPUT);
#endif

    lv_init();
    lv_disp_draw_buf_init(&s_drawBuf, s_buf1, nullptr, TFT_WIDTH * 20);
    lv_disp_drv_init(&s_dispDrv);
    s_dispDrv.hor_res = TFT_WIDTH;
    s_dispDrv.ver_res = TFT_HEIGHT;
    s_dispDrv.flush_cb = dispFlush;
    s_dispDrv.draw_buf = &s_drawBuf;
    lv_disp_drv_register(&s_dispDrv);

#ifdef BOARD_CYD
    lv_indev_drv_init(&s_indevDrv);
    s_indevDrv.type = LV_INDEV_TYPE_POINTER;
    s_indevDrv.read_cb = touchRead;
    lv_indev_drv_register(&s_indevDrv);
#endif

    createMainScreen();
    _initialized = true;
    return true;
}

void TouchUI::createMainScreen() {
    lv_obj_t* scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x1a1a2e), 0);

    s_lblFreq = lv_label_create(scr);
    lv_label_set_text(s_lblFreq, "0.000 MHz");
#ifdef BOARD_TDISPLAY
    lv_obj_set_style_text_font(s_lblFreq, &lv_font_montserrat_20, 0);
    lv_obj_align(s_lblFreq, LV_ALIGN_TOP_MID, 0, 8);
#else
    lv_obj_set_style_text_font(s_lblFreq, &lv_font_montserrat_28, 0);
    lv_obj_align(s_lblFreq, LV_ALIGN_TOP_MID, 0, 20);
#endif
    lv_obj_set_style_text_color(s_lblFreq, lv_color_hex(0x00ff88), 0);

    s_lblMode = lv_label_create(scr);
    lv_label_set_text(s_lblMode, "USB");
#ifdef BOARD_TDISPLAY
    lv_obj_set_style_text_font(s_lblMode, &lv_font_montserrat_14, 0);
    lv_obj_align(s_lblMode, LV_ALIGN_TOP_MID, 0, 34);
#else
    lv_obj_set_style_text_font(s_lblMode, &lv_font_montserrat_20, 0);
    lv_obj_align(s_lblMode, LV_ALIGN_TOP_MID, 0, 60);
#endif

    s_lblPtt = lv_label_create(scr);
    lv_label_set_text(s_lblPtt, "RX");
    lv_obj_set_style_text_color(s_lblPtt, lv_color_hex(0x00ccff), 0);
#ifdef BOARD_TDISPLAY
    lv_obj_align(s_lblPtt, LV_ALIGN_TOP_MID, 0, 54);
    lv_obj_t* hint = lv_label_create(scr);
    lv_label_set_text(hint, "GPIO0:-1k  GPIO35:+1k");
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(hint, lv_color_hex(0x888888), 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -4);
#else
    lv_obj_align(s_lblPtt, LV_ALIGN_TOP_MID, 0, 90);

    s_btnUp = lv_btn_create(scr);
    lv_obj_set_size(s_btnUp, 80, 50);
    lv_obj_align(s_btnUp, LV_ALIGN_BOTTOM_LEFT, 20, -20);
    lv_obj_add_event_cb(s_btnUp, btnUpCb, LV_EVENT_CLICKED, nullptr);
    lv_obj_t* lblUp = lv_label_create(s_btnUp);
    lv_label_set_text(lblUp, "+1kHz");
    lv_obj_center(lblUp);

    s_btnDown = lv_btn_create(scr);
    lv_obj_set_size(s_btnDown, 80, 50);
    lv_obj_align(s_btnDown, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
    lv_obj_add_event_cb(s_btnDown, btnDownCb, LV_EVENT_CLICKED, nullptr);
    lv_obj_t* lblDn = lv_label_create(s_btnDown);
    lv_label_set_text(lblDn, "-1kHz");
    lv_obj_center(lblDn);
#endif

    s_catPtr = _cat;
    s_freqPtr = &_displayFreq;
}

void TouchUI::updateState(const RadioState& state) {
    if (!_initialized) return;
    _displayFreq = state.freqHz;

    char buf[32];
    snprintf(buf, sizeof(buf), "%.3f MHz", state.freqHz / 1e6);
    lv_label_set_text(s_lblFreq, buf);
    lv_label_set_text(s_lblMode, state.mode);
    lv_label_set_text(s_lblPtt, state.ptt ? "TX" : "RX");
    lv_obj_set_style_text_color(s_lblPtt,
        state.ptt ? lv_color_hex(0xff4444) : lv_color_hex(0x00ccff), 0);
}

void TouchUI::loop() {
    if (!_initialized) return;

#ifdef BOARD_TDISPLAY
    if (millis() - s_btnDebounce > 180) {
        bool upNow = digitalRead(BTN_UP_PIN) == LOW;
        bool downNow = digitalRead(BTN_DOWN_PIN) == LOW;
        if (upNow && !s_btnUpLast) {
            freqStep(1000);
            s_btnDebounce = millis();
        }
        if (downNow && !s_btnDownLast) {
            freqStep(-1000);
            s_btnDebounce = millis();
        }
        s_btnUpLast = upNow;
        s_btnDownLast = downNow;
    }
#endif

    lv_timer_handler();
}

#else
// Headless fallback when no display board defined
bool TouchUI::begin() { _initialized = true; return true; }
void TouchUI::loop() {}
void TouchUI::updateState(const RadioState&) {}
void TouchUI::createMainScreen() {}
#endif
