#pragma once
#include <esp_now.h>
#include <WiFi.h>

enum MessageType : uint8_t {
    MSG_CONNECTION_CHECK = 0,
    MSG_CARD_DATA = 1,
    MSG_PLAYER_ACTION = 2,
};
typedef struct {
    MessageType msgType;
    uint8_t senderID;
} ConnectionCheck;
typedef struct {
    MessageType msgType;
    uint8_t playerID;           // Which player (1-6)
    uint8_t card1;              // First hole card (0-51 index)
    uint8_t card2;              // Second hole card (0-51 index)
} CardData;

typedef struct {
    MessageType msgType;
    uint8_t playerID;
    uint32_t betSize;
} PlayerAction;