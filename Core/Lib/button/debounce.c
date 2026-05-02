#include "debounce.h"
/******************************************************************************/
void Debounce_Reset(debouncer_t *debouncer) {
    debouncer->DebCounter = 0;
    debouncer->lastInput  = 0;
    debouncer->lastOut    = 0;
}
/******************************************************************************/

// DebCounter → sample counter

// lastInput → last raw input

// lastOut → debounced output

/* out = in if in = in1 = in2 = ...= in for filter count */

__attribute__((optimize("-Ofast"))) uint32_t Debounce(debouncer_t *debouncer, uint32_t inp) {
    if (inp == debouncer->lastInput) {
        if (++debouncer->DebCounter >= debouncer->DebCount) {
            debouncer->lastOut    = inp;
            debouncer->DebCounter = 0;
        }
    }
    else
        debouncer->DebCounter = 0;

    debouncer->lastInput = inp;

    return debouncer->lastOut;
}
/************************ (C) COPYRIGHT IRANDYNAMICS *****END OF FILE****/
