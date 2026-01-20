// PlayerComms.h
// Handles wireless communication FROM dealer TO this player

#pragma once
#include <esp_now.h>
#include <WiFi.h>
#include "Structs.h"

// ============ PLAYER FUNCTIONS ==============

// Initialize ESP-NOW for receiving
// Parameters:
//   - playerID: Which player this device is (1-6)
//   - onCardsReceived: Callback function to call when cards arrive
// Returns: true if successful, false if failed
bool initPlayerComms(uint8_t playerID, void (*onCardsReceived)(CardData));

// Get this device's MAC address
// Useful for debugging which device is which
String getPlayerMAC();