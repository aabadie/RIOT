/*
 * SPDX-FileCopyrightText: 2017 Fundacion Inria Chile
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @{
 * @ingroup     net
 * @file
 * @brief       Implementation of OpenThread radio platform abstraction on the
 *              IEEE 802.15.4 Radio HAL
 *
 * The capabilities of the radio are given to OpenThread: what the radio
 * doesn't do (CSMA-CA, retransmissions, ACK timeout) is done by the SubMac of
 * OpenThread. The ACKs of the received frames are sent by the radio (as the
 * SubMac of RIOT assumes without the `ieee802154_submac_soft_ack` module).
 *
 * The Radio HAL callback can run in interrupt context: it only posts events
 * to the OpenThread event queue, every radio access is done in the
 * OpenThread thread. A received frame is read out of the radio before any
 * other access to the radio (transmission, state change), even if its event
 * is handled later: with a single frame buffer (at86rf2xx) a transmission
 * would overwrite it, and the RX done event must not change the state of the
 * radio during a transmission (its end would be taken for a reception).
 *
 * The frame pending bit of the ACKs to the Data Requests of the children
 * follows the source address match table of OpenThread: in the ACKs sent in
 * software, through the table of the radio if it has one, otherwise in the
 * ACKs to all the Data Requests as long as a child has pending frames.
 *
 * @author      Jose Ignacio Alamos <jialamos@uc.cl>
 * @}
 */

#include <errno.h>
#include <string.h>

#include "byteorder.h"
#include "event.h"
#include "iolist.h"
#include "irq.h"
#include "kernel_defines.h"
#include "luid.h"
#include "net/eui64.h"
#include "net/ieee802154.h"
#include "net/ieee802154/radio.h"
#include "time_units.h"

#include "mac_frame.h"
#include "openthread/platform/radio.h"
#include "ot.h"

#define ENABLE_DEBUG 0
#include "debug.h"

/* PSDU of an ACK frame: frame control, sequence number, FCS */
#define ACK_PSDU_LEN            (IEEE802154_ACK_FRAME_LEN)
#define DEFAULT_CHANNEL         (CONFIG_IEEE802154_DEFAULT_CHANNEL)
#define DEFAULT_TX_POWER        (0)

static ieee802154_dev_t *_dev;
static otRadioState _state = OT_RADIO_STATE_DISABLED;
static uint8_t _channel = DEFAULT_CHANNEL;
static int8_t _tx_power = DEFAULT_TX_POWER;
static int8_t _cca_threshold = -75;
static bool _promiscuous;
/* waiting for the ACK of the last frame, in software */
static bool _wait_ack;
/* the radio transmits a frame: it is not accessed until its TX done event */
static bool _tx_busy;
/* error of a transmission that didn't start, reported by _ev_tx_failed */
static otError _tx_error;
/* transmission power changed during a transmission, configured after it */
static bool _phy_pending;

/* a frame indicated by the radio (RX done or CRC error), not read yet */
static volatile bool _rx_pending;
static volatile bool _rx_crc_error;
/* length of the frame read in _rx_psdu, not given to OpenThread yet */
static int _rx_len;
static ieee802154_rx_info_t _rx_info;
/* the Radio HAL can't measure the RSSI: the one of the last received frame */
static int8_t _last_rssi = OT_RADIO_RSSI_INVALID;

/* source address match table of OpenThread: the children with pending
 * frames, OpenThread gives the extended addresses in little endian */
static bool _src_match;
static otShortAddress _src_match_short[OPENTHREAD_SRC_MATCH_ENTRIES];
static unsigned _src_match_short_num;
static otExtAddress _src_match_ext[OPENTHREAD_SRC_MATCH_ENTRIES];
static unsigned _src_match_ext_num;

static otRadioFrame _tx_frame;
static uint8_t _tx_psdu[OT_RADIO_FRAME_MAX_SIZE];
static otRadioFrame _rx_frame;
static uint8_t _rx_psdu[OT_RADIO_FRAME_MAX_SIZE];
static otRadioFrame _ack_frame;
static uint8_t _ack_psdu[ACK_PSDU_LEN];

static void _rx_done_handler(event_t *event);
static void _tx_start_handler(event_t *event);
static void _tx_done_handler(event_t *event);
static void _tx_failed_handler(event_t *event);

