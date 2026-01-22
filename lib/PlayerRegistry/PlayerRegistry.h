#pragma once
#include <Arduino.h>

#define MAX_PLAYERS 8

// Information about each player
typedef struct {
    uint8_t playerID;           // 1-6
    bool isConnected;           // Are they currently connected?
    String playerName;          // Player's name (optional)
    unsigned long lastSeen;     // millis() when last heard from 
    uint32_t chipCount;   // Total stack size (using 32-bit)
    uint32_t currentBet;

} PlayerInfo;

class PlayerRegistry {
private:
    PlayerInfo players[MAX_PLAYERS];
    
public:
    PlayerRegistry();
    
    // Mark a player as connected
    // Parameters: playerID (1-6), MAC address, optional name
    void registerPlayer(uint8_t playerID, String name = "");
    
    // Mark a player as disconnected (haven't heard from them in a while)
    void disconnectPlayer(uint8_t playerID);
    
    // Check if a player is connected
    bool isPlayerConnected(uint8_t playerID);
    
    // Get player info
    PlayerInfo* getPlayer(uint8_t playerID);
    
    // Get count of connected players
    uint8_t getConnectedCount();
    
    // Update last seen timestamp
    void updateLastSeen(uint8_t playerID);
    
    // Check for timeouts (call this periodically)
    // Marks players as disconnected if not heard from in X seconds
    void checkTimeouts(unsigned long timeoutMs = 10000);
    
    // Print all connected players (for debugging)
    void printConnected();
    
    // Clear all registrations
    void clearAll();
};