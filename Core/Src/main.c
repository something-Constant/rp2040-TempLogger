#include "main.h"

/// @brief LCD Controller parameters
#define Font_H 9
#define FontY(Line) (scale * Font_H * Line)
uint8_t scale = 1, deph = 1;
extern uint8_t Buffer[BufferSize];
char Data[50];

/// function prototype
void init();
void adc_dma_handler();
int16_t NTC_ADC2Temperature(uint16_t adc_value);
void Ds3231_Print_Temp(int16_t fixed_temp, uint8_t *buff);

ds3231_time t  = {.hour = 21, .min = 37, .time_format = _24hour_mode, .AM_PM = AM};
ds3231_date d  = {.day_w = 1, .day = 20, .month = 1, .year = 26};
ds3231_init in = {.INT_SQW_Function = Intrupt, .Osc_onBat = 1, .SquareWaveFerq = _1hz, .SqWave_onBat = 0};

ds3231_Alarm1 ds_alarm = {.time_format = _24hour_mode, .A1_hour = 21, .A1_min = 38, .A1_sec = 30};

MAF mof;
MAF Cor;
MDF NTC_ADC;
MDF Core_ADC;

/// buttons instanse
key key1, key2;

Key_State key1, key2;

int dma_adc;

uint16_t ADC_Buffer[2], NTC_Temperature;
const float conversion_factor = 3.3f / (1 << 12);
float ADC_Voltage, Core_Temp;

/// menus
enum menus { MAIN, SETTING, TIME_SETTING, DATE_SETTING, SAVEING_SETTING, FIRMWARE_INFO };
typedef enum menus MENUS;

MENUS menu = MAIN;

