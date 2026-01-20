#include <Arduino.h>

// Visual Libraries
#include <Arduino_GFX_Library.h>
#include <lvgl.h>

// Touchscreen Library
#include <bsp_cst816.h>

// Networking Libraries
#include <esp_now.h>
#include <WiFi.h>

#include <BoardConfig.h>
#include <Deck.h>
#include <Colors.h>

// Forward Declarations
int* shuffleDeck();
void drawFiveCards(int flop1, int flop2, int flop3, int turn, int river);
void sendCardsToPlayers(int* deck);

// ============ ESP-NOW DATA STRUCTURE ==============
// This defines what data we're sending to players.
// IMPORTANT: Both dealer and player must have THE EXACT SAME structure
// or they won't be able to decode each other's messages.
typedef struct {
    uint8_t playerID;           // Which player this is for (1-6)
    uint8_t card1;              // First hole card (index into deck array)
    uint8_t card2;              // Second hole card
} CardData;

// BROADCAST ADDRESS:
// FF:FF:FF:FF:FF:FF is a special MAC address that means "send to everyone"
// This way we don't need to know each player's unique MAC address
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

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

// LVGL Buffers
static lv_disp_draw_buf_t draw_buf; 

// --- GLUE 1: DISPLAY FLUSH ---
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    dealerGfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
    lv_disp_flush_ready(disp);
}

// --- GLUE 2: INPUT READ ---
void my_touch_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
    uint16_t touchpad_x = 0;
    uint16_t touchpad_y = 0;
    
    data->state = LV_INDEV_STATE_RELEASED;
    
    bsp_touch_read();
    
    if (bsp_touch_get_coordinates(&touchpad_x, &touchpad_y)) {
        if (touchpad_x < 320 && touchpad_y < 240) {
            data->point.x = touchpad_x;
            data->point.y = touchpad_y;
            data->state = LV_INDEV_STATE_PRESSED;
        }
    }
}

// --- UI CODE ---
void create_ui() {
    lv_obj_t *btn = lv_btn_create(lv_scr_act());
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(btn, 140, 60);

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "DEAL HAND");
    lv_obj_center(label);

    lv_obj_add_event_cb(btn, [](lv_event_t *e) {
        int* newHand = shuffleDeck();
        sendCardsToPlayers(newHand);
        drawFiveCards(newHand[2], newHand[3], newHand[4], newHand[6], newHand[8]);
    }, LV_EVENT_CLICKED, NULL);
}

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

Arduino_GFX *tableGfx = new Arduino_ILI9488_18bit(
    bus2, EXT_RST, 
    3 /* rotation */, false /* IPS */
);

