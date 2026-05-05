#include "main.h"

/// @brief LCD Controller parameters
#define Font_H 9
#define FontY(Line) (scale * Font_H * Line)
uint8_t scale = 1, deph = 1;
extern uint8_t Buffer[BufferSize];
char Data[50];

#define Led 25
#define Button1 16
#define Button2 17

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

uint16_t ADC;
uint16_t ADC_Buffer[8];

const float conversion_factor = 3.3f / (1 << 12);

uint16_t i = 0, ADC_raw;
float ADC_Voltage, Core_Temp, temp;

MAF mof;
MAF Cor;
MDF NTC_ADC;
MDF Core_ADC;

void RTC_Callback();
void init();

// Print temperature
void Ds3231_Print_Temp(int16_t fixed_temp, uint8_t *buff) {
    int8_t integer   = Ds3231_Get_Temp_Integer(fixed_temp);
    uint8_t fraction = Ds3231_Get_Temp_Fraction(fixed_temp);

    sprintf(buff, "RTC T: %d.%02d°C", integer, fraction);
}

ds3231_time t          = {.hour = 21, .min = 37, .time_format = _24hour_mode, .AM_PM = AM};
ds3231_date d          = {.day_w = 1, .day = 20, .month = 1, .year = 26};
ds3231_init in         = {.INT_SQW_Function = Intrupt, .Osc_onBat = 1, .SquareWaveFerq = _1hz, .SqWave_onBat = 0};
ds3231_Alarm1 ds_alarm = {.time_format = _24hour_mode, .A1_hour = 21, .A1_min = 38, .A1_sec = 30};

datetime_t DateTime  = {.year = 2026, .month = 4, .day = 26, .hour = 13, .min = 30};
datetime_t rtc_alarm = {};

int16_t NTC_ADC2Temperature(uint16_t adc_value);

uint8_t last_sec = 0;
int dma_adc;
int dma_adc_controll;

int main() {
    init();

    Ds3231_Init(&in);
    // Ds3231_SetTime(&t, Bin);
    // Ds3231_SetDate(&d, Bin);

    // Ds3231_SetAlarm1(&ds_alarm, sec_match, Bin);
    // Ds3231_Reset_Alarm_Flag(Alarm1);
    // Ds3231_SetAlarm_Interrupt(Alarm1, 1);

    while (1) {
        Ds3231_GetTime(&t, Bin);
        Ds3231_GetDate(&d, Bin);

        /*
                //// Core temperature
                adc_select_input(4);
                ADC_raw     = median_update(&Core_ADC, adc_read());
                ADC_Voltage = conversion_factor * ADC_raw;
                // Core_Temp   = (float) (moving_average_update(&Cor, (int32_t) ((27 - (ADC_Voltage - 0.706) / 0.001721) * 100.0)) / 100.0);
                Core_Temp = ((27 - (ADC_Voltage - 0.706) / 0.001721));

                //// NTC temperature
                adc_select_input(0);
                // temp = moving_average_update(&mof, NTC_ADC2Temperature(median_update(&NTC_ADC, adc_read())));
                temp = NTC_ADC2Temperature(median_update(&NTC_ADC, adc_read()));

                sprintf(Data, "CoreT:%0.1fC  NTCT:%0.0fC", Core_Temp, temp);
                draw_text(Data, 0, (scale * Font_H * 0), scale, deph);
        */

        if (t.time_format == _12hour_mode) {
            if (t.AM_PM)
                sprintf(Data, "Time: %02d:%02d:%02d  %s\r\n", t.hour, t.min, t.sec, "PM");
            else
                sprintf(Data, "Time: %02d:%02d:%02d  %s\r\n", t.hour, t.min, t.sec, "AM");
        }
        else {
            sprintf(Data, "Time: %02d:%02d:%02d\r\n", t.hour, t.min, t.sec);
        }

        draw_text(Data, 0, (scale * Font_H * 1), scale, deph);
        // sprintf(Data, "Date: 20%02d/%02d/%02d \r\n", d.year, d.month, d.day);
        // draw_text(Data, 0, (scale * Font_H * 2), scale, deph);

        // Ds3231_Print_Temp(Ds3231_Read_Temp(), Data);
        // draw_text(Data, 0, (scale * Font_H * 2), scale, deph);

        // adc_run(false);
        // adc_fifo_drain();
        // dma_channel_abort(dma_adc);

        // adc_select_input(0); // Select ADC0

        // dma_channel_start(dma_adc);
        // adc_run(true);

        // dma_channel_wait_for_finish_blocking(dma_adc);

        // temp = NTC_ADC2Temperature(median_update(&NTC_ADC, ADC_Buffer[0]));

        // sprintf(Data, "NTC T:%0.0fC", temp);
        // draw_text(Data, 0, (scale * Font_H * 0), scale, deph);

        // sprintf(Data, "[0]: %x", ADC_Buffer[0]);
        // draw_text(Data, 0, (scale * Font_H * 2), scale, deph);

        // sprintf(Data, "[1]: %x", ADC_Buffer[1]);
        // draw_text(Data, 0, (scale * Font_H * 0), scale, deph);

        SendBuffer(Buffer);
    }
}