static event_t _ev_rx_done = { .handler = _rx_done_handler };
static event_t _ev_tx_start = { .handler = _tx_start_handler };
static event_t _ev_tx_done = { .handler = _tx_done_handler };
static event_t _ev_tx_failed = { .handler = _tx_failed_handler };

static bool _has_cap(uint32_t cap)
{
    return ieee802154_radio_has_capability(_dev, cap);
}

/* the radio waits for the ACK (and retransmits) */
static bool _radio_handles_ack(void)
{
    return _has_cap(IEEE802154_CAP_FRAME_RETRANS) || _has_cap(IEEE802154_CAP_IRQ_ACK_TIMEOUT);
}

static void _hal_cb(ieee802154_dev_t *dev, ieee802154_trx_ev_t status)
{
    (void)dev;
    event_queue_t *queue = openthread_get_evq();

    switch (status) {
    /* the frame buffer holds the last indicated frame */
    case IEEE802154_RADIO_INDICATION_RX_DONE:
        _rx_crc_error = false;
        _rx_pending = true;
        event_post(queue, &_ev_rx_done);
        break;
    case IEEE802154_RADIO_INDICATION_CRC_ERROR:
        _rx_crc_error = true;
        _rx_pending = true;
        event_post(queue, &_ev_rx_done);
        break;
    case IEEE802154_RADIO_INDICATION_TX_START:
        event_post(queue, &_ev_tx_start);
        break;
    case IEEE802154_RADIO_CONFIRM_TX_DONE:
        event_post(queue, &_ev_tx_done);
        break;
    default:
        break;
    }
}

static void _set_rx(void)
{
    if (ieee802154_radio_set_rx(_dev) < 0) {
        DEBUG("openthread: cannot set the radio in RX\n");
    }
}

/* the transceiver must be IDLE to change the PHY configuration */
static int _config_phy(uint8_t channel, int8_t power)
{
    ieee802154_phy_conf_t conf = {
        .phy_mode = IEEE802154_PHY_OQPSK,
        .channel = channel,
        .pow = power,
    };
    int res;

    if (_tx_busy) {
        /* configured at the end of the transmission */
        _tx_power = power;
        _phy_pending = true;
        return 0;
    }
    ieee802154_radio_set_idle(_dev, true);
    res = ieee802154_radio_config_phy(_dev, &conf);
    if (res == 0) {
        _channel = channel;
        _tx_power = power;
    }
    if ((_state == OT_RADIO_STATE_RECEIVE) || (_state == OT_RADIO_STATE_TRANSMIT)) {
        _set_rx();
    }
    return res;
}

/* OpenThread gives the extended addresses in little endian, the Radio HAL
 * takes them in big endian */
static void _radio_ext_addr(eui64_t *addr, const otExtAddress *ext_addr)
{
    for (unsigned i = 0; i < sizeof(*addr); i++) {
        addr->uint8[i] = ext_addr->m8[sizeof(*addr) - 1 - i];
    }
}

static void _set_filter_mode(void)
{
    ieee802154_radio_set_frame_filter_mode(_dev, _promiscuous ? IEEE802154_FILTER_PROMISC
                                                              : IEEE802154_FILTER_ACCEPT);
}

static void _stop_wait_ack(void)
{
    if (_wait_ack) {
        _wait_ack = false;
        _set_filter_mode();
    }
}

/* ACK of the transmitted frame, for the radios that wait for the ACK
 * themselves: OpenThread only uses its frame pending bit */
static otRadioFrame *_make_ack(bool frame_pending)
{
    _ack_psdu[0] = IEEE802154_FCF_TYPE_ACK | (frame_pending ? IEEE802154_FCF_FRAME_PEND : 0);
    _ack_psdu[1] = 0;
    _ack_psdu[2] = _tx_psdu[2];
    _ack_frame.mLength = ACK_PSDU_LEN;
    _ack_frame.mChannel = _channel;
    _ack_frame.mInfo.mRxInfo.mRssi = OT_RADIO_RSSI_INVALID;
    _ack_frame.mInfo.mRxInfo.mLqi = 0;
    return &_ack_frame;
}

static bool _ack_requested(const otRadioFrame *frame)
{
    return frame->mPsdu[0] & IEEE802154_FCF_ACK_REQ;
}

