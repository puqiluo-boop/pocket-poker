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

// --- 1. TOUCH SETTINGS (From your working example) ---
#define TOUCH_ADDR 0x15
// Note: These should match the pins in your BoardConfig.h
// If you haven't updated BoardConfig yet, we define them here to be safe:
#define TOUCH_SDA 48
#define TOUCH_SCL 47
#define TOUCH_RST -1
#define TOUCH_INT -1

// Power
#define BOARD_BAT_ADC 1  // If you ever read battery level