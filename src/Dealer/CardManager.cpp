#include "CardManager.h"
#include <Arduino.h>

int* shuffleDeck() {
    // Static so it persists after function returns
    static int deckOrder[52];
    
    // Initialize deck in order (0-51)
    for (int i = 0; i < 52; i++) {
        deckOrder[i] = i;
    }
    
    // Fisher-Yates shuffle algorithm
    for (int i = 51; i > 0; i--) {
        int j = random(0, i + 1);  // Random index from 0 to i
        
        // Swap cards at positions i and j
        int temp = deckOrder[i];
        deckOrder[i] = deckOrder[j];
        deckOrder[j] = temp;
    }
    
    Serial.println("Deck shuffled");
    return deckOrder;
}