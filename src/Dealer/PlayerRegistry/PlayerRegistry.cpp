#include <optional>
#include "PlayerRegistry.h"

PlayerRegistry::PlayerRegistry() {
    players.reserve(MAX_PLAYERS);
}

bool PlayerRegistry::connectPlayer(String playerMAC, String playerName) {
    if (isPlayerConnected(playerMAC)) {
        Serial.printf("Failed to connect: duplicate MACs (%s)\n", playerMAC);
        return false;
    } else if (getConnectedCount() == MAX_PLAYERS) {
        Serial.printf("Failed to connect: maximum players reached (%s)\n", playerMAC);
        return false;
    }
    
    PlayerInfo newPlayer;
    
    newPlayer.playerMAC = playerMAC;
    newPlayer.playerName = playerName;
    newPlayer.lastSeen = millis();
    newPlayer.chipCount = 1000; //Everyone starts with 1000 chips
    
    players.push_back(newPlayer);
    Serial.printf("Player \"%s\" connected (%s)\n", playerName, playerMAC);
    return true;
}

bool PlayerRegistry::disconnectPlayer(String playerMAC) {
    if (players.size() == 0) {
        Serial.printf("Failed to disconnect: no players (%s)\n", playerMAC);
        return false;
    }
    for (size_t i = 0; i < players.size(); i++) {
        if (players[i].playerMAC == playerMAC) {
            String name = players[i].playerName;
            players[i] = players.back();
            players.pop_back();
            Serial.printf("Player \"%s\" disconnected (%s)\n", name, playerMAC);
            return true;
        }
    }
    Serial.printf("Failed to disconnect: not found (%s)\n", playerMAC);
    return false;
}

bool PlayerRegistry::isPlayerConnected(String playerMAC) const {
    if (players.size() == 0) {
        return false;
    }
    for (size_t i = 0; i < players.size(); i++) {
        if (players[i].playerMAC == playerMAC) return true;
    }
    return false;
}

std::optional<PlayerInfo> PlayerRegistry::getPlayer(String playerMAC) {
    for (size_t i = 0; i < players.size(); i++) {
        if (players[i].playerMAC == playerMAC) return players[1];
    }
    return std::nullopt;
}

size_t PlayerRegistry::getConnectedCount() const {
    return players.size();
}

bool PlayerRegistry::updateLastSeen(String playerMAC) {
    if (players.size() == 0) {
        Serial.printf("updateLastSeen failed: no players (%s)\n", playerMAC);
        return false;
    }
    for (size_t i = 0; i < players.size(); i++) {
        if (players[i].playerMAC == playerMAC) {
            players[i].lastSeen = millis();
            return true;
        }
    }
    Serial.printf("updateLastSeen failed: not found (%s)\n", playerMAC);
    return false;
}

uint8_t PlayerRegistry::checkTimeouts(unsigned long timeoutMs) {
    unsigned long now = millis();
    uint8_t count = 0;
    
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (now - players[i].lastSeen > timeoutMs) {
            String playerName = players[i].playerName;
            String playerMAC = players[i].playerMAC;

            disconnectPlayer(players[i].playerMAC);

            Serial.printf("Player \"%s\" timed out (%s)\n", playerName, playerMAC);
            count++;
        }
    }

    return count;
}

void PlayerRegistry::printConnected() const {
    Serial.println("=== Connected Players ===");
    for (int i = 0; i < players.size(); i++) {
        Serial.printf("Player \"%s\" (%s)\n",
            players[i].playerName,
            players[i].playerMAC
        );
    }
    Serial.println("=========================");
}

void PlayerRegistry::clearAll() {
    players.clear();
}