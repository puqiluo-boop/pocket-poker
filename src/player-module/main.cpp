#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <lvgl.h>

#include <BoardConfig.h>
#include <Colors.h>
#include <Deck.h>
#include <Trim.h>

#include <PlayerComms.h>
#include "Structs.h"

extern "C" {
    #include "ui/ui.h"
}

#include <bsp_cst816.h>

#define PLAYER_ID 2
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// ============ UI STATE MANAGEMENT ==============
// This tracks what mode the buttons are in
enum ButtonState {
    STATE_NORMAL,          // All buttons show their regular text
    STATE_CONFIRMING_FOLD, // Fold shows "Confirm", others show "Cancel"
    STATE_CONFIRMING_CHECK,
    STATE_CONFIRMING_CALL,
    STATE_CONFIRMING_BET
};

ButtonState currentButtonState = STATE_NORMAL;

// Forward declarations (tell compiler these functions exist)
void setButtonsToNormal();
void setButtonsToConfirm(ButtonState confirmingAction);

CardData g_currentHand;
bool g_hasCards = false;

// ============ UI HELPER FUNCTIONS ==============

// Reset all buttons to their original text and colors
void setButtonsToNormal() {
    currentButtonState = STATE_NORMAL;
    
    // Button 1: FOLD (red)
    lv_label_set_text(ui_Label1, "FOLD");
    lv_obj_set_style_bg_color(ui_Button1, lv_color_hex(0xFF3434), LV_PART_MAIN | LV_STATE_DEFAULT);
    
    // Button 2: CHECK (white)
    lv_label_set_text(ui_Label2, "CHECK");
    lv_obj_set_style_bg_color(ui_Button2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    
    // Button 3: CALL (gray)
    lv_label_set_text(ui_Label3, "CALL");
    lv_obj_set_style_bg_color(ui_Button3, lv_color_hex(0x959595), LV_PART_MAIN | LV_STATE_DEFAULT);
    
    // Button 4: BET (green)
    lv_label_set_text(ui_Label4, "BET");
    lv_obj_set_style_bg_color(ui_Button4, lv_color_hex(0x00B706), LV_PART_MAIN | LV_STATE_DEFAULT);
}

// Change one button to "Confirm" and all others to "Cancel"
void setButtonsToConfirm(ButtonState confirmingAction) {
    currentButtonState = confirmingAction;
    
    // Make all buttons orange "Cancel" first
    lv_obj_set_style_bg_color(ui_Button1, lv_color_hex(0xFF8C00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Button2, lv_color_hex(0xFF8C00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Button3, lv_color_hex(0xFF8C00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Button4, lv_color_hex(0xFF8C00), LV_PART_MAIN | LV_STATE_DEFAULT);
    
    lv_label_set_text(ui_Label1, "CANCEL");
    lv_label_set_text(ui_Label2, "CANCEL");
    lv_label_set_text(ui_Label3, "CANCEL");
    lv_label_set_text(ui_Label4, "CANCEL");
    
    // Then override the ONE button that's confirming
    switch (confirmingAction) {
        case STATE_CONFIRMING_FOLD:
            lv_label_set_text(ui_Label1, "CONFIRM");
            lv_obj_set_style_bg_color(ui_Button1, lv_color_hex(0x00FF00), LV_PART_MAIN | LV_STATE_DEFAULT);
            break;
        case STATE_CONFIRMING_CHECK:
            lv_label_set_text(ui_Label2, "CONFIRM");
            lv_obj_set_style_bg_color(ui_Button2, lv_color_hex(0x00FF00), LV_PART_MAIN | LV_STATE_DEFAULT);
            break;
        case STATE_CONFIRMING_CALL:
            lv_label_set_text(ui_Label3, "CONFIRM");
            lv_obj_set_style_bg_color(ui_Button3, lv_color_hex(0x00FF00), LV_PART_MAIN | LV_STATE_DEFAULT);
            break;
        case STATE_CONFIRMING_BET:
            lv_label_set_text(ui_Label4, "CONFIRM");
            lv_obj_set_style_bg_color(ui_Button4, lv_color_hex(0x00FF00), LV_PART_MAIN | LV_STATE_DEFAULT);
            break;
        default:
            break;
    }
}

// ============ BUTTON EVENT HANDLERS ==============

// Called when Button 1 (FOLD) is clicked
void onButton1Clicked(lv_event_t * e) {
    if (currentButtonState == STATE_NORMAL) {
        // First click: Enter confirm mode
        Serial.println("Fold pressed - waiting for confirmation");
        setButtonsToConfirm(STATE_CONFIRMING_FOLD);
        
    } else if (currentButtonState == STATE_CONFIRMING_FOLD) {
        // Second click on same button: CONFIRM the action
        Serial.println("Fold confirmed! Sending action...");
        sendPlayerAction(PLAYER_ID, 0);  // betSize = 0 for fold
        setButtonsToNormal();
        
    } else {
        // Click on a different button: CANCEL
        Serial.println("Action cancelled");
        setButtonsToNormal();
    }
}

// Called when Button 2 (CHECK) is clicked
void onButton2Clicked(lv_event_t * e) {
    if (currentButtonState == STATE_NORMAL) {
        Serial.println("Check pressed - waiting for confirmation");
        setButtonsToConfirm(STATE_CONFIRMING_CHECK);
        
    } else if (currentButtonState == STATE_CONFIRMING_CHECK) {
        Serial.println("Check confirmed! Sending action...");
        sendPlayerAction(PLAYER_ID, 0);  // betSize = 0 for check
        setButtonsToNormal();
        
    } else {
        Serial.println("Action cancelled");
        setButtonsToNormal();
    }
}

// Called when Button 3 (CALL) is clicked
void onButton3Clicked(lv_event_t * e) {
    if (currentButtonState == STATE_NORMAL) {
        Serial.println("Call pressed - waiting for confirmation");
        setButtonsToConfirm(STATE_CONFIRMING_CALL);
        
    } else if (currentButtonState == STATE_CONFIRMING_CALL) {
        Serial.println("Call confirmed! Sending action...");
        sendPlayerAction(PLAYER_ID, 100);  // TODO: Use actual call amount
        setButtonsToNormal();
        
    } else {
        Serial.println("Action cancelled");
        setButtonsToNormal();
    }
}

// Called when Button 4 (BET) is clicked
void onButton4Clicked(lv_event_t * e) {
    if (currentButtonState == STATE_NORMAL) {
        Serial.println("Bet pressed - waiting for confirmation");
        setButtonsToConfirm(STATE_CONFIRMING_BET);
        
    } else if (currentButtonState == STATE_CONFIRMING_BET) {
        Serial.println("Bet confirmed! Sending action...");
        sendPlayerAction(PLAYER_ID, 200);  // TODO: Use slider value
        setButtonsToNormal();
        
    } else {
        Serial.println("Action cancelled");
        setButtonsToNormal();
    }
}

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
    
    // --- CALIBRATION SETTINGS ---
    // If touch is too low (cursor is below finger), use a NEGATIVE y_offset to move it up.
    // If touch is too high (cursor is above finger), use a POSITIVE y_offset to move it down.
    const int X_OFFSET = 0;  
    const int Y_OFFSET = -20; // Start with -20 if it registers "lower" than your finger
    
    // Set to true to see coordinates in Serial Monitor
    const bool DEBUG_TOUCH = true; 
    // ---------------------------

    data->state = LV_INDEV_STATE_RELEASED;
    
    bsp_touch_read();
    
    if (bsp_touch_get_coordinates(&touchpad_x, &touchpad_y)) {
        
        // 1. Apply Calibration Offsets
        int cal_x = touchpad_x + X_OFFSET;
        int cal_y = touchpad_y + Y_OFFSET;

        // 2. Clamp values to screen edges (prevents crashing if offset pushes it < 0)
        if(cal_x < 0) cal_x = 0;
        if(cal_y < 0) cal_y = 0;
        if(cal_x > SCREEN_WIDTH - 1) cal_x = SCREEN_WIDTH - 1;
        if(cal_y > SCREEN_HEIGHT - 1) cal_y = SCREEN_HEIGHT - 1;

        // 3. Debugging (View in Serial Monitor)
        if (DEBUG_TOUCH) {
            Serial.printf("Raw: (%d, %d) -> Calibrated: (%d, %d)\n", 
                          touchpad_x, touchpad_y, cal_x, cal_y);
        }

        data->point.x = cal_x;
        data->point.y = cal_y;
        data->state = LV_INDEV_STATE_PRESSED;
    }
}

bool shouldSkipPixel(int row, int col, int scale) {
    for (int i = 0; i < 24; i++) {
        if (trim[i][0] == row && trim[i][1] == col) {
            return true;
        }
    }
    return false;
}

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
    int y = 10;
    int x1 = 16;
    int x2 = 90;
    drawScaledCard(card1Index, x1, y, 1);
    drawScaledCard(card2Index, x2, y, 1);
}

void onCardsReceived(CardData cards) {
    Serial.println("====== CARDS RECEIVED ======");
    Serial.printf("Player ID: %d\n", cards.playerID);
    Serial.printf("Card 1: %d\n", cards.card1);
    Serial.printf("Card 2: %d\n", cards.card2);
    Serial.println("===========================");
    
    g_currentHand = cards;           // Copy data
    g_hasCards = true;               // Mark as valid
    drawTwoCards(g_currentHand.card1, g_currentHand.card2);
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.printf("Player %d starting...\n", PLAYER_ID);
    
    // 1. Init Hardware Display
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    
    gfx->begin();
    gfx->fillScreen(0x0000);
    
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

    // 7. Init SquareLine UI
    ui_init();
    
    // 8. NEW: Attach event handlers to buttons
    lv_obj_add_event_cb(ui_Button1, onButton1Clicked, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ui_Button2, onButton2Clicked, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ui_Button3, onButton3Clicked, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ui_Button4, onButton4Clicked, LV_EVENT_CLICKED, NULL);
    
    Serial.println("Button handlers attached");

    // 9. Init Comms
    if (!initPlayerComms(PLAYER_ID, onCardsReceived)) {
        Serial.println("Communication setup failed!");
        return;
    }
    
    Serial.printf("MAC: %s\n", getPlayerMAC().c_str());
}

void loop() { 
    // LVGL Timer Management
    lv_timer_handler(); 
    delay(5);
    
    static uint32_t last_tick = 0;
    uint32_t now = millis();
    if (now > last_tick) {
        lv_tick_inc(now - last_tick);
        last_tick = now;
    }
}