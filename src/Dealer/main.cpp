#include <Arduino.h>
#include "Display/Display.h"
#include "UI/UI.h"
#include "CardUtils/CardUtils.h"
#include "Storage/Storage.h"
#include <ESPNOW_Shared.h>
#include "PlayerRegistry/PlayerRegistry.h"
#include "ESPNOW/ESPNOW.h"

PlayerRegistry playerRegistry;

void handleIncomingNetworkData(BaseMessage* msg) {
    
    // 1. The Front Door Filter
    // If it's not meant for the Dealer (ID 0) and it's not a broadcast, ignore it
    if (msg->reciever != DEALER && msg->reciever != BROADCAST) {
        return; 
    }

    // 2. The Router
    switch (msg->msgType) {
        
        case MSG_CONNECTION: {
            ConnectionMsg* connMsg = (ConnectionMsg*)msg;
            if(!playerRegistry.isPlayerConnected(connMsg->sender)) {
                playerRegistry.connectPlayer(connMsg->sender, String("Player ") + connMsg->sender);
            }
            playerRegistry.updateLastSeen(connMsg->sender);
            Serial.printf("Player %s confirmed connection!\n", connMsg->sender.c_str());
            break;
        }

        case MSG_ACTION: {
            // Because you have the Front Door filter above, you don't even 
            // need to check if the action was sent to the dealer!
            
            // PlayerAction* actionMsg = (PlayerAction*)msg;
            // processPlayerBet(actionMsg);
            break;
        }
        
        default: {
            Serial.printf("Dealer received unknown message type: %d\n", msg->msgType);
            break;
        }
    }
}

// Callback when "DEAL HAND" button is pressed
void onDealHandPressed() {
    Serial.println("Dealing new hand...");
    
    // Shuffle the deck
    shuffleDeck(deck);
    
    // Send cards to players
    broadcastCards(deck, playerRegistry);
    
    // Display community cards on table screen
    int offset = 2 * MAX_PLAYERS;
    drawCommunityCards(
        deck[offset],  // Flop 1
        deck[offset + 1],  // Flop 2
        deck[offset + 2],  // Flop 3
        deck[offset + 3],  // Turn
        deck[offset + 4]   // River
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
    if (!initComms(0, handleIncomingNetworkData)) {
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