// --- HELPER: DRAW 5 CARDS ---
void drawFiveCards(int flop1, int flop2, int flop3, int turn, int river) {
  // Start fresh
  tableGfx->fillScreen(TABLE_GREEN);
  delay(10); // Give screen time to clear
  
  // ILI9488 in rotation 3: 480 width x 320 height
  int yPos = (320 - CARD_HEIGHT) / 2; 

  // Calculate spacing
  int totalCardWidth = 5 * CARD_WIDTH;
  int availableSpace = 480 - 80;
  int totalGapWidth = availableSpace - totalCardWidth;
  int gap = totalGapWidth / 4;
  
  int xPositions[5];
  xPositions[0] = 40;
  for (int i = 1; i < 5; i++) {
    xPositions[i] = xPositions[i-1] + CARD_WIDTH + gap;
  }

  const uint16_t* cards[5] = {
    deck[flop1], deck[flop2], deck[flop3], deck[turn], deck[river]
  };

  // Draw each card with small delay
  for (int i = 0; i < 5; i++) {
    tableGfx->draw16bitRGBBitmap(xPositions[i], yPos, (uint16_t*)cards[i], CARD_WIDTH, CARD_HEIGHT);
    delay(5); // Small delay between cards
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

// ============ ESP-NOW FUNCTIONS ==============

// CALLBACK: This function is called automatically AFTER we send data
// It tells us if the transmission was successful or failed
// WHY: Helps with debugging - if sends are failing, we know there's a range/interference issue
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    // status is either ESP_NOW_SEND_SUCCESS or ESP_NOW_SEND_FAIL
    Serial.printf("Send Status: %s\n", status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

// SEND CARDS TO PLAYERS
// This loops through 6 players and sends each their hole cards
void sendCardsToPlayers(int* deckOrder) {
    // Loop through 6 players (you can change this to however many you have)
    for (int player = 0; player < 1; player++) {
        // Create a CardData packet for this specific player
        CardData data;
        
        // Set which player this is for (1-6, not 0-5, so add 1)
        data.playerID = player + 1;
        
        // CARD DISTRIBUTION:
        data.card1 = deckOrder[0 + (player * 2)];
        data.card2 = deckOrder[1 + (player * 2)];
        
        // SEND THE DATA:
        // esp_now_send takes 3 parameters:
        // 1. MAC address to send to (broadcastAddress = everyone)
        // 2. Pointer to the data (we cast our struct to uint8_t*)
        // 3. Size of the data in bytes
        esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&data, sizeof(data));
        
        // Check if send was successful
        if (result == ESP_OK) {
            Serial.printf("Sent to Player %d\n", player + 1);
        } else {
            Serial.printf("Error sending to Player %d\n", player + 1);
        }
        
        // Small delay between sends to avoid overwhelming the radio
        delay(10);
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    // 1. Init Hardware (displays)
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    dealerGfx->begin();
    dealerGfx->fillScreen(BLACK);

    if(tableGfx) {
        tableGfx->begin();
        tableGfx->fillScreen(TABLE_GREEN);
    }

    // 2. Init Touch
    Wire.begin(TOUCH_SDA, TOUCH_SCL);
    if (!bsp_touch_init(&Wire, 3, 320, 240)) {
        Serial.println("Warning: Touch initialization failed");
    }

    // ============ 3. INIT ESP-NOW ==============
    
    // WHY WIFI_STA MODE:
    // ESP-NOW requires the WiFi radio to be on, but in "Station" mode
    // This does NOT connect to any WiFi network - it just turns on the radio
    // Think of it like turning on Bluetooth without connecting to a device
    WiFi.mode(WIFI_STA);
    
    // INITIALIZE ESP-NOW:
    // This starts the ESP-NOW protocol on this device
    // Returns ESP_OK if successful
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return; // Stop setup if this fails
    }
    
    // REGISTER SEND CALLBACK:
    // This tells ESP-NOW to call OnDataSent() whenever we finish sending data
    // Useful for debugging and knowing if messages got through
    esp_now_register_send_cb(OnDataSent);
    
    // ADD BROADCAST PEER:
    // A "peer" is another device we can communicate with
    // We need to register the broadcast address as a peer before we can send to it
    esp_now_peer_info_t peerInfo = {}; // Create empty peer info struct
    
    // Copy broadcast address into peer info
    memcpy(peerInfo.peer_addr, broadcastAddress, 6); // MAC addresses are 6 bytes
    
    // Channel 0 = auto-select channel (use whatever WiFi channel we're on)
    peerInfo.channel = 0;
    
    // encrypt = false means no encryption (faster, but less secure)
    // For a poker game, speed matters more than security
    peerInfo.encrypt = false;
    
    // Register this peer with ESP-NOW
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }
    
    Serial.println("ESP-NOW Ready!");

    // 4. Init LVGL (unchanged)
    lv_init();
    
    uint32_t bufSize = 320 * 240;
    lv_color_t *disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (!disp_draw_buf) {
        disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize * sizeof(lv_color_t), MALLOC_CAP_8BIT);
    }
    
    if (!disp_draw_buf) {
        Serial.println("ERROR: LVGL buffer allocation failed!");
        while(1);
    }
    
    lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, bufSize);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 320;
    disp_drv.ver_res = 240;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touch_read;
    lv_indev_drv_register(&indev_drv);

    create_ui();
    
    Serial.println("Dealer ready!");
}

void loop() {
    static uint32_t last_tick = 0;
    uint32_t now = millis();
    lv_tick_inc(now - last_tick);
    last_tick = now;
    
    lv_timer_handler(); 
    delay(5);
}