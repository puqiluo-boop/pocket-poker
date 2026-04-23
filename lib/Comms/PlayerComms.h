// PlayerComms.h
// Handles wireless communication FROM dealer TO this player

#pragma once
#include <esp_now.h>
#include <WiFi.h>
#include "Structs.h"

// ============ PLAYER FUNCTIONS ==============

bool initPlayerComms(uint8_t playerID, void (*onMessage)(BaseMessage*));

String getPlayerMAC();

template <typename T>
bool sendMessage(const T& msgPacket) {
    
    // Cast to raw bytes for transmission
    const uint8_t* rawData = (const uint8_t*)&msgPacket;
    size_t dataSize = sizeof(T);
    
    // Send it
    esp_err_t result = esp_now_send(broadcastAddress, rawData, dataSize);
    
    // If it fails, read the base data!
    if (result != ESP_OK) {
        // 1. Cast the generic struct to a BaseMessage pointer
        const BaseMessage* basePtr = (const BaseMessage*)&msgPacket;
        
        // 2. Print the universally shared variables
        Serial.println("⚠️ Failed to send message! ⚠️");
        Serial.print(" | Type: ");
        Serial.print(basePtr->msgType);
        Serial.print(" | Sender ID: ");
        Serial.println(basePtr->senderID);
        Serial.print(" | Receiver ID: ");
        Serial.println(basePtr->recieverID);
        
        return false;
    }
    
    return true;
}