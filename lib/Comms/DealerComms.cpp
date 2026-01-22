// DealerComms.cpp
// Implementation of dealer wireless communication

#include "DealerComms.h"

// ============ INTERNAL STATE ==============

// Broadcast MAC address (FF:FF:FF:FF:FF:FF = send to everyone)
static uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static void (*connectionCallback)(ConnectionCheck) = nullptr;

// ============ CALLBACKS ==============

static void onDataReceived(const uint8_t *mac, const uint8_t *data, int len) {
    uint8_t msgType = data[0];

    switch (msgType) {
        case MSG_CONNECTION_CHECK: {
            ConnectionCheck receivedData;
            memcpy(&receivedData, data, sizeof(receivedData));

            if (connectionCallback != nullptr) {
                connectionCallback(receivedData);
            }
            return;
        }
        case MSG_PLAYER_ACTION: {
            
            return;
        }
        default: {
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

bool initDealerComms(void (*onSuccessfulConnection)(ConnectionCheck)) {
    // Step 1: Enable WiFi in Station mode (required for ESP-NOW)
    WiFi.mode(WIFI_STA);
    connectionCallback = onSuccessfulConnection;
    
    // Step 2: Initialize ESP-NOW protocol
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init failed");
        return false;
    }
    
    // Step 3: Register callback for send status
    esp_now_register_recv_cb(onDataReceived);
    esp_now_register_send_cb(onSendComplete);
    
    // Step 4: Add broadcast address as a peer
    // WHY: We must register any address we want to send to
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;       // Auto-select channel
    peerInfo.encrypt = false;   // No encryption (faster)
    
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add broadcast peer");
        return false;
    }
    
    Serial.println("Dealer comms initialized");
    return true;
}

void broadcastCards(int* deckOrder) {
    // Send cards to 6 players
    for (int player = 0; player < 6; player++) {
        CardData packet;
        
        packet.msgType = MSG_CARD_DATA;

        // Player numbering: 1-6 (not 0-5)
        packet.playerID = player + 1;
        
        packet.card1 = deckOrder[0 + (player * 2)];
        packet.card2 = deckOrder[1 + (player * 2)];
        
        // Broadcast this packet
        esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&packet, sizeof(packet));
        
        if (result != ESP_OK) {
            Serial.printf("Failed to send to Player %d\n", player + 1);
        }
        
        // Small delay to avoid overwhelming the radio
        delay(10);
    }
}

void broadcastConnectionCheck() {
    ConnectionCheck packet;
    packet.msgType = MSG_CONNECTION_CHECK;
    packet.senderID = 0; // Dealer ID is 0

    // Broadcast this packet
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&packet, sizeof(packet));

    if (result != ESP_OK) {
        Serial.println("Failed to send connection check");
    }
}