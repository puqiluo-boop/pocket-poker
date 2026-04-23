// Storage.h
// Handles persistent storage for game state and player data

#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include <LittleFS.h>

// ============ PLAYER BALANCES ==============

// Save a single player's chip balance
void savePlayerBalance(uint8_t playerID, uint32_t balance);

// Load a single player's chip balance (returns default if not found)
uint32_t loadPlayerBalance(uint8_t playerID, uint32_t defaultBalance = 1000);

// Save all 6 player balances at once
void saveAllPlayerBalances(uint32_t balances[6]);

// Load all 6 player balances
void loadAllPlayerBalances(uint32_t balances[6]);

// ============ GAME STATE ==============

// Save current game state (pot, dealer button, active game flag)
void saveGameState(uint32_t pot, uint8_t dealerButton, bool gameActive);

// Load game state
void loadGameState();

// Check if there's an active game in progress
bool isGameInProgress();

// Get saved pot amount
uint32_t getSavedPot();

// Get saved dealer button position
uint8_t getSavedDealerButton();

// ============ TRANSACTION LOG ==============

// Log an action to file (for dispute resolution)
void logAction(String action);

// Read last N lines from log
void printLastActions(int numLines = 10);

// Clear the log file (start fresh)
void clearLog();