/*
 * Copyright (C) 2025 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

#pragma once

/**
 * @defgroup        cpu_scum SCuM CPU
 * @ingroup         cpu
 * @brief           SCuM CPU configuration
 * @{
 *
 * @file
 * @brief           Implementation specific CPU configuration options
 *
 * @author          Alexandre Abadie <alexandre.abadie@inria.fr>
 */

#include "scum.h"
#include "cpu_conf_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   ARM Cortex-M specific CPU configuration
 * @{
 */
#define CPU_DEFAULT_IRQ_PRIO    (1U)
#define CPU_IRQ_NUMOF           (16U)
#define CPU_FLASH_BASE          (0x00000000)
/** @} */

/**
 * @brief   Due to RAM restrictions, we need to limit the default GNRC packet
 *          buffer size on these CPUs
 * @{
 */
#ifndef CONFIG_GNRC_PKTBUF_SIZE
#define CONFIG_GNRC_PKTBUF_SIZE                (2048)
#endif
/** @} */

#ifdef __cplusplus
}
#endif

/** @} */
