#include "CardUtils.h"
#include <Arduino.h>

void shuffleDeck(int deck[52]) {
    // Initialize deck in order (0-51)
    for (int i = 0; i < 52; i++) {
        deck[i] = i;
    }
    
    // Fisher-Yates shuffle algorithm
    for (int i = 51; i > 0; i--) {
        int j = random(0, i + 1);  // Random index from 0 to i
        
        // Swap cards at positions i and j
        int temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;
    }
}