static void _tx_done(otRadioFrame *ack, otError error)
{
    _state = OT_RADIO_STATE_RECEIVE;
    otPlatRadioTxDone(openthread_get_instance(), &_tx_frame, ack, error);
}

static void _tx_start_handler(event_t *event)
{
    (void)event;
    if (_state == OT_RADIO_STATE_TRANSMIT) {
        otPlatRadioTxStarted(openthread_get_instance(), &_tx_frame);
    }
}

static void _tx_done_handler(event_t *event)
{
    (void)event;
    ieee802154_tx_info_t info = { 0 };

    /* not the transmission of OpenThread: the ACK of a received frame */
    if (!_tx_busy) {
        return;
    }
    if (ieee802154_radio_confirm_transmit(_dev, &info) < 0) {
        /* not finished yet: the radio sends TX done again, or poll */
        if (!_has_cap(IEEE802154_CAP_IRQ_TX_DONE)) {
            event_post(openthread_get_evq(), &_ev_tx_done);
        }
        return;
    }
    _tx_busy = false;
    if (_phy_pending) {
        _phy_pending = false;
        _config_phy(_channel, _tx_power);
    }

    if (_radio_handles_ack()) {
        _set_rx();
        switch (info.status) {
        case TX_STATUS_SUCCESS:
            _tx_done(_ack_requested(&_tx_frame) ? _make_ack(false) : NULL, OT_ERROR_NONE);
            break;
        case TX_STATUS_FRAME_PENDING:
            _tx_done(_make_ack(true), OT_ERROR_NONE);
            break;
        case TX_STATUS_NO_ACK:
            _tx_done(NULL, OT_ERROR_NO_ACK);
            break;
        case TX_STATUS_MEDIUM_BUSY:
        default:
            _tx_done(NULL, OT_ERROR_CHANNEL_ACCESS_FAILURE);
            break;
        }
        return;
    }

    if (info.status == TX_STATUS_MEDIUM_BUSY) {
        _set_rx();
        _tx_done(NULL, OT_ERROR_CHANNEL_ACCESS_FAILURE);
    }
    else if (_ack_requested(&_tx_frame)) {
        /* the ACK timeout is handled by the SubMac of OpenThread, which
         * calls otPlatRadioReceive() when it expires */
        _wait_ack = true;
        ieee802154_radio_set_frame_filter_mode(_dev, IEEE802154_FILTER_ACK_ONLY);
        _set_rx();
    }
    else {
        _set_rx();
        _tx_done(NULL, OT_ERROR_NONE);
    }
}

static void _tx_failed_handler(event_t *event)
{
    (void)event;
    if (_state == OT_RADIO_STATE_TRANSMIT) {
        /* an RX done event may have read a frame since: back to RX */
        _set_rx();
        _tx_done(NULL, _tx_error);
    }
}

/* the source address of @p frame is in the source address match table */
static bool _src_match_find(const otRadioFrame *frame)
{
    otMacAddress src;

    if (otMacFrameGetSrcAddr(frame, &src) != OT_ERROR_NONE) {
        return false;
    }
    if (src.mType == OT_MAC_ADDRESS_TYPE_SHORT) {
        for (unsigned i = 0; i < _src_match_short_num; i++) {
            if (_src_match_short[i] == src.mAddress.mShortAddress) {
                return true;
            }
        }
    }
    else if (src.mType == OT_MAC_ADDRESS_TYPE_EXTENDED) {
        otExtAddress addr;

        /* the frame has the address in big endian */
        for (unsigned i = 0; i < sizeof(addr.m8); i++) {
            addr.m8[i] = src.mAddress.mExtAddress.m8[sizeof(addr.m8) - 1 - i];
        }
        for (unsigned i = 0; i < _src_match_ext_num; i++) {
            if (!memcmp(&_src_match_ext[i], &addr, sizeof(addr))) {
                return true;
            }
        }
    }
    return false;
}

/* frame pending bit of the ACKs to all the Data Requests, for the radios that
 * send the ACKs themselves without address table: set as long as a child has
 * pending frames (or the table isn't used, it overflowed) */
static bool _radio_frame_pending(void)
{
    return !_src_match || (_src_match_short_num > 0) || (_src_match_ext_num > 0);
}

