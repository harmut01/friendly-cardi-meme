/* mbed Microcontroller Library
 * Copyright (c) 2021 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "max30102.h"


int main()
{
    I2C i2c_handler(I2C_SDA, I2C_SCL);
    MAX30102 heart_sensor(i2c_handler);

    // call initialiser which sets LEDS and SPO2 sensor 
    printf("Hello, Mbed!\n");
    return 0;
}