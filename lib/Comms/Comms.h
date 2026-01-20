// PokerComms.h
// This file defines the communication protocol for the poker game
// Both dealer and players include this to ensure they speak the same language

#pragma once
#include <esp_now.h>
#include <WiFi.h>

// ============ DATA STRUCTURES ==============

// Message sent from Dealer → Players
// Contains all the cards for one hand
typedef struct {
    uint8_t playerID;           // Which player (1-6)
    uint8_t card1;              // First hole card (0-51 index)
    uint8_t card2;              // Second hole card (0-51 index)
} CardData;

// Future: Message sent from Player → Dealer
// (You'll add this later for player actions)
typedef struct {
    uint8_t playerID;
    uint8_t action;    // 0=Fold, 1=Call, 2=Raise, etc.
    uint16_t amount;   // Bet amount
} PlayerAction;

// ============ DEALER FUNCTIONS ==============

// Initialize ESP-NOW as a dealer (broadcaster)
// WHY SEPARATE: Dealer needs to add broadcast peer, player doesn't
bool initDealerComms();

// Send shuffled cards to all players
// PARAMETERS: deckOrder - array of 52 card indices (shuffled)
void sendCardsToPlayers(int* deckOrder);

// ============ PLAYER FUNCTIONS ==============

// Initialize ESP-NOW as a player (receiver)
// PARAMETERS: 
//   - playerID: Which player this device is (1-6)
//   - callback: Function to call when cards arrive
bool initPlayerComms(uint8_t playerID, void (*onCardsReceived)(CardData));

// Get this device's MAC address as a string
// WHY USEFUL: For debugging - know which device is which
String getMAC();