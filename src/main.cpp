#include <Arduino.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include "deck.h"

TFT_eSPI tft = TFT_eSPI();

// --- HELPER: SCALED DRAWING (For Giant Cards) ---
// Draws an image but makes it 'scale' times bigger
void drawScaledImage(int w, int h, const uint16_t *data, int scale) {
  int x = (320-(w * scale)) / 2; // Centered for 320px height
  int y = (480-(h * scale)) / 2; // Centered for 480px width
  for (int row = 0; row < h; row++) {
    for (int col = 0; col < w; col++) {
      // 1. Get the color from the array
      // Note: Some converters use uint8_t (bytes), so we might need to combine them.
      // If your array is 'uint16_t', use this:
      uint16_t color = data[row * w + col];

      // 2. Draw a big square instead of a single dot
      tft.fillRect(x + (col * scale), y + (row * scale), scale, scale, color);
    }
  }
}

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(2); 
  tft.fillScreen(0x000);
  tft.setSwapBytes(true);
  drawScaledImage(64, 96, deck[0], 5);
}

void loop() {
  static unsigned long lastSwitch = 0;
  static int currentCard = 0;

  if (millis() - lastSwitch >= 1000) {
    lastSwitch = millis();
    currentCard = (currentCard + 1) % 52;
    //tft.fillScreen(0x000);
    drawScaledImage(64, 96, deck[currentCard], 5);
  }
}