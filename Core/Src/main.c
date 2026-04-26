#include "main.h"

/// @brief LCD Controller parameters
#define Font_H 9
#define FontY(Line) (scale * Font_H * Line)
uint8_t scale = 1, deph = 1;
extern uint8_t Buffer[BufferSize];
char Data[50];

#define SSD1306_I2C_CLK 400000 // 400khz
#define Led 25
#define Button 0
#define Blu_stat_pin 6

const float conversion_factor = 3.3f / (1 << 12);

uint16_t i = 0, ADC_raw;
float ADC_Voltage, Core_Temp, temp;

MAF mof;
MAF Cor;
MDF NTC_ADC;
MDF Core_ADC;

void RTC_Callback();

void ds3231_read(uint8_t reg, uint8_t *data, int len) {
    // Fixed: use correct function names
    i2c_write_blocking(i2c0, 0x68, &reg, 1, true);
    i2c_read_blocking(i2c0, 0x68, data, len, false);
}

void ds3231_write(uint8_t reg, uint8_t data) {
    uint8_t buffer[2] = {reg, data};
    i2c_write_blocking(i2c0, 0x68, buffer, 2, false);
}

uint8_t bcd2dec(uint8_t bcd) { return ((bcd >> 4) * 10) + (bcd & 0x0F); }

uint8_t dec2bcd(uint8_t dec) { return ((dec / 10) << 4) | (dec % 10); }

void ds3231_read_time(ds3231_time *t) {
    // uint8_t data[3];
    // ds3231_read(0x00, data, 3);        // Read sec, min, hour at once

    // t->sec  = bcd2dec(data[0] & 0x7F); // CH bit is bit 7
    // t->min  = bcd2dec(data[1] & 0x7F); // Mask unused bits
    // t->hour = bcd2dec(data[2] & 0x3F); // Mask for 24-hour mode

    uint8_t data[2];
    ds3231_read(Seconds_Reg, data, 1); // Read sec, min, hour at once

    t->sec = data[0];

    ds3231_read(Minutes_Reg, data, 1); // Read sec, min, hour at once

    t->min = data[0];
    ds3231_read(Hours_Reg, data, 1);   // Read sec, min, hour at once

    t->hour = data[0];
}

