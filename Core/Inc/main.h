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

#define SD_CS_PIN 13

