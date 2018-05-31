/*
 * Copyright (C) 2017 Kaspar Schleiser <kaspar@schleiser.de>
 *               2014 FU Berlin
 *               2018 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_stm32_common
 * @ingroup     drivers_periph_i2c
 * @{
 *
 * @file
 * @brief       Low-level I2C driver implementation
 *
 * This driver supports the STM32 F4 families.
 *
 * @note This implementation only implements the 7-bit addressing mode.
 *
 * @author      Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @}
 */

#include <stdint.h>

#include "cpu.h"
#include "irq.h"
#include "mutex.h"
#include "pm_layered.h"

#include "periph_conf.h"
#include "periph/gpio.h"
#include "periph/i2c.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

#define I2C_IRQ_PRIO            (1)
#define I2C_FLAG_READ           (I2C_READ)
#define I2C_FLAG_WRITE          (0)

/* static function definitions */
static void _i2c_init(I2C_TypeDef *i2c, uint32_t clk, int ccr);
static void _start(I2C_TypeDef *dev, uint8_t address, uint8_t rw_flag, uint8_t flags);
static inline void _clear_addr(I2C_TypeDef *dev);
static inline void _write(I2C_TypeDef *dev, const uint8_t *data, int length);
static inline void _stop(I2C_TypeDef *dev);

/**
 * @brief Array holding one pre-initialized mutex for each I2C device
 */
static mutex_t locks[I2C_NUMOF];

void i2c_init(i2c_t dev)
{
    assert(dev < I2C_NUMOF);

    DEBUG("[i2c] init: initializing device\n");
    mutex_init(&locks[dev]);

    I2C_TypeDef *i2c = i2c_config[dev].dev;

    assert(i2c != NULL);

    int ccr;
    /* read speed configuration */
    switch (i2c_config[dev].speed) {
        case I2C_SPEED_LOW:
            /* 10Kbit/s */
            ccr = i2c_config[dev].clk / 20000;
            break;

        case I2C_SPEED_NORMAL:
            /* 100Kbit/s */
            ccr = i2c_config[dev].clk / 200000;
            break;

        case I2C_SPEED_FAST:
            ccr = i2c_config[dev].clk / 800000;
            break;

        default:
            break;
    }

    periph_clk_en(i2c_config[dev].bus, i2c_config[dev].rcc_mask);
    NVIC_SetPriority(i2c_config[dev].irqn, I2C_IRQ_PRIO);
    NVIC_EnableIRQ(i2c_config[dev].irqn);

    /* configure pins */
    // _pin_config(port_scl, port_sda, pin_scl, pin_sda);
    gpio_init(i2c_config[dev].scl_pin, GPIO_OD_PU);
    gpio_init_af(i2c_config[dev].scl_pin, i2c_config[dev].scl_af);
    gpio_init(i2c_config[dev].sda_pin, GPIO_OD_PU);
    gpio_init_af(i2c_config[dev].sda_pin, i2c_config[dev].sda_af);

    /* configure device */
    _i2c_init(i2c, i2c_config[dev].clk, ccr);

    /* make sure the analog filters don't hang -> see errata sheet 2.14.7 */
    if (i2c->SR2 & I2C_SR2_BUSY) {
        DEBUG("LINE BUSY AFTER RESET -> toggle pins now\n");
        /* disable peripheral */
        i2c->CR1 &= ~I2C_CR1_PE;
        /* toggle both pins to reset analog filter */
        gpio_init(i2c_config[dev].scl_pin, GPIO_OD);
        gpio_init(i2c_config[dev].sda_pin, GPIO_OD);
        gpio_set(i2c_config[dev].sda_pin);
        gpio_set(i2c_config[dev].scl_pin);
        gpio_clear(i2c_config[dev].sda_pin);
        gpio_clear(i2c_config[dev].scl_pin);
        gpio_set(i2c_config[dev].sda_pin);
        gpio_set(i2c_config[dev].scl_pin);
        /* reset pins for alternate function */
        gpio_init(i2c_config[dev].scl_pin, GPIO_OD_PU);
        gpio_init_af(i2c_config[dev].scl_pin, i2c_config[dev].scl_af);
        gpio_init(i2c_config[dev].sda_pin, GPIO_OD_PU);
        gpio_init_af(i2c_config[dev].sda_pin, i2c_config[dev].sda_af);
        /* make peripheral soft reset */
        i2c->CR1 |= I2C_CR1_SWRST;
        i2c->CR1 &= ~I2C_CR1_SWRST;
        /* enable device */
        _i2c_init(i2c, i2c_config[dev].clk, ccr);
    }
}

