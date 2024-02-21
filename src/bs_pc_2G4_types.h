/*
 * Copyright 2018 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef BS_P2G4_TYPES_H
#define BS_P2G4_TYPES_H

#include "bs_types.h"
#include "bs_pc_base_types.h"
#include "bs_pc_2G4_modulations.h"

#ifdef __cplusplus
extern "C"{
#endif

/*
 * From 0.0 to 80.0 in which frequency is the transmission centered
 * format 8.8 in MHz, offset relative to 2400
 * (can be negative for blockers < 2400)
 */
typedef int16_t  p2G4_freq_t;
#define P2G4_freq_FRACB 8
#define P2G4_INVALID_FREQ 0x7FFF

/* Power level in dBm or gain in dB (depending on context).
 * Format 8.8 (-128.0 .. 127.99609375)*/
typedef int16_t  p2G4_power_t;
#define P2G4_POWER_MIN INT16_MIN
/* RSSI measured power level in dBm, format signed 16.16
 * from -32768.0 .. 32768-1/2^16 dBm with a resolution of 1/2^16 = 1.5e-5dBm */
typedef int32_t  p2G4_rssi_power_t;
#define P2G4_RSSI_POWER_MIN INT32_MIN

typedef uint16_t p2G4_modulation_t;

#define P2G4_RXV2_MAX_ADDRESSES 16
typedef uint64_t p2G4_address_t;

typedef struct __attribute__ ((packed)) {
  /* At this point the Tx/Rx will be stopped abruptly
   * If not used, set to TIME_NEVER */
  bs_time_t abort_time;
  /* Time in which the phy should check if the device wants
   * to change its mind regarding the next abort time.
   * If not used set to TIME_NEVER */
  bs_time_t recheck_time;
} p2G4_abort_t;

typedef struct __attribute__ ((packed)) {
  /* One of P2G4_MOD_* */
  p2G4_modulation_t modulation;
  /* Carrier frequency */
  p2G4_freq_t  center_freq;
} p2G4_radioparams_t;

typedef struct __attribute__ ((packed)) {
  /* Absolute microsecond when the measurement should be taken */
  bs_time_t meas_time;
  p2G4_radioparams_t radio_params;
  p2G4_power_t antenna_gain;
} p2G4_rssi_t ;

typedef struct __attribute__ ((packed)) {
  /* RSSI measured value by the modem */
  p2G4_rssi_power_t RSSI;
} p2G4_rssi_done_t;

typedef struct __attribute__ ((packed)) {
  /* Absolute us when the receiver starts scanning */
  bs_time_t start_time;
  /* Time in which we need to get a preamble + address match before giving up
   * Once we get a preamble + address match we will continue to receive the
   * whole packet, unless there is a header error.
   * We scan in the range [ start_time,  start_time + scan_duration - 1] us */
  uint32_t scan_duration;
  /* Address we search for */
  uint32_t phy_address;
  p2G4_radioparams_t radio_params;
  p2G4_power_t antenna_gain;
  /* Reception tolerance: */
  /* How many errors do we accept before considering the preamble + address sync lost
   * (if there is less errors than this the packet can be received correctly) */
  uint16_t sync_threshold;
  /* How many errors do we accept in the header before giving a header error
   * (automatically in the phy)
   * Note: any header error will result at least in a CRC error
   */
  uint16_t header_threshold;
  /* Packet parameters: */
  /*in us, duration of the preamble and start flag */
  uint16_t pream_and_addr_duration;
  /* in us duration of the BLE header */
  uint16_t header_duration;
  /*
   * Data rate in bits per second
   */
  uint32_t bps;
  /*
   * Structure defining when the device may want to abort the reception
   * Note: abort_time shall be > start_time
   */
  p2G4_abort_t abort;
} p2G4_rx_t;

