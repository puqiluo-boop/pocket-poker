#include <Arduino.h>

// Visual Libraries
#include <Arduino_GFX_Library.h>
#include <lvgl.h>

// Touchscreen Library
#include <bsp_cst816.h>

#include <DealerComms.h>
#include <Structs.h>

#include <BoardConfig.h>
#include <Deck.h>
#include <Colors.h>

#include <PlayerRegistry.h>

// Forward Declarations
int* shuffleDeck();
void drawFiveCards(int flop1, int flop2, int flop3, int turn, int river);
void handlePlayerConnection(ConnectionCheck cc);

PlayerRegistry playerRegistry;

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
        int playerCount = playerRegistry.getConnectedCount();
        broadcastCards(newHand, playerCount);
        drawFiveCards(newHand[2*playerCount+1], newHand[2*playerCount+2], newHand[2*playerCount+3], newHand[2*playerCount+5], newHand[2*playerCount+7]);
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
    1 /* rotation */, true /* IPS */
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

void drawLargeCard(int cardIndex) {
    const uint16_t* cardData = deck[cardIndex];
    int w = 64;  // Width of the original image
    int h = 96;  // Height of the original image
    int scale = 5; // Scale factor
    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
        // 1. Get the color from the array
        // Note: Some converters use uint8_t (bytes), so we might need to combine them.
        // If your array is 'uint16_t', use this:
        uint16_t color = cardData[row * w + col];

        // 2. Draw a big square instead of a single dot
        tableGfx->fillRect((row * scale), (col * scale), 5, 5, color);
        }
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

void handlePlayerConnection(ConnectionCheck cc) {
    playerRegistry.registerPlayer(cc.senderID, String("Player ") + cc.senderID);
    playerRegistry.updateLastSeen(cc.senderID);
}


void setup() {
    Serial.begin(115200);
    delay(5000);
    Serial.println("Starting Dealer Setup...");

    // 1. Init Hardware (displays)
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    dealerGfx->begin();
    dealerGfx->fillScreen(BLACK);

    if(tableGfx) {
        tableGfx->begin();
        //tableGfx->fillScreen(TABLE_GREEN);
        drawLargeCard(0);
    }

    // 2. Init Touch
    Wire.begin(TOUCH_SDA, TOUCH_SCL);
    if (!bsp_touch_init(&Wire, 3, 320, 240)) {
        Serial.println("Warning: Touch initialization failed");
    }

    // 3. Init ESP-NOW
    if (!initDealerComms(handlePlayerConnection)) {
        Serial.println("Communication setup failed!");
        return;
    }

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

    playerRegistry.clearAll();
    delay(1000);
    broadcastConnectionCheck();
    delay(1000);
    playerRegistry.printConnected();
}

void loop() {
    static uint32_t last_tick = 0;
    uint32_t now = millis();
    lv_tick_inc(now - last_tick);
    last_tick = now;
    
    lv_timer_handler(); 
    delay(5);
}