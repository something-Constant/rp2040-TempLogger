#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <ctype.h>

#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"

#include "ssd1306.h"
#include "font_5x7/font_render_engine.h"

int NTC_table[10] = {-36, -14, 0, 10, 20, 31, 45, 68, 165};
// int NTC_table[10] = {-10, 17, 34, 47, 60, 73, 93, 123, 261};

int NTC_ADC2Temperature(unsigned int adc_value) {
    // Clamp ADC value to valid range
    if (adc_value >= 4095) {
        return NTC_table[8];
    }

    // Calculate index (0-7)
    unsigned int index = adc_value >> 9; // Divide by 512 highest result is 7

    // Safety check (should not happen with proper ADC range)
    if (index >= 8) {
        index = 7;
    }

    // Get interpolation points
    int p1 = NTC_table[index];
    int p2 = NTC_table[index + 1];

    // Calculate offset within the section (0-511)

    unsigned int offset = adc_value & 0x01FF;

    // Interpolate with rounding for better accuracy
    // Formula: p1 + ((p2 - p1) * offset + 256) / 512
    int diff = p2 - p1;

    // Use 64-bit to prevent overflow
    long long temp = (long long) diff * offset + 256;

    return p1 + (int) (temp >> 9);
}