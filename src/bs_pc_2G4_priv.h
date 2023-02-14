/*
 * Copyright 2018 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef _BS_P2G4_PRIV_H
#define _BS_P2G4_PRIV_H

/**
 * Internal set of functions for libPhyCom 2G4 phy
 */

#include "bs_pc_2G4_types.h"
#include "bs_pc_base.h"

#ifdef __cplusplus
extern "C"{
#endif

void p2G4_dev_req_tx_i(pb_dev_state_t *pb_dev_state, p2G4_tx_t *tx_s, uint8_t *p);
void p2G4_dev_req_txv2_i(pb_dev_state_t *pb_dev_state, p2G4_txv2_t *s, uint8_t *buf);
int p2G4_dev_handle_tx_resp_i(pb_dev_state_t *pb_dev_state, pc_header_t header, p2G4_tx_done_t *tx_done_s);
int p2G4_dev_get_tx_resp_i(pb_dev_state_t *pb_dev_state, p2G4_tx_done_t *tx_done_s);
void p2G4_dev_req_cca_i(pb_dev_state_t *pb_dev_state, p2G4_cca_t *s);
int p2G4_dev_handle_cca_resp_i(pb_dev_state_t *pb_dev_state, pc_header_t header, p2G4_cca_done_t *cca_done_s);
int p2G4_dev_get_rssi_resp_i(pb_dev_state_t *pb_dev_state, p2G4_rssi_done_t *RSSI_done_s);
int p2G4_rx_pick_packet(pb_dev_state_t *pb_dev_state, size_t rx_size, uint8_t **buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif
