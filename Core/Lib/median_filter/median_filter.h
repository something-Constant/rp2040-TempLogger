#ifndef MEDIAN_FILLTER_H
#define MEDIAN_FILLTER_H

#include <stdint.h>

#define Median_WINDOW 5

typedef struct {
    int32_t output;
    uint8_t window_size;
    uint8_t medianIndex;
    uint32_t medianBuffer[Median_WINDOW];

} MDF;

inline static void SwapIfGreater(uint32_t *a, uint32_t *b) {
    if (*a > *b) {
        *a = *a ^ *b;
        *b = *a ^ *b;
        *a = *a ^ *b;
    }
}

void sort5(uint32_t *data) {
    SwapIfGreater(&data[0], &data[1]);
    SwapIfGreater(&data[2], &data[3]);
    SwapIfGreater(&data[1], &data[3]);
    SwapIfGreater(&data[0], &data[2]);
    SwapIfGreater(&data[0], &data[1]);
    SwapIfGreater(&data[2], &data[3]);
    SwapIfGreater(&data[1], &data[2]);
    SwapIfGreater(&data[3], &data[4]);
    SwapIfGreater(&data[2], &data[3]);
    SwapIfGreater(&data[1], &data[2]);
}

void median_init(MDF *filt, uint32_t preinit) {
    // filt->output      = preinit;
    // filt->window_size = window_size;

    for (int8_t i = 4; i > 0; i--) {
        filt->medianBuffer[i] = preinit;
    }
}

uint32_t median_update(MDF *filt, uint32_t new_input) {
    static uint8_t flag = 0;

    filt->medianBuffer[filt->medianIndex] = new_input;

    filt->medianIndex++;

    if (filt->medianIndex >= Median_WINDOW) {
        filt->medianIndex = 0;
        flag              = 1;

        sort5(filt->medianBuffer);

        filt->output = (filt->medianBuffer[2]);
    }

    return filt->output;
}

#endif