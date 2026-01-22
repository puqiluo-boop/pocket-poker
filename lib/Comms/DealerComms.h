// DealerComms.h
// Handles wireless communication FROM dealer TO players

#pragma once
#include <esp_now.h>
#include <WiFi.h>
#include "Structs.h"

// ============ DEALER FUNCTIONS ==============

// Initialize ESP-NOW for broadcasting
// Returns: true if successful, false if failed
bool initDealerComms(void (*onSuccessfulConnection)(ConnectionCheck));

// Broadcast shuffled cards to all players (1-6)
// Parameters: deckOrder - array of 52 shuffled card indices
void broadcastCards(int* deckOrder);

void broadcastConnectionCheck();