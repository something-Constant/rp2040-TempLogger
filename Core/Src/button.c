#include "main.h"

/// @brief LCD Controller parameters
#define Font_H 9
#define FontY(Line) (scale * Font_H * Line)
uint8_t scale = 1, deph = 1;
extern uint8_t Buffer[BufferSize];
char Data[50];

#define SSD1306_I2C_CLK 400000 // 400khz
#define Led 25
#define Button 16
#define Blu_stat_pin 6

struct _key {
    uint8_t Deb_Counter;
    uint8_t Timeout_Counter, Timeout_Hold;
    uint8_t key_Hold_Flag;
    uint8_t Deb_Press, Deb_Hold, Deb_DPress, output;
};
typedef struct _key key;

enum _Key_State { NotActive, key_Press = 1, key_DPress, key_Hold };
typedef enum _Key_State Key_State;

uint8_t KeyDetect(key *root, uint32_t input, uint8_t status) {
    if (status == 1U) {
        if (input == 1) {
            root->Deb_Counter++;
            root->Timeout_Hold = 0;

            if (root->Deb_Counter > root->Deb_Hold) {
                root->key_Hold_Flag   = 1;
                root->Timeout_Counter = 0;
                root->Deb_Counter     = 0;
                root->output          = key_Hold;
            }
        }
        else {
            if ((root->Deb_Counter > root->Deb_Press) && (root->key_Hold_Flag != 1)) {
                root->Timeout_Counter++;

                if (root->Timeout_Counter > root->Deb_DPress) {
                    // root->key_Press_Flag  = 1;
                    root->Deb_Counter     = 0;
                    root->Timeout_Counter = 0;
                    root->output          = key_Press;
                }

                else if ((root->Timeout_Counter > 1) && (root->Timeout_Hold != root->Deb_Counter)) {
                    // root->key_DPress_Flag = 1;
                    root->Deb_Counter     = 0;
                    root->Timeout_Counter = 0;
                    root->output          = key_DPress;
                }

                root->Timeout_Hold = root->Deb_Counter;
            }
            else {
                root->Timeout_Counter = 0;
                root->Deb_Counter     = 0;
                root->Timeout_Hold    = 0;
                root->key_Hold_Flag   = 0;
            }
        }

        return root->output;
    }
    else {
        return NotActive;
    }
}

void init();

key key1 = {.Deb_Press = 1, .Deb_Hold = 20, .Deb_DPress = 5};

int main() {
    init();
    uint8_t i, j, k = 1;

    while (1) {
        // switch (KeyDetect(&key1, gpio_get(Button), 1)) {
        //     case 1 :
        //         sprintf(Data, "key Press");
        //         draw_text(Data, 0, (scale * Font_H * 0), scale, deph);

        //         SendBuffer(Buffer);
        //         ClearBuffer(Buffer);
        //         sleep_ms(100);

        //         break;

        //     case 2 :
        //         sprintf(Data, "key DPress");
        //         draw_text(Data, 0, (scale * Font_H * 0), scale, deph);

        //         SendBuffer(Buffer);
        //         ClearBuffer(Buffer);
        //         sleep_ms(100);

        //         break;

        //     case 3 :
        //         sprintf(Data, "key Hold");
        //         draw_text(Data, 0, (scale * Font_H * 0), scale, deph);

        //         SendBuffer(Buffer);
        //         ClearBuffer(Buffer);
        //         sleep_ms(100);

        //         break;

        //     default :
        //         sprintf(Data, "key clear");
        //         draw_text(Data, 0, (scale * Font_H * 0), scale, deph);

        //         SendBuffer(Buffer);
        //         ClearBuffer(Buffer);
        //         break;
        // }

        // key1.output = 0;

        sprintf(Data, "%d", KeyDetect(&key1, gpio_get(Button), 1));
        draw_text(Data, 0, (scale * Font_H * 0), scale, deph);

        sprintf(Data, "i: %03d, j: %03d", key1.Deb_Counter, key1.Timeout_Counter);
        draw_text(Data, 0, (scale * Font_H * 1), scale, deph);

        sprintf(Data, "k: %03d", key1.Timeout_Hold);
        draw_text(Data, 0, (scale * Font_H * 2), scale, deph);

        SendBuffer(Buffer);
        ClearBuffer(Buffer);

        // sleep_ms(500);
        /*
                KeyDetect(&key1, gpio_get(Button));

                if (key1.key_Press_Flag == 1) {
                    sprintf(Data, "key Press");
                    draw_text(Data, 0, (scale * Font_H * 0), scale, deph);

                    SendBuffer(Buffer);
                    ClearBuffer(Buffer);
                    key1.key_Press_Flag = 0;
                }

                else if (key1.key_DPress_Flag == 1) {
                    sprintf(Data, "key DPress");
                    draw_text(Data, 0, (scale * Font_H * 0), scale, deph);

                    SendBuffer(Buffer);
                    ClearBuffer(Buffer);
                    key1.key_DPress_Flag = 0;
                }

                else if (key1.key_Hold_Flag == 1) {
                    sprintf(Data, "key Hold");
                    draw_text(Data, 0, (scale * Font_H * 0), scale, deph);

                    SendBuffer(Buffer);
                    ClearBuffer(Buffer);
                    key1.key_Hold_Flag = 0;
                }
        */
        /*


        if ((gpio_get(Button))) {
            k = 0;
            i++;

            // hold
            if ((i > key1.Deb_Hold)) {
                sprintf(Data, "key Hold");
                draw_text(Data, 0, (scale * Font_H * 0), scale, deph);
                i = 0;

                SendBuffer(Buffer);
                sleep_ms(500);
            }
        }
        else {
            /// press
            if ((i > key1.Deb_Press) && (i < key1.Deb_Hold)) {
                j++;

                if ((j > 5)) {
                    sprintf(Data, "key Press");
                    draw_text(Data, 0, (scale * Font_H * 0), scale, deph);

                    SendBuffer(Buffer);
                    sleep_ms(500);
                    i = 0;
                    j = 0;
                }

                if ((k != i) && (j > 1)) {
                    sprintf(Data, "key DPress");
                    draw_text(Data, 0, (scale * Font_H * 0), scale, deph);

                    SendBuffer(Buffer);
                    sleep_ms(500);
                    i = 0;
                }

                k = i;
            }
            else {
                i = 0;
                j = 0;
            }
        }

        sprintf(Data, "i: %03d, j: %03d", i, j);
        draw_text(Data, 0, (scale * Font_H * 1), scale, deph);

        sprintf(Data, "k: %03d", k);
        draw_text(Data, 0, (scale * Font_H * 2), scale, deph);

        SendBuffer(Buffer);
        ClearBuffer(Buffer);
*/
    }
}

void init() {
    i2c_init(i2c_default, SSD1306_I2C_CLK);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

    gpio_init(Button);
    gpio_set_dir(Button, GPIO_IN);
    gpio_pull_down(Button);

    // LCD Init
    LCD_init();
    SetXY(0, 0);
    ClearLcd(0, 0);
}