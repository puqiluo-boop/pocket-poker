// PokerComms.cpp
// Implementation of poker game wireless communication

#include "Comms.h"

// ============ INTERNAL STATE ==============
// These are "private" to this file - main.cpp doesn't need to see them

// Broadcast MAC address (send to everyone)
static uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Callback function pointer for player to receive cards
// WHY STATIC: Only this file needs to access it
static void (*playerCardCallback)(CardData) = nullptr;

// Which player this device is (only relevant for players)
static uint8_t thisDeviceID = -1;

// ============ INTERNAL CALLBACKS ==============

// Called when dealer finishes sending data
// WHY: Helps us debug if sends are failing
static void onSendComplete(const uint8_t *mac_addr, esp_now_send_status_t status) {
    // Only print errors to avoid spamming serial
    if (status != ESP_NOW_SEND_SUCCESS) {
        Serial.println("Send failed!");
    }
}

// Called when player receives data
// WHY: Filters messages and calls the user's callback
static void onReceiveData(const uint8_t *mac, const uint8_t *data, int len) {
    // Parse the incoming bytes as CardData
    CardData receivedData;
    memcpy(&receivedData, data, sizeof(receivedData));
    
    // Only process if this message is for our player
    if (receivedData.playerID == thisDeviceID) {
        // Call the user's callback function
        // WHY: Let main.cpp decide what to do with the cards
        if (playerCardCallback != nullptr) {
            playerCardCallback(receivedData);
        }
    }
}

// ============ PUBLIC FUNCTIONS ==============

bool initDealerComms() {
    // Step 1: Put WiFi in Station mode (required for ESP-NOW)
    WiFi.mode(WIFI_STA);
    
    // Step 2: Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init failed");
        return false;
    }
    
    // Step 3: Register callback for send confirmation
    esp_now_register_send_cb(onSendComplete);
    
    // Step 4: Add broadcast peer
    // WHY: We must register any address we want to send to
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;   // Auto-select channel
    peerInfo.encrypt = false; // No encryption (faster)
    
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add broadcast peer");
        return false;
    }
    
    Serial.println("Dealer comms ready");
    return true;
}

void sendCardsToPlayers(int* deckOrder) {
    // Send cards to 6 players
    // WHY LOOP: Each player needs different hole cards
    for (int player = 0; player < 1; player++) {
        CardData packet;
        
        // Player numbering: 1-6 (not 0-5)
        packet.playerID = player + 1;
        
        // Card distribution:
        // Cards 0-4: Community (shown on table)
        // Cards 5-6: Player 1
        // Cards 7-8: Player 2, etc.
        packet.card1 = deckOrder[0 + (player * 2)];
        packet.card2 = deckOrder[1 + (player * 2)];
        
        esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&packet, sizeof(packet));
        
        if (result != ESP_OK) {
            Serial.printf("Failed to send to Player %d\n", player + 1);
        }
        
        // Small delay to avoid overwhelming the radio
        delay(10);
    }
}

bool initPlayerComms(uint8_t playerID, void (*onCardsReceived)(CardData)) {
    // Save player ID and callback
    // WHY: We need these in onReceiveData callback
    thisDeviceID = playerID;
    playerCardCallback = onCardsReceived;
    
    // Step 1: Put WiFi in Station mode
    WiFi.mode(WIFI_STA);
    
    // Step 2: Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init failed");
        return false;
    }
    
    // Step 3: Register receive callback
    // WHY: This is how we get notified when data arrives
    esp_now_register_recv_cb(onReceiveData);
    
    Serial.printf("Player %d listening...\n", playerID);
    return true;
}

String getMAC() {
    // Return this device's MAC address as a readable string
    // WHY: Useful for debugging multi-device setups
    return WiFi.macAddress();
}