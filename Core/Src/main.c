// #include <stdio.h>
// #include <string.h>
// #include <stdlib.h>
// #include <ctype.h>
// #include "pico/stdlib.h"
// #include "pico/binary_info.h"
// // #include "../../Driver/ssd1306/ssd1306.h"
// #include <stdint.h>
// #include "ssd1306.h"
#include "main.h"

#define SSD1306_I2C_CLK 400000
// #define SSD1306_I2C_CLK 1000000

#define Led 25
#define Button 0

bool state = 0, flag = 0;
uint16_t i = 0;

char A[] = {0x1e, 0x28, 0x48, 0x88, 0x48, 0x28, 0x1e, 0x00};
extern uint8_t Buffer[BufferSize];

int main() {
    // stdio_init_all();

    // bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));

    i2c_init(i2c_default, SSD1306_I2C_CLK);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

    gpio_init(Led);
    gpio_set_dir(Led, GPIO_OUT);
    gpio_put(Led, 1);

    LCD_init();
    SetXY(0, 0);
    SetPixel(0, 0, 1);
    SetXY(0, 0);

    DrawLine(0, 0, 50, 100, Buffer);

    while (1) {
        gpio_put(Led, 1);
        sleep_ms(250);

        DrawLine(0, 0, 50, 100, Buffer);
        SendBuffer(Buffer);
    
        gpio_put(Led, 0);
        sleep_ms(250);
    }
}

// int main(){
// 	gpio_init(Led);
// 	gpio_init(Button);

// 	//gpio_set_dir(Button, false);
// 	gpio_set_dir(Button, GPIO_IN);
// 	gpio_set_dir(Led, GPIO_OUT);

// 	gpio_set_input_enabled(Button, true);

// 	gpio_pull_up(Button);
// 	gpio_is_pulled_up(Button);

// 	while(1){
// 		/*
// 		if((gpio_get(Button))&&(flag == 0)){
// 			state = 1;
// 			flag = 1;
// 		}

// 		if(state == 1)&&(flag == 1)){
// 			state = 0;
// 			flag = 0;
// 		}
// 		*/
// 		if(flag == 0){
// 			if(gpio_get(Button) == 0){
// 				state = 1;
// 				flag = 1;
// 				sleep_ms(150);
// 			}
// 		}

// 		if(flag == 1){
// 			if(gpio_get(Button) == 0){
// 				state = 0;
// 				flag = 0;
// 				sleep_ms(150);
// 			}
// 		}

// 		gpio_put(Led, state);

// 	}
// }
