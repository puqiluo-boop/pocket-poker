#include <Arduino.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include "../lib/Deck/Deck.h"
#include "../lib/Colors.h"

TFT_eSPI tft = TFT_eSPI();

// --- HELPER: SCALED DRAWING (For Giant Cards) ---
void drawScaledCard(int cardIndex) {
  tft.setRotation(2); 
  const uint16_t *data = deck[cardIndex];
  int scale = 5; // Scale factor
  for (int row = 0; row < CARD_HEIGHT; row++) {
    for (int col = 0; col < CARD_WIDTH; col++) {
      // 1. Get the color from the array
      uint16_t color = data[row * w + col];

      // 2. Draw a big square instead of a single dot
      tft.fillRect((col * scale), (row * scale), 5, 5, color);
    }
  }
}

void drawFiveCards(int flop1, int flop2, int flop3, int turn, int river) {
  tft.setRotation(1); // Landscape mode
  tft.fillScreen(TABLE_GREEN);

  int positions[5][2] = {
    {40, 160 - (CARD_HEIGHT/2)},   // Flop 1
    {60 + (CARD_WIDTH), 160 - (CARD_HEIGHT/2)},   // Flop 2
    {80 + (2*CARD_WIDTH), 160 - (CARD_HEIGHT/2)},  // Flop 3
    {100 + (3*CARD_WIDTH), 160 - (CARD_HEIGHT/2)},  // Turn
    {120 + (4*CARD_WIDTH), 160 - (CARD_HEIGHT/2)}   // River
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

int* shuffleDeck() {
  static int deckOrder[52];
  for (int i = 0; i < 52; i++) {
    deckOrder[i] = i;
  }

  // Simple Fisher-Yates shuffle
  for (int i = 51; i > 0; i--) {
    int j = random(0, i + 1);
    // Swap
    int temp = deckOrder[i];
    deckOrder[i] = deckOrder[j];
    deckOrder[j] = temp;
  }
  return deckOrder;
}

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.fillScreen(0x000);
  tft.setSwapBytes(true);

  drawFiveCards(9, 10, 11, 12, 0); // Example: 10, J, Q, K, A of Spades
}

void loop() {
  static unsigned long lastSwitch = 0;

  if (millis() - lastSwitch >= 5000) {
    int* shuffledDeck = shuffleDeck();
    drawFiveCards(shuffledDeck[1], shuffledDeck[2], shuffledDeck[3], shuffledDeck[5], shuffledDeck[7]);
    lastSwitch = millis();
  }
}