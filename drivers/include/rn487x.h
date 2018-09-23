/*
 * Copyright (C) 2018 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_rn487x RN4870/1 BLE module driver
 * @ingroup     drivers_netdev
 * @brief       High-level driver for the RN4870/1 BLE modules
 * @{
 *
 * @file
 * @brief       High-level driver for the RN4870/1 BLE modules
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 */

#ifndef RN487X_H
#define RN487X_H

#include <stdint.h>

#include "xtimer.h"
#include "periph/uart.h"
#include "periph/gpio.h"
#include "net/netdev.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Maximum length of an exchanged messages with commands
 */
#define RN487X_MAX_BUF                  (40U)

/**
 * @brief   Maximum length of an RX message
 *
 *          LoRaMAC payload can be up to 250 bytes.
 */
#define RN487X_RX_MAX_BUF               (250U)

/**
 * @brief   Command responses and server replies status codes
 */
enum {
    RN487X_OK,                         /**< Command is valid */
    RN487X_DATA,                       /**< Command returned data */
    RN487X_TIMEOUT,                    /**< Command timeout */
    RN487X_ERR,                        /**< Invalid command */
};

/**
 * @brief   Configuration parameters for RN4870/1 device
 */
typedef struct {
    uart_t uart;                       /**< UART interfaced the device is connected to */
    uint32_t baudrate;                 /**< baudrate to use */
    gpio_t pin_reset;                  /**< GPIO pin that is connected to the STATUS pin
                                            set to GPIO_UNDEF if not used */
} rn487x_params_t;

/**
 * @brief   RN4870/1 device descriptor
 */
typedef struct {
    netdev_t netdev;                   /**< Netdev parent struct */
    /* device driver specific fields */
    rn487x_params_t p;                 /**< configuration parameters */

    /* values for the UART TX state machine  */
    char cmd_buf[RN487X_MAX_BUF];      /**< command to send data buffer */
    mutex_t cmd_lock;                  /**< mutex to allow only one
                                        *   command at a time */

    /* buffer and synchronization for command responses */
    mutex_t resp_lock;                 /**< mutex for waiting for command
                                        *   response */
    char resp_buf[RN487X_MAX_BUF];     /**< command response data buffer */
    uint16_t resp_size;                /**< counter for received char in response */
    uint8_t resp_done;                 /**< check if response has completed */
} rn487x_t;

/**
 * @brief   Prepares the given RN4870/1 device
 *
 * @param[out] dev          RN487X device to initialize
 * @param[in]  params       parameters for device initialization
 */
void rn487x_setup(rn487x_t *dev, const rn487x_params_t *params);

/**
 * @brief   Initializes the RN4870/1 device
 *
 * @param[in] dev           RN487X device descriptor
 *
 * @return                  RN487X_OK on success
 * @return                 -ENXIO if UART initialization failed
 * @return                  RN487X_TIMEOUT if UART communication failed
 */
int rn487x_init(rn487x_t *dev);

/**
 * @brief   Reset the module by software
 *
 * @param[in] dev           RN487X device descriptor
 *
 * @return                  RN487X_OK on success
 * @return                  RN487X_TIMEOUT if UART communication failed
 */
int rn487x_reset(rn487x_t *dev);

/**
 * @brief   Performs a factory reset of the module
 *
 * @param[in] dev           RN487X device descriptor
 *
 * @return                  RN487X_OK on success
 * @return                  RN487X_TIMEOUT if UART communication failed
 */
int rn487x_factory_reset(rn487x_t *dev);

/**
 * @brief   Writes a command to the RN4870/1 device
 *
 * @param[in] dev           RN487X device descriptor
 *
 * @return                  RN487X_OK on success
 * @return                  RN487X_TIMEOUT if no response is received from the module
 * @return                  RN487X_ERR if command is invalid
 */
int rn487x_write_cmd(rn487x_t *dev);

/**
 * @brief   Waits for a response to a command from the device
 *
 * @param[in] dev           RN487X device descriptor
 *
 * @return                  RN487X_OK on success
 * @return                  RN487X_TIMEOUT if no response is received from the module
 * @return                  RN487X_ERR if command is invalid
 */
int rn487x_wait_response(rn487x_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* RN487X_H */
/** @} */
