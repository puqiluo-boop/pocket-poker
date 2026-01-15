#include <Arduino.h>
#include <Arduino_GFX_Library.h>

// --- FINAL CONFIRMED PINOUT ---
#define TFT_BL    1   
#define TFT_MOSI 38   
#define TFT_SCK  39   
#define TFT_MISO 40   // (Not strictly needed for writing, but good to have)
#define TFT_CS    45  // Found in Demo #2
#define TFT_DC    42  // Found in Demo #2
#define TFT_RST   -1  // Found in Demo #2 (Automatic Reset)

Arduino_DataBus *bus = new Arduino_ESP32SPI(
    TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, TFT_MISO
);

// --- THE FIX IS HERE ---
// We changed the last number from 20 to 0.
Arduino_GFX *gfx = new Arduino_ST7789(
    bus, TFT_RST, 
    0 /* rotation */, true /* IPS */, 
    240, 320, 
    0, 0, // col offset 1, row offset 1 (WAS 0, 20)
    0, 0  // col offset 2, row offset 2
);

void setup() {
    // 1. Keep Pin 1 safe for Backlight
    // (You can try Serial.begin(115200) later, but let's get a clean image first)
    delay(500);

    // 2. Turn on Backlight
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    // 3. Init Display
    if (!gfx->begin()) {
        // Init failed
    }

    // 4. Fill Screen RED to check edges
    // If the static is gone, the whole screen will be solid Red.
    gfx->fillScreen(RED);
    
    // 5. Draw Corner Markers
    // This helps prove the alignment is perfect
    gfx->setTextColor(WHITE);
    gfx->setTextSize(2);
    
    // Top-Left
    gfx->setCursor(0, 0);
    gfx->print("TL");

    // Bottom-Right
    gfx->setCursor(215, 300);
    gfx->print("BR");
    
    // Center Text
    gfx->setTextSize(3);
    gfx->setCursor(20, 150);
    gfx->println("PERFECT!");
}

void loop() {
    // Enjoy your victory
}