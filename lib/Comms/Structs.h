#pragma once
#include <esp_now.h>
#include <WiFi.h>

const uint8_t BROADCAST_ID = 255;

enum MessageType : uint8_t {
    MSG_CONNECTION = 0,
    MSG_HOLE_CARDS = 1,
    MSG_ACTION = 2
    //...
};

struct BaseMessage {
    MessageType msgType;
    uint8_t senderID;
    uint8_t recieverID;
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