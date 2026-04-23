#include "CardDisplay.h"
#include <Arduino_GFX_Library.h>
#include <Deck.h>
#include <Trim.h>
#include <Colors.h>

#define CARD_Y 10
#define CARD_X1 16
#define CARD_X2 90

static int holeCards[2] = {};

extern Arduino_GFX *gfx;  // Defined in UIManager.cpp

// Helper functions (internal only)
static bool shouldSkipPixel(int row, int col) {
    for (int i = 0; i < 24; i++) {
        if (trim[i][0] == row && trim[i][1] == col) {
            return true;
        }
    }
    return false;
}

/**
 * @param cardIndex Use -1 for clearing the card (uses TABLE_GREEN)
 */
static void drawScaledCard(int cardIndex, int x, int y, int scale) {
    if(cardIndex != -1) {
        for (int row = 0; row < CARD_HEIGHT; row++) {
            for (int col = 0; col < CARD_WIDTH; col++) {
                if (shouldSkipPixel(row, col)) continue;
                
                uint16_t color = deck[cardIndex][row * CARD_WIDTH + col];
                gfx->fillRect(x + (col * scale), y + (row * scale), scale, scale, color);
            }
        }
    } else {
        for (int row = 0; row < CARD_HEIGHT; row++) {
            for (int col = 0; col < CARD_WIDTH; col++) {
                if (shouldSkipPixel(row, col)) continue;
                
                gfx->fillRect(x + (col * scale), y + (row * scale), scale, scale, TABLE_GREEN);
            }
        }
    }
}

// Public functions
void setHoleCards(int card1, int card2) {
    holeCards[0] = card1;
    holeCards[1] = card2;
}

void displayHoleCards() {
    drawScaledCard(holeCards[0], CARD_X1, CARD_Y, 1);
    drawScaledCard(holeCards[1], CARD_X2, CARD_Y, 1);
}

void clearCards() {
    drawScaledCard(-1, CARD_X1, CARD_Y, 1);
    drawScaledCard(-1, CARD_X2, CARD_Y, 1);
}