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
/* RSSI measured power level in dBm, format signed 16.16
 * from -32768.0 .. 32768-1/2^16 dBm with a resolution of 1/2^16 = 1.5e-5dBm */
typedef int32_t  p2G4_rssi_power_t;

typedef uint16_t p2G4_modulation_t;

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
#define P2G4_RXSTATUS_CRC_ERROR     0x2
#define P2G4_RXSTATUS_HEADER_ERROR  0x3
#define P2G4_RXSTATUS_NOSYNC        0x4 /*Nothing was synchronized*/
#define P2G4_RXSTATUS_INPROGRESS    0x5 /*The reception is/was ongoing*/

typedef struct __attribute__ ((packed)) {
  /* absolute us this message is sent */
  bs_time_t end_time;
} p2G4_tx_done_t;

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

/** From phy to device **/
/* Tx completed (fully or not) */
#define P2G4_MSG_TX_END          0x100
/* Poking the device to see if it wants to change its abort time */
#define P2G4_MSG_ABORTREEVAL     0x101
/* Matching address found while receiving */
#define P2G4_MSG_RX_ADDRESSFOUND 0x102
/* Rx completed (successfully or not) */
#define P2G4_MSG_RX_END          0x103
/* RSSI measurement completed */
#define P2G4_MSG_RSSI_END        0x104

#ifdef __cplusplus
}
#endif

#endif
