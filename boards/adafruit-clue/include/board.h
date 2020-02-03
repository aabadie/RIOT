/*
 * Copyright (C) 2020 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_adafruit-clue
 * @{
 *
 * @file
 * @brief       Board specific configuration for the Adafruit Clue board
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 */

#ifndef BOARD_H
#define BOARD_H

#include "cpu.h"
#include "board_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    LEDs pin configuration
 * @{
 */
#define LED0_PIN                        GPIO_PIN(1, 1)
#define LED0_MASK                       (1 << 1)
#define LED0_ON                         (NRF_P1->OUTCLR = LED0_MASK)
#define LED0_OFF                        (NRF_P1->OUTSET = LED0_MASK)
#define LED0_TOGGLE                     (NRF_P1->OUT   ^= LED0_MASK)
/** @} */

/**
 * @name    Button pin configuration
 * @{
 */
#define BTN0_PIN                        GPIO_PIN(1, 10) /* Right button */
#define BTN0_MODE                       GPIO_IN_PU

#define BTN1_PIN                        GPIO_PIN(1, 2) /* Left button */
#define BTN1_MODE                       GPIO_IN_PU
/** @} */

/**
 * @name    LIS3MDL 3-axis magnetometer I2C address
 */
#define LIS3MDL_PARAM_ADDR              (0x1C)
/** @} */

/**
 * @name    STH31 temperature and humidity sensor I2C address
 */
#define SHT3X_PARAM_I2C_ADDR            (SHT3X_I2C_ADDR_1)
/** @} */

/**
 * @name    Button pin configuration
 * @{
 */
#define ILI9341_PARAM_SPI               SPI_DEV(1)
#define ILI9341_PARAM_CS                GPIO_PIN(0, 12)
#define ILI9341_PARAM_DCX               GPIO_PIN(0, 13)
#define ILI9341_PARAM_NUM_LINES         (240U)
#define ILI9341_PARAM_RGB               (1)
#define ILI9341_PARAM_INVERTED          (1)
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H */
/** @} */
