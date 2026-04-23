#include "PlayerComms.h"

// ============ INTERNAL STATE ==============
static uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static uint8_t thisPlayerID;

// NEW: A universal callback that accepts any message
static void (*appMessageCallback)(BaseMessage*) = nullptr; 

// ============ CALLBACKS ==============

static void onDataReceived(const uint8_t *mac, const uint8_t *data, int len) {
    // If we have a callback registered, hand the data to the main code
    if (appMessageCallback != nullptr) {
        
        // Cast the raw bytes to our universal base pointer
        BaseMessage* incomingMsg = (BaseMessage*)data;
        
        // Pass it to main.cpp!
        appMessageCallback(incomingMsg);
    }
}

static void onSendComplete(const uint8_t *mac_addr, esp_now_send_status_t status) {
    if (status != ESP_NOW_SEND_SUCCESS) {
        Serial.println("Send failed!");
    }
}

// ============ PUBLIC FUNCTIONS ==============

bool initPlayerComms(uint8_t playerID, void (*onMessage)(BaseMessage*)) {
    
    thisPlayerID = playerID;
    
    // Save the universal callback
    appMessageCallback = onMessage; 
    
    // Step 1: Enable WiFi in Station mode
    WiFi.mode(WIFI_STA);
    
    // Step 2: Initialize ESP-NOW protocol
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init failed");
        return false;
    }
    
    // Step 3: Register callback for when data arrives
    esp_now_register_recv_cb(onDataReceived);
    esp_now_register_send_cb(onSendComplete);

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;       // Auto-select channel
    peerInfo.encrypt = false;   // No encryption (faster)
    
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add broadcast peer");
        return false;
    }

    Serial.printf("Player %d listening for cards...\n", playerID);
    return true;
}

String getPlayerMAC() {
    return WiFi.macAddress();
}