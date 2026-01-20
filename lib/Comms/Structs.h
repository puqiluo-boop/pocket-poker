#pragma once
#include <esp_now.h>
#include <WiFi.h>

typedef struct {
    uint8_t playerID;           // Which player (1-6)
    uint8_t card1;              // First hole card (0-51 index)
    uint8_t card2;              // Second hole card (0-51 index)
} CardData;