typedef struct __attribute__ ((packed)) {
  /* absolute us when the first bit of the packet is sent to the air
   * = the begining of the preamble */
  bs_time_t start_time;
  /* absolute us when the last bit of the packet is sent to the air */
  bs_time_t end_time;
  /*
   * Structure defining when the device may want to abort the transmission
   * Note: abort_time shall be > start_time
   */
  p2G4_abort_t abort;
  /* Phy address/access code used in the packet */
  uint32_t phy_address;
  p2G4_radioparams_t radio_params;
  /* In dBm, transmitter power level (including antenna gain) */
  p2G4_power_t power_level;
  /* Packet size in bytes; Only used for moving the payload, not modeling related */
  uint16_t packet_size;
} p2G4_tx_t;

typedef struct __attribute__ ((packed)) {
  /* one of P2G4_RXSTATUS* */
  uint16_t status;
  /* Found packet size in bytes */
  uint16_t packet_size;
  /* if Status != P2G4_RXSTATUS_NOSYNC: absolute us when the address ended.
   * otherwise when the scan window ended */
  bs_time_t rx_time_stamp;
  /* Absolute us this message is sent */
  bs_time_t end_time;
  /* RSSI measured values by the modem */
  p2G4_rssi_done_t rssi;
} p2G4_rx_done_t;

#define P2G4_RXSTATUS_OK            0x1
#define P2G4_RXSTATUS_CRC_ERROR     0x2 /* At least 1 bit error was detected during the header or payload */
#define P2G4_RXSTATUS_PACKET_CONTENT_ERROR 0x2 /* Rename of CRC_ERROR */
#define P2G4_RXSTATUS_HEADER_ERROR  0x3 /* More bit errors found during header than <header_threshold> */
#define P2G4_RXSTATUS_NOSYNC        0x4 /*Nothing was synchronized*/
#define P2G4_RXSTATUS_INPROGRESS    0x5 /*The reception is/was ongoing*/

typedef struct __attribute__ ((packed)) {
  /* absolute us this message is sent */
  bs_time_t end_time;
} p2G4_tx_done_t;


/************************************************************
 * V2 API extension
 * In *Alpha* state, API backward/forward compatibility NOT guaranteed.
 * API breaking changes can be expected in future releases
 *
 * This includes:
 *  * Txv2 procedure p2G4_txv2_t (followed by p2G4_tx_done_t)
 *  * Rxv2 procedure p2G4_rxv2_t & p2G4_rxv2_done_t
 *  * CCA search procedure p2G4_search_comp_mod_done_t & p2G4_search_comp_mod_t
 *
 * Note this API includes the Txv2 and Rxv2 procedures which provide
 * a superset of the functionality their v1 counterparts supported
 * The v1 and v2 procedures are fully cross-compatible
 * (within the limitation imposed by the v1 party)
 *
 * The v1 API is NOT deprecated:
 * Devices utilizing the v1 API are not expected to migrate to the
 * new API, and are discouraged from doing so while the
 * API is in alpha and beta state.
 ************************************************************/

