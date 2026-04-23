// UIManager.h
// Handles all UI initialization and button interactions

#pragma once
#include <lvgl.h>

// UI States
enum ButtonState {
    STATE_NORMAL,
    STATE_DISABLED,
    STATE_CONFIRMING_FOLD,
    STATE_CONFIRMING_CHECK,
    STATE_CONFIRMING_CALL,
    STATE_CONFIRMING_BET
};

// Initialization functions
void initDisplay();
void initTouch();
void initLVGL();
void initUI();
void attachButtonHandlers();

// UI update functions
void updateUI();
void unconfirmButtons();
void confirmButtons(ButtonState confirmingAction);
void disableButtons(); //TODO
void enableButtons(); //TODO

// Get current UI state
ButtonState getCurrentButtonState();