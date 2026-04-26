#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"

// // I2C configuration
// #define I2C_BUS i2c0
// #define SDA_PIN PICO_DEFAULT_I2C_SCL_PIN
// #define SCL_PIN PICO_DEFAULT_I2C_SDA_PIN

#define I2C_BUS i2c1
#define SDA_PIN 18
#define SCL_PIN 19

// UART configuration
#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

char Data[64];

void init_uart() {
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
}

void init_i2c() {
    i2c_init(I2C_BUS, 100000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);
}

void scan_i2c_addresses() {
    int devices_found = 0;

    uart_puts(UART_ID, "\r\nScanning I2C addresses...\r\n");

    for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
        uint8_t rx_data;

        // Try to read 1 byte - this is more reliable than write-only check
        // A real device will ACK its address and send data
        int result = i2c_read_blocking(I2C_BUS, addr, &rx_data, 1, false);

        if (result > 0) { // Must receive actual data, not just PICO_ERROR_GENERIC
            sprintf(Data, "0x%02X\r\n", addr);
            uart_puts(UART_ID, Data);
            devices_found++;
        }
    }

    if (devices_found == 0) {
        uart_puts(UART_ID, "No devices found\r\n");
    }
    else {
        sprintf(Data, "Found %d device(s)\r\n", devices_found);
        uart_puts(UART_ID, Data);
    }

    uart_puts(UART_ID, "Scan complete\r\n");
}

int main() {
    stdio_init_all();
    sleep_ms(2000);

    init_uart();
    init_i2c();

    scan_i2c_addresses();

    while (1) {
        tight_loop_contents();
    }

    return 0;
}