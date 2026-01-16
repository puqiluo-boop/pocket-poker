#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <TouchDrv.h>

#include <BoardConfig.h>
#include <Deck.h>
#include <Colors.h>

// ============ Dealer Screen ==============
Arduino_DataBus *bus1 = new Arduino_ESP32SPI(
    TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, TFT_MISO
);

Arduino_GFX *dealerGfx = new Arduino_ST7789(
    bus1, TFT_RST, 
    3 /* rotation */, true /* IPS */, 
    240, 320, 
    0, 0, 0, 0
);

// ============ Table Screen ==============
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

// --- HELPERS ---
void drawButton(bool pressed) {
    uint16_t color = pressed ? BLUE : DARKGREY;
    uint16_t textColor = pressed ? BLACK : WHITE;
    
    dealerGfx->fillRoundRect(40, 80, 160, 80, 10, color);
    dealerGfx->drawRoundRect(40, 80, 160, 80, 10, WHITE);
    
    dealerGfx->setCursor(75, 110);
    dealerGfx->setTextColor(textColor);
    dealerGfx->setTextSize(2);
    dealerGfx->println(pressed ? "DEALING..." : "DEAL NEW");
}

// --- HELPER: DRAW DEALER VIEW ---
void drawDealerView() {
    dealerGfx->fillScreen(BLACK);
    
    // Draw a "Button" to show where to touch
    dealerGfx->fillRoundRect(40, 80, 160, 80, 10, DARKGREY);
    dealerGfx->drawRoundRect(40, 80, 160, 80, 10, WHITE);
    
    dealerGfx->setCursor(75, 110);
    dealerGfx->setTextColor(WHITE);
    dealerGfx->setTextSize(2);
    dealerGfx->println("DEAL NEW");
    dealerGfx->setCursor(90, 130);
    dealerGfx->println("HAND");
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
    Serial.begin(115200);
    delay(1000); 

    // Init Touch
    touch_init(TOUCH_SDA, TOUCH_SCL);
    Serial.println("Touch Driver Started");

    // Init Backlight
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    // Init Screens
    dealerGfx->begin();
    dealerGfx->fillScreen(BLACK);
    drawButton(false); 

    tableGfx->begin();
    tableGfx->fillScreen(TABLE_GREEN);
}

void loop() {
    int x = 0, y = 0;

    // 1. Read Touch (Clean one-liner!)
    if (touch_read(TOUCH_ADDR, &x, &y)) {
        
        Serial.printf("Touch: %d, %d\n", x, y);

        if (x > 40 && x < 200 && y > 80 && y < 160) {
            drawButton(true); 
            int* newHand = shuffleDeck();
            drawFiveCards(newHand[1], newHand[2], newHand[3], newHand[5], newHand[7]);
            delay(200);
            drawButton(false);
            delay(500); 
        }
    }
    delay(20); 
}