void init() {
    /// I2C init
    i2c_init(i2c1, SSD1306_I2C_CLK);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    /// SPI init
    spi_init(spi1, 2000000);
    spi_set_format(spi1, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    gpio_set_function(SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_RX_PIN, GPIO_FUNC_SPI);

    gpio_init(SPI_CSN_PIN);
    gpio_set_dir(SPI_CSN_PIN, GPIO_OUT);
    gpio_pull_up(SPI_CSN_PIN);

    // Gpio Inint
    gpio_init(Led);
    gpio_set_dir(Led, GPIO_OUT);
    gpio_put(Led, 0);

    gpio_init(Button1);
    gpio_set_dir(Button1, GPIO_IN);
    gpio_pull_up(Button1);

    gpio_init(Button2);
    gpio_set_dir(Button2, GPIO_IN);
    gpio_pull_up(Button2);

    // ADC Inint
    adc_init();
    adc_set_clkdiv(48000); // (48,000,000/48,000)/96  : 10 sps
    adc_gpio_init(26);
    adc_set_temp_sensor_enabled(true);

    adc_set_round_robin((1 << 0) | (1 << 3));
    adc_fifo_setup(true, true, 7, false, false);
    adc_run(true);

    // DMA Inint
    dma_adc              = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(dma_adc);

    // 3. Configure the DMA Channel for this specific transfer
    channel_config_set_transfer_data_size(&c, DMA_SIZE_16);
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);
    channel_config_set_dreq(&c, DREQ_ADC);

    // channel_config_set_chain_to(&c, dma_adc); // Chain to ITSELF for continuous loop!
    channel_config_set_ring(&c, true, 7); // Ring buffer wrap (adjust size as needed)

    // Set up interrupt on completion
    // dma_channel_set_irq0_enabled(dma_adc, true);
    // irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    // irq_set_enabled(DMA_IRQ_0, true);

    // 4. Configure and start the DMA transfer
    dma_channel_configure(dma_adc,       // Channel to configure
                          &c,            // Channel config
                          ADC_Buffer,    // Write address:
                          &adc_hw->fifo, // Read address:
                          7,             // Total number of transfers to perform
                          false          // Start immediately
    );

    // dma_adc_controll      = dma_claim_unused_channel(true);
    // dma_channel_config c1 = dma_channel_get_default_config(dma_adc_controll);

    // // 3. Configure the DMA Channel for this specific transfer
    // channel_config_set_transfer_data_size(&c1, DMA_SIZE_32);
    // channel_config_set_read_increment(&c1, false);
    // channel_config_set_write_increment(&c1, false);
    // channel_config_set_chain_to(&c1, dma_adc);

    // // 4. Configure and start the DMA transfer
    // dma_channel_configure(dma_adc_controll, &c1, &dma_hw->ch[dma_adc].write_addr, ADC_Buffer, 1, false);

    // LCD Init
    LCD_init();
    SetXY(0, 0);
    ClearLcd(0, 0);

    // NTC ADC 0 input filters init
    adc_select_input(0);
    median_init(&NTC_ADC, adc_read());
    moving_average_init(&mof, NTC_ADC2Temperature(median_update(&NTC_ADC, adc_read())), 4, 16);

    // Core temperature ADC 4 input filters init
    adc_select_input(4);
    ADC_raw     = median_update(&Core_ADC, adc_read());
    ADC_Voltage = conversion_factor * ADC_raw;
    moving_average_init(&Cor, (int32_t) ((27 - (ADC_Voltage - 0.706) / 0.001721) * 100.0), 4, 16);
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

int16_t NTC_ADC2Temperature(uint16_t adc_value) {
    int8_t NTC_table[10] = {-36, -14, 0, 10, 20, 31, 45, 68, 165};

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

/*
void sd_init() {
    FATFS fs;
    FATFS *fs_p;
    DWORD fre_clust, fre_sect, tot_sect;
    FRESULT res;

    res = f_mount(&fs, "0:", 1);

    if (res != FR_OK) {
        // Show error pattern
        sprintf(Data, "fr: %d", res);
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

    // 1 MB = 1024 KB = 1024 * 1024 bytes
    // Total bytes = tot_sect * 512
    // Total MB = (tot_sect * 512) / (1024 * 1024) = tot_sect / 2048
    // Show GB with 1 decimal place
    uint32_t total_gb_int  = tot_sect >> 20;
    uint32_t total_gb_frac = ((tot_sect % 2097152) * 10) >> 20;

    sprintf(Data, "Total: %lu.%lu GB", total_gb_int, total_gb_frac);
    draw_text(Data, 0, (scale * Font_H * 0), scale, deph);

    uint32_t free_gb_int  = fre_sect >> 20;
    uint32_t free_gb_frac = ((fre_sect % 2097152) * 10) >> 20;

    sprintf(Data, "Free: %lu.%lu GB", free_gb_int, free_gb_frac);
    draw_text(Data, 0, (scale * Font_H * 1), scale, deph);

    SendBuffer(Buffer);
    ClearBuffer(Buffer);

    // move to root 0:
    res = f_chdir("0:");

    if (res) {
        sprintf(Data, "f_chdir(0:): %d", res);
        draw_text(Data, 0, (scale * Font_H * 0), scale, deph);

        SendBuffer(Buffer);
        ClearBuffer(Buffer);

        while (1) {
            gpio_put(Led, 1);
            sleep_ms(10);

            gpio_put(Led, 0);
            sleep_ms(10);
        }
    }

    FILINFO fno;
    FIL file;
    DIR dir;
    char fname[50] = "0:Log-Output/temperature-Log-Output.csv";

    // Check if dir exist
    res = f_stat(fname, &fno);

    switch (res) {
        case FR_NO_FILE :

            res = f_open(&file, fname, FA_CREATE_ALWAYS | FA_WRITE);

            if (res) {
                sprintf(Data, "f_open: %d", res);
                draw_text(Data, 0, (scale * Font_H * 0), scale, deph);

                SendBuffer(Buffer);
                ClearBuffer(Buffer);

                while (1) {
                    gpio_put(Led, 1);
                    sleep_ms(10);

                    gpio_put(Led, 0);
                    sleep_ms(10);
                }
            }

            if (! f_printf(&file, "Timestamp,NTC Temperature, Core Temperature, RTC Temperature\n")) {
                sprintf(Data, "f_printf: %d", res);
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
            f_close(&file);
            f_closedir(&dir);

            break;

        case FR_NO_PATH :

            res = f_mkdir("Log-Output");
            if (res == FR_OK) {
                // res = f_chdir("0:Log-Output");
                // if (! res)
                res = f_open(&file, fname, FA_CREATE_ALWAYS | FA_WRITE);

                if (res) {
                    sprintf(Data, "f_open: %d", res);
                    draw_text(Data, 0, (scale * Font_H * 0), scale, deph);

                    SendBuffer(Buffer);
                    ClearBuffer(Buffer);

                    while (1) {
                        gpio_put(Led, 1);
                        sleep_ms(10);

                        gpio_put(Led, 0);
                        sleep_ms(10);
                    }
                }

                if (! f_printf(&file, "Timestamp,NTC Temperature, Core Temperature, RTC Temperature\n")) {
                    sprintf(Data, "f_printf: %d", res);
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

                f_close(&file);
                f_closedir(&dir);
            }

            else {
                sprintf(Data, "f_mkdir: %d", res);
                draw_text(Data, 0, (scale * Font_H * 0), scale, deph);

                SendBuffer(Buffer);
                ClearBuffer(Buffer);

                while (1) {
                    gpio_put(Led, 1);
                    sleep_ms(10);

                    gpio_put(Led, 0);
                    sleep_ms(10);
                }
            }

            break;
    }

    res = f_open(&file, fname, FA_OPEN_APPEND | FA_WRITE);

    if (res) {
        sprintf(Data, "f_open: %d", res);
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

    uint8_t last_sec = 0;
    while (1) {
        Ds3231_GetTime(&t, Bin);

        //// Core temperature start
        adc_select_input(4);
        ADC_raw     = median_update(&Core_ADC, adc_read());
        ADC_Voltage = conversion_factor * ADC_raw;
        Core_Temp   = (float) (moving_average_update(&Cor, (int32_t) ((27 - (ADC_Voltage - 0.706) / 0.001721) * 100.0)) / 100.0);

        //// NTC temperature start
        adc_select_input(0);
        temp = moving_average_update(&mof, NTC_ADC2Temperature(median_update(&NTC_ADC, adc_read())));

        sprintf(Data, "CoreT:%0.1fC NTCT:%0.0fC", Core_Temp, temp);
        draw_text(Data, 0, (scale * Font_H * 0), scale, deph);

        // t.sec = Generic_i2c_Read(Seconds_Reg);
        if (t.time_format == _12hour_mode) {
            if (t.AM_PM)
                sprintf(Data, "Time: %02d:%02d:%02d  %s\r\n", t.hour, t.min, t.sec, "PM");
            else
                sprintf(Data, "Time: %02d:%02d:%02d  %s\r\n", t.hour, t.min, t.sec, "AM");
        }
        else {
            sprintf(Data, "Time: %02d:%02d:%02d\r\n", t.hour, t.min, t.sec);
        }

        draw_text(Data, 0, (scale * Font_H * 1), scale, deph);

        Ds3231_Print_Temp(Ds3231_Read_Temp(), Data);

        // sprintf(Data, "Temp: %02d\r\n", Ds3231_Read_Temp());
        draw_text(Data, 0, (scale * Font_H * 2), scale, deph);

        SendBuffer(Buffer);
        ClearBuffer(Buffer);

        // uart_puts(uart0, Data);
        // sleep_ms(100);

        // sprintf(Data,  "%02d:%02d:%02d, %02d\n", t.hour, t.min, t.sec, Ds3231_Get_Temp_Integer(Ds3231_Read_Temp()));
        if (last_sec != t.min)
            f_printf(&file, "%02d:%02d:%02d,%02d C,%02d C,%02u.%02d C\n", t.hour, t.min, t.sec, (int8_t) temp, (int8_t) Core_Temp,
                     Ds3231_Get_Temp_Integer(Ds3231_Read_Temp()), Ds3231_Get_Temp_Fraction(Ds3231_Read_Temp()));

        if (gpio_get(Button)) {
            break;
        }

        last_sec = t.min;
    }

    gpio_put(Led, 1);
    f_close(&file);

    while (1)
        ;
}
*/