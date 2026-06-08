#pragma once
#include <esp_now.h>
#include <WiFi.h>

// ============ FUNCTIONS ==============

bool initComms(uint8_t deviceID, void (*onMessage)(BaseMessage*));

String getMAC();

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
        Serial.print(" | Err Type: ");
        Serial.println(esp_err_to_name(result));
        Serial.print(" | Msg Type: ");
        Serial.print(basePtr->msgType);
        Serial.print(" | Sender ID: ");
        Serial.println(basePtr->senderID);
        Serial.print(" | Receiver ID: ");
        Serial.println(basePtr->recieverID);
        
        return false;
    }
    
    return true;
}

// ============ CONSTANTS, ENUMS, and STRUCTS ==============

const String BROADCAST = "BROADCAST";
const String DEALER = "DEALER";

enum MessageType {
    MSG_CONNECTION,
    MSG_HOLE_CARDS,
    MSG_ACTION,
    MSG_GAME_UPDATE
    //...
};

struct BaseMessage {
    MessageType msgType;
    String sender;
    String reciever;
};

struct ConnectionMsg : public BaseMessage {
};

struct HoleCardsMsg : public BaseMessage {
    uint8_t card1;              // First hole card (0-51 index)
    uint8_t card2;              // Second hole card (0-51 index)
};

enum PlayerActionType {
    CHECK,
    CALL,
    FOLD,
    BET_RAISE
};
struct PlayerActionMsg : public BaseMessage {
    PlayerActionType playerActionType;
    uint32_t betRaiseSize;
};