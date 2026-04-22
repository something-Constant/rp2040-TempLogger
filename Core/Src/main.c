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

    /// RTC init
    rtc_init();
    rtc_set_datetime(&DateTime);
    rtc_get_datetime(&DateTime);
    sleep_us(64);

    rtc_enable_alarm();

    rtc_get_datetime(&rtc_alarm);
    sleep_us(70);
    
    rtc_alarm.sec += 30;
    if (rtc_alarm.sec >= 60) {
        rtc_alarm.sec -= 60;
        rtc_alarm.min += 1;
    }
    rtc_set_alarm(&rtc_alarm, RTC_Callback);

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

    adc_select_input(0);
    moving_average_init(&mof, NTC_ADC2Temperature(adc_read()), 4, 16);
    median_init(&NTC_ADC, adc_read());

    moving_average_init(&Cor, 0, 4, 16);

    adc_select_input(4);
    median_init(&Core_ADC, adc_read());

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
        sprintf(Data, "%02d:%02d:%02d", DateTime.hour, DateTime.min, DateTime.sec);
        draw_text(Data, 0, (scale * Font_H * 1), scale, deph);

        sprintf(Data, "alarm: %02d:%02d:%02d", rtc_alarm.hour, rtc_alarm.min, rtc_alarm.sec);
        draw_text(Data, 0, (scale * Font_H * 2), scale, deph);

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