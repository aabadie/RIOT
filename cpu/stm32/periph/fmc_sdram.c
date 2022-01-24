/*
 * Copyright (C) 2022 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_stm32
 * @{
 *
 * @file
 * @brief       Low-level FMC (Flexible Memory controller) SDRAM driver implementation
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @}
 */

#include <assert.h>
#include <stdint.h>
#include <stdalign.h>
#include <string.h>

#include "cpu.h"
#include "board.h"
#include "periph/gpio.h"
#include "periph_conf.h"

#define ENABLE_DEBUG        0
#include "debug.h"

static void _init_gpios_list(const gpio_t *pins, size_t len)
{
    for (uint8_t pin = 0; pin < len; ++pin) {
        if (!gpio_is_valid(pins[pin])) {
            continue;
        }
        gpio_init(pins[pin], GPIO_OUT);
        gpio_init_af(pins[pin], fmc_sdram_config.af);
    }
}

static void _init_gpios(void)
{
    gpio_init(fmc_sdram_config.sdclk_pin, GPIO_OUT);
    gpio_init_af(fmc_sdram_config.sdclk_pin, fmc_sdram_config.af);
    _init_gpios_list(fmc_sdram_config.sdcke_pins, SDCKE_PINS_NUMOF);
    _init_gpios_list(fmc_sdram_config.sdne_pins, SDNE_PINS_NUMOF);
    _init_gpios_list(fmc_sdram_config.addr_pins, ADDR_PINS_NUMOF);
    _init_gpios_list(fmc_sdram_config.data_pins, DATA_PINS_NUMOF);
    _init_gpios_list(fmc_sdram_config.bank_addr_pins, BANK_ADDR_PINS_NUMOF);
    gpio_init(fmc_sdram_config.nras_pin, GPIO_OUT);
    gpio_init_af(fmc_sdram_config.nras_pin, fmc_sdram_config.af);
    gpio_init(fmc_sdram_config.ncas_pin, GPIO_OUT);
    gpio_init_af(fmc_sdram_config.ncas_pin, fmc_sdram_config.af);
    gpio_init(fmc_sdram_config.sdnwe_pin, GPIO_OUT);
    gpio_init_af(fmc_sdram_config.sdnwe_pin, fmc_sdram_config.af);
    _init_gpios_list(fmc_sdram_config.nbl_pins, NBL_PINS_NUMOF);
}

static void _fmc_configure(void)
{
    /* Timing configuration for 100Mhz as SD clock frequency (System clock is up to 200Mhz) */
    // Timing.LoadToActiveDelay    = 2;
    // Timing.ExitSelfRefreshDelay = 7;
    // Timing.SelfRefreshTime      = 4;
    // Timing.RowCycleDelay        = 7;
    // Timing.WriteRecoveryTime    = 2;
    // Timing.RPDelay              = 2;
    // Timing.RCDDelay             = 2;

    // sdramHandle.Init.SDBank             = FMC_SDRAM_BANK1;
    // sdramHandle.Init.ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_8;
    // sdramHandle.Init.RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_12;
    // sdramHandle.Init.MemoryDataWidth    = SDRAM_MEMORY_WIDTH;
    // sdramHandle.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
    // sdramHandle.Init.CASLatency         = FMC_SDRAM_CAS_LATENCY_2;
    // sdramHandle.Init.WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
    // sdramHandle.Init.SDClockPeriod      = SDCLOCK_PERIOD;
    // sdramHandle.Init.ReadBurst          = FMC_SDRAM_RBURST_ENABLE;
    // sdramHandle.Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_0;    
}

void fmc_sdram_init(void)
{
    DEBUG("[fmc] init: initializing device\n");

    periph_clk_en(fmc_sdram_config.bus, fmc_sdram_config.rcc_mask);

    _init_gpios();
    _fmc_configure();
}
