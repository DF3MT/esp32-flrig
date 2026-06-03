#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

#define LV_COLOR_DEPTH     16
#define LV_COLOR_16_SWAP   0
#define LV_USE_PERF_MONITOR 0
#define LV_USE_LOG          0

#define LV_HOR_RES_MAX      320
#define LV_VER_RES_MAX      240

#define LV_MEM_CUSTOM       0
#define LV_MEM_SIZE         (32U * 1024U)

#define LV_DISP_DEF_REFR_PERIOD 30
#define LV_INDEV_DEF_READ_PERIOD 30

#define LV_USE_LABEL        1
#define LV_USE_BTN          1
#define LV_USE_BTNMATRIX    1
#define LV_USE_SLIDER       1
#define LV_USE_ARC          1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_28 1

#define LV_USE_THEME_DEFAULT 1
#define LV_TICK_CUSTOM      1
#define LV_TICK_CUSTOM_INCLUDE "Arduino.h"
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())

#endif
