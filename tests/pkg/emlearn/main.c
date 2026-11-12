/*
 * SPDX-FileCopyrightText: 2019 Inria
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Emlearn test application
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @}
 */

#include <inttypes.h>
#include <stdalign.h>
#include <stdio.h>

#include "model.h"

/* the digit array will be casted to float, so it has to meet that alignment */
alignas(float)
#include "blob/digit.h"

/* the digit is an image of 8x8 pixels, stored as floats */
#define DIGIT_PIXELS    (sizeof(digit) / sizeof(float))

int main(void)
{
    /* Use intermediate cast to uintptr_t to silence -Wcast-align. Since
     * we add the alignas() attribute, the alignment is ensured */
    const float *digit_as_float = (const float *)(uintptr_t)digit;

    /* the model takes integer features, the pixels are integers from 0 to 16 */
    int16_t features[DIGIT_PIXELS];
    for (unsigned i = 0; i < DIGIT_PIXELS; i++) {
        features[i] = (int16_t)digit_as_float[i];
    }

    printf("Predicted digit: %" PRIi32 "\n",
           model_predict(features, DIGIT_PIXELS));

    return 0;
}
