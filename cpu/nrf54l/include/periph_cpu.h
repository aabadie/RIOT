/*
 * SPDX-FileCopyrightText: 2026 Inria
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @ingroup         cpu_nrf54l
 * @{
 *
 * @file
 * @brief           nRF54L specific definitions for handling peripherals
 *
 * @author          Alexandre Abadie <alexandre.abadie@inria.fr>
 */

#include "periph_cpu_common.h"
#include "macros/units.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief    Peripheral clock speed of the slow peripheral domains (TIMER2x,
 *           UARTE2x, ...)
 */
#define PERIPH_CLOCK        MHZ(16)

/**
 * @brief    The watchdog of the nRF54L family can be stopped (STOP task,
 *           unlocked via the TSEN register)
 */
#define WDT_HAS_STOP        (1)

#ifndef DOXYGEN
/**
 * @brief   Override the ADC resolution values
 * @{
 */
#define HAVE_ADC_RES_T
typedef enum {
    ADC_RES_6BIT  = 0xf0,                       /**< not supported by hardware */
    ADC_RES_8BIT  = SAADC_RESOLUTION_VAL_8bit,  /**< ADC resolution: 8 bit */
    ADC_RES_10BIT = SAADC_RESOLUTION_VAL_10bit, /**< ADC resolution: 10 bit */
    ADC_RES_12BIT = SAADC_RESOLUTION_VAL_12bit, /**< ADC resolution: 12 bit */
    ADC_RES_14BIT = SAADC_RESOLUTION_VAL_14bit, /**< ADC resolution: 14 bit */
    ADC_RES_16BIT = 0xf2                        /**< not supported by hardware */
} adc_res_t;
/** @} */
#endif /* ndef DOXYGEN */

#ifdef __cplusplus
}
#endif

/** @} */
