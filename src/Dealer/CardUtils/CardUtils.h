#pragma once
#include <Arduino.h>

/**
@brief Shuffles an array of 52 ints representing cards.
*/
void shuffleDeck(int[52]);

uint32_t evaluatePlayerHand(int hole1, int hole2, const int communityBoard[5])

uint8_t getHandRank(uint32_t handValue);

