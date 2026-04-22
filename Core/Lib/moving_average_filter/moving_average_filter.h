#ifndef MOVING_AVERAGE_FILTER_H
#define MOVING_AVERAGE_FILTER_H

#include <stdint.h>

#define MOVING_AVG_WINDOW 16 // Must be power of 2

typedef struct {
    int32_t movingAvgSum;
    int32_t output;
    int8_t window_size;
    int8_t shift; // log2(window_size)
    int8_t movingAvgIndex;
    int32_t movingAvgBuffer[MOVING_AVG_WINDOW];
} MAF;

void moving_average_init(MAF *filt, int32_t preinit, int8_t shift, int8_t window_size) {
    filt->movingAvgSum   = preinit * window_size;
    filt->output         = preinit;
    filt->window_size    = window_size;
    filt->shift          = shift;
    filt->movingAvgIndex = 0;

    for (int8_t i = 0; i < window_size; i++) {
        filt->movingAvgBuffer[i] = preinit;
    }
}

int32_t moving_average_update(MAF *filt, int32_t new_input) {
    filt->movingAvgSum -= filt->movingAvgBuffer[filt->movingAvgIndex];
    filt->movingAvgBuffer[filt->movingAvgIndex] = new_input;
    filt->movingAvgSum += new_input;

    filt->movingAvgIndex++;
    if (filt->movingAvgIndex >= filt->window_size) {
        filt->movingAvgIndex = 0;
    }

    // Fast division using bit shift (only for power of 2 windows)
    filt->output = filt->movingAvgSum >> filt->shift;

    return filt->output;
}

#endif