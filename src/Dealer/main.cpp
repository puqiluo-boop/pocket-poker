#include <Arduino.h>
#include "DisplayManager.h"
#include "UIController.h"
#include "CardManager.h"
#include "Storage.h"
#include <DealerComms.h>
#include <PlayerRegistry.h>

PlayerRegistry playerRegistry;

// Callback when a player connects
void handlePlayerConnection(ConnectionCheck cc) {
    playerRegistry.registerPlayer(cc.senderID, String("Player ") + cc.senderID);
    playerRegistry.updateLastSeen(cc.senderID);
    Serial.printf("Player %d connected\n", cc.senderID);
}

// Callback when "DEAL HAND" button is pressed
void onDealHandPressed() {
    Serial.println("Dealing new hand...");
    
    // Shuffle the deck
    int* newHand = shuffleDeck();
    
    // Send cards to players
    int playerCount = playerRegistry.getConnectedCount();
    broadcastCards(newHand, playerCount);
    
    // Display community cards on table screen
    int offset = 2 * playerCount;
    drawCommunityCards(
        newHand[offset + 1],  // Flop 1
        newHand[offset + 2],  // Flop 2
        newHand[offset + 3],  // Flop 3
        newHand[offset + 5],  // Turn
        newHand[offset + 7]   // River
    );
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("=== DEALER STARTING ===");
    
    // 1. Initialize displays
    initDealerDisplay();
    initTableDisplay();
    
    // 2. Initialize touch
    initTouch();
    
    // 3. Initialize ESP-NOW communication
    if (!initDealerComms(handlePlayerConnection)) {
        Serial.println("ERROR: Communication failed!");
        while(1);
    }
    
    // 4. Initialize LVGL and create UI
    initLVGL();
    createDealerUI(onDealHandPressed);
    
    // 5. Load saved game state
    loadGameState();
    
    Serial.println("=== DEALER READY ===");
    
    // 6. Discover players on network
    playerRegistry.clearAll();
    delay(500);
    broadcastConnectionCheck();
    delay(500);
    playerRegistry.printConnected();
}

void loop() {
    updateLVGL();  // Handle UI updates
    delay(5);
}