typedef struct __attribute__ ((packed)) {
  /* Absolute us when the transmittion starts.
   * This will typically be the same as start_packet_time.
   * But, it may be:
   *  * Earlier, if the transmitter starts emitting a spurious carrier
   *    or similar before the actual packet start
   *  * The transmitter eats into the preamble by ramping up too late
   *
   * Note: The device must send this request no later than the Phy simulated time start_tx_time
   */
  bs_time_t start_tx_time;
  /* Absolute us when the transmitter stops transmitting
   * This will typically be the same as end_packet_time.
   * But, it may be:
   *  * Longer, if the transmitter continuous emitting a spurious carrier
   *    or desired (i.e. BLE CTE) after the actual packet end
   */
  bs_time_t end_tx_time;

  /* Absolute us when the first bit of a protocol compliant packet would be sent to the air
   * i.e. typically the beginning of the preamble
   * Note that a real transmitter may send some carrier/noise before this,
   * or may be purposely configured to truncate the beginning of the preamble.
   * Neither of these 2 effects should be reflected in this value
   */
  bs_time_t start_packet_time;
  /* Absolute us when the last bit of the packet is sent to the air
   * Note that this should only include "data" bits, and not extra transmitted tones
   * either desired (for ex. BLE CTE) or undesired (possible spurious tails/carrier)
   */
  bs_time_t end_packet_time;

  /* {Phy,access} {address,code}/{sync,start} {word,flag,delimiter} used in the packet
   * If the protocol does not use it, or uses less than 64 bits
   * you must set the unused bits to 0
   * For BLE this should be set to the 32bit "access address" (not encoded in case of coded phy)
   * For 802.15.4 this should be set to the 8 bit SFD
   * For other protocols, the address may include more than just this,
   * while both Tx and Rx agree by convention on what it includes.
   */
  p2G4_address_t phy_address;

  /*
   * Structure defining when the device may want to abort the transmission
   * Note: abort_time shall be > start_tx_time
   */
  p2G4_abort_t abort;

  p2G4_radioparams_t radio_params;
  /* In dBm, transmitter power level (including antenna gain) */
  p2G4_power_t power_level;

  /* (if coded) Which coding rate, the data is sent with
   * Note that this is only used for a == or != check.*/
  uint16_t coding_rate;

  /* Packet size in bytes; Only used for moving the payload, not modeling related */
  uint16_t packet_size;

  /* Note: For mapping the old p2G4_tx_t API to this,
   * start_tx_time = start_packet_time = v1 start_time
   * end_tx_time = end_packet_time = v1 end_time
   * phy_address is just 0 extended
   * coding_rate = 0
   */
} p2G4_txv2_t;