static void _update_radio_frame_pending(void)
{
    if (IS_USED(MODULE_OPENTHREAD_SOFT_ACK)) {
        return;
    }
    /* with a table, the radio sets the bit for the addresses of the table */
    bool enable = _has_cap(IEEE802154_CAP_SRC_ADDR_MATCH) || _radio_frame_pending();

    ieee802154_radio_config_src_address_match(_dev, IEEE802154_SRC_MATCH_EN, &enable);
}

/* frame pending bit of the ACK to a received frame */
static bool _acked_with_frame_pending(const otRadioFrame *frame)
{
    if (!otMacFrameIsAckRequested(frame)) {
        return false;
    }
    if (IS_USED(MODULE_OPENTHREAD_SOFT_ACK)) {
        /* sent in software, as by the simulation platform of OpenThread
         * (Thread 1.2: the data frames of the children too) */
        return (otMacFrameIsDataRequest(frame) || otMacFrameIsData(frame))
               && (!_src_match || _src_match_find(frame));
    }
    /* sent by the radio: Data Requests only */
    if (!otMacFrameIsDataRequest(frame)) {
        return false;
    }
    return _has_cap(IEEE802154_CAP_SRC_ADDR_MATCH) ? _src_match_find(frame)
                                                   : _radio_frame_pending();
}

/* ACK of the received frame, for the radios that don't send it themselves:
 * only the Imm-Ack of the 2003/2006 frames */
static void _send_ack(bool frame_pending)
{
    static uint8_t ack_psdu[IEEE802154_ACK_FRAME_LEN];
    static otRadioFrame ack = { .mPsdu = ack_psdu };
    ieee802154_filter_mode_t mode = IEEE802154_FILTER_ACCEPT;

    if ((_rx_frame.mLength < IEEE802154_ACK_FRAME_LEN) || !otMacFrameIsAckRequested(&_rx_frame)
        || otMacFrameIsAck(&_rx_frame) || otMacFrameIsVersion2015(&_rx_frame)) {
        return;
    }
    /* frames received in promiscuous mode aren't acknowledged */
    if ((ieee802154_radio_get_frame_filter_mode(_dev, &mode) == 0)
        && (mode != IEEE802154_FILTER_ACCEPT)) {
        return;
    }

    otMacFrameGenerateImmAck(&_rx_frame, frame_pending, &ack);

    iolist_t psdu = { .iol_base = ack_psdu, .iol_len = ack.mLength - IEEE802154_FCS_LEN };

    if (_has_cap(IEEE802154_CAP_AUTO_CSMA)) {
        /* no CSMA-CA for an ACK */
        ieee802154_radio_set_csma_params(_dev, NULL, -1);
    }
    if ((ieee802154_radio_write(_dev, &psdu) < 0)
        || (ieee802154_radio_request_transmit(_dev) < 0)) {
        return;
    }
    /* about 350 us on air: wait for the end of the transmission, the TX
     * done event that follows is ignored (the radio isn't in TRANSMIT) */
    while (ieee802154_radio_confirm_transmit(_dev, NULL) == -EAGAIN) {}
}

/* Reads the frame indicated by the radio out of it, if it wasn't read yet:
 * the RX done event or an access to the radio (transmission, state change)
 * read it first, the RX done event gives it to OpenThread. The radio is left
 * IDLE. Returns false if there was no frame to read. */
static bool _read_rx_frame(void)
{
    unsigned state = irq_disable();
    bool pending = _rx_pending;
    bool crc_error = _rx_crc_error;

    _rx_pending = false;
    irq_restore(state);

    if (!pending) {
        return false;
    }
    ieee802154_radio_set_idle(_dev, false);
    if (crc_error) {
        ieee802154_radio_read(_dev, NULL, 0, NULL);
        return true;
    }
    /* read exactly the length of the frame, as the SubMac of RIOT: some
     * radios (socket_zep) only accept the exact length */
    int res = ieee802154_radio_len(_dev);

    if ((res > 0) && (res <= (int)sizeof(_rx_psdu))) {
        res = ieee802154_radio_read(_dev, _rx_psdu, res, &_rx_info);
    }
    else {
        res = -ENOBUFS;
    }
    if (res < 0) {
        /* doesn't fit: discard it */
        ieee802154_radio_read(_dev, NULL, 0, NULL);
        return true;
    }
    _rx_len = res;
    return true;
}

