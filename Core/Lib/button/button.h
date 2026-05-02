#ifndef BUTTON_H_
#define BUTTON_H_

#include "stdint.h"

typedef struct {
    uint32_t Deb_Counter;
    uint8_t key_Press_Flag, key_Hold_Flagm, key_DPress_Flag;
    uint8_t Deb_Press, Deb_Hold, Key_flag, en_flag;

} key;

void KeyDetect(key *root, uint32_t input) {
    if (root->en_flag == 0U) {
        if (input == 0U) {
            root->Deb_Counter++;

            if ((root->Deb_Counter > root->Deb_Hold) && (root->Key_flag == 0U)) {
                root->key_Hold_Flag = 1;
                root->Key_flag      = 1;
                root->Deb_Counter   = 0;
            }
        }
        else {
            if ((root->Deb_Counter > root->Deb_Press) && (root->Key_flag == 0U)) {
                root->key_Press_Flag = 1;
            }
            else
                root->Key_flag = 0;

            root->Deb_Counter = 0;
        }
    }
    else
        return;
}

// if (Rotary_key.key_Press_Flag) {
//     Rotary_key.key_Press_Flag = 0;

//     CurrentMenu = MAIN;
// }
// else if (Rotary_key.key_Hold_Flag) {
//     Rotary_key.key_Hold_Flag = 0;

//     CurrentMenu = MAIN;
// }

#endif