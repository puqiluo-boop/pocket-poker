#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

// 1. Color Settings
#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0

// 2. Memory
#define LV_MEM_CUSTOM 0
#define LV_MEM_SIZE (48 * 1024U)

// 3. Tick
#define LV_TICK_CUSTOM 0

// 4. Input Device Settings
#define LV_INDEV_DEF_READ_PERIOD 30

// 5. Features
#define LV_USE_LOG 0
#define LV_USE_BTN 1
#define LV_USE_LABEL 1
#define LV_USE_SLIDER 1
#define LV_USE_BAR 1
#define LV_USE_TEXTAREA 1

// 6. Fonts - CRITICAL: Set to 1 to enable compilation
#define LV_FONT_MONTSERRAT_8  0
#define LV_FONT_MONTSERRAT_10 0
#define LV_FONT_MONTSERRAT_12 0
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 0
#define LV_FONT_MONTSERRAT_18 0
#define LV_FONT_MONTSERRAT_20 1  // MUST be 1
#define LV_FONT_MONTSERRAT_22 0
#define LV_FONT_MONTSERRAT_24 0
#define LV_FONT_MONTSERRAT_26 0
#define LV_FONT_MONTSERRAT_28 0
#define LV_FONT_MONTSERRAT_30 0
#define LV_FONT_MONTSERRAT_32 0
#define LV_FONT_MONTSERRAT_34 0
#define LV_FONT_MONTSERRAT_36 0
#define LV_FONT_MONTSERRAT_38 0
#define LV_FONT_MONTSERRAT_40 0
#define LV_FONT_MONTSERRAT_42 0
#define LV_FONT_MONTSERRAT_44 0
#define LV_FONT_MONTSERRAT_46 0
#define LV_FONT_MONTSERRAT_48 0

#define LV_FONT_DEFAULT &lv_font_montserrat_14

#endif