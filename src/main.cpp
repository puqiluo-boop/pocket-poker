#include <Arduino.h>
#include <TFT_eSPI.h> // Hardware-specific library

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library

void setup() {
  Serial.begin(115200);
  
  // Initialize the screen
  tft.init();
  
  // Rotate to landscape (1 or 3)
  tft.setRotation(1); 
  
  // Fill screen with black
  tft.fillScreen(TFT_BLACK);
  
  // Draw some test text
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("ST7796 Driver Test");
  
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(4);
  tft.println("It Works!");
  
  // Draw a red box
  tft.fillRect(50, 100, 100, 50, TFT_RED);
}

void loop() {
  // Blink the red box color
  tft.fillRect(50, 100, 100, 50, TFT_RED);
  delay(1000);
  tft.fillRect(50, 100, 100, 50, TFT_BLUE);
  delay(1000);
}