#pragma once

// --- WAVESHARE ESP32-S3 BOARD PINOUT ---
// These are hardwired on the PCB. Both Dealer and Player use these.

// Built-in LCD (ST7789)
#define TFT_BL    1
#define TFT_MOSI 38
#define TFT_SCK  39
#define TFT_MISO 40   // (Optional, mostly unused for display)
#define TFT_CS    45
#define TFT_DC    42
#define TFT_RST   -1  // Tied to global reset

// Built-in Touch (I2C)
#define TOUCH_SDA 8   // (EXAMPLE - verify if you use touch)
#define TOUCH_SCL 9   // (EXAMPLE - verify if you use touch)
#define TOUCH_RST -1
#define TOUCH_IRQ -1

// Power
#define BOARD_BAT_ADC 1  // If you ever read battery level