#include "PlayerRegistry.h"

PlayerRegistry::PlayerRegistry() {
    clearAll();
}

void PlayerRegistry::registerPlayer(uint8_t playerID, String name) {
    if (playerID < 1 || playerID > MAX_PLAYERS) return;
    
    int index = playerID - 1;  // Convert to 0-based index
    
    players[index].playerID = playerID;
    players[index].isConnected = true;
    players[index].playerName = name;
    players[index].lastSeen = millis();
    players[index].chipCount = 0;
    players[index].currentBet = 0;
    
    Serial.printf("Player %d registered: %s\n", playerID, name.c_str());
}

void PlayerRegistry::disconnectPlayer(uint8_t playerID) {
    if (playerID < 1 || playerID > MAX_PLAYERS) return;
    
    int index = playerID - 1;
    players[index].isConnected = false;
    Serial.printf("Player %d disconnected\n", playerID);
}

bool PlayerRegistry::isPlayerConnected(uint8_t playerID) {
    if (playerID < 1 || playerID > MAX_PLAYERS) return false;
    return players[playerID - 1].isConnected;
}

PlayerInfo* PlayerRegistry::getPlayer(uint8_t playerID) {
    if (playerID < 1 || playerID > MAX_PLAYERS) return nullptr;
    return &players[playerID - 1];
}

uint8_t PlayerRegistry::getConnectedCount() {
    uint8_t count = 0;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (players[i].isConnected) count++;
    }
    return count;
}

void PlayerRegistry::updateLastSeen(uint8_t playerID) {
    if (playerID < 1 || playerID > MAX_PLAYERS) return;
    players[playerID - 1].lastSeen = millis();
}

void PlayerRegistry::checkTimeouts(unsigned long timeoutMs) {
    unsigned long now = millis();
    
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (players[i].isConnected) {
            // Check if we haven't heard from them in a while
            if (now - players[i].lastSeen > timeoutMs) {
                disconnectPlayer(players[i].playerID);
            }
        }
    }
}

void PlayerRegistry::printConnected() {
    Serial.println("=== Connected Players ===");
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (players[i].isConnected) {
            Serial.printf("Player %d: %s\n",
                players[i].playerID,
                players[i].playerName.c_str()
            );
        }
    }
    Serial.printf("Total: %d/%d players\n", getConnectedCount(), MAX_PLAYERS);
}

void PlayerRegistry::clearAll() {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        players[i].playerID = i + 1;
        players[i].isConnected = false;
        players[i].playerName = "";
        players[i].lastSeen = 0;
        players[i].chipCount = 0;
        players[i].currentBet = 0;
    }
}