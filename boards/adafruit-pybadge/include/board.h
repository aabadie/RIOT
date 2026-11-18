/*
 * SPDX-FileCopyrightText: 2020 Inria
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @ingroup     boards_adafruit-pybadge
 * @{
 *
 * @file
 * @brief       Board specific definitions for the Adafruit PyBadge
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 */

#include "cpu.h"
#include "periph/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    LED pin definitions and handlers
 * @{
 */
#define LED0_PIN            GPIO_PIN(PA, 23)                    /**< LED0 pin */

#define LED_PORT            PORT->Group[PA]                     /**< LED0 port */
#define LED0_MASK           (1 << 23)                           /**< LED0 mask */

#define LED0_ON             (LED_PORT.OUTSET.reg = LED0_MASK)   /**< LED0 on macro */
#define LED0_OFF            (LED_PORT.OUTCLR.reg = LED0_MASK)   /**< LED0 off macro */
#define LED0_TOGGLE         (LED_PORT.OUTTGL.reg = LED0_MASK)   /**< LED0 toggle macro */

#define LED0_NAME           "LED(Red)"                          /**< LED0 name */
/** @} */

/**
 * @name    Buttons control (TI SN74HC165 shift register)
 * @{
 */
#define BUTTON_LATCH        GPIO_PIN(PB, 0)                     /**< Latch pin */
#define BUTTON_CLK          GPIO_PIN(PB, 31)                    /**< Clock pin */
#define BUTTON_OUT          GPIO_PIN(PB, 30)                    /**< Serial output pin */

#define SN74HC165_PARAM_PL      BUTTON_LATCH    /**< Parallel load pin */
#define SN74HC165_PARAM_CLK     BUTTON_CLK      /**< Clock pin */
#define SN74HC165_PARAM_Q7      BUTTON_OUT      /**< Serial output pin */
/** @} */

/**
 * @name    Button bits in the value read with @ref sn74hc165_read
 *
 * A bit is set while the button is pressed.
 * @{
 */
#define BUTTON_MASK_LEFT    (1U << 0)                           /**< Left button */
#define BUTTON_MASK_UP      (1U << 1)                           /**< Up button */
#define BUTTON_MASK_DOWN    (1U << 2)                           /**< Down button */
#define BUTTON_MASK_RIGHT   (1U << 3)                           /**< Right button */
#define BUTTON_MASK_SELECT  (1U << 4)                           /**< Select button */
#define BUTTON_MASK_START   (1U << 5)                           /**< Start button */
#define BUTTON_MASK_A       (1U << 6)                           /**< A button */
#define BUTTON_MASK_B       (1U << 7)                           /**< B button */
/** @} */

/**
 * @name    LIS3DH accelerometer, on I2C
 * @{
 */
#define LIS3DH_PARAM_I2C    I2C_DEV(0)                          /**< I2C device */
#define LIS3DH_PARAM_ADDR   (0x18)                              /**< I2C address, SDO low */
#define LIS3DH_PARAM_INT1   GPIO_UNDEF                          /**< INT1 pin, not used */
#define LIS3DH_PARAM_INT2   GPIO_UNDEF                          /**< INT2 pin, not used */
/** @} */

/**
 * @name    Backlight control
 * @{
 */
#define BACKLIGHT_PIN     GPIO_PIN(PA, 1)                  /**< Backlight pin */
#define BACKLIGHT_PORT    PORT->Group[PA]                  /**< Backlight pin port */
#define BACKLIGHT_MASK    (1 << 1)                         /**< Backlight pin mask */
#define BACKLIGHT_ON      (BACKLIGHT_PORT.OUTSET.reg = BACKLIGHT_MASK) /**< Turn backlight on */
#define BACKLIGHT_OFF     (BACKLIGHT_PORT.OUTCLR.reg = BACKLIGHT_MASK) /**< Turn backlight off */
/** @} */

/**
 * @name    Display configuration (not supported yet)
 * @{
 */
#define ST77XX_PARAM_CNTRL      ST77XX_CNTRL_ST7735             /**< ST77xx controller variant */
#define ST77XX_PARAM_SPI        SPI_DEV(1)                      /**< SPI device */
#define ST77XX_PARAM_CS         GPIO_PIN(PB, 7)                 /**< Chip select pin */
#define ST77XX_PARAM_DCX        GPIO_PIN(PB, 5)                 /**< DCX pin */
#define ST77XX_PARAM_RST        GPIO_PIN(PA, 0)                 /**< Reset pin */
#define ST77XX_PARAM_NUM_LINES  (160U)                          /**< Number of screen lines */

/**
 * @brief Number of screen rgb channel (height)
 */
#define ST77XX_PARAM_RGB_CHANNELS   (128U)
#define ST77XX_PARAM_RGB        (1)                             /**< RGB configuration */
#define ST77XX_PARAM_INVERTED   (0)                             /**< Inversion configuration */
#define ST77XX_PARAM_ROTATION   ST77XX_ROTATION_90              /**< Rotation mode */
#define LCD_SCREEN_WIDTH        (ST77XX_PARAM_NUM_LINES)        /**< LCD screen width */
#define LCD_SCREEN_HEIGHT       (ST77XX_PARAM_RGB_CHANNELS)     /**< LCD screen height */
/** @} */

/**
 * @name    Neopixel LEDs (not supported yet)
 * @{
 */

/**
 * @brief GPIO pin connected to the data pin of the first LED
 */
#define WS281X_PARAM_PIN        (GPIO_PIN(PA, 15))
#define WS281X_PARAM_NUMOF      (5U)                            /**< Number of LEDs chained */

/** @} */

/**
 * @name    Speaker (supported via DAC DDS module)
 * @{
 */
#define SPEAKER_ENABLE_PIN      GPIO_PIN(PA, 27)                /**< Speaker enable pin */
#define SPEAKER_PIN             GPIO_PIN(PA, 2)                 /**< Speaker pin (A0) */
/** @} */

/**
 * @name MTD configuration
 * @{
 */
#define MTD_0 mtd_dev_get(0)                                    /**< MTD_0 constant */
/** @} */

/**
 * @name    Xtimer configuration
 * @{
 */
#define XTIMER_WIDTH                (32)                        /**< Default timer is 32bit width */
#define XTIMER_HZ                   (1000000ul)                 /**< Default timer runs at 1MHz */
/** @} */

#define BOOTLOADER_UF2    1     /**< This board uses the UF2 bootloader */

/**
 * @name    PDM microphone of the EdgeBadge
 *
 * Clock range of a MP34DT01-M type MEMS microphone, unverified for the
 * microphone of the EdgeBadge.
 * @{
 */
#ifndef PDM_MIC_CLK_MIN_HZ
#  define PDM_MIC_CLK_MIN_HZ    1000000
#endif
#ifndef PDM_MIC_CLK_MAX_HZ
#  define PDM_MIC_CLK_MAX_HZ    3250000
#endif
/** @} */

#ifdef __cplusplus
}
#endif

/** @} */
