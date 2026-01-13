#include <Arduino.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include "deck.h"

#define TABLE_GREEN 0x3329

TFT_eSPI tft = TFT_eSPI();

// --- HELPER: SCALED DRAWING (For Giant Cards) ---
// Draws an image but makes it 'scale' times bigger
void drawScaledImage(const uint16_t *data) {
  tft.setRotation(2); 
  int w = 64;  // Width of the original image
  int h = 96;  // Height of the original image
  int scale = 5; // Scale factor
  for (int row = 0; row < h; row++) {
    for (int col = 0; col < w; col++) {
      // 1. Get the color from the array
      // Note: Some converters use uint8_t (bytes), so we might need to combine them.
      // If your array is 'uint16_t', use this:
      uint16_t color = data[row * w + col];

      // 2. Draw a big square instead of a single dot
      tft.fillRect((col * scale), (row * scale), 5, 5, color);
    }
  }
}

void drawFiveCards(int flop1, int flop2, int flop3, int turn, int river) {
  tft.setRotation(1); // Landscape mode
  tft.fillScreen(TABLE_GREEN);

  int w = 64; // Width of the original image
  int h = 96; // Height of the original image 

  int positions[5][2] = {
    {40, 160 - (h/2)},   // Flop 1
    {60 + (w), 160 - (h/2)},   // Flop 2
    {80 + (2*w), 160 - (h/2)},  // Flop 3
    {100 + (3*w), 160 - (h/2)},  // Turn
    {120 + (4*w), 160 - (h/2)}   // River
  };

  const uint16_t* cards[5] = {
    deck[flop1],
    deck[flop2],
    deck[flop3],
    deck[turn],
    deck[river]
  };

  for (int i = 0; i < 5; i++) {
    tft.pushImage(positions[i][0], positions[i][1], w, h, cards[i]);
  }
}
void setup() {
  Serial.begin(115200);
  tft.init();
  tft.fillScreen(0x000);
  tft.setSwapBytes(true);

  drawFiveCards(9, 10, 11, 12, 0); // Example: 10, J, Q, K, A of Spades
}

void loop() {
  /*
  static unsigned long lastSwitch = 0;
  static int currentCard = 0;

  if (millis() - lastSwitch >= 5000) {
    lastSwitch = millis();
    currentCard = (currentCard + 1) % 53; // Assuming 52 cards + 1 back
    if (currentCard == 52) {
      drawScaledImage(Backside);
    } else {
    drawScaledImage(deck[currentCard]);
    }
  }
    */
}