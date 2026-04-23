// UIManager.cpp
#include "UIManager.h"
#include <Arduino_GFX_Library.h>
#include <bsp_cst816.h>
#include <BoardConfig.h>
#include "ui/ui.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// Internal state (only visible in this file)
static ButtonState currentButtonState = STATE_NORMAL;
static Arduino_GFX *gfx = nullptr;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[320 * 240 / 10];

// Forward declarations for internal functions
static void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
static void my_touch_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);

void initDisplay() {
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    
    Arduino_DataBus *bus = new Arduino_ESP32SPI(
        TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, TFT_MISO
    );
    
    gfx = new Arduino_ST7789(bus, TFT_RST, 3, true, 240, 320, 0, 0, 0, 0);
    gfx->begin();
    gfx->fillScreen(0x0000);
}

void initTouch() {
    Wire.begin(TOUCH_SDA, TOUCH_SCL);
    if (!bsp_touch_init(&Wire, 3, 320, 240)) {
        Serial.println("⚠️ Warning: Touch failed ⚠️");
    }
}

void initLVGL() {
    lv_init();
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, 320 * 240 / 10);
    
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
}

void initUI() {
    ui_init();
}

void updateUI() {
    lv_timer_handler();
    delay(5);
    
    static uint32_t last_tick = 0;
    uint32_t now = millis();
    if (now > last_tick) {
        lv_tick_inc(now - last_tick);
        last_tick = now;
    }
}

void unconfirmButtons() {
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

void confirmButtons(ButtonState confirmingAction) {
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

ButtonState getCurrentButtonState() {
    return currentButtonState;
}

// Internal helper functions (static = only visible in this file)
static void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
    lv_disp_flush_ready(disp);
}

static void my_touch_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
    uint16_t touchpad_x = 0;
    uint16_t touchpad_y = 0;
    
    // --- CALIBRATION SETTINGS ---
    // If touch is too low (cursor is below finger), use a NEGATIVE y_offset to move it up.
    // If touch is too high (cursor is above finger), use a POSITIVE y_offset to move it down.
    const int X_OFFSET = 0;  
    const int Y_OFFSET = -20; // Start with -20 if it registers "lower" than your finger
    
    // Set to true to see coordinates in Serial Monitor
    const bool DEBUG_TOUCH = false; 
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