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

// Touch Controller (CST816)
#define TOUCH_ADDR 0x15
#define TOUCH_SDA 48
#define TOUCH_SCL 47
#define TOUCH_RST -1  // No separate reset pin
#define TOUCH_INT 7

// For BSP compatibility
#define EXAMPLE_PIN_NUM_TP_RST TOUCH_RST

// Power
#define BOARD_BAT_ADC 1  // If you ever read battery level