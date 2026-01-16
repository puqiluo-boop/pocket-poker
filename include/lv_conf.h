#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

// 1. Color Settings
#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0

// 2. Memory
#define LV_MEM_CUSTOM 0
#define LV_MEM_SIZE (48 * 1024U)

// 3. Tick - Don't use custom tick, handle it manually
#define LV_TICK_CUSTOM 0

// 4. Input Device Settings
#define LV_INDEV_DEF_READ_PERIOD 30

// 5. Features
#define LV_USE_LOG 0
#define LV_USE_BTN 1
#define LV_USE_LABEL 1
#define LV_USE_SLIDER 1
#define LV_USE_BAR 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_DEFAULT &lv_font_montserrat_14

#endif