/* gives the frame read in _rx_psdu to OpenThread; @p read_here: read by the
 * RX done event, the radio is IDLE and set back to RX (else it was read
 * before a transmission or a state change, the radio isn't touched) */
static void _receive_done(bool read_here)
{
    _rx_frame.mLength = _rx_len + IEEE802154_FCS_LEN;
    _rx_len = 0;
    _rx_frame.mChannel = _channel;
    _rx_frame.mInfo.mRxInfo.mRssi = ieee802154_rssi_to_dbm(_rx_info.rssi);
    _rx_frame.mInfo.mRxInfo.mLqi = _rx_info.lqi;
    _rx_frame.mInfo.mRxInfo.mTimestamp = _has_cap(IEEE802154_CAP_RX_TIMESTAMP)
                                         ? _rx_info.timestamp / NS_PER_US : 0;
    _rx_frame.mInfo.mRxInfo.mAckedWithFramePending = false;
    _rx_frame.mInfo.mRxInfo.mAckedWithSecEnhAck = false;
    _last_rssi = _rx_frame.mInfo.mRxInfo.mRssi;

    if (_wait_ack) {
        /* the transmission is done, the radio waits for the ACK in RX */
        bool ack = otMacFrameIsAck(&_rx_frame) && (_rx_psdu[2] == _tx_psdu[2]);

        _set_rx();
        if (ack && (_state == OT_RADIO_STATE_TRANSMIT)) {
            _stop_wait_ack();
            _tx_done(&_rx_frame, OT_ERROR_NONE);
        }
        return;
    }

    _rx_frame.mInfo.mRxInfo.mAckedWithFramePending = _acked_with_frame_pending(&_rx_frame);
    if (IS_USED(MODULE_OPENTHREAD_SOFT_ACK) && read_here && (_state == OT_RADIO_STATE_RECEIVE)) {
        _send_ack(_rx_frame.mInfo.mRxInfo.mAckedWithFramePending);
    }
    if (read_here) {
        _set_rx();
    }
    if ((_state == OT_RADIO_STATE_RECEIVE) || (_state == OT_RADIO_STATE_TRANSMIT)) {
        otPlatRadioReceiveDone(openthread_get_instance(), &_rx_frame, OT_ERROR_NONE);
    }
}

static void _rx_done_handler(event_t *event)
{
    (void)event;

    /* a frame read before a transmission or a state change */
    if (_rx_len > 0) {
        _receive_done(false);
    }
    /* the frame indicated since, if any, not during a transmission (it would
     * wait for its end and set RX: the end would be taken for a reception) */
    if (!_tx_busy && _read_rx_frame()) {
        if (_rx_len > 0) {
            _receive_done(true);
        }
        else {
            _set_rx();
        }
    }
}

int openthread_radio_init(ieee802154_dev_t *dev)
{
    int res;

    _dev = dev;
    _dev->cb = _hal_cb;
    _tx_frame.mPsdu = _tx_psdu;
    _rx_frame.mPsdu = _rx_psdu;
    _ack_frame.mPsdu = _ack_psdu;

    res = ieee802154_radio_request_on(_dev);
    if (res < 0) {
        return res;
    }
    while ((res = ieee802154_radio_confirm_on(_dev)) == -EAGAIN) {}
    if (res < 0) {
        return res;
    }

    _set_filter_mode();
    _update_radio_frame_pending();
    ieee802154_radio_set_cca_threshold(_dev, _cca_threshold);
    return _config_phy(DEFAULT_CHANNEL, DEFAULT_TX_POWER);
}

void otPlatRadioGetIeeeEui64(otInstance *aInstance, uint8_t *aIeeeEui64)
{
    (void)aInstance;
    luid_get_eui64((eui64_t *)aIeeeEui64);
}

void otPlatRadioSetPanId(otInstance *aInstance, uint16_t aPanId)
{
    (void)aInstance;
    ieee802154_radio_config_addr_filter(_dev, IEEE802154_AF_PANID, &aPanId);
}

void otPlatRadioSetExtendedAddress(otInstance *aInstance, const otExtAddress *aExtAddress)
{
    (void)aInstance;
    eui64_t addr;

    _radio_ext_addr(&addr, aExtAddress);
    ieee802154_radio_config_addr_filter(_dev, IEEE802154_AF_EXT_ADDR, &addr);
}

