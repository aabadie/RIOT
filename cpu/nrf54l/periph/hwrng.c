/*
 * SPDX-FileCopyrightText: 2026 Inria
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     cpu_nrf54l
 * @ingroup     drivers_periph_hwrng
 * @{
 *
 * @file
 * @brief       HWRNG implementation for the nRF54L family, based on the TRNG
 *              of the CRACEN peripheral
 *
 * The CRACEN crypto accelerator embeds a true random number generator (ring
 * oscillator noise sources followed by conditioning and health tests), which
 * fills a 16 word FIFO with random data. The TRNG is only powered during a
 * hwrng_read() call: after being enabled it runs its startup health tests
 * and starts filling the FIFO, from which the driver pops the requested
 * amount of data.
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @}
 */

#include "cpu.h"
#include "periph/hwrng.h"

#define RNG_CONTROL     (NRF_CRACENCORE->RNGCONTROL)

static void _enable(void)
{
    /* the conditioning and health test configuration reset values are kept */
    RNG_CONTROL.CONTROL |= CRACENCORE_RNGCONTROL_CONTROL_ENABLE_Msk;
}

static inline uint32_t _state(void)
{
    return (RNG_CONTROL.STATUS & CRACENCORE_RNGCONTROL_STATUS_STATE_Msk) >>
           CRACENCORE_RNGCONTROL_STATUS_STATE_Pos;
}

void hwrng_init(void)
{
    /* nothing to do here, the TRNG is powered on demand in hwrng_read() */
}

void hwrng_read(void *buf, unsigned int num)
{
    uint8_t *b = (uint8_t *)buf;
    unsigned int count = 0;

    /* power up the RNG module of CRACEN and start the TRNG */
    NRF_CRACEN->ENABLE |= CRACEN_ENABLE_RNG_Msk;
    _enable();

    while (count < num) {
        /* if a startup or online health test failed, reset and restart the
         * TRNG to give it a fresh chance to produce data */
        if (_state() == CRACENCORE_RNGCONTROL_STATUS_STATE_ERROR) {
            RNG_CONTROL.CONTROL = CRACENCORE_RNGCONTROL_CONTROL_SOFTRST_Msk;
            RNG_CONTROL.CONTROL = 0;
            _enable();
        }

        if (RNG_CONTROL.FIFOLEVEL == 0) {
            continue;
        }

        /* reading the FIFO array pops one 32-bit word of random data */
        uint32_t word = RNG_CONTROL.FIFO[0];
        for (unsigned int i = 0; (i < 4) && (count < num); i++) {
            b[count++] = (uint8_t)(word >> (8 * i));
        }
    }

    /* power off the TRNG and the RNG module of CRACEN */
    RNG_CONTROL.CONTROL &= ~CRACENCORE_RNGCONTROL_CONTROL_ENABLE_Msk;
    NRF_CRACEN->ENABLE &= ~CRACEN_ENABLE_RNG_Msk;
}
