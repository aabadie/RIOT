/*
 * Copyright (C) 2014-2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     cpu_nrf51
 * @{
 *
 * @file
 * @brief       Interrupt vector definitions
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdint.h>
#include "cpu.h"
#include "vectors_cortexm.h"

/* define a local dummy handler as it needs to be in the same compilation unit
 * as the alias definition */
void dummy_handler(void) {
    dummy_handler_default();
}

/* Cortex-M common interrupt vectors */
WEAK_DEFAULT void isr_svc(void);
WEAK_DEFAULT void isr_pendsv(void);
WEAK_DEFAULT void isr_systick(void);
/* SCuM specific interrupt vectors */
WEAK_DEFAULT void UART_Handler(void);
WEAK_DEFAULT void EXT_GPIO3_ACTIVEHIGH_DEBOUNCED_Handler(void);
WEAK_DEFAULT void EXT_OPTICAL_IRQ_IN_Handler(void);
WEAK_DEFAULT void ADC_Handler(void);
WEAK_DEFAULT void RF_Handler(void);
WEAK_DEFAULT void RFTIMER_Handler(void);
WEAK_DEFAULT void RAWCHIPS_STARTVAL_Handler(void);
WEAK_DEFAULT void RAWCHIPS_32_Handler(void);
WEAK_DEFAULT void OPTICAL_SFD_Handler(void);
WEAK_DEFAULT void EXT_GPIO9_ACTIVELOW_Handler(void);
WEAK_DEFAULT void EXT_GPIO10_ACTIVELOW_Handler(void); 

/* GPIO8 is used for calibration */
void EXT_GPIO8_ACTIVEHIGH_Handler(void) {
    extern void OPTICAL_SFD_Handler(void);
    OPTICAL_SFD_Handler();
}

/* CPU specific interrupt vector table */
ISR_VECTOR(1) const isr_t vector_cpu[CPU_IRQ_NUMOF] = {
    [ 0] = UART_Handler,
    [ 1] = EXT_GPIO3_ACTIVEHIGH_DEBOUNCED_Handler,
    [ 2] = EXT_OPTICAL_IRQ_IN_Handler,
    [ 3] = ADC_Handler,
    [ 6] = RF_Handler,
    [ 7] = RFTIMER_Handler,
    [ 8] = RAWCHIPS_STARTVAL_Handler,
    [ 9] = RAWCHIPS_32_Handler,
    [11] = OPTICAL_SFD_Handler,
    [12] = EXT_GPIO8_ACTIVEHIGH_Handler,
    [13] = EXT_GPIO9_ACTIVELOW_Handler,
    [14] = EXT_GPIO10_ACTIVELOW_Handler,
};
