#include "stdint.h"

typedef struct {
    uint32_t Deb_Counter;
    uint8_t key_Press_Flag, key_Hold_Flagm, key_DPress_Flag;
    uint8_t Deb_Press, Deb_Hold, Key_flag, en_flag;

} key;

enum KeyFlag { key_Press, key_Hold, key_DPress };
typedef enum KeyFlag Key_State;

Key_State i;

i = key_DPress;

void KeyDetect(key *root, uint32_t input) {}

// if (Rotary_key.key_Press_Flag) {
//     Rotary_key.key_Press_Flag = 0;

//     CurrentMenu = MAIN;
// }
// else if (Rotary_key.key_Hold_Flag) {
//     Rotary_key.key_Hold_Flag = 0;

//     CurrentMenu = MAIN;
// }