void otPlatRadioSetShortAddress(otInstance *aInstance, uint16_t aShortAddress)
{
    (void)aInstance;
    network_uint16_t addr = byteorder_htons(aShortAddress);

    ieee802154_radio_config_addr_filter(_dev, IEEE802154_AF_SHORT_ADDR, &addr);
}

bool otPlatRadioIsEnabled(otInstance *aInstance)
{
    (void)aInstance;
    return _state != OT_RADIO_STATE_DISABLED;
}

otError otPlatRadioEnable(otInstance *aInstance)
{
    (void)aInstance;
    if (_state == OT_RADIO_STATE_DISABLED) {
        _state = OT_RADIO_STATE_SLEEP;
    }
    return OT_ERROR_NONE;
}

otError otPlatRadioDisable(otInstance *aInstance)
{
    (void)aInstance;
    _read_rx_frame();
    _stop_wait_ack();
    ieee802154_radio_set_idle(_dev, true);
    _state = OT_RADIO_STATE_DISABLED;
    return OT_ERROR_NONE;
}

otError otPlatRadioSleep(otInstance *aInstance)
{
    (void)aInstance;
    if (_state == OT_RADIO_STATE_DISABLED) {
        return OT_ERROR_INVALID_STATE;
    }
    _read_rx_frame();
    _stop_wait_ack();
    ieee802154_radio_set_idle(_dev, true);
    _state = OT_RADIO_STATE_SLEEP;
    return OT_ERROR_NONE;
}

otError otPlatRadioReceive(otInstance *aInstance, uint8_t aChannel)
{
    (void)aInstance;
    if (_state == OT_RADIO_STATE_DISABLED) {
        return OT_ERROR_INVALID_STATE;
    }
    _read_rx_frame();
    _stop_wait_ack();
    _state = OT_RADIO_STATE_RECEIVE;
    if (aChannel != _channel) {
        _config_phy(aChannel, _tx_power);
    }
    else {
        _set_rx();
    }
    return OT_ERROR_NONE;
}

otRadioFrame *otPlatRadioGetTransmitBuffer(otInstance *aInstance)
{
    (void)aInstance;
    return &_tx_frame;
}

otError otPlatRadioTransmit(otInstance *aInstance, otRadioFrame *aFrame)
{
    uint8_t max_retries = aFrame->mInfo.mTxInfo.mMaxFrameRetries;
    uint8_t max_backoffs = aFrame->mInfo.mTxInfo.mMaxCsmaBackoffs;
    bool csma = aFrame->mInfo.mTxInfo.mCsmaCaEnabled;
    iolist_t psdu = {
        .iol_base = aFrame->mPsdu,
        .iol_len = aFrame->mLength - IEEE802154_FCS_LEN,
    };

    if (_state != OT_RADIO_STATE_RECEIVE) {
        return OT_ERROR_INVALID_STATE;
    }

    /* a received frame is read before the frame is written: its RX done
     * event, handled during the transmission, doesn't touch the radio */
    if (_read_rx_frame()) {
        _set_rx();
    }
    if (aFrame->mChannel != _channel) {
        _config_phy(aFrame->mChannel, _tx_power);
    }

    bool radio_csma = _has_cap(IEEE802154_CAP_AUTO_CSMA)
                      || _has_cap(IEEE802154_CAP_FRAME_RETRANS);

    /* the backoff was done by OpenThread: assess the channel before the
     * frame is written, the CCA may leave the radio in RX (nrf802154) */
    if (!radio_csma && csma && (ieee802154_radio_cca(_dev) == 0)) {
        /* busy: the transmission ends after this function returns */
        _state = OT_RADIO_STATE_TRANSMIT;
        _tx_error = OT_ERROR_CHANNEL_ACCESS_FAILURE;
        event_post(openthread_get_evq(), &_ev_tx_failed);
        return OT_ERROR_NONE;
    }

    ieee802154_radio_set_idle(_dev, true);

    if (_has_cap(IEEE802154_CAP_FRAME_RETRANS)) {
        ieee802154_radio_set_frame_retrans(_dev, max_retries);
    }
    if (radio_csma) {
        ieee802154_radio_set_csma_params(_dev, NULL, csma ? max_backoffs : -1);
    }

    if ((ieee802154_radio_write(_dev, &psdu) < 0)
        || (ieee802154_radio_request_transmit(_dev) < 0)) {
        /* reported as the end of the transmission: an error returned here
         * is an assertion failure of the SubMac of OpenThread */
        _state = OT_RADIO_STATE_TRANSMIT;
        _tx_error = OT_ERROR_ABORT;
        event_post(openthread_get_evq(), &_ev_tx_failed);
        return OT_ERROR_NONE;
    }
    _tx_busy = true;
    _state = OT_RADIO_STATE_TRANSMIT;

    if (!_has_cap(IEEE802154_CAP_IRQ_TX_START)) {
        otPlatRadioTxStarted(aInstance, aFrame);
    }
    if (!_has_cap(IEEE802154_CAP_IRQ_TX_DONE)) {
        /* no TX done event: poll the end of the transmission */
        event_post(openthread_get_evq(), &_ev_tx_done);
    }
    return OT_ERROR_NONE;
}

