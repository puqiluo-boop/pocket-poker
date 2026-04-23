// Storage.cpp
#include "Storage.h"

static Preferences prefs;

// Internal cached values (so we don't read flash constantly)
static uint32_t cachedPot = 0;
static uint8_t cachedDealerButton = 0;
static bool cachedGameActive = false;

// ============ PLAYER BALANCES ==============

void savePlayerBalance(uint8_t playerID, uint32_t balance) {
    if (playerID > 5) {
        Serial.println("ERROR: Invalid player ID");
        return;
    }
    
    prefs.begin("poker", false);  // Read-write mode
    
    String key = "chips_" + String(playerID);
    prefs.putUInt(key.c_str(), balance);
    
    prefs.end();
    
    Serial.printf("Saved: Player %d = %d chips\n", playerID, balance);
}

uint32_t loadPlayerBalance(uint8_t playerID, uint32_t defaultBalance) {
    if (playerID > 5) {
        Serial.println("ERROR: Invalid player ID");
        return defaultBalance;
    }
    
    prefs.begin("poker", true);  // Read-only mode
    
    String key = "chips_" + String(playerID);
    uint32_t balance = prefs.getUInt(key.c_str(), defaultBalance);
    
    prefs.end();
    
    return balance;
}

void saveAllPlayerBalances(uint32_t balances[6]) {
    prefs.begin("poker", false);
    
    for (int i = 0; i < 6; i++) {
        String key = "chips_" + String(i);
        prefs.putUInt(key.c_str(), balances[i]);
    }
    
    prefs.end();
    
    Serial.println("All player balances saved");
}

void loadAllPlayerBalances(uint32_t balances[6]) {
    prefs.begin("poker", true);
    
    for (int i = 0; i < 6; i++) {
        String key = "chips_" + String(i);
        balances[i] = prefs.getUInt(key.c_str(), 1000);  // Default 1000 chips
    }
    
    prefs.end();
    
    Serial.println("All player balances loaded");
}

// ============ GAME STATE ==============

void saveGameState(uint32_t pot, uint8_t dealerButton, bool gameActive) {
    prefs.begin("poker", false);
    
    prefs.putUInt("pot", pot);
    prefs.putUChar("dealer_btn", dealerButton);
    prefs.putBool("active", gameActive);
    prefs.putULong("timestamp", millis());  // When was this saved?
    
    prefs.end();
    
    // Update cache
    cachedPot = pot;
    cachedDealerButton = dealerButton;
    cachedGameActive = gameActive;
    
    Serial.printf("Game state saved: Pot=%d, Button=%d, Active=%s\n", 
                  pot, dealerButton, gameActive ? "YES" : "NO");
}

void loadGameState() {
    prefs.begin("poker", true);
    
    cachedGameActive = prefs.getBool("active", false);
    cachedPot = prefs.getUInt("pot", 0);
    cachedDealerButton = prefs.getUChar("dealer_btn", 0);
    
    if (cachedGameActive) {
        unsigned long savedTime = prefs.getULong("timestamp", 0);
        Serial.println("=== FOUND SAVED GAME ===");
        Serial.printf("Pot: %d chips\n", cachedPot);
        Serial.printf("Dealer button: %d\n", cachedDealerButton);
        Serial.printf("Saved %lu ms ago\n", millis() - savedTime);
        Serial.println("========================");
    } else {
        Serial.println("No active game found");
    }
    
    prefs.end();
}

bool isGameInProgress() {
    return cachedGameActive;
}

uint32_t getSavedPot() {
    return cachedPot;
}

uint8_t getSavedDealerButton() {
    return cachedDealerButton;
}

// ============ TRANSACTION LOG ==============

void logAction(String action) {
    if (!LittleFS.begin()) {
        Serial.println("ERROR: LittleFS mount failed");
        return;
    }
    
    File log = LittleFS.open("/game_log.txt", "a");  // Append mode
    if (log) {
        // Format: [timestamp] action
        log.printf("[%lu] %s\n", millis(), action.c_str());
        log.close();
    } else {
        Serial.println("ERROR: Could not open log file");
    }
    
    LittleFS.end();
}

void printLastActions(int numLines) {
    if (!LittleFS.begin()) {
        Serial.println("ERROR: LittleFS mount failed");
        return;
    }
    
    File log = LittleFS.open("/game_log.txt", "r");
    if (!log) {
        Serial.println("No log file found");
        LittleFS.end();
        return;
    }
    
    // Simple approach: read all lines, print last N
    String lines[100];  // Store up to 100 lines
    int count = 0;
    
    while (log.available() && count < 100) {
        lines[count] = log.readStringUntil('\n');
        count++;
    }
    
    log.close();
    LittleFS.end();
    
    // Print last numLines
    int start = (count > numLines) ? (count - numLines) : 0;
    
    Serial.printf("=== LAST %d ACTIONS ===\n", count - start);
    for (int i = start; i < count; i++) {
        Serial.println(lines[i]);
    }
    Serial.println("=======================");
}

void clearLog() {
    if (!LittleFS.begin()) {
        Serial.println("ERROR: LittleFS mount failed");
        return;
    }
    
    LittleFS.remove("/game_log.txt");
    LittleFS.end();
    
    Serial.println("Transaction log cleared");
}