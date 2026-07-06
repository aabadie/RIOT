/*
 * SPDX-FileCopyrightText: 2026 Inria
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     cpu_nrf54l
 * @ingroup     drivers_periph_adc
 * @{
 *
 * @file
 * @brief       ADC driver implementation for the nRF54L family, based on the
 *              SAADC peripheral
 *
 * The board configuration maps each ADC line to one of the analog capable
 * GPIOs (AIN0-AIN7, all located on port P1). All conversions are done on
 * SAADC channel 0, which is re-wired to the requested input for each sample.
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @}
 */

#include <assert.h>

#include "cpu.h"
#include "mutex.h"
#include "periph/adc.h"
#include "periph_conf.h"

/**
 * @name    Default ADC reference, gain configuration and timings
 *
 * Can be overridden by the board configuration if needed. The default
 * configuration uses the internal 0.9 V reference with a gain of 2/8,
 * resulting in a 3.6 V input range that covers the full supply range. The
 * input is sampled for 10 us, the conversion time is set to the maximum
 * (2 us), which is sufficient for all resolutions.
 * @{
 */
#ifndef ADC_REF
#define ADC_REF             SAADC_CH_CONFIG_REFSEL_Internal
#endif
#ifndef ADC_GAIN
#define ADC_GAIN            SAADC_CH_CONFIG_GAIN_Gain2_8
#endif
#ifndef ADC_TACQ
#define ADC_TACQ            (79U)   /* (79 + 1) * 125 ns = 10 us */
#endif
#ifndef ADC_TCONV
#define ADC_TCONV           (7U)    /* (7 + 1) * 250 ns = 2 us */
#endif
/** @} */

/* port/pin encoding of gpio_t, see GPIO_PIN() */
#define PORT_POS            (5U)
#define PIN_MASK            (0x1f)

/**
 * @brief   Lock to prevent concurrency issues when used from different threads
 */
static mutex_t lock = MUTEX_INIT;

/**
 * @brief   We use a static result buffer so we do not have to reprogram the
 *          result pointer register
 */
static int16_t result;

static inline void prep(void)
{
    mutex_lock(&lock);
    NRF_SAADC->ENABLE = SAADC_ENABLE_ENABLE_Enabled;
}

static inline void done(void)
{
    NRF_SAADC->ENABLE = SAADC_ENABLE_ENABLE_Disabled;
    mutex_unlock(&lock);
}

/* the PSELP PIN and PORT fields hold the plain GPIO pin and port numbers */
static inline uint32_t pselp(gpio_t pin)
{
    return (SAADC_CH_PSELP_CONNECT_AnalogInput << SAADC_CH_PSELP_CONNECT_Pos) |
           ((pin >> PORT_POS) << SAADC_CH_PSELP_PORT_Pos) |
           ((pin & PIN_MASK) << SAADC_CH_PSELP_PIN_Pos);
}

int adc_init(adc_t line)
{
    if (line >= ADC_NUMOF) {
        return -1;
    }

    prep();

    /* prevent multiple initialization by checking the result ptr register */
    if (NRF_SAADC->RESULT.PTR != (uint32_t)&result) {
        /* set the result buffer, MAXCNT counts bytes (one sample is two) */
        NRF_SAADC->RESULT.MAXCNT = sizeof(result);
        NRF_SAADC->RESULT.PTR = (uint32_t)&result;

        /* configure the first channel (the only one we use):
         * - single ended mode, negative input shorted to GND
         * - reference, gain and timings as defined by the board (or the
         *   defaults above)
         * - no oversampling */
        NRF_SAADC->CH[0].CONFIG =
            (ADC_GAIN << SAADC_CH_CONFIG_GAIN_Pos) |
            (ADC_REF << SAADC_CH_CONFIG_REFSEL_Pos) |
            (SAADC_CH_CONFIG_MODE_SE << SAADC_CH_CONFIG_MODE_Pos) |
            (ADC_TACQ << SAADC_CH_CONFIG_TACQ_Pos) |
            (ADC_TCONV << SAADC_CH_CONFIG_TCONV_Pos);
        NRF_SAADC->CH[0].PSELN = 0;
        NRF_SAADC->OVERSAMPLE = 0;

        /* calibrate SAADC */
        NRF_SAADC->EVENTS_CALIBRATEDONE = 0;
        NRF_SAADC->TASKS_CALIBRATEOFFSET = 1;
        while (NRF_SAADC->EVENTS_CALIBRATEDONE == 0) {}
    }

    done();

    return 0;
}

int32_t adc_sample(adc_t line, adc_res_t res)
{
    assert(line < ADC_NUMOF);

    /* check if resolution is valid */
    if (res > ADC_RES_14BIT) {
        return -1;
    }

    /* prepare device */
    prep();

    /* set resolution */
    NRF_SAADC->RESOLUTION = res;
    /* select the input to sample */
    NRF_SAADC->CH[0].PSELP = pselp(adc_config[line]);

    /* start the SAADC and wait for the started event */
    NRF_SAADC->EVENTS_STARTED = 0;
    NRF_SAADC->TASKS_START = 1;
    while (NRF_SAADC->EVENTS_STARTED == 0) {}

    /* trigger the actual conversion */
    NRF_SAADC->EVENTS_END = 0;
    NRF_SAADC->TASKS_SAMPLE = 1;
    while (NRF_SAADC->EVENTS_END == 0) {}

    /* stop the SAADC */
    NRF_SAADC->EVENTS_STOPPED = 0;
    NRF_SAADC->TASKS_STOP = 1;
    while (NRF_SAADC->EVENTS_STOPPED == 0) {}

    /* free device */
    done();

    /* the remaining offset error can result in a small negative value when
     * the input is connected to GND, clamp it to 0 */
    return (result < 0) ? 0 : (int32_t)result;
}
