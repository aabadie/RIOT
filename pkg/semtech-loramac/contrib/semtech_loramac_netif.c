/*
 * Copyright (C) 2017 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 * @ingroup     pkg_semtech-loramac
 *
 * @file
 * @brief       Semtech LoRaMAC adaption to netif
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 * @}
 */

#include <stdlib.h>
#include <string.h>

#include "net/netopt.h"
#include "net/gnrc/nettype.h"
#include "net/gnrc/netif.h"
#include "net/gnrc/netif/hdr.h"
#include "net/netdev.h"
#include "net/loramac.h"

#include "sx127x_netdev.h"

#include "semtech_loramac.h"
#include "semtech-loramac/board.h"
#include "LoRaMac.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

kernel_pid_t semtech_loramac_pid;
extern RadioEvents_t radio_events;
extern uint8_t payload[LORAWAN_APP_DATA_MAX_SIZE];

static void _semtech_loramac_init(gnrc_netif_t *netif);
static int _send(gnrc_netif_t *netif, gnrc_pktsnip_t *pkt);
static gnrc_pktsnip_t *_recv(gnrc_netif_t *netif);
static void _semtech_loramac_msg_handler(gnrc_netif_t *netif, msg_t *msg);
static void _semtech_loramac_event_cb(netdev_t *dev, netdev_event_t event);

static const gnrc_netif_ops_t semtech_loramac_ops = {
    .init = _semtech_loramac_init,
    .send = _send,
    .recv = _recv,
    .get = gnrc_netif_get_from_netdev,
    .set = gnrc_netif_set_from_netdev,
    .msg_handler = _semtech_loramac_msg_handler,
};

gnrc_netif_t *gnrc_netif_semtech_loramac_create(char *stack, int stacksize,
                                                char priority, char *name,
                                                netdev_t *dev)
{
    LoRaMacPrimitives_t primitives;
    LoRaMacCallback_t callbacks;
    _init_loramac(&primitives, &callbacks);
    semtech_loramac_set_dr(LORAMAC_DEFAULT_DR);
    semtech_loramac_set_adr(LORAMAC_DEFAULT_ADR);
    semtech_loramac_set_public_network(LORAMAC_DEFAULT_PUBLIC_NETWORK);

    return gnrc_netif_create(stack, stacksize, priority, name, dev,
                             &semtech_loramac_ops);
}

static gnrc_pktsnip_t *_recv(gnrc_netif_t *netif)
{
    netdev_t *dev = netif->dev;
    dev->driver->recv(dev, NULL, 0, NULL);
    return NULL;
}

static int _send(gnrc_netif_t *netif, gnrc_pktsnip_t *pkt)
{
    (void) netif;
    (void) pkt;
    return 0;
}

static void _semtech_loramac_msg_handler(gnrc_netif_t *netif, msg_t *msg)
{
    switch (msg->type) {
        case MSG_TYPE_ISR:
        {
            netdev_t *dev = msg->content.ptr;
            dev->driver->isr(dev);
        }
            break;

        case MSG_TYPE_RX_TIMEOUT:
            DEBUG("[semtech-loramac] RX timer timeout\n");
            radio_events.RxTimeout();
            break;

        case MSG_TYPE_TX_TIMEOUT:
            DEBUG("[semtech-loramac] TX timer timeout\n");
            radio_events.TxTimeout();
            break;

        case MSG_TYPE_MAC_TIMEOUT:
        {
            DEBUG("[semtech-loramac] MAC timer timeout\n");
            void (*callback)(void) = msg->content.ptr;
            callback();
        }
            break;

        case MSG_TYPE_LORAMAC_CMD:
        {
            DEBUG("[semtech-loramac] loramac cmd\n");
            semtech_loramac_call_t *call = msg->content.ptr;
            call->func(call->arg);
        }
            break;

        default:
            DEBUG("[semtech-loramac] Unexpected msg type '%04x'\n", msg->type);
            break;
    }
}

static void _semtech_loramac_event_cb(netdev_t *dev, netdev_event_t event)
{
    gnrc_netif_t *netif = (gnrc_netif_t *) dev->context;

    if (event == NETDEV_EVENT_ISR) {
        msg_t msg;

        msg.type = MSG_TYPE_ISR;
        msg.content.ptr = (void*)netif;

        if (msg_send(&msg, netif->pid) <= 0) {
            DEBUG("[semtech-loramac] gnrc_netdev: possibly lost interrupt.\n");
        }
    }
    else {
        DEBUG("[semtech-loramac] gnrc_netdev: event triggered -> %i\n", event);
        switch (event) {

            case NETDEV_EVENT_TX_COMPLETE:
                sx127x_set_sleep((sx127x_t*)dev);
                radio_events.TxDone();
                DEBUG("[semtech-loramac] Transmission completed\n");
                break;

            case NETDEV_EVENT_TX_TIMEOUT:
            {
                msg_t msg;
                msg.type = MSG_TYPE_TX_TIMEOUT;

                if (msg_send(&msg, semtech_loramac_pid) <= 0) {
                    DEBUG("[semtech-loramac] TX timeout.\n");
                }
            }
                break;

            case NETDEV_EVENT_RX_COMPLETE:
            {
                netdev_sx127x_lora_packet_info_t rx_info;
                size_t len;
                len = dev->driver->recv(dev, NULL, 0, 0);
                dev->driver->recv(dev, payload, len, &rx_info);
                radio_events.RxDone((uint8_t*)payload, len, rx_info.rssi,
                                     rx_info.snr);
            }
                break;

            case NETDEV_EVENT_RX_TIMEOUT:
            {
                msg_t msg;
                msg.type = MSG_TYPE_RX_TIMEOUT;

                if (msg_send(&msg, semtech_loramac_pid) <= 0) {
                    DEBUG("[semtech-loramac] RX timeout.\n");
                }
            }
                break;

            case NETDEV_EVENT_CRC_ERROR:
                DEBUG("[semtech-loramac] RX CRC error\n");
                radio_events.RxError();
                break;

            case NETDEV_EVENT_FHSS_CHANGE_CHANNEL:
                DEBUG("[semtech-loramac] FHSS channel change\n");
                radio_events.FhssChangeChannel(((sx127x_t*)dev)->_internal.last_channel);
                break;

            case NETDEV_EVENT_CAD_DONE:
                DEBUG("[DEBUG] test: CAD done\n");
                radio_events.CadDone(((sx127x_t*)dev)->_internal.is_last_cad_success);
                break;

            default:
                DEBUG("[semtech-loramac] unexpected netdev event\n");
                break;
        }
    }
}

static void _semtech_loramac_init(gnrc_netif_t *netif)
{
    netdev_t *dev;
    dev = netif->dev;
    dev->event_callback = _semtech_loramac_event_cb;

    semtech_loramac_pid = netif->pid;
}
