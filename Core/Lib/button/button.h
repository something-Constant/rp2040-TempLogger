#ifndef BUTTON_H_
#define BUTTON_H_

#include <stdint.h>

#define Low 0U
#define High 1U

#define input_active Low

struct _key {
    uint8_t Deb_Counter;
    uint8_t Timeout_Counter, Timeout_Hold;
    uint8_t key_Hold_Flag;
    uint8_t Deb_Press, Deb_Hold, Deb_DPress, output;
};
typedef struct _key key;

enum _Key_State { NotActive, key_Press, key_DPress, key_Hold };
typedef enum _Key_State Key_State;

void KeyIint(key *root, uint8_t Deb_Press, uint8_t Deb_DPress, uint8_t Deb_Hold);
Key_State KeyDetect(key *root, uint32_t input, uint8_t status);

#endif