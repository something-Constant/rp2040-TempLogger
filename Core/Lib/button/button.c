#include "button.h"

Key_State KeyDetect(key *root, uint32_t input, uint8_t status) {
    if (status == 1U) {
        if (input == input_active) {
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