static void _i2c_init(I2C_TypeDef *i2c, uint32_t clk, int ccr)
{
    /* disable device and set ACK bit */
    i2c->CR1 = I2C_CR1_ACK;
    /* configure I2C clock */
    i2c->CR2 = (clk / 1000000);
    i2c->CCR = ccr;
    i2c->TRISE = (clk / 1000000) + 1;
    /* configure device */
    i2c->OAR1 = 0;              /* makes sure we are in 7-bit address mode */
    /* enable device */
    i2c->CR1 |= I2C_CR1_PE;
}

int i2c_acquire(i2c_t dev)
{
    assert(dev < I2C_NUMOF);

    mutex_lock(&locks[dev]);
    /* block STOP mode */
    pm_block(STM32_PM_STOP);

    periph_clk_en(i2c_config[dev].bus, i2c_config[dev].rcc_mask);

    return 0;
}

int i2c_release(i2c_t dev)
{
    assert(dev < I2C_NUMOF);

    while (i2c_config[dev].dev->SR2 & I2C_SR2_BUSY) {}
    periph_clk_dis(i2c_config[dev].bus, i2c_config[dev].rcc_mask);

    /* unblock STOP mode */
    pm_unblock(STM32_PM_STOP);

    mutex_unlock(&locks[dev]);
    return 0;
}

int i2c_read_bytes(i2c_t dev, uint16_t address, void *data, size_t length,
                   uint8_t flags)
{
    assert(dev < I2C_NUMOF);

    size_t n = length;
    char *in = (char *)data;

    I2C_TypeDef *i2c = i2c_config[dev].dev;

    assert(i2c != NULL);

    if (!(flags & I2C_NOSTART)) {
        DEBUG("Send Slave address and wait for ADDR == 1\n");
        _start(i2c, address, I2C_FLAG_READ, flags);
    }

    if (length == 1) {
        DEBUG("Set ACK = 0\n");
        i2c->CR1 &= ~(I2C_CR1_ACK);
    }
    else {
        i2c->CR1 |= I2C_CR1_ACK;
    }

    _clear_addr(i2c);

    while (n--) {
        /* wait for reception to complete */
        while (!(i2c->SR1 & I2C_SR1_RXNE)) {}

        if (n == 1) {
            /* disable ACK */
            i2c->CR1 &= ~(I2C_CR1_ACK);
        }

        /* read byte */
        *(in++) = i2c->DR;
    }

    if (!(flags & I2C_NOSTOP)) {
        /* set STOP */
        i2c->CR1 |= (I2C_CR1_STOP);

        while (i2c->CR1 & I2C_CR1_STOP) {}
    }

    return length;
}

int i2c_read_regs(i2c_t dev, uint16_t address, uint16_t reg, void *data,
                  size_t length, uint8_t flags)
{
    assert(dev < I2C_NUMOF);

    I2C_TypeDef *i2c = i2c_config[dev].dev;
    assert(i2c != NULL);

    /* send start condition and slave address */
    DEBUG("Send slave address and clear ADDR flag\n");
    _start(i2c, address, I2C_FLAG_WRITE, flags);
    DEBUG("Write reg into DR\n");
    _clear_addr(i2c);
    while (!(i2c->SR1 & I2C_SR1_TXE)) {}
    i2c->DR = reg;
    while (!(i2c->SR1 & I2C_SR1_TXE)) {}
    DEBUG("Now start a read transaction\n");
    return i2c_read_bytes(dev, address, data, length, flags);
}

