// PlayerComms.cpp
// Implementation of player wireless communication

#include "PlayerComms.h"

// ============ INTERNAL STATE ==============

// Which player this device is
static uint8_t thisPlayerID = 0;

// Callback function to call when cards arrive
static void (*cardReceivedCallback)(CardData) = nullptr;

// ============ CALLBACKS ==============

// Called when data arrives via ESP-NOW
static void onDataReceived(const uint8_t *mac, const uint8_t *data, int len) {
    // Parse incoming bytes as CardData
    CardData receivedData;
    memcpy(&receivedData, data, sizeof(receivedData));
    
    // Only process if this message is for our player ID
    if (receivedData.playerID == thisPlayerID) {
        // Call the user's callback function
        if (cardReceivedCallback != nullptr) {
            cardReceivedCallback(receivedData);
        }
    }
    // Otherwise ignore (message is for a different player)
}

// ============ PUBLIC FUNCTIONS ==============

bool initPlayerComms(uint8_t playerID, void (*onCardsReceived)(CardData)) {
    // Save player ID and callback for later use
    thisPlayerID = playerID;
    cardReceivedCallback = onCardsReceived;
    
    // Step 1: Enable WiFi in Station mode
    WiFi.mode(WIFI_STA);
    
    // Step 2: Initialize ESP-NOW protocol
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init failed");
        return false;
    }
    
    // Step 3: Register callback for when data arrives
    esp_now_register_recv_cb(onDataReceived);
    
    Serial.printf("Player %d listening for cards...\n", playerID);
    return true;
}

String getPlayerMAC() {
    return WiFi.macAddress();
}