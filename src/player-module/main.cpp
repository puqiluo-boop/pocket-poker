#include <Arduino.h>
#include <Arduino_GFX_Library.h>

#include <BoardConfig.h>
#include <Colors.h>
#include <Deck.h>
#include <Trim.h>
#include <Comms.h>

//=========== PLAYER ID ==============
#define PLAYER_ID 1
//===================================

// Store the current hand data received from dealer
CardData currentHand;
bool hasCards = false;

Arduino_DataBus *bus = new Arduino_ESP32SPI(
    TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, TFT_MISO
);

Arduino_GFX *gfx = new Arduino_ST7789(
    bus, TFT_RST, 
    3 /* rotation */, true /* IPS */, 
    240, 320, 
    0, 0, // col offset 1, row offset 1 (WAS 0, 20)
    0, 0  // col offset 2, row offset 2
);

// Helper function to determine if a pixel should be skipped based on trim array
bool shouldSkipPixel(int row, int col, int scale) {
    for (int i = 0; i < 24; i++) {
        if (trim[i][0] == row && trim[i][1] == col) {
            return true;
        }
    }
    return false;
}

// Helper function to draw a card at position (x, y)
void drawScaledCard(int cardIndex, int x, int y, int scale) {
    for (int row = 0; row < CARD_HEIGHT; row++) {
        for (int col = 0; col < CARD_WIDTH; col++) {
            if (shouldSkipPixel(row, col, scale)) {
                continue; // Skip this pixel
            }

            // Get the color from the array
            uint16_t color = deck[cardIndex][row * CARD_WIDTH + col];

            // Draw a big square instead of a single dot
            gfx->fillRect(x + (col * scale), y + (row * scale), scale, scale, color);
        }
    }
}

void drawTwoCards(int card1Index, int card2Index) {
    gfx->fillScreen(TABLE_GREEN);

    // Cards are 64x96 when scaled by 2 = 128x192
    int scaledWidth = CARD_WIDTH * 2;   // 128
    int scaledHeight = CARD_HEIGHT * 2; // 192

    // Screen is 320x240
    int screenWidth = 320;
    int screenHeight = 240;
    
    // Vertical centering
    int y = (screenHeight - scaledHeight) / 2;  // (320 - 192) / 2 = 64
    
    // Horizontal spacing: two cards side by side with gap
    int gap = 10;
    int totalWidth = (scaledWidth * 2) + gap;
    int x1 = (screenWidth - totalWidth) / 2;
    int x2 = x1 + scaledWidth + gap;
    
    drawScaledCard(card1Index, x1, y, 2);
    drawScaledCard(card2Index, x2, y, 2);
}

void drawHoleCards() {
    // Clear screen
    gfx->fillScreen(TABLE_GREEN);
    
    // If we haven't received cards yet, show waiting screen
    if (!hasCards) {
        gfx->setTextColor(WHITE);
        gfx->setTextSize(2);
        gfx->setCursor(60, 140);
        gfx->printf("Player %d", PLAYER_ID);
        gfx->setCursor(40, 170);
        gfx->print("Waiting...");
        return; // Exit function early
    }
    
    drawTwoCards(currentHand.card1, currentHand.card2);
}

void onCardsReceived(CardData cards) {
    currentHand = cards;
    hasCards = true;
    
    Serial.printf("Got cards: %d and %d\n", cards.card1, cards.card2);
    
    // Update display
    drawHoleCards();
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.printf("Player %d starting...\n", PLAYER_ID);
    
    // 1. Init display
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    
    gfx->begin();
    gfx->fillScreen(TABLE_GREEN);
    
    // Show initial waiting screen
    drawHoleCards();
    
    // Init ESP-NOW with our callback
    if (!initPlayerComms(PLAYER_ID, onCardsReceived)) {
        Serial.println("Communication setup failed!");
        return;
    }
    
    Serial.printf("MAC: %s\n", getMAC().c_str());
}

void loop() {
    // ESP-NOW handles everything via callbacks
    delay(10);
}