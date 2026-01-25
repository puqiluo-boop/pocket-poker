#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <lvgl.h>

#include <BoardConfig.h>
#include <Colors.h>
#include <Deck.h>
#include <Trim.h>

#include <PlayerComms.h>
#include <Structs.h>

extern "C" {
    #include "ui/ui.h"
}

#include <bsp_cst816.h>

#define PLAYER_ID 2
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

CardData currentHand;
bool hasCards = false;

Arduino_DataBus *bus = new Arduino_ESP32SPI(
    TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, TFT_MISO
);

Arduino_GFX *gfx = new Arduino_ST7789(
    bus, TFT_RST, 
    3 /* rotation */, true /* IPS */, 
    240, 320, 
    0, 0,
    0, 0 
);

// --- LVGL BUFFERS & DRIVERS ---
static lv_disp_draw_buf_t draw_buf;
// allocate a buffer for 1/10th of the screen size
static lv_color_t buf[SCREEN_WIDTH * SCREEN_HEIGHT / 10]; 

static lv_disp_drv_t disp_drv;
static lv_indev_drv_t indev_drv;

// --- GLUE 1: DISPLAY FLUSH ---
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    
    // Draw the bitmap to the GFX library
    gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
    
    // Tell LVGL we are ready
    lv_disp_flush_ready(disp);
}

// --- GLUE 2: INPUT READ ---
void my_touch_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
    uint16_t touchpad_x = 0;
    uint16_t touchpad_y = 0;
    
    data->state = LV_INDEV_STATE_RELEASED;
    
    bsp_touch_read();
    
    if (bsp_touch_get_coordinates(&touchpad_x, &touchpad_y)) {
        // Ensure coordinates are within bounds
        if (touchpad_x < SCREEN_WIDTH && touchpad_y < SCREEN_HEIGHT) {
            data->point.x = touchpad_x;
            data->point.y = touchpad_y;
            data->state = LV_INDEV_STATE_PRESSED;
        }
    }
}

// ... [Keep your helper functions: shouldSkipPixel, drawScaledCard, etc.] ...

void onCardsReceived(CardData cards) {
    currentHand = cards;
    hasCards = true;
    Serial.printf("Got cards: %d and %d\n", cards.card1, cards.card2);
    // Note: You might need to update a UI Label here instead of raw drawing
}

void setup() {
    Serial.begin(115200);
    // delay(1000); // Optional
    
    Serial.printf("Player %d starting...\n", PLAYER_ID);
    
    // 1. Init Hardware Display
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    
    gfx->begin();
    gfx->fillScreen(0x0000); // Clear noise immediately
    
    // 2. Init Touch
    Wire.begin(TOUCH_SDA, TOUCH_SCL);
    if (!bsp_touch_init(&Wire, 3, SCREEN_WIDTH, SCREEN_HEIGHT)) {
        Serial.println("Warning: Touch initialization failed");
    }

    // 3. Init LVGL Core
    lv_init();

    // 4. Init LVGL Buffer
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, SCREEN_WIDTH * SCREEN_HEIGHT / 10);

    // 5. Register Display Driver
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    // 6. Register Touch Driver
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touch_read;
    lv_indev_drv_register(&indev_drv);

    // 7. Init Comms
    if (!initPlayerComms(PLAYER_ID, onCardsReceived)) {
        Serial.println("Communication setup failed!");
        // Don't return, let UI load anyway
    }
    
    // 8. Init SquareLine UI
    ui_init();

    Serial.printf("MAC: %s\n", getPlayerMAC().c_str());
}

void loop() {
    // LVGL Timer Management
    lv_timer_handler(); 
    delay(5);
    
    // Keep tick increment generic (using built-in millis)
    // You don't strictly need manual tick_inc if you use lv_conf.h correctly,
    // but if you do use it, it looks like this:
    static uint32_t last_tick = 0;
    uint32_t now = millis();
    if (now > last_tick) {
        lv_tick_inc(now - last_tick);
        last_tick = now;
    }
}