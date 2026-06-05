#include "UI.h"
#include "Display.h"
#include <bsp_cst816.h>
#include <BoardConfig.h>

// LVGL buffers
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *disp_draw_buf = nullptr;

// Store callback for button click
static void (*dealButtonCallback)() = nullptr;

// LVGL display flush callback
static void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    
    Arduino_GFX* gfx = getDealerGfx();
    gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
    
    lv_disp_flush_ready(disp);
}

// LVGL touch input callback
static void my_touch_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
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

void initTouch() {
    Wire.begin(TOUCH_SDA, TOUCH_SCL);
    if (!bsp_touch_init(&Wire, 3, 320, 240)) {
        Serial.println("Warning: Touch initialization failed");
    } else {
        Serial.println("Touch initialized");
    }
}

void initLVGL() {
    lv_init();
    
    // Allocate buffer for full screen
    uint32_t bufSize = 320 * 240;
    
    // Try internal RAM first, fall back to any RAM
    disp_draw_buf = (lv_color_t *)heap_caps_malloc(
        bufSize * sizeof(lv_color_t), 
        MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT
    );
    
    if (!disp_draw_buf) {
        disp_draw_buf = (lv_color_t *)heap_caps_malloc(
            bufSize * sizeof(lv_color_t), 
            MALLOC_CAP_8BIT
        );
    }
    
    if (!disp_draw_buf) {
        Serial.println("ERROR: LVGL buffer allocation failed!");
        while(1);
    }
    
    lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, bufSize);
    
    // Register display driver
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 320;
    disp_drv.ver_res = 240;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
    
    // Register touch driver
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touch_read;
    lv_indev_drv_register(&indev_drv);
    
    Serial.println("LVGL initialized");
}

void createDealerUI(void (*onDealPressed)()) {
    // Save callback
    dealButtonCallback = onDealPressed;
    
    // Create "DEAL HAND" button
    lv_obj_t *btn = lv_btn_create(lv_scr_act());
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(btn, 140, 60);
    
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "DEAL HAND");
    lv_obj_center(label);
    
    // Attach event handler
    lv_obj_add_event_cb(btn, [](lv_event_t *e) {
        if (dealButtonCallback != nullptr) {
            dealButtonCallback();
        }
    }, LV_EVENT_CLICKED, NULL);
    
    Serial.println("Dealer UI created");
}

void updateLVGL() {
    static uint32_t last_tick = 0;
    uint32_t now = millis();
    lv_tick_inc(now - last_tick);
    last_tick = now;
    
    lv_timer_handler();
}