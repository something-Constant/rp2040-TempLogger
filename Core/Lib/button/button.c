#include "button.h"

/**
 * @brief init for key instans
 *
 * @param root key instans
 * @param Deb_Press     amount of loop iteration for key to be press
 * @param Deb_DPress    amount of loop iteration for key timeout to double press
 * @param Deb_Hold      amount of loop iteration for key to be hold
 */
void KeyIint(key *root, uint8_t Deb_Press, uint8_t Deb_DPress, uint8_t Deb_Hold) {
    root->Deb_Press  = Deb_Press;
    root->Deb_DPress = Deb_DPress;
    root->Deb_Hold   = Deb_Hold;
}

/**
 * @brief return an statte of Key_State base on Deb_Press , Deb_DPress and hold Deb_Hold,
 *  dose not need ant timer work base on loot time
 *
 * @param root      key instans
 * @param input     kay satste, can be low active or high active by defining input_active.
 * @param status    activalte the output
 *
 * @return Key_State: NotActive, key_Press, key_DPress, key_Hold
 */
Key_State KeyDetect(key *root, uint32_t input, uint8_t status) {
    // if (status == 1U) {
    if (input == input_active) {
        root->Deb_Counter++;
        root->Timeout_Hold = 0;

        if (root->Deb_Counter >= root->Deb_Hold) {
            root->key_Hold_Flag   = 1;
            root->Timeout_Counter = 0;
            root->Deb_Counter     = 0;
            root->output          = key_Hold;
        }
    }
    else {
        if ((root->Deb_Counter >= root->Deb_Press) && (root->key_Hold_Flag != 1)) {
            root->Timeout_Counter++;

            if (root->Timeout_Counter >= root->Deb_DPress) {
                root->Deb_Counter     = 0;
                root->Timeout_Counter = 0;
                root->output          = key_Press;
            }

            else if ((root->Timeout_Counter > 1) && (root->Timeout_Hold != root->Deb_Counter)) {
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
    // }
    // else {
    //     return NotActive;
    // }
}