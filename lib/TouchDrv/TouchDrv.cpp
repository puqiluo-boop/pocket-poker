#include "TouchDrv.h"

void touch_init(int sda, int scl) {
    // Start I2C with the pins passed from main
    Wire.begin(sda, scl);
}

bool touch_read(uint8_t addr, int *x, int *y) {
    uint8_t data[6];

    // 1. Send "Read" command to register 0x01
    Wire.beginTransmission(addr);
    Wire.write(0x01);
    if (Wire.endTransmission() != 0) {
        return false; // I2C Error
    }

    // 2. Request 6 bytes of data
    Wire.requestFrom(addr, (uint8_t)6);
    
    if (Wire.available() >= 6) {
        for (int i = 0; i < 6; i++) {
            data[i] = Wire.read();
        }

        // Byte 1 contains number of touch points
        uint8_t points = data[1] & 0x0F;
        
        if (points > 0) {
            // Combine bytes for X and Y
            *x = ((data[2] & 0x0F) << 8) | data[3];
            *y = ((data[4] & 0x0F) << 8) | data[5];
            return true;
        }
    }
    return false;
}