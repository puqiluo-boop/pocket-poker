// PlayerComms.cpp
// Implementation of player wireless communication

#include "PlayerComms.h"

// ============ INTERNAL STATE ==============

// Broadcast MAC address (FF:FF:FF:FF:FF:FF = send to everyone)
static uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Which player this device is
static uint8_t thisPlayerID = 0;

// Callback function to call when cards arrive
static void (*cardReceivedCallback)(CardData) = nullptr;

static void confirmConnection();

// ============ CALLBACKS ==============

static void onDataReceived(const uint8_t *mac, const uint8_t *data, int len) {
    Serial.println(">>> ESP-NOW DATA RECEIVED <<<");  // NEW
    Serial.printf("Length: %d bytes\n", len);          // NEW
    
    uint8_t msgType = data[0];
    Serial.printf("Message Type: %d\n", msgType);      // NEW
    
    switch (msgType) {
        case MSG_CONNECTION_CHECK: {
            Serial.println("-> Connection check message");  // NEW
            ConnectionCheck receivedData;
            memcpy(&receivedData, data, sizeof(receivedData));
            if (receivedData.senderID == 0) {
                confirmConnection();
            }
            return;
        }
        case MSG_CARD_DATA: {
            Serial.println("-> Card data message");         // NEW
            CardData receivedData;
            memcpy(&receivedData, data, sizeof(receivedData));
            
            Serial.printf("Target player: %d, This player: %d\n",   // NEW
                         receivedData.playerID, thisPlayerID);
        
            if (receivedData.playerID == thisPlayerID) {
                Serial.println("-> Match! Calling callback");        // NEW
                if (cardReceivedCallback != nullptr) {
                    cardReceivedCallback(receivedData);
                } else {
                    Serial.println("-> ERROR: Callback is NULL!");   // NEW
                }
            } else {
                Serial.println("-> Player ID mismatch, ignoring");   // NEW
            }
            return;
        }
        default: {
            Serial.printf("-> Unknown message type: %d\n", msgType);  // NEW
            return;
        }
    }
}

static void onSendComplete(const uint8_t *mac_addr, esp_now_send_status_t status) {
    if (status != ESP_NOW_SEND_SUCCESS) {
        Serial.println("Send failed!");
    }
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

void confirmConnection() {
    ConnectionCheck packet;
    packet.msgType = MSG_CONNECTION_CHECK;
    packet.senderID = thisPlayerID;

    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&packet, sizeof(packet));
    if (result != ESP_OK) {
        Serial.println("Failed to send connection confirmation");
    } else {
        Serial.println("Connection confirmation sent");
    }
}