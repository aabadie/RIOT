/*
 * Copyright (C) 2022 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup         cpu_stm32
 * @{
 *
 * @file
 * @brief           FMC SDRAM definitions for the STM32 family
 *
 * @author          Alexandre Abadie <alexandre.abadie@inria.fr>
 */

#ifndef PERIPH_CPU_FMC_SDRAM_H
#define PERIPH_CPU_FMC_SDRAM_H

#include <stdint.h>

#include "periph/cpu_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SDCKE_PINS_NUMOF            2
#define SDNE_PINS_NUMOF             2
#define ADDR_PINS_NUMOF             12
#define DATA_PINS_NUMOF             32
#define BANK_ADDR_PINS_NUMOF        2
#define NBL_PINS_NUMOF              4

/**
 * @brief   FMC Peripheral configuration
 */
typedef struct {
    uint8_t bus;                                    /**< APB bus */
    uint32_t rcc_mask;                              /**< bit in clock enable register */
    gpio_t sdclk_pin;                               /**< SDRAM clock */
    gpio_t sdcke_pins[SDCKE_PINS_NUMOF];            /**< SDRAM Bank Clock Enable */
    gpio_t sdne_pins[SDNE_PINS_NUMOF];              /**< SDRAM Bank Chip Enable */
    gpio_t addr_pins[ADDR_PINS_NUMOF];              /**< Address */
    gpio_t data_pins[DATA_PINS_NUMOF];              /**< Bidirectional data bus */
    gpio_t bank_addr_pins[BANK_ADDR_PINS_NUMOF];    /**< Bank Address */
    gpio_t nras_pin;                                /**< Row Address Strobe */
    gpio_t ncas_pin;                                /**< Row Column Strobe */
    gpio_t sdnwe_pin;                               /**< Write Enable */
    gpio_t nbl_pins[NBL_PINS_NUMOF];                /**< Output Byte Mask for write accesses */
    gpio_af_t af;                                   /**< GPIO alternate function for FMC */
} fmc_sdram_conf_t;

/**
 * @brief   Initialize the FMC (Flexible Memory Controller) SDRAM peripheral
 */
void fmc_sdram_init(void);

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CPU_FMC_SDRAM_H */
/** @} */
