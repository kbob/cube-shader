#include "noise.h"

#include <stdlib.h>

void init_noise(void)
{
    srandom(69069);             // Historical reasons
}

void make_noise_f(float *out, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        out[i] = (float) ((double)random() / (double)RAND_MAX);
    }
}