typedef struct __attribute__ ((packed)) {
  /* Absolute us when the receiver starts scanning */
  bs_time_t start_time;

  /*
   * Structure defining when the device may want to abort the reception
   * Note: abort_time shall be > start_time
   */
  p2G4_abort_t abort;

  /* Time in which we need to get a preamble + address match before giving up
   * Once we get a preamble + address match we will continue to receive the
   * whole packet, unless there is a header error.
   * We scan in the range [ start_time,  start_time + scan_duration - 1] us, unless
   * scan_duration == UINT32_MAX, in which case the scan does not end (until aborted) */
  uint32_t scan_duration;

  /* Duration of the packet the receiver will listen for.
   * That is, for how long the receiver will be receiving bits,
   * instead of during Tx.start_packet_time -> Tx.end_packet_time
   * The receiver will think the packet lasts Tx.start_packet_time -> (Tx.start_packet_time+forced_packet_duration-1)
   * A value of UINT32_MAX or 0 means it follows the Tx packet duration.
   * Note: This is not a filter, but a way to model receivers mistakenly
   * decoding the length or code/rate indication fields.
   */
  uint32_t forced_packet_duration;

  /* Error calculation rate, in times per second.
   * Typically the data rate in bits per second (For binary FSK, PSK, ASK.. modulations)
   * Note: This is just the bit error and SNR calculation rate.
   * It does not affect the actual packet duration in any way.
   * It is up to the receiver to set it in accordance with the type of modulation.
   * For more complex modulations or coded packets, it may be the chip-rate,
   * symbol-rate or coded-bit-rate */
  uint32_t error_calc_rate;

  p2G4_radioparams_t radio_params;

  p2G4_power_t antenna_gain;

  /* (if coded) Which coding rate, the data is received with
   * Note that this is only used for a == or != check. And that when set
   * different than for the transmitter, the BER will be 50%
   * For BLE CodedPhy, this should be set to 2 or 8 for S=2 and S=8 respectively
   */
  uint16_t coding_rate;

  /* Packet parameters: */

  /* In us, duration of the preamble and start flag
   * It can be set to zero */
  uint16_t pream_and_addr_duration;

  /* In us duration of the "header". It can be 0.
   * For uncoded BLE this correspond to the BLE header
   * For 15.4 this should be set to 0
   * For Nordic's ESB this should be set to the duration of 8 bits */
  uint16_t header_duration;

  /* Reception tolerance: */

  /* How many us into the transmitted preamble we accept having opened the receiver in,
   * and consider it is still possible to receive a packet
   *
   * Note: The missing piece of the preamble will NOT be counted as having "bit errors"
   * for the sync_threshold calculation. Actually the receive procedure will just skip
   * the whole acceptable_pre_truncation, and start checking for bit errors after.
   * Note: acceptable_pre_truncation must be <= pream_and_addr_duration
   */
  uint16_t acceptable_pre_truncation;

  /* How many errors do we accept before considering the preamble + address sync lost */
  uint16_t sync_threshold;

  /* How many errors do we accept in the header before giving a header error
   * (automatically in the phy)
   * Note: any header error will result at least in a CRC/packet error
   */
  uint16_t header_threshold;

  /* When set to 0 the Rx is not pre-locked.
   * When set to 1, the Rx is already pre-locked.
   * In that case, the Rx will not search for a transmitter but continue receiving from the last
   * transmitter that it just was.
   * The receiver will go directly to sync mode.
   *
   * Note that in this case :
   *   * scan_duration should be set to pream_and_addr_duration + 1.
   *   * acceptable_pre_truncation should be set to 0
   *   * it may fail during sync depending on the sync_threshold and/or the transmitter disappearing.
   *     even if pream_and_addr_duration == 0 (an instantaneous check will be performed still).
   *     You can avoid this from happening for bit errors by setting sync_threshold high enough (for ex. UINT16_MAX)
   *   * Similarly it may fail during the header reception just like with a normal packet.
   *   * Any given phy_addr[] will be ignored.
   *
   * Note: prelocked_tx when the previous reception failed to sync leads to undefined behaviour
   */
  uint8_t prelocked_tx;

  /* Requested type of response
   *  * 0: Basic response
   *  * (reserved) 1: Include also bit error mask
   *  * (reserved) all others
   */
  uint8_t resp_type;

  /* How many addresses we actively search for, must be >= 1 & < 8
   * And how many elements of p2G4_address_t phy_addr[] follow */
  uint8_t n_addr;

  /* Note: For mapping the old p2G4_rx_t API to this,
   * n_addr = 1
   * phy_address[0] = phy_address (0 extended)
   * error_calc_rate = v1 bps
   * acceptable_pre_truncation = 0
   * resp_type = 0;
   * forced_packet_duration = UINT32_MAX
   * coding_rate = 0
   */
} p2G4_rxv2_t;


typedef struct __attribute__ ((packed)) {
  /* if Status != P2G4_RXSTATUS_NOSYNC: absolute us when the address ended.
   * otherwise when the scan window ended */
  bs_time_t rx_time_stamp;
  /* Absolute us this message is sent */
  bs_time_t end_time;

  /*Matched address (if any)*/
  p2G4_address_t phy_address;

  /*The transmitter coding rate*/
  uint16_t coding_rate;

  /* RSSI measured value by the modem */
  p2G4_rssi_done_t rssi;

  /* one of P2G4_RXSTATUS* */
  uint16_t status;
  /* Found packet size in bytes */
  uint16_t packet_size;

} p2G4_rxv2_done_t;


/**
 * Search for a compatible modulation and/or
 * do an average energy measurements on the channel
 * Typically used for CCA procedures
 */
