/*
 * SPDX-FileCopyrightText: 2026 Inria
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @defgroup    drivers_adafruit_thermal Adafruit thermal printer
 * @ingroup     drivers_misc
 * @brief       Driver for the thermal receipt printers sold by Adafruit
 *
 * The Mini Thermal Receipt Printer of Adafruit (product 597, a CSN-A2
 * mechanism) and its siblings take commands over a TTL serial line, a
 * subset of ESC/POS. The driver only sends: the printer RX pin is the only
 * one to connect, with the ground. Its logic accepts 3.3 V.
 *
 * The printer has a small input buffer and no reliable way to tell it is
 * full, so the driver waits after each line and each part of an image for
 * the time the printer takes to print it, like the Arduino library of
 * Adafruit. When the DTR pin of the printer is connected, the driver
 * enables the hardware flow control instead and waits while the pin is high.
 *
 * The baud rate is printed on the test page, the one the printer prints
 * when its button is held at power up: 19200 for the recent ones, 9600 for
 * older ones.
 *
 * The printing head is 384 dots wide, 8 dots per mm. The default font is
 * 12x24 dots, 32 characters per line, in code page 437.
 *
 * The functions are not thread safe: use a device from a single thread.
 *
 * @{
 *
 * @file
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "periph/gpio.h"
#include "periph/uart.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Width of the printing head in dots
 */
#define ADAFRUIT_THERMAL_WIDTH          (384U)

/**
 * @brief   Bytes of a full width image row
 */
#define ADAFRUIT_THERMAL_ROW_LEN        (ADAFRUIT_THERMAL_WIDTH / 8)

/**
 * @name    Print modes, combined in @ref adafruit_thermal_set_mode
 * @{
 */
#define ADAFRUIT_THERMAL_MODE_NORMAL        (0x00U)     /**< default font */
#define ADAFRUIT_THERMAL_MODE_UPSIDE_DOWN   (1U << 2)   /**< upside down */
#define ADAFRUIT_THERMAL_MODE_BOLD          (1U << 3)   /**< bold */
#define ADAFRUIT_THERMAL_MODE_DOUBLE_HEIGHT (1U << 4)   /**< twice as high */
#define ADAFRUIT_THERMAL_MODE_DOUBLE_WIDTH  (1U << 5)   /**< twice as wide */
#define ADAFRUIT_THERMAL_MODE_STRIKE        (1U << 6)   /**< struck out */
/** @} */

/**
 * @brief   Alignment of the text
 */
typedef enum {
    ADAFRUIT_THERMAL_LEFT   = 0,    /**< left */
    ADAFRUIT_THERMAL_CENTER = 1,    /**< centered */
    ADAFRUIT_THERMAL_RIGHT  = 2,    /**< right */
} adafruit_thermal_justify_t;

/**
 * @brief   Device parameters
 */
typedef struct {
    uart_t uart;                /**< UART connected to the RX pin of the printer */
    uint32_t baudrate;          /**< baud rate of the printer */
    gpio_t dtr_pin;             /**< DTR pin of the printer, GPIO_UNDEF if not
                                     connected */
    uint8_t heat_dots;          /**< dots heated at once, unit of 8 dots */
    uint8_t heat_time;          /**< heating time, unit of 10 us: darker and
                                     slower when longer */
    uint8_t heat_interval;      /**< time between two heatings, unit of 10 us */
    uint8_t density;            /**< print density, 50 % + 5 % per step, 0 to 31 */
    uint8_t break_time;         /**< print break time, unit of 250 us, 0 to 7 */
    uint32_t dot_print_us;      /**< time to print a row of dots */
    uint32_t dot_feed_us;       /**< time to feed the paper by a dot */
} adafruit_thermal_params_t;

/**
 * @brief   Device descriptor
 */
typedef struct {
    adafruit_thermal_params_t params;   /**< parameters */
    uint32_t wait_us;                   /**< time to wait before the next byte */
    bool flow_control;                  /**< the printer drives DTR */
    uint8_t column;                     /**< column of the next character */
    uint8_t max_column;                 /**< characters per line */
    uint8_t char_height;                /**< height of a character in dots */
    uint8_t line_spacing;               /**< dots between two lines */
    char prev;                          /**< last character sent */
} adafruit_thermal_t;

/**
 * @brief   Prepare the UART and reset the printer to its defaults
 *
 * The printer needs half a second after its power up before it reads
 * commands: the first command waits for it.
 *
 * @param[out] dev      device descriptor
 * @param[in]  params   parameters
 *
 * @return  0 on success
 * @return  negative errno of uart_init() or gpio_init() on error
 */
