/*
 * Copyright (C) 2021 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_stm32f746g-disco
 * @{
 *
 * @file
 * @brief       Board specific definitions for the STM32F746G-DISCO
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 */

#ifndef BOARD_H
#define BOARD_H

#include "cpu.h"
#include "periph_conf.h"
#include "periph_cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name User button
 * @{
 */
#define BTN0_PIN            GPIO_PIN(PORT_I, 11)    /**< BTN0 pin */
#define BTN0_MODE           GPIO_IN                 /**< BTN0 pin mode */
/** @} */

/**
 * @name FT5336 touch panel configuration
 * @{
 */
#define FT5336_PARAM_I2C_DEV            I2C_DEV(1)              /**< I2C device */
#define FT5336_PARAM_INT_PIN            GPIO_PIN(PORT_I, 13)    /**< Interrupt pin */
#define FT5336_PARAM_XMAX               (480)                   /**< Max width */
#define FT5336_PARAM_YMAX               (272)                   /**< Max height */
/** @} */

/**
 * @brief   Initialize board specific hardware
 */
void board_init(void);

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H */
/** @} */