int i2c_write_bytes(i2c_t dev, uint16_t address, const void *data,
                    size_t length, uint8_t flags)
{
    assert(dev < I2C_NUMOF);

    I2C_TypeDef *i2c = i2c_config[dev].dev;
    assert(i2c != NULL);

    if (!(flags & I2C_NOSTART)) {
        /* start transmission and send slave address */
        DEBUG("sending start sequence\n");
        _start(i2c, address, I2C_FLAG_WRITE, flags);
    }

    _clear_addr(i2c);
    /* send out data bytes */
    _write(i2c, data, length);
    if (!(flags & I2C_NOSTOP)) {
        /* end transmission */
        DEBUG("Ending transmission\n");
        _stop(i2c);
        DEBUG("STOP condition was send out\n");
    }

    return length;
}

int i2c_write_regs(i2c_t dev, uint16_t address, uint16_t reg, const void *data,
                   size_t length, uint8_t flags)
{
    assert(dev < I2C_NUMOF);

    I2C_TypeDef *i2c = i2c_config[dev].dev;
    assert(i2c != NULL);

    /* start transmission and send slave address */
    _start(i2c, address, I2C_FLAG_WRITE, flags);
    _clear_addr(i2c);
    /* send register address and wait for complete transfer to be finished*/
    _write(i2c, (uint8_t *)&reg, 1);
    /* write data to register */
    _write(i2c, data, length);
    /* finish transfer */
    _stop(i2c);
    /* return number of bytes send */
    return length;
}

static void _start(I2C_TypeDef *dev, uint8_t address, uint8_t rw_flag, uint8_t flags)
{
    (void)flags;
start:

    /* generate start condition */
    dev->CR1 |= I2C_CR1_START;

    /* Wait for SB flag to be set */
    while (!(dev->SR1 & I2C_SR1_SB)) {}

    /* send address and read/write flag */
    dev->DR = (address << 1) | rw_flag;

    /* Wait for ADDR flag to be set */
    while (!(dev->SR1 & I2C_SR1_ADDR)) {
        if (dev->SR1 & I2C_SR1_AF) {
            /* if the device answers NACK on sending the address, retry */
            dev->SR1 &= ~(I2C_SR1_AF);
            goto start;
        }
    }
}

static inline void _clear_addr(I2C_TypeDef *dev)
{
    dev->SR1;
    dev->SR2;
}

static inline void _write(I2C_TypeDef *dev, const uint8_t *data, int length)
{
    DEBUG("Looping through bytes\n");

    for (int i = 0; i < length; i++) {
        /* write data to data register */
        dev->DR = data[i];
        DEBUG("Written %i byte to data reg, now waiting for DR to be empty again\n", i);

        /* wait for transfer to finish */
        while (!(dev->SR1 & I2C_SR1_TXE)) {}

        DEBUG("DR is now empty again\n");
    }
}

static inline void _stop(I2C_TypeDef *dev)
{
    /* make sure last byte was send */
    DEBUG("Wait if last byte hasn't been sent\n");

    while (!(dev->SR1 & I2C_SR1_BTF)) {}

    /* send STOP condition */
    dev->CR1 |= I2C_CR1_STOP;
}

#if I2C_0_EN
void I2C_0_ERR_ISR(void)
{
    unsigned state = I2C_0_DEV->SR1;
    DEBUG("\n\n### I2C ERROR OCCURED ###\n");
    DEBUG("status: %08x\n", state);
    if (state & I2C_SR1_OVR) {
        DEBUG("OVR\n");
    }
    if (state & I2C_SR1_AF) {
        DEBUG("AF\n");
    }
    if (state & I2C_SR1_ARLO) {
        DEBUG("ARLO\n");
    }
    if (state & I2C_SR1_BERR) {
        DEBUG("BERR\n");
    }
    if (state & I2C_SR1_PECERR) {
        DEBUG("PECERR\n");
    }
    if (state & I2C_SR1_TIMEOUT) {
        DEBUG("TIMEOUT\n");
    }
    if (state & I2C_SR1_SMBALERT) {
        DEBUG("SMBALERT\n");
    }
    while (1) {}
}
#endif /* I2C_0_EN */
