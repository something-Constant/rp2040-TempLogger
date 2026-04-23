#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/util/datetime.h"
#include "pico/multicore.h"
#include "pico/time.h"

#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "hardware/rtc.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/spi.h"
#include "hardware/dma.h"

#include "ssd1306.h"
#include "font_5x7/font_render_engine.h"
#include "moving_average_filter/moving_average_filter.h"
#include "median_filter/median_filter.h"

#include "ff.h"
#include "fatfs_sd.h"

// #define spi1 ((spi_inst_t *)spi1_hw)
#define SD_CS_PIN 9

datetime_t DateTime = {.year = 2026, .month = 4, .day = 26, .hour = 13, .min = 30};

datetime_t rtc_alarm = {};

int8_t NTC_table[10] = {-36, -14, 0, 10, 20, 31, 45, 68, 165};

int16_t NTC_ADC2Temperature(uint16_t adc_value) {
    // Clamp ADC value to valid range
    if (adc_value >= 4095) {
        return NTC_table[8];
    }

    // Calculate index (0-7)
    uint8_t index = adc_value >> 9; // Divide by 512 highest result is 7

    // Safety check (should not happen with proper ADC range)
    if (index >= 8) {
        index = 7;
    }

    // Get interpolation points
    int8_t p1 = NTC_table[index];
    int8_t p2 = NTC_table[index + 1];

    // Calculate offset within the section (0-511)

    uint16_t offset = adc_value & 0x01FF;

    // Interpolate with rounding for better accuracy
    // Formula: p1 + ((p2 - p1) * offset + 256) / 512
    int16_t diff = p2 - p1;

    // Use 64-bit to prevent overflow
    int32_t temp = (int32_t) diff * offset + 256;

    return p1 + (int32_t) (temp >> 9);
}