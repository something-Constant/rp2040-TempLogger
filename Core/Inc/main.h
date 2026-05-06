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
#include "ds3231.h"
#include "button.h"

#define SD_CS_PIN 13

#define Led 25
#define Ds3231_Gpio_Interrupt 16
#define Button1 17
#define Button2 18

#define SSD1306_I2C_CLK 400000 // 400khz
#define I2C_SDA_PIN 14
#define I2C_SCL_PIN 15

#define SPI_SCK_PIN 10
#define SPI_TX_PIN 11
#define SPI_RX_PIN 12
#define SPI_CSN_PIN 13

#define NTC_ADC_Channel 0
#define CoreTemp_ADC_Channel 4

#define NTC_ADC_DMA_Channel 0
