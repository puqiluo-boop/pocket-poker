#include <Arduino.h>
#include <Arduino_GFX_Library.h>

#include <esp_now.h>
#include <WiFi.h>

#include <BoardConfig.h>
#include <Colors.h>
#include <Deck.h>
#include <Trim.h>

//=========== PLAYER ID ==============
#define PLAYER_ID 1
//===================================

typedef struct {
    uint8_t playerID;
    uint8_t card1;
    uint8_t card2;
} CardData;

// ============ GAME STATE ==============
// Store the current hand data received from dealer
CardData currentHand;

// Flag to track if we've received cards yet
// WHY: So we can show "Waiting..." before cards arrive
bool hasCards = false;

Arduino_DataBus *bus = new Arduino_ESP32SPI(
    TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, TFT_MISO
);

Arduino_GFX *gfx = new Arduino_ST7789(
    bus, TFT_RST, 
    3 /* rotation */, true /* IPS */, 
    240, 320, 
    0, 0, // col offset 1, row offset 1 (WAS 0, 20)
    0, 0  // col offset 2, row offset 2
);

// Helper function to determine if a pixel should be skipped based on trim array
bool shouldSkipPixel(int row, int col, int scale) {
    for (int i = 0; i < 24; i++) {
        if (trim[i][0] == row && trim[i][1] == col) {
            return true;
        }
    }
    return false;
}

// Helper function to draw a card at position (x, y)
void drawScaledCard(int cardIndex, int x, int y, int scale) {
    for (int row = 0; row < CARD_HEIGHT; row++) {
        for (int col = 0; col < CARD_WIDTH; col++) {
            if (shouldSkipPixel(row, col, scale)) {
                continue; // Skip this pixel
            }

            // Get the color from the array
            uint16_t color = deck[cardIndex][row * CARD_WIDTH + col];

            // Draw a big square instead of a single dot
            gfx->fillRect(x + (col * scale), y + (row * scale), scale, scale, color);
        }
    }
}

void drawTwoCards(int card1Index, int card2Index) {
    gfx->fillScreen(TABLE_GREEN);

    // Cards are 64x96 when scaled by 2 = 128x192
    int scaledWidth = CARD_WIDTH * 2;   // 128
    int scaledHeight = CARD_HEIGHT * 2; // 192

    // Screen is 320x240
    int screenWidth = 320;
    int screenHeight = 240;
    
    // Vertical centering
    int y = (screenHeight - scaledHeight) / 2;  // (320 - 192) / 2 = 64
    
    // Horizontal spacing: two cards side by side with gap
    int gap = 10;
    int totalWidth = (scaledWidth * 2) + gap;
    int x1 = (screenWidth - totalWidth) / 2;
    int x2 = x1 + scaledWidth + gap;
    
    drawScaledCard(card1Index, x1, y, 2);
    drawScaledCard(card2Index, x2, y, 2);
}

void drawHoleCards() {
    // Clear screen
    gfx->fillScreen(TABLE_GREEN);
    
    // If we haven't received cards yet, show waiting screen
    if (!hasCards) {
        gfx->setTextColor(WHITE);
        gfx->setTextSize(2);
        gfx->setCursor(60, 140);
        gfx->printf("Player %d", PLAYER_ID);
        gfx->setCursor(40, 170);
        gfx->print("Waiting...");
        return; // Exit function early
    }
    
    drawTwoCards(currentHand.card1, currentHand.card2);
}

// ============ ESP-NOW RECEIVE CALLBACK ==============
// This function is called AUTOMATICALLY whenever data arrives via ESP-NOW
// PARAMETERS:
// - mac: MAC address of sender (the dealer in this case)
// - incomingData: Pointer to the received bytes
// - len: How many bytes were received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
    // Create a temporary CardData structure
    CardData receivedData;
    
    // Copy the incoming bytes into our structure
    // WHY memcpy: The data arrives as raw bytes, we need to interpret them as CardData
    memcpy(&receivedData, incomingData, sizeof(receivedData));
    
    // CHECK IF THIS MESSAGE IS FOR US:
    // The dealer broadcasts to everyone, but each message has a playerID
    // We only want to process messages meant for our PLAYER_ID
    if (receivedData.playerID == PLAYER_ID) {
        // This message is for us! Save it
        currentHand = receivedData;
        hasCards = true;
        
        Serial.printf("Got cards: %d and %d\n", currentHand.card1, currentHand.card2);
        
        // Update the screen to show our new cards
        drawHoleCards();
    }
    // If receivedData.playerID != PLAYER_ID, we ignore this message
    // (it's meant for a different player)
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.printf("Player %d starting...\n", PLAYER_ID);
    
    // 1. Init display
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    
    gfx->begin();
    gfx->fillScreen(TABLE_GREEN);
    
    // Show initial "Waiting..." screen
    drawHoleCards();
    
    // ============ 2. INIT ESP-NOW ==============
    
    // Put WiFi in Station mode (same reason as dealer)
    WiFi.mode(WIFI_STA);
    
    // PRINT MAC ADDRESS:
    // This is useful for debugging - you can see which device is which
    // Each ESP32 has a unique MAC address (like a network fingerprint)
    Serial.printf("MAC Address: %s\n", WiFi.macAddress().c_str());
    
    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    
    // REGISTER RECEIVE CALLBACK:
    // This tells ESP-NOW to call OnDataRecv() whenever data arrives
    // WHY: Unlike the dealer who only sends, players need to receive
    esp_now_register_recv_cb(OnDataRecv);
    
    Serial.println("Listening for cards...");
}

void loop() {
    // Players don't need to do anything in loop!
    // ESP-NOW handles everything via the OnDataRecv callback
    // When data arrives, OnDataRecv is called automatically
    delay(10);
}