int8_t otPlatRadioGetRssi(otInstance *aInstance)
{
    (void)aInstance;
    /* the Radio HAL has no RSSI measure: the RSSI of the last received frame,
     * as the netdev port did, also sampled by the energy scan of OpenThread */
    return _last_rssi;
}

otRadioCaps otPlatRadioGetCaps(otInstance *aInstance)
{
    (void)aInstance;
    otRadioCaps caps = OT_RADIO_CAPS_NONE;

    if (_has_cap(IEEE802154_CAP_FRAME_RETRANS)) {
        caps |= OT_RADIO_CAPS_ACK_TIMEOUT | OT_RADIO_CAPS_TRANSMIT_RETRIES
                | OT_RADIO_CAPS_CSMA_BACKOFF;
    }
    else {
        if (_has_cap(IEEE802154_CAP_AUTO_CSMA)) {
            caps |= OT_RADIO_CAPS_CSMA_BACKOFF;
        }
        if (_has_cap(IEEE802154_CAP_IRQ_ACK_TIMEOUT)) {
            caps |= OT_RADIO_CAPS_ACK_TIMEOUT;
        }
    }
    return caps;
}

bool otPlatRadioGetPromiscuous(otInstance *aInstance)
{
    (void)aInstance;
    return _promiscuous;
}

void otPlatRadioSetPromiscuous(otInstance *aInstance, bool aEnable)
{
    (void)aInstance;
    _promiscuous = aEnable;
    if (!_wait_ack) {
        _set_filter_mode();
    }
}

otError otPlatRadioEnergyScan(otInstance *aInstance, uint8_t aScanChannel, uint16_t aScanDuration)
{
    (void)aInstance;
    (void)aScanChannel;
    (void)aScanDuration;
    return OT_ERROR_NOT_IMPLEMENTED;
}

otError otPlatRadioGetTransmitPower(otInstance *aInstance, int8_t *aPower)
{
    (void)aInstance;
    *aPower = _tx_power;
    return OT_ERROR_NONE;
}

otError otPlatRadioSetTransmitPower(otInstance *aInstance, int8_t aPower)
{
    (void)aInstance;
    return (_config_phy(_channel, aPower) == 0) ? OT_ERROR_NONE : OT_ERROR_INVALID_ARGS;
}

otError otPlatRadioGetCcaEnergyDetectThreshold(otInstance *aInstance, int8_t *aThreshold)
{
    (void)aInstance;
    *aThreshold = _cca_threshold;
    return OT_ERROR_NONE;
}

otError otPlatRadioSetCcaEnergyDetectThreshold(otInstance *aInstance, int8_t aThreshold)
{
    (void)aInstance;
    if (ieee802154_radio_set_cca_threshold(_dev, aThreshold) < 0) {
        return OT_ERROR_INVALID_ARGS;
    }
    _cca_threshold = aThreshold;
    return OT_ERROR_NONE;
}

int8_t otPlatRadioGetReceiveSensitivity(otInstance *aInstance)
{
    (void)aInstance;
    return OPENTHREAD_RX_SENSITIVITY;
}

/* Source address match: disabled by OpenThread, the ACKs to all the Data
 * Requests have the frame pending bit set; enabled, only those of the
 * addresses of the table (the children with pending frames). */
void otPlatRadioEnableSrcMatch(otInstance *aInstance, bool aEnable)
{
    (void)aInstance;
    _src_match = aEnable;
    _update_radio_frame_pending();
}

