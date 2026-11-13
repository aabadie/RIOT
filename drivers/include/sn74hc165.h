/*
 * SPDX-FileCopyrightText: 2026 Inria
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @defgroup    drivers_sn74hc165 SN74HC165 parallel-in serial-out shift register
 * @ingroup     drivers_misc
 * @brief       Driver for the 74HC165 8-bit parallel-in serial-out shift
 *              register, often used to read many buttons with 3 GPIOs
 *
 * Up to 4 chips can be chained: the serial input (DS, also called SER) of a
 * chip is connected to the output Q7 (also called QH) of the next one, and
 * only the Q7 output of the first chip is connected to the MCU.
 *
 * The inputs are sampled when the parallel load pin (PL, also called SH/LD)
 * is pulsed low, then shifted out on the rising edges of the clock pin (CP).
 *
 * @{
 *
 * @file
 * @brief       SN74HC165 driver interface
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 */

#include <stdint.h>

#include "periph/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup    drivers_sn74hc165_config SN74HC165 driver compile configuration
 * @ingroup     config_drivers_misc
 * @{
 */
/**
 * @brief   Duration of each half clock period and of the load pulse, in us
 *
 * 1 us is far above the needs of the chip, even at 2 V, and still reads
 * 8 inputs in about 20 us. 0 drives the pins as fast as the GPIOs go.
 */
#ifndef CONFIG_SN74HC165_DELAY_US
#define CONFIG_SN74HC165_DELAY_US   (1U)
#endif
/** @} */

/**
 * @brief   Maximum number of chained chips
 */
#define SN74HC165_CHAIN_MAX         (4U)

/**
 * @brief   Device initialization parameters
 */
typedef struct {
    gpio_t pl;          /**< parallel load pin (PL or SH/LD), active low */
    gpio_t clk;         /**< clock pin (CP or CLK) */
    gpio_t q7;          /**< serial output of the first chip (Q7 or QH) */
    gpio_t ce;          /**< clock enable pin (CE or CLK INH), active low,
                             GPIO_UNDEF when tied to ground */
    uint8_t chain;      /**< number of chained chips, 1 to 4 */
} sn74hc165_params_t;

/**
 * @brief   Device descriptor
 */
typedef struct {
    sn74hc165_params_t params;  /**< device parameters */
} sn74hc165_t;

/**
 * @brief   Initialize the GPIOs of a shift register chain
 *
 * @param[out] dev      device descriptor
 * @param[in]  params   device parameters
 *
 * @retval  0           on success
 * @retval  -EINVAL     invalid number of chained chips
 * @retval  -EIO        a GPIO could not be initialized
 */
int sn74hc165_init(sn74hc165_t *dev, const sn74hc165_params_t *params);

/**
 * @brief   Sample and read all the inputs of the chain
 *
 * Bit (8 * c + i) of the result is the input D<i> (A to H for i = 0 to 7)
 * of chip c, chip 0 being the one whose Q7 output goes to the MCU. Bits of
 * missing chips are 0.
 *
 * @param[in] dev       device descriptor
 *
 * @return  the state of the inputs, a 1 bit for a high input
 */
uint32_t sn74hc165_read(const sn74hc165_t *dev);

#ifdef __cplusplus
}
#endif

/** @} */
