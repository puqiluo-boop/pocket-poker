#include "Display.h"
#include <BoardConfig.h>
#include <Deck.h>
#include <Colors.h>

// ============ DEALER SCREEN (ST7789) ==============
static Arduino_DataBus *dealerBus = nullptr;
static Arduino_GFX *dealerGfx = nullptr;

void initDealerDisplay() {
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    
    dealerBus = new Arduino_ESP32SPI(
        TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, TFT_MISO
    );
    
    dealerGfx = new Arduino_ST7789(
        dealerBus, TFT_RST, 
        3 /* rotation */, true /* IPS */, 
        240, 320, 
        0, 0, 0, 0
    );
    
    dealerGfx->begin();
    dealerGfx->fillScreen(BLACK);
    
    Serial.println("Dealer display initialized");
}

Arduino_GFX* getDealerGfx() {
    return dealerGfx;
}

// ============ TABLE SCREEN (ILI9488) ==============
#define EXT_SCK  13
#define EXT_MOSI 11
#define EXT_MISO 15
#define EXT_CS   9
#define EXT_DC   12
#define EXT_RST  14

static Arduino_DataBus *tableBus = nullptr;
static Arduino_GFX *tableGfx = nullptr;

void initTableDisplay() {
    tableBus = new Arduino_ESP32SPI(
        EXT_DC, EXT_CS, EXT_SCK, EXT_MOSI, EXT_MISO
    );
    
    tableGfx = new Arduino_ILI9488_18bit(
        tableBus, EXT_RST, 
        1 /* rotation */, true /* IPS */
    );
    
    tableGfx->begin();
    tableGfx->fillScreen(TABLE_GREEN);
    
    Serial.println("Table display initialized");
}

Arduino_GFX* getTableGfx() {
    return tableGfx;
}

void drawCommunityCards(int flop1, int flop2, int flop3, int turn, int river) {
    if (!tableGfx) {
        Serial.println("ERROR: Table display not initialized");
        return;
    }
    
    // Clear screen
    tableGfx->fillScreen(TABLE_GREEN);
    delay(10);
    
    // ILI9488 in rotation 1: 480 width x 320 height
    int yPos = (320 - CARD_HEIGHT) / 2;
    
    // Calculate card positions with gaps
    int totalCardWidth = 5 * CARD_WIDTH;
    int availableSpace = 480 - 80;  // 40px margin on each side
    int totalGapWidth = availableSpace - totalCardWidth;
    int gap = totalGapWidth / 4;  // Space between 5 cards = 4 gaps
    
    int xPositions[5];
    xPositions[0] = 40;  // Left margin
    for (int i = 1; i < 5; i++) {
        xPositions[i] = xPositions[i-1] + CARD_WIDTH + gap;
    }
    
    // Card indices
    const uint16_t* cards[5] = {
        deck[flop1], 
        deck[flop2], 
        deck[flop3], 
        deck[turn], 
        deck[river]
    };
    
    // Draw each card with small delay for visual effect
    for (int i = 0; i < 5; i++) {
        tableGfx->draw16bitRGBBitmap(
            xPositions[i], 
            yPos, 
            (uint16_t*)cards[i], 
            CARD_WIDTH, 
            CARD_HEIGHT
        );
        delay(5);
    }
    
    Serial.println("Community cards drawn on table");
}