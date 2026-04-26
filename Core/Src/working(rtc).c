#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"

#define I2C_BUS i2c0
#define DS3231_ADDR 0x68
#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

char Data[64]; // Buffer for UART messages

// Helper functions
uint8_t bcd2dec(uint8_t bcd) { return ((bcd >> 4) * 10) + (bcd & 0x0F); }

uint8_t dec2bcd(uint8_t dec) { return ((dec / 10) << 4) | (dec % 10); }

void ds3231_read(uint8_t reg, uint8_t *data, int len) {
    // Fixed: use correct function names
    i2c_write_blocking(I2C_BUS, DS3231_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_BUS, DS3231_ADDR, data, len, false);
}

void ds3231_write(uint8_t reg, uint8_t data) {
    uint8_t buffer[2] = {reg, data};
    i2c_write_blocking(I2C_BUS, DS3231_ADDR, buffer, 2, false);
}

void ds3231_init() {
    // 1. Enable oscillator (clear EOSC bit in control register)
    uint8_t control;
    ds3231_read(0x0E, &control, 1);
    ds3231_write(0x0E, control & ~0x80); // Clear bit 7 (EOSC)

    // 2. Clear Oscillator Stop Flag
    uint8_t status;
    ds3231_read(0x0F, &status, 1);
    ds3231_write(0x0F, status & ~0x80); // Clear bit 7 (OSF)

    // 3. Set initial time (only if OSF was set - oscillator was stopped)
    if (status & 0x80) {
        // Write in BCD format!
        ds3231_write(0x00, 0x00); // 00 seconds
        ds3231_write(0x01, 0x30); // 30 minutes
        ds3231_write(0x02, 0x12); // 12 hours (BCD 0x12 = 12)
    }

    uart_puts(UART_ID, "DS3231 initialized\r\n");
}

void ds3231_read_time(uint8_t *hour, uint8_t *min, uint8_t *sec) {
    uint8_t data[3];
    ds3231_read(0x00, data, 3);      // Read sec, min, hour at once

    *sec  = bcd2dec(data[0] & 0x7F); // CH bit is bit 7
    *min  = bcd2dec(data[1] & 0x7F); // Mask unused bits
    *hour = bcd2dec(data[2] & 0x3F); // Mask for 24-hour mode
}

void init_uart() {
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Optional: Configure UART
    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
}

void init_i2c() {
    i2c_init(I2C_BUS, 100000);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
}

int main() {
    stdio_init_all();
    sleep_ms(2000); // Wait for USB/UART to stabilize

    init_uart();
    init_i2c();

    uart_puts(UART_ID, "\r\nDS3231 Real-Time Clock\r\n");
    uart_puts(UART_ID, "======================\r\n");

    // Initialize DS3231
    // ds3231_init();

    uint8_t hour, min, sec;
    uint8_t last_sec         = 255;
    static int stuck_counter = 0;

    while (1) {
        ds3231_read_time(&hour, &min, &sec);

        // Check if seconds changed
        if (sec != last_sec) {
            sprintf(Data, "Time: %02d:%02d:%02d\r\n", hour, min, sec);
            uart_puts(UART_ID, Data);
            last_sec      = sec;
            stuck_counter = 0; // Reset stuck counter
        }
        else {
            // If seconds don't change for multiple checks, show warning
            stuck_counter++;
            if (stuck_counter > 4) { // 2 seconds at 500ms checks
                sprintf(Data, "WARNING: Clock may be stuck! Seconds = %02d\r\n", sec);
                uart_puts(UART_ID, Data);

                // Check oscillator stop flag
                uint8_t status;
                ds3231_read(0x0F, &status, 1);
                sprintf(Data, "Status register: 0x%02X\r\n", status);
                uart_puts(UART_ID, Data);

                if (status & 0x80) {
                    uart_puts(UART_ID, "OSF bit set! Oscillator was stopped. Clearing...\r\n");
                    ds3231_write(0x0F, status & ~0x80);

                    // Re-enable oscillator
                    uint8_t control;
                    ds3231_read(0x0E, &control, 1);
                    ds3231_write(0x0E, control & ~0x80);
                    uart_puts(UART_ID, "Oscillator re-enabled\r\n");
                }

                stuck_counter = 0;
            }
        }

        sleep_ms(500);
    }

    return 0;
}