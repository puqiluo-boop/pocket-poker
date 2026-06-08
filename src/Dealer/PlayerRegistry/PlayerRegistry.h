#pragma once
#include <Arduino.h>
#include <optional>
#include <vector>

#define MAX_PLAYERS 8

// Information about each player
typedef struct {
    String playerMAC;          
    String playerName;          // Player's name (optional)
    unsigned long lastSeen;     // millis() when last heard from 
    uint32_t chipCount;   // Total stack size (using 32-bit)
} PlayerInfo;

class PlayerRegistry {
private:
    std::vector<PlayerInfo> players;
    
public:
    PlayerRegistry();
    
    // Mark a player as connected
    // Parameters: MAC address, optional name
    bool connectPlayer(String playerMAC, String name = "");
    
    // Mark a player as disconnected (haven't heard from them in a while)
    bool disconnectPlayer(String playerMAC);
    
    // Check if a player is connected
    bool isPlayerConnected(String playerMAC) const;
    
    // Get player info
    std::optional<PlayerInfo> getPlayer(String playerMAC);

    std::optional<uint32_t> getChipCount(String playerMAC);
    
    // Get count of connected players
    size_t getConnectedCount() const;
    
    // Update last seen timestamp
    bool updateLastSeen(String playerMAC);
    
    // Check for timeouts (call this periodically)
    // Marks players as disconnected if not heard from in X seconds
    uint8_t checkTimeouts(unsigned long timeoutMs = 5000);
    
    // Print all connected players (for debugging)
    void printConnected() const;
    
    // Clear all registrations
    void clearAll();
};