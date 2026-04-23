// UIController.h
// Handles LVGL initialization and UI creation

#pragma once
#include <lvgl.h>

// Initialize LVGL system
void initLVGL();

// Initialize touchscreen
void initTouch();

// Create the dealer UI (button, labels, etc.)
// onDealPressed = callback when "DEAL HAND" button is clicked
void createDealerUI(void (*onDealPressed)());

// Call this in loop() to update LVGL
void updateLVGL();