int adafruit_thermal_init(adafruit_thermal_t *dev,
                          const adafruit_thermal_params_t *params);

/**
 * @brief   Reset the printer: default font, alignment and spacing
 *
 * @param[in] dev   device descriptor
 */
void adafruit_thermal_reset(adafruit_thermal_t *dev);

/**
 * @brief   Print text
 *
 * Lines end with a newline character, the printer also starts a new line
 * after the last column. Carriage returns are dropped. The printer uses
 * code page 437: plain ASCII prints as is.
 *
 * @param[in] dev   device descriptor
 * @param[in] text  text
 * @param[in] len   length of @p text
 */
void adafruit_thermal_write(adafruit_thermal_t *dev, const char *text, size_t len);

/**
 * @brief   Print a null terminated string
 *
 * @param[in] dev   device descriptor
 * @param[in] str   string
 */
void adafruit_thermal_print(adafruit_thermal_t *dev, const char *str);

/**
 * @brief   Feed the paper by lines of text
 *
 * @param[in] dev   device descriptor
 * @param[in] lines number of lines
 */
void adafruit_thermal_feed(adafruit_thermal_t *dev, uint8_t lines);

/**
 * @brief   Feed the paper by rows of dots
 *
 * @param[in] dev   device descriptor
 * @param[in] rows  number of rows of dots
 */
void adafruit_thermal_feed_rows(adafruit_thermal_t *dev, uint8_t rows);

/**
 * @brief   Set the print mode of the next characters
 *
 * @param[in] dev   device descriptor
 * @param[in] mode  ADAFRUIT_THERMAL_MODE_* flags
 */
void adafruit_thermal_set_mode(adafruit_thermal_t *dev, uint8_t mode);

/**
 * @brief   Set the alignment of the next lines of text
 *
 * The images always start on the left.
 *
 * @param[in] dev       device descriptor
 * @param[in] justify   alignment
 */
void adafruit_thermal_justify(adafruit_thermal_t *dev,
                              adafruit_thermal_justify_t justify);

/**
 * @brief   Fill an image row
 *
 * @param[in]  arg  argument given to @ref adafruit_thermal_bitmap
 * @param[in]  y    row, from 0
 * @param[out] row  row to fill, 8 dots per byte, the most significant bit
 *                  on the left, 1 for black
 * @param[in]  len  bytes of @p row
 */
typedef void (*adafruit_thermal_row_cb_t)(void *arg, unsigned y,
                                          uint8_t *row, size_t len);

/**
 * @brief   Print an image, row by row
 *
 * The image needs no buffer: the callback fills each row just before it is
 * sent.
 *
 * @param[in] dev       device descriptor
 * @param[in] width     width in dots, clipped to @ref ADAFRUIT_THERMAL_WIDTH
 * @param[in] height    height in dots
 * @param[in] cb        callback filling each row
 * @param[in] arg       argument of @p cb
 */
void adafruit_thermal_bitmap(adafruit_thermal_t *dev, unsigned width,
                             unsigned height, adafruit_thermal_row_cb_t cb,
                             void *arg);

/**
 * @brief   Print an image from a buffer
 *
 * @param[in] dev       device descriptor
 * @param[in] width     width in dots, clipped to @ref ADAFRUIT_THERMAL_WIDTH
 * @param[in] height    height in dots
 * @param[in] bitmap    rows of (@p width + 7) / 8 bytes, the most
 *                      significant bit on the left, 1 for black
 */
void adafruit_thermal_bitmap_buf(adafruit_thermal_t *dev, unsigned width,
                                 unsigned height, const uint8_t *bitmap);

/**
 * @brief   Print the test page of the printer
 *
 * @param[in] dev   device descriptor
 */
void adafruit_thermal_test_page(adafruit_thermal_t *dev);

/**
 * @brief   Let the printer sleep after some time without data
 *
 * @param[in] dev       device descriptor
 * @param[in] seconds   seconds before the sleep, at least 1
 */
void adafruit_thermal_sleep(adafruit_thermal_t *dev, uint16_t seconds);

/**
 * @brief   Wake the printer up and disable its sleep
 *
 * @param[in] dev   device descriptor
 */
void adafruit_thermal_wake(adafruit_thermal_t *dev);

#ifdef __cplusplus
}
#endif

/** @} */
