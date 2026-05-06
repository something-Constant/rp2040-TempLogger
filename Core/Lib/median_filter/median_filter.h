#ifndef MEDIAN_FILLTER_H
#define MEDIAN_FILLTER_H

#include <stdint.h>

#define Median_WINDOW 5

typedef struct {
    uint8_t medianIndex;
    int32_t medianBuffer[Median_WINDOW];
    int32_t output;

} MDF;

inline static void SwapIfGreater(uint32_t *a, uint32_t *b) {
    if (*a > *b) {
        uint32_t temp = *a;
        *a            = *b;
        *b            = temp;
    }
}

void sort5(uint32_t *data) {
    SwapIfGreater(&data[0], &data[1]); // Step 1
    SwapIfGreater(&data[3], &data[4]); // Step 1
    SwapIfGreater(&data[2], &data[4]); // Step 2
    SwapIfGreater(&data[2], &data[3]); // Step 2
    SwapIfGreater(&data[0], &data[3]); // Step 3
    SwapIfGreater(&data[0], &data[2]); // Step 3
    SwapIfGreater(&data[1], &data[4]); // Step 4
    SwapIfGreater(&data[1], &data[3]); // Step 4
    SwapIfGreater(&data[1], &data[2]); // Step 5
}

void median_init(MDF *filt, int32_t preinit) {
    for (int8_t i = 4; i >= 0; i--) {
        filt->medianBuffer[i] = preinit;
    }

    sort5(filt->medianBuffer);
    filt->medianIndex = 0;
    filt->output      = filt->medianBuffer[2];
}

int32_t median_update(MDF *filt, int32_t new_input) {
    filt->medianBuffer[filt->medianIndex] = new_input;

    filt->medianIndex++;

    if (filt->medianIndex >= Median_WINDOW) {
        filt->medianIndex = 0;
        sort5(filt->medianBuffer);
        filt->output = filt->medianBuffer[2];
    }

    return filt->output;
}

#endif