#pragma once
#include <Arduino_GFX_Library.h>

// Initialize the dealer's small screen (ST7789)
void initDealerDisplay();

// Initialize the large table screen (ILI9488)
void initTableDisplay();

// Draw 5 community cards on the table screen
void drawCommunityCards(int flop1, int flop2, int flop3, int turn, int river);

// Get pointer to dealer screen (for LVGL)
Arduino_GFX* getDealerGfx();

// Get pointer to table screen (for direct drawing)
Arduino_GFX* getTableGfx();