int main() {
    init();

    Ds3231_Init(&in);
    Ds3231_SetTime(&t, Bin);
    Ds3231_SetDate(&d, Bin);

    Ds3231_SetAlarm1(&ds_alarm, sec_match, Bin);
    Ds3231_SetAlarm_Interrupt(Alarm1, 1);
    Ds3231_Reset_Alarm_Flag(Alarm1);

    KeyIint(&key1, 1, 4, 50);
    KeyIint(&key2, 1, 4, 20);


    while (1) {
        Ds3231_GetTime(&t, Bin);
        Ds3231_GetDate(&d, Bin);

        if (gpio_get(Ds3231_Gpio_Interrupt) == 0U) {
            Ds3231_Reset_Alarm_Flag(Alarm1);
        }

        KeyDetect(&key1, gpio_get(Button1), 1);
        KeyDetect(&key2, gpio_get(Button1), 1);

        switch (menu) {
            case MAIN :

                if (KeyDetect(&key1, gpio_get(Button1), 1)) {}

                if (t.time_format == _12hour_mode) {
                    if (t.AM_PM)
                        sprintf(Data, "%02d:%02d:%02d  %s\r\n", t.hour, t.min, t.sec, "PM");
                    else
                        sprintf(Data, "%02d:%02d:%02d  %s\r\n", t.hour, t.min, t.sec, "AM");
                }
                else {
                    sprintf(Data, "%02d:%02d:%02d\r\n", t.hour, t.min, t.sec);
                }

                draw_text(Data, 0, (scale * Font_H * 1), 2, deph);

                sprintf(Data, "20%02d/%02d/%02d \r\n", d.year, d.month, d.day);
                draw_text(Data, 60, 23, scale, deph);

                // Ds3231_Print_Temp(Ds3231_Read_Temp(), Data);
                // draw_text(Data, 0, (scale * Font_H * 2), scale, deph);

                dma_channel_start(dma_adc);
                adc_run(true);
                dma_channel_wait_for_finish_blocking(dma_adc);

                //// Core temperature
                ADC_Voltage = conversion_factor * median_update(&Core_ADC, ADC_Buffer[1]);
                Core_Temp   = ((27 - (ADC_Voltage - 0.706) / 0.001721));

                //// NTC temperature
                NTC_Temperature = NTC_ADC2Temperature(median_update(&NTC_ADC, ADC_Buffer[0]));

                sprintf(Data, "Core:%02dC, Aria:%3dd", (uint8_t) Core_Temp, (uint8_t) NTC_Temperature);
                draw_text(Data, 0, (scale * Font_H * 0), scale, deph);

                break;

            case SETTING :
                break;

            case TIME_SETTING :

                if (t.time_format == _12hour_mode) {
                    if (t.AM_PM)
                        sprintf(Data, "%02d:%02d:%02d  %s\r\n", t.hour, t.min, t.sec, "PM");
                    else
                        sprintf(Data, "%02d:%02d:%02d  %s\r\n", t.hour, t.min, t.sec, "AM");
                }
                else {
                    sprintf(Data, "%02d:%02d:%02d\r\n", t.hour, t.min, t.sec);
                }

                draw_text(Data, 0, (scale * Font_H * 1), 2, deph);

                break;

            case DATE_SETTING :

                sprintf(Data, "20%02d/%02d/%02d \r\n", d.year, d.month, d.day);
                draw_text(Data, 60, 23, scale, deph);
                break;

            case SAVEING_SETTING :
                break;

            case FIRMWARE_INFO :
                break;
        }

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

    // RTC Gpio Interrupt Inint
    gpio_init(Ds3231_Gpio_Interrupt);
    gpio_set_dir(Ds3231_Gpio_Interrupt, GPIO_IN);
    gpio_pull_up(Ds3231_Gpio_Interrupt);

    // irq_set_exclusive_handler(IO_IRQ_BANK0, Ds3231_Alarm_handler);
    // gpio_set_irq_enabled(Ds3231_Gpio_Interrupt, GPIO_IRQ_EDGE_FALL, true);
    // irq_set_enabled(IO_IRQ_BANK0, true);

    // ADC Inint
    adc_init();
    adc_set_clkdiv(48000); // (48,000,000/48,000)/96  : 10 sps
    adc_gpio_init(26);
    adc_set_temp_sensor_enabled(true);

    adc_set_round_robin((1 << 0) | (1 << 4));
    adc_fifo_setup(true, true, 2, false, false);
    adc_select_input(0); // Select ADC0

    // DMA Inint
    dma_adc              = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(dma_adc);

    // 3. Configure the DMA Channel for this specific transfer
    channel_config_set_transfer_data_size(&c, DMA_SIZE_16);
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);
    channel_config_set_dreq(&c, DREQ_ADC);

    // channel_config_set_chain_to(&c, dma_adc); // Chain to ITSELF for continuous loop!
    // channel_config_set_ring(&c, true, 2); // Ring buffer wrap (adjust size as needed)

    // Set up interrupt on completion
    dma_channel_set_irq0_enabled(dma_adc, true);
    irq_set_exclusive_handler(DMA_IRQ_0, adc_dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    // 4. Configure and start the DMA transfer
    dma_channel_configure(dma_adc,       // Channel to configure
                          &c,            // Channel config
                          ADC_Buffer,    // Write address:
                          &adc_hw->fifo, // Read address:
                          2,             // Total number of transfers to perform
                          false          // Start immediately
    );

    // LCD Init
    LCD_init();
    SetXY(0, 0);
    ClearLcd(0, 0);

    dma_channel_start(dma_adc);
    adc_run(true);
    dma_channel_wait_for_finish_blocking(dma_adc);

    // NTC ADC 0 input filters init
    median_init(&NTC_ADC, ADC_Buffer[0]);
    moving_average_init(&mof, NTC_ADC2Temperature(median_update(&NTC_ADC, ADC_Buffer[0])), 4, 16);

    // Core temperature ADC 4 input filters init
    ADC_Voltage = conversion_factor * median_update(&Core_ADC, ADC_Buffer[1]);
    moving_average_init(&Cor, (int32_t) ((27 - (ADC_Voltage - 0.706) / 0.001721) * 100.0), 4, 16);
}

void adc_dma_handler() {
    dma_hw->ints0 = 1u << dma_adc;
    dma_channel_set_write_addr(dma_adc, ADC_Buffer, false);

    adc_run(false);
    adc_fifo_drain();
    dma_channel_abort(dma_adc);
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

void Ds3231_Print_Temp(int16_t fixed_temp, uint8_t *buff) {
    int8_t integer   = Ds3231_Get_Temp_Integer(fixed_temp);
    uint8_t fraction = Ds3231_Get_Temp_Fraction(fixed_temp);

    sprintf(buff, "RTC T: %d.%02d°C", integer, fraction);
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
        NTC_Temperature = moving_average_update(&mof, NTC_ADC2Temperature(median_update(&NTC_ADC, adc_read())));

        sprintf(Data, "CoreT:%0.1fC NTCT:%0.0fC", Core_Temp, NTC_Temperature);
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
            f_printf(&file, "%02d:%02d:%02d,%02d C,%02d C,%02u.%02d C\n", t.hour, t.min, t.sec, (int8_t) NTC_Temperature, (int8_t) Core_Temp,
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