typedef struct __attribute__ ((packed)) {
  /* Absolute us when the receiver starts */
  bs_time_t start_time;

  /*
   * Structure defining when the device may want to abort the reception
   * Note: abort_time shall be > start_time
   */
  p2G4_abort_t abort;

  /* For how long the device will check for
   * We scan in the range [ start_time,  start_time + scan_duration - 1] us */
  uint32_t scan_duration;

  /* Scan period
   * How often the device wants to check
   * The Phy will do ceil(scan_duration/scan_period) measurements, at start_time + i*scan_period */
  uint32_t scan_period;

  /*Modulation we search for (and in which the receiver is set) and center frequency */
  p2G4_radioparams_t radio_params;

  /* Rx power threshold with which a compatible transmitter will be considered found */
  p2G4_rssi_power_t mod_threshold;
  /* RSSI power threshold with which the energy detection will be considered over threshold */
  p2G4_rssi_power_t rssi_threshold;

  /*Gain of the Rx antenna*/
  p2G4_power_t antenna_gain;

  /* Stop as soon as a compatible modulation is found with power over mod_threshold (1)
   * Stop as soon as any RRSI measurement is over rssi_threshold (2)
   * either (3)
   * Or continue until the end of the scan_duration (0) */
  uint8_t stop_when_found;
} p2G4_cca_t;


typedef struct __attribute__ ((packed)) {
  /* Absolute us this message is sent */
  bs_time_t end_time;

  /* The averaged RSSI measurement (Of all scan_periods) */
  p2G4_rssi_power_t RSSI_ave;
  /* The max RSSI measurement (Of all scan_periods) */
  p2G4_rssi_power_t RSSI_max;
  /* RRSI power level measured when a matching Tx modulation was found (if several are found over time, the greater level) */
  p2G4_rssi_power_t mod_rx_power;
  /* Was a compatible transmitter with power over mod_threshold found (1) or not (0) */
  uint8_t mod_found;
  /* Was at any point the RRSI level over rssi_threshold (1) or not (0) */
  uint8_t rssi_overthreshold;
} p2G4_cca_done_t;


/*
 * Commands and responses IDs:
 */

/** Message headers from device to phy **/
/* The device will transmit */
#define P2G4_MSG_TX             0x02
/* The device wants to attempt to receive */
#define P2G4_MSG_RX             0x11
/* Continue receiving (the device likes the address and headers of the packet) */
#define P2G4_MSG_RXCONT         0x12
/* Stop reception (the device does not likes the address or headers of the packet) => The phy will stop this reception */
#define P2G4_MSG_RXSTOP         0x13
/* Do an RSSI measurement*/
#define P2G4_MSG_RSSIMEAS       0x14
/* Device is successfully providing a new p2G4_abort_t */
#define P2G4_MSG_RERESP_ABORTREEVAL 0x15
/* Continue receiving (the device likes the address and headers of the packet, and it provides an updated abort substructure) */
#define P2G4_MSG_RXV2CONT       0x16


/* The device will transmit (updated/v2 Tx API) */
#define P2G4_MSG_TXV2           0x22
/* The device wants to attempt to receive (updated/v2 Rx API) */
#define P2G4_MSG_RXV2           0x31
/* The device wants to do a CCA check (new v2 API) */
#define P2G4_MSG_CCA_MEAS       0x32
/* Device is requesting an immediate RSSI measurement during an Rx abort reevaluation (new for v2 API) */
#define P2G4_MSG_RERESP_IMMRSSI 0x16


/** From Phy to device **/
/* Tx completed (fully or not) */
#define P2G4_MSG_TX_END          0x100
/* Poking the device to see if it wants to change its abort time */
#define P2G4_MSG_ABORTREEVAL     0x101
/* Phy responds to the device with the immediate RRSI measurement
 * while re-requesting a new abort reeval.(new for v2 API) */
#define P2G4_MSG_IMMRSSI_RRSI_DONE 0x102


/* Matching address found while receiving */
#define P2G4_MSG_RX_ADDRESSFOUND 0x102
/* Rx completed (successfully or not) */
#define P2G4_MSG_RX_END          0x103
/* RSSI measurement completed */
#define P2G4_MSG_RSSI_END        0x104

/* Matching address found while receiving (new v2 API) */
#define P2G4_MSG_RXV2_ADDRESSFOUND 0x112
/* Rx completed (successfully or not) (new v2 API) */
#define P2G4_MSG_RXV2_END          0x113
/* Search CCA check completed (new v2 API) */
#define P2G4_MSG_CCA_END           0x114

#ifdef __cplusplus
}
#endif

#endif
