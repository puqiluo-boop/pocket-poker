#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "../lib/Deck/Deck.h"
#include "../lib/Deck/Trim.h"
#include "../lib/Colors.h"

// --- FINAL CONFIRMED PINOUT ---
#define TFT_BL    1   
#define TFT_MOSI 38   
#define TFT_SCK  39   
#define TFT_MISO 40   // (Not strictly needed for writing, but good to have)
#define TFT_CS    45  // Found in Demo #2
#define TFT_DC    42  // Found in Demo #2
#define TFT_RST   -1  // Found in Demo #2 (Automatic Reset)

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

void setup() {
    // Keep Pin 1 safe for Backlight
    // (You can try Serial.begin(115200) later, but let's get a clean image first)
    delay(500);

    // Turn on Backlight
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    // Init Display
    if (!gfx->begin()) {
        // Init failed
    }

    // Fill Screen with Table Green
    gfx->fillScreen(TABLE_GREEN);
    
   drawTwoCards(0, 13); // Example: Ace of Spades and Ace of Hearts
}

void loop() {
    // Enjoy your victory
}