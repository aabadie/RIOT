/*
 * Copyright (C) 2021 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_ft5336
 * @{
 *
 * @file
 * @brief       Internal register addresses, bitfields and constants
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 */

#ifndef FT5336_CONSTANTS_H
#define FT5336_CONSTANTS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name Constants
 * @{
 */
#define FT5336_I2C_DEFAULT_ADDRESS                  (0x38)
#define FT5336_CHIP_ID                              (0x51)
#define FT5336_TOUCHES_COUNT_MAX                    (5)
/** @} */

/**
 * @name Register adresses
 * @{
 */
#define FT5336_DEVIDE_MODE_REG                      (0x00)
#define FT5336_GESTURE_ID_REG                       (0x01)
#define FT5336_TD_STATUS_REG                        (0x02)
#define FT5336_TOUCH1_XH_REG                        (0x03)
#define FT5336_TOUCH1_XL_REG                        (0x04)
#define FT5336_TOUCH1_YH_REG                        (0x05)
#define FT5336_TOUCH1_YL_REG                        (0x06)
#define FT5336_TOUCH2_XH_REG                        (0x09)
#define FT5336_TOUCH2_XL_REG                        (0x0A)
#define FT5336_TOUCH2_YH_REG                        (0x0B)
#define FT5336_TOUCH2_YL_REG                        (0x0C)
#define FT5336_TOUCH3_XH_REG                        (0x0F)
#define FT5336_TOUCH3_XL_REG                        (0x11)
#define FT5336_TOUCH3_YH_REG                        (0x12)
#define FT5336_TOUCH3_YL_REG                        (0x13)
#define FT5336_TOUCH4_XH_REG                        (0x15)
#define FT5336_TOUCH4_XL_REG                        (0x16)
#define FT5336_TOUCH4_YH_REG                        (0x17)
#define FT5336_TOUCH4_YL_REG                        (0x18)
#define FT5336_TOUCH5_XH_REG                        (0x1B)
#define FT5336_TOUCH5_XL_REG                        (0x1C)
#define FT5336_TOUCH5_YH_REG                        (0x1D)
#define FT5336_TOUCH5_YL_REG                        (0x1E)
#define FT5336_G_AUTO_CLB_MODE_REG                  (0xA4)
#define FT5336_G_MODE_REG                           (0xA4)
#define FT5336_CHIP_ID_REG                          (0xA8)
/** @} */

/**
 * @name Gesture ID register bitfields
 * @{
 */
#define FT5336_GESTURE_ID_MOVE_UP                   (0x10)
#define FT5336_GESTURE_ID_MOVE_LEFT                 (0x14)
#define FT5336_GESTURE_ID_MOVE_DOWN                 (0x18)
#define FT5336_GESTURE_ID_MOVE_RIGHT                (0x1C)
#define FT5336_GESTURE_ID_ZOOM_IN                   (0x48)
#define FT5336_GESTURE_ID_ZOOM_OUT                  (0x49)
#define FT5336_GESTURE_ID_NONE                      (0x00)
/** @} */

/**
 * @name Touch detect status register bitfields
 * @{
 */
#define FT5336_TD_STATUS_MASK                       (0x0F)
/** @} */

/**
 * @name Touch position (LSB/MSB) registers masks
 * @{
 */
#define FT5336_TOUCH_POS_LSB_MASK                   (0xFF)
#define FT5336_TOUCH_POS_MSB_MASK                   (0x0F)
/** @} */

/**
 * @name Interrupt mode register masks and bitfields
 * @{
 */
#define FT5336_G_MODE_INTERRUPT_MASK                (0x03)
#define FT5336_G_MODE_INTERRUPT_SHIFT               (0x00)
#define FT5336_G_MODE_INTERRUPT_POLLING             (0x00)
#define FT5336_G_MODE_INTERRUPT_TRIGGER             (0x01)
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* FT5336_CONSTANTS_H */
/** @} */