/* entries of the table of the radio, if it has one, with the addresses in
 * the byte order of the Radio HAL */
static int _radio_src_match_short(ieee802154_src_match_t cmd, otShortAddress short_addr)
{
    network_uint16_t addr = byteorder_htons(short_addr);

    if (!_has_cap(IEEE802154_CAP_SRC_ADDR_MATCH)) {
        return 0;
    }
    return ieee802154_radio_config_src_address_match(_dev, cmd, &addr);
}

static int _radio_src_match_ext(ieee802154_src_match_t cmd, const otExtAddress *ext_addr)
{
    eui64_t addr;

    if (!_has_cap(IEEE802154_CAP_SRC_ADDR_MATCH)) {
        return 0;
    }
    _radio_ext_addr(&addr, ext_addr);
    return ieee802154_radio_config_src_address_match(_dev, cmd, &addr);
}

otError otPlatRadioAddSrcMatchShortEntry(otInstance *aInstance, otShortAddress aShortAddress)
{
    (void)aInstance;
    for (unsigned i = 0; i < _src_match_short_num; i++) {
        if (_src_match_short[i] == aShortAddress) {
            return OT_ERROR_NONE;
        }
    }
    if ((_src_match_short_num == ARRAY_SIZE(_src_match_short))
        || (_radio_src_match_short(IEEE802154_SRC_MATCH_SHORT_ADD, aShortAddress) < 0)) {
        return OT_ERROR_NO_BUFS;
    }
    _src_match_short[_src_match_short_num++] = aShortAddress;
    _update_radio_frame_pending();
    return OT_ERROR_NONE;
}

otError otPlatRadioAddSrcMatchExtEntry(otInstance *aInstance, const otExtAddress *aExtAddress)
{
    (void)aInstance;
    for (unsigned i = 0; i < _src_match_ext_num; i++) {
        if (!memcmp(&_src_match_ext[i], aExtAddress, sizeof(*aExtAddress))) {
            return OT_ERROR_NONE;
        }
    }
    if ((_src_match_ext_num == ARRAY_SIZE(_src_match_ext))
        || (_radio_src_match_ext(IEEE802154_SRC_MATCH_EXT_ADD, aExtAddress) < 0)) {
        return OT_ERROR_NO_BUFS;
    }
    _src_match_ext[_src_match_ext_num++] = *aExtAddress;
    _update_radio_frame_pending();
    return OT_ERROR_NONE;
}

otError otPlatRadioClearSrcMatchShortEntry(otInstance *aInstance, otShortAddress aShortAddress)
{
    (void)aInstance;
    for (unsigned i = 0; i < _src_match_short_num; i++) {
        if (_src_match_short[i] == aShortAddress) {
            _radio_src_match_short(IEEE802154_SRC_MATCH_SHORT_CLEAR, aShortAddress);
            _src_match_short[i] = _src_match_short[--_src_match_short_num];
            _update_radio_frame_pending();
            return OT_ERROR_NONE;
        }
    }
    return OT_ERROR_NO_ADDRESS;
}

otError otPlatRadioClearSrcMatchExtEntry(otInstance *aInstance, const otExtAddress *aExtAddress)
{
    (void)aInstance;
    for (unsigned i = 0; i < _src_match_ext_num; i++) {
        if (!memcmp(&_src_match_ext[i], aExtAddress, sizeof(*aExtAddress))) {
            _radio_src_match_ext(IEEE802154_SRC_MATCH_EXT_CLEAR, aExtAddress);
            _src_match_ext[i] = _src_match_ext[--_src_match_ext_num];
            _update_radio_frame_pending();
            return OT_ERROR_NONE;
        }
    }
    return OT_ERROR_NO_ADDRESS;
}

void otPlatRadioClearSrcMatchShortEntries(otInstance *aInstance)
{
    while (_src_match_short_num > 0) {
        otPlatRadioClearSrcMatchShortEntry(aInstance, _src_match_short[0]);
    }
}

void otPlatRadioClearSrcMatchExtEntries(otInstance *aInstance)
{
    while (_src_match_ext_num > 0) {
        otExtAddress addr = _src_match_ext[0];

        otPlatRadioClearSrcMatchExtEntry(aInstance, &addr);
    }
}
