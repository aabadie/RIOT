/*
 * Copyright (C) 2019 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/* This file adapts the accerelometer handler example with the lsm6dsl
   accelerometer sensor available in RIOT. */

#include <math.h>
#include <string.h>
#include <stdio.h>

#include "tensorflow/lite/micro/examples/magic_wand/accelerometer_handler.h"

#include "tensorflow/lite/micro/examples/magic_wand/constants.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "lsm6dsl.h"
#include "lsm6dsl_params.h"

#define ENABLE_DEBUG        (0)
#include "debug.h"
#ifdef __cplusplus
}
#endif

#define DATA_ARRAY_SIZE     (600)

int begin_index = 0;
static lsm6dsl_t imu;

TfLiteStatus SetupAccelerometer(tflite::ErrorReporter* error_reporter)
{
    if (lsm6dsl_init(&imu, &lsm6dsl_params[0]) != LSM6DSL_OK) {
      error_reporter->Report("Failed to initialize IMU");
      return kTfLiteError;
    }

    lsm6dsl_acc_power_up(&imu);

    error_reporter->Report("Magic starts!");

    return kTfLiteOk;
}

bool ReadAccelerometer(tflite::ErrorReporter* error_reporter, float* input,
                       int length, bool reset_buffer)
{
    lsm6dsl_3d_data_float_t acc_value;
    if (lsm6dsl_read_acc_float(&imu, &acc_value) != LSM6DSL_OK) {
        error_reporter->Report("Failed to read data");
        return false;
    }

    input[begin_index++] = acc_value.x;
    input[begin_index++] = acc_value.y;
    input[begin_index++] = acc_value.z;

    DEBUG("Index: %i, Data - x: %i.%i, y: %i.%i, z: %i.%i\n", (begin_index - 2),
          (int)acc_value.x, (int)(acc_value.x * 1000) % 1000,
          (int)acc_value.y, (int)(acc_value.y * 1000) % 1000,
          (int)acc_value.z, (int)(acc_value.z * 1000) % 1000);

    if (begin_index >= DATA_ARRAY_SIZE) {
        begin_index = 0;
    }

    return true;
}
