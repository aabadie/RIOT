/*
 * Copyright (C) 2018 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Example demonstrating the use of LoRaWAN with RIOT
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "msg.h"
#include "thread.h"
#include "fmt.h"
#ifdef MODULE_PM_LAYERED
#include "pm_layered.h"
#endif

#include "periph/rtc.h"

#include "net/loramac.h"
#include "semtech_loramac.h"

#include "ds75lx.h"
#include "ds75lx_params.h"

/* Messages are sent every 20s to respect the duty cycle on each channel */
#define PERIOD              (20U)

/* Low-power mode level */
#define PM_LOCK_LEVEL       (1)

#define SENDER_PRIO         (THREAD_PRIORITY_MAIN - 1)
static kernel_pid_t sender_pid;
static char sender_stack[THREAD_STACKSIZE_MAIN / 2];

semtech_loramac_t loramac;
ds75lx_t ds75lx;

static char message[32];

#ifdef USE_OTAA
static uint8_t deveui[LORAMAC_DEVEUI_LEN];
static uint8_t appeui[LORAMAC_APPEUI_LEN];
static uint8_t appkey[LORAMAC_APPKEY_LEN];
#endif

#ifdef USE_ABP
static uint8_t devaddr[LORAMAC_DEVADDR_LEN];
static uint8_t nwkskey[LORAMAC_NWKSKEY_LEN];
static uint8_t appskey[LORAMAC_APPSKEY_LEN];
#endif

static void rtc_cb(void *arg)
{
    (void) arg;

#ifdef MODULE_PM_LAYERED
    /* block sleep level mode until the next sending cycle has completed */
    pm_block(PM_LOCK_LEVEL);
#endif

    msg_t msg;
    msg_send(&msg, sender_pid);
}

static void _prepare_next_alarm(void)
{
    struct tm time;
    rtc_get_time(&time);
    /* set initial alarm */
    time.tm_sec += PERIOD;
    mktime(&time);
    rtc_set_alarm(&time, rtc_cb, NULL);
}

static void _send_message(void)
{
    int16_t temperature;
    ds75lx_wakeup(&ds75lx);
    /* Get temperature in degrees celsius */
    ds75lx_read_temperature(&ds75lx, &temperature);
    ds75lx_shutdown(&ds75lx);
    bool negative = (temperature < 0);
    if (negative) {
        temperature = -temperature;
    }
    sprintf(message, "%c%u.%u", negative ? '-': '+',
            (temperature / 10), (temperature % 10));
    printf("Sending: %s\n", message);
    /* Try to send the message */
    uint8_t ret = semtech_loramac_send(&loramac,
                                       (uint8_t *)message, strlen(message));
    if (ret != SEMTECH_LORAMAC_TX_OK) {
        printf("Cannot send message '%s', ret code: %d\n", message, ret);
        return;
    }
    /* The send was successfully scheduled, now wait until the send cycle has
       completed and a reply is received from the MAC */
    semtech_loramac_recv(&loramac);
}

static void *sender(void *arg)
{
    (void)arg;

    msg_t msg;
    msg_t msg_queue[8];
    msg_init_queue(msg_queue, 8);

    while (1) {
        msg_receive(&msg);

        /* Trigger the message send */
        _send_message();

        /* Schedule the next wake-up alarm */
        _prepare_next_alarm();

#ifdef MODULE_PM_LAYERED
        /* go back to sleep */
        pm_unblock(PM_LOCK_LEVEL);
#endif
    }

    /* this should never be reached */
    return NULL;
}

int main(void)
{
    puts("LoRaWAN Class A low-power application");
    puts("=====================================");

    int result = ds75lx_init(&ds75lx, &ds75lx_params[0]);
    if (result != DS75LX_OK) {
        puts("[Error] Failed to initialize DS75LX sensor");
        return 1;
    }

    /* Initialize the loramac stack */
    semtech_loramac_init(&loramac);

#ifdef USE_OTAA /* OTAA activation mode */
    /* Convert identifiers and keys strings to byte arrays */
    fmt_hex_bytes(deveui, DEVEUI);
    fmt_hex_bytes(appeui, APPEUI);
    fmt_hex_bytes(appkey, APPKEY);
    semtech_loramac_set_deveui(&loramac, deveui);
    semtech_loramac_set_appeui(&loramac, appeui);
    semtech_loramac_set_appkey(&loramac, appkey);

    /* Use a fast datarate, e.g. BW125/SF7 in EU868 */
    semtech_loramac_set_dr(&loramac, LORAMAC_DR_5);

    /* Start the Over-The-Air Activation (OTAA) procedure to retrieve the
     * generated device address and to get the network and application session
     * keys.
     */
    puts("Starting join procedure");
    if (semtech_loramac_join(&loramac, LORAMAC_JOIN_OTAA) != SEMTECH_LORAMAC_JOIN_SUCCEEDED) {
        puts("Join procedure failed");
        return 1;
    }
#endif

#ifdef USE_ABP /* ABP activation mode */
    /* Convert identifiers and keys strings to byte arrays */
    fmt_hex_bytes(devaddr, DEVADDR);
    fmt_hex_bytes(nwkskey, NWKSKEY);
    fmt_hex_bytes(appskey, APPSKEY);
    semtech_loramac_set_devaddr(&loramac, devaddr);
    semtech_loramac_set_nwkskey(&loramac, nwkskey);
    semtech_loramac_set_appskey(&loramac, appskey);

    /* Configure RX2 parameters */
    semtech_loramac_set_rx2_freq(&loramac, RX2_FREQ);
    semtech_loramac_set_rx2_dr(&loramac, RX2_DR);

    /* Use a fast datarate, e.g. BW125/SF7 in EU868 */
    semtech_loramac_set_dr(&loramac, LORAMAC_DR_5);

    /* ABP join procedure always succeeds */
    semtech_loramac_join(&loramac, LORAMAC_JOIN_ABP);
#endif
    puts("Join procedure succeeded");

    /* start the sender thread */
    sender_pid = thread_create(sender_stack, sizeof(sender_stack),
                               SENDER_PRIO, 0, sender, NULL, "sender");

    /* trigger the first send */
    msg_t msg;
    msg_send(&msg, sender_pid);
    return 0;
}
