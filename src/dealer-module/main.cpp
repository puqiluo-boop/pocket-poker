#include <Arduino.h>
#include <Arduino_GFX_Library.h>

#include <BoardConfig.h>
#include <Deck.h>
#include <Colors.h>

// ==========================================
// SCREEN 1: INTERNAL (Dealer View)
// ==========================================
Arduino_DataBus *bus1 = new Arduino_ESP32SPI(
    TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, TFT_MISO
);

Arduino_GFX *dealerGfx = new Arduino_ST7789(
    bus1, TFT_RST, 
    3 /* rotation */, true /* IPS */, 
    240, 320, 
    0, 0, 0, 0
);

// ==========================================
// SCREEN 2: EXTERNAL 4-INCH (Table View)
// ==========================================
// WIRING (Using the Right Header pins we discussed)
#define EXT_SCK  13
#define EXT_MOSI 11
#define EXT_MISO 15
#define EXT_CS   9
#define EXT_DC   12
#define EXT_RST  14

Arduino_DataBus *bus2 = new Arduino_ESP32SPI(
    EXT_DC, EXT_CS, EXT_SCK, EXT_MOSI, EXT_MISO
);

// 4-inch screens are usually ILI9488 (480x320)
// We use the "18bit" version because SPI often requires it for this chip
Arduino_GFX *tableGfx = new Arduino_ILI9488_18bit(
    bus2, EXT_RST, 
    3 /* rotation */, false /* IPS */
);

// --- HELPER: DRAW DEALER VIEW ---
void drawDealerView() {
    dealerGfx->fillScreen(BLACK);
    dealerGfx->setCursor(10, 20);
    dealerGfx->setTextColor(WHITE);
    dealerGfx->setTextSize(3);
    dealerGfx->println("DEALER");
    dealerGfx->setTextSize(2);
    dealerGfx->println("Waiting for bets...");
}

// --- HELPER: SCALED DRAWING (Fixed for Arduino_GFX) ---
void drawScaledCard(int cardIndex) {
  const uint16_t *data = deck[cardIndex];
  int scale = 5; 

  for (int row = 0; row < CARD_HEIGHT; row++) {
    for (int col = 0; col < CARD_WIDTH; col++) {
      uint16_t color = data[row * CARD_WIDTH + col];
      tableGfx->fillRect((col * scale), (row * scale), 5, 5, color);
    }
  }
}

// --- HELPER: DRAW 5 CARDS ---
void drawFiveCards(int flop1, int flop2, int flop3, int turn, int river) {
  // Clear screen with Green
  tableGfx->fillScreen(TABLE_GREEN);

  // Center the cards vertically: (Screen Height 360 - Card Height) / 2
  int yPos = (320 - CARD_HEIGHT) / 2; 

  // Calculate spacing (adjust these X numbers to fit your screen width)
  int xPositions[5] = {
    40,                     // Card 1 X
    40 + (CARD_WIDTH + 20),  // Card 2 X (plus 5px gap)
    40 + (2 * (CARD_WIDTH + 20)), 
    40 + (3 * (CARD_WIDTH + 20)), 
    40 + (4 * (CARD_WIDTH + 20)) 
  };

  const uint16_t* cards[5] = {
    deck[flop1], deck[flop2], deck[flop3], deck[turn], deck[river]
  };

  for (int i = 0; i < 5; i++) {
    // TRANSLATION: tft.pushImage -> gfx->draw16bitRGBBitmap
    tableGfx->draw16bitRGBBitmap(xPositions[i], yPos, (uint16_t*)cards[i], CARD_WIDTH, CARD_HEIGHT);
  }
}

int* shuffleDeck() {
  static int deckOrder[52];
  for (int i = 0; i < 52; i++) {
    deckOrder[i] = i;
  }

  for (int i = 51; i > 0; i--) {
    int j = random(0, i + 1);
    int temp = deckOrder[i];
    deckOrder[i] = deckOrder[j];
    deckOrder[j] = temp;
  }
  return deckOrder;
}

void setup() {
  // Start Serial FIRST so we don't miss errors
    Serial.begin(115200);

  // *ESP32-S3 Specific Note:* // Since you have 'ARDUINO_USB_CDC_ON_BOOT=1' in your ini file,
  // the USB connection takes a second to wake up. 
  // Add a short delay if you want to see the very first "Hello" message.
  delay(1000); 
  Serial.println("Booting Dealer Module...");

  //Setup Backlights
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
    
  // Start Screen 1 (Dealer)
  if (!dealerGfx->begin()) {
      Serial.println("Dealer Screen Failed!");
  }
  dealerGfx->fillScreen(BLUE); // Flash Blue to prove it works

  // Start Screen 2 (Table)
  if (!tableGfx->begin()) {
      Serial.println("Table Screen Failed!");
  }
  tableGfx->fillScreen(RED); // Flash Red to prove it works

  delay(1000);

  // 4. Draw Initial State
  drawDealerView();
  drawFiveCards(0, 1, 2, 3, 4); // Draw Ace, 2, 3, 4, 5 of Hearts
}

void loop() {
  static unsigned long lastSwitch = 0;

  // Draw new hand every 5 seconds
  if (millis() - lastSwitch >= 5000) {
    int* shuffledDeck = shuffleDeck();
    drawFiveCards(shuffledDeck[1], shuffledDeck[2], shuffledDeck[3], shuffledDeck[5], shuffledDeck[7]);
    lastSwitch = millis();
  }
}