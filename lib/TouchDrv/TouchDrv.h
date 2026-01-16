#pragma once

#include <Arduino.h>
#include <Wire.h>

// Initialize the I2C bus with specific pins
void touch_init(int sda, int scl);

// Read the touch coordinates. Returns true if touched.
bool touch_read(uint8_t addr, int *x, int *y);