// DealerComms.h
// Handles wireless communication FROM dealer TO players

#pragma once
#include <esp_now.h>
#include <WiFi.h>

// ============ DATA STRUCTURES ==============

// Card data sent from Dealer → Players
typedef struct {
    uint8_t playerID;           // Which player (1-6)
    uint8_t card1;              // First hole card (0-51 index)
    uint8_t card2;              // Second hole card (0-51 index)
} CardData;

// ============ DEALER FUNCTIONS ==============

// Initialize ESP-NOW for broadcasting
// Returns: true if successful, false if failed
bool initDealerComms();

// Broadcast shuffled cards to all players (1-6)
// Parameters: deckOrder - array of 52 shuffled card indices
void broadcastCards(int* deckOrder);