int main() {
    uart_init(uart0, 115200);
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);

    gpio_init(Blu_stat_pin);
    gpio_set_dir(Blu_stat_pin, GPIO_IN);
    gpio_pull_down(Blu_stat_pin);

    i2c_init(i2c_default, SSD1306_I2C_CLK);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

    /// SPI init
    spi_init(spi1, 2000000);
    spi_set_format(spi1, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    gpio_set_function(10, GPIO_FUNC_SPI);
    gpio_set_function(11, GPIO_FUNC_SPI);
    gpio_set_function(12, GPIO_FUNC_SPI);
    // gpio_set_function(13, GPIO_FUNC_SPI);

    gpio_init(SD_CS_PIN);
    gpio_set_dir(SD_CS_PIN, GPIO_OUT);
    gpio_pull_up(SD_CS_PIN);

    /// RTC init
    rtc_init();
    rtc_set_datetime(&DateTime);
    rtc_get_datetime(&DateTime);
    sleep_us(64);

    // rtc_enable_alarm();

    // rtc_get_datetime(&rtc_alarm);
    // sleep_us(70);

    // rtc_alarm.sec += 30;
    // if (rtc_alarm.sec >= 60) {
    //     rtc_alarm.sec -= 60;
    //     rtc_alarm.min += 1;
    // }
    // rtc_set_alarm(&rtc_alarm, RTC_Callback);

    gpio_init(Led);
    gpio_set_dir(Led, GPIO_OUT);

    gpio_put(Led, 0);

    // ADC Inint
    adc_init();
    adc_set_clkdiv(48000); // 48,000,000/48,000 : 1,000 sps
    adc_gpio_init(26);
    adc_set_temp_sensor_enabled(true);

    // LCD Init
    LCD_init();
    SetXY(0, 0);
    ClearLcd(0, 0);

    adc_select_input(0);
    moving_average_init(&mof, NTC_ADC2Temperature(adc_read()), 4, 16);
    median_init(&NTC_ADC, adc_read());

    moving_average_init(&Cor, 0, 4, 16);

    adc_select_input(4);

    median_init(&Core_ADC, adc_read());

    // // Power up SD card
    // gpio_put(SD_CS_PIN, 1); // CS high (inactive)
    // sleep_ms(100);          // Wait for SD card to power up

    // // Send at least 74 clock cycles with CS high
    // for (int i = 0; i < 10; i++) {
    //     uint8_t dummy = 0xFF;
    //     spi_write_blocking(spi1, &dummy, 1);
    // }

    FATFS fs;
    FATFS *fs_p;
    DWORD fre_clust, fre_sect, tot_sect;
    FRESULT res;
    FRESULT fr;

    fr = f_mount(&fs, "0:", 1);

    if (fr != FR_OK) {
        // Show error pattern
        sprintf(Data, "fr: %d", fr);
        draw_text(Data, 0, (scale * Font_H * 2), scale, deph);
        SendBuffer(Buffer);
        ClearBuffer(Buffer);

        while (1) {
            gpio_put(Led, 1);
            sleep_ms(500);

            gpio_put(Led, 0);
            sleep_ms(500);
        }
    }

    res = f_getfree("0:", &fre_clust, &fs_p);

    if (res) {
        sprintf(Data, "res: %d", res);
        draw_text(Data, 0, (scale * Font_H * 1), scale, deph);

        SendBuffer(Buffer);
        ClearBuffer(Buffer);

        while (1) {
            gpio_put(Led, 1);
            sleep_ms(10);

            gpio_put(Led, 0);
            sleep_ms(10);
        }
    }

    // Calculate sizes
    tot_sect = (fs_p->n_fatent - 2) * fs_p->csize;
    fre_sect = fre_clust * fs_p->csize;

    // Convert to MB (assuming 512-byte sectors)
    // 1 MB = 1024 KB = 1024 * 1024 bytes
    // Total bytes = tot_sect * 512
    // Total MB = (tot_sect * 512) / (1024 * 1024) = tot_sect / 2048
    // Show GB with 1 decimal place
    uint32_t total_gb_int  = tot_sect / 2097152;
    uint32_t total_gb_frac = ((tot_sect % 2097152) * 10) / 2097152;

    sprintf(Data, "Total: %lu.%lu GB", total_gb_int, total_gb_frac);
    draw_text(Data, 0, (scale * Font_H * 0), scale, deph);

    uint32_t free_gb_int  = fre_sect / 2097152;
    uint32_t free_gb_frac = ((fre_sect % 2097152) * 10) / 2097152;

    sprintf(Data, "Free: %lu.%lu GB", free_gb_int, free_gb_frac);
    draw_text(Data, 0, (scale * Font_H * 1), scale, deph);

    SendBuffer(Buffer);
    ClearBuffer(Buffer);

    ds3231_time t;

    // t.sec = 0x30;
    // Ds3231_SetTime(&t, Bin);

    // uint8_t data[2];

    // data[0] = Status_Reg;
    // data[1] = 0;

    // i2c_write_blocking_until(i2c_bus, ds3231_add, &data[0], 1, true, 20000);
    // i2c_read_blocking_until(i2c_bus, ds3231_add, &data[1], 1, false, 20000);

    // sprintf(Data, "reg:%x", data[1]);
    // draw_text(Data, 0, (scale * Font_H * 1), scale, deph);

    // uart_puts(uart0, Data);
    // data[0] = Control_Reg;
    // data[1] = 0;

    // i2c_write_blocking_until(i2c_bus, ds3231_add, &data[0], 1, true, 20000);
    // i2c_read_blocking_until(i2c_bus, ds3231_add, &data[1], 1, false, 20000);

    // sprintf(Data, "reg:%x", data[1]);
    // draw_text(Data, 0, (scale * Font_H * 0), scale, deph);

    // uart_puts(uart0, Data);

    // SendBuffer(Buffer);
    // ClearBuffer(Buffer);

    // // sleep_ms(5000);

    // data[1] = (data[1] & (~(1 << 7)));

    // i2c_write_blocking_until(i2c_bus, ds3231_add, data, 2, false, 200);

    // Ds3231_GetTime(&t, Bcd);

    // t.sec = Generic_i2c_Resive(Seconds_Reg);

    // Ds3231_GetTime(&t, Bin);

    // sprintf(Data, "time: %02d:%02d:%02d", t.hour, t.min, t.sec);
    // draw_text(Data, 0, (scale * Font_H * 1), scale, deph);

    // SendBuffer(Buffer);
    // ClearBuffer(Buffer);

    // Set minutes to 59
    uint8_t min_data[2] = {Minutes_Reg, 0x59}; // 59 minutes (valid BCD)
    Generic_i2c_Send(min_data, 2);

                                               // Set seconds to 50
    uint8_t sec_data[2] = {Seconds_Reg, 0x50}; // 50 seconds (valid BCD)
    Generic_i2c_Send(sec_data, 2);

    uint8_t d[2] = {Hours_Reg, 0x11};

    Generic_i2c_Send(d, 2);

    d[1] = Generic_i2c_Resive(Hours_Reg);

    d[1] = d[1] | (1 << 6);

    Generic_i2c_Send(d, 2);

    while (1) {
        Ds3231_GetTime(&t, Bin);
        // ds3231_read_time(&t);

        // t.sec = Generic_i2c_Resive(Seconds_Reg);

        sprintf(Data, "Time: %02d:%02d:%02d  %d\r\n", t.hour, t.min, t.sec, t.AM_PM);
        draw_text(Data, 0, (scale * Font_H * 0), scale, deph);

        SendBuffer(Buffer);
        ClearBuffer(Buffer);

        uart_puts(uart0, Data);
        sleep_ms(50);
    }

    while (1) {
        //// Core temperture start
        adc_select_input(4);
        ADC_raw     = median_update(&Core_ADC, adc_read());
        ADC_Voltage = conversion_factor * ADC_raw;
        Core_Temp   = (float) (moving_average_update(&Cor, (int32_t) ((27 - (ADC_Voltage - 0.706) / 0.001721) * 100.0)) / 100.0);

        //// Core temperture end

        //// NTC temperture start
        adc_select_input(0);
        temp = moving_average_update(&mof, NTC_ADC2Temperature(median_update(&NTC_ADC, adc_read())));

        sprintf(Data, "CoreT:%0.1fC  NTCT:%0.0fC", Core_Temp, temp);
        draw_text(Data, 0, (scale * Font_H * 0), scale, deph);

        /// print time
        rtc_get_datetime(&DateTime);
        sleep_us(70);
        sprintf(Data, "time: %02d:%02d:%02d", DateTime.hour, DateTime.min, DateTime.sec);
        draw_text(Data, 0, (scale * Font_H * 1), scale, deph);

        // sprintf(Data, "alarm: %02d:%02d:%02d", rtc_alarm.hour, rtc_alarm.min, rtc_alarm.sec);
        // draw_text(Data, 0, (scale * Font_H * 2), scale, deph);

        // sprintf(Data, "%10lu KiB total drive space. %10lu KiB available.", tot_sect / 2, fre_sect / 2);
        // draw_text(Data, 0, (scale * Font_H * 2), scale, deph);

        //// NTC temperture end

        if (gpio_get(Blu_stat_pin) == 1) {
            // sprintf(Data, "%f\n\r", temp);
            // uart_puts(uart0, Data);

            gpio_put(Led, 1);
        }
        else {
            gpio_put(Led, 0);
        }

        SendBuffer(Buffer);
        ClearBuffer(Buffer);
    }
}

void RTC_Callback() {
    rtc_disable_alarm();

    sprintf(Data, "%f\n\r", temp);
    uart_puts(uart0, Data);

    rtc_get_datetime(&rtc_alarm);
    // sleep_us(70);

    rtc_alarm.sec += 30;
    if (rtc_alarm.sec >= 60) {
        rtc_alarm.sec -= 60;
        rtc_alarm.min += 1;
    }

    rtc_set_alarm(&rtc_alarm, RTC_Callback);
}