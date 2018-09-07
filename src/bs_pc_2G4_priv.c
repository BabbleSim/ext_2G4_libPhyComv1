/*
 * Copyright 2018 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stddef.h>
#include <unistd.h>
#include "bs_tracing.h"
#include "bs_pc_2G4_types.h"
#include "bs_pc_2G4_priv.h"
#include "bs_pc_base.h"
#include "bs_oswrap.h"

void p2G4_dev_req_tx_i(pb_dev_state_t *pb_dev_state, p2G4_tx_t *s,
                       uint8_t *buf)
{
  pb_send_msg(pb_dev_state->ff_dtp, P2G4_MSG_TX, (void *)s, sizeof(p2G4_tx_t));
  pb_send_payload(pb_dev_state->ff_dtp, buf, s->packet_size);
}

int p2G4_dev_handle_tx_resp_i(pb_dev_state_t *pb_dev_state, pc_header_t header,
                              p2G4_tx_done_t *tx_done_s)
{
  int ret;
  if (header == P2G4_MSG_TX_END) {
    ret = pb_dev_read(pb_dev_state, tx_done_s, sizeof(p2G4_tx_done_t));
    if (ret == -1)
      return -1;
    else
      return 0;
  } else if (header == PB_MSG_DISCONNECT) {
    pb_dev_clean_up(pb_dev_state);
    return -1;
  } else {
    INVALID_RESP(header);
    return -1;
  }
}

int p2G4_dev_get_tx_resp_i(pb_dev_state_t *pb_dev_state,
                           p2G4_tx_done_t *tx_done_s)
{
  pc_header_t header;
  int ret;

  ret = pb_dev_read(pb_dev_state, &header, sizeof(header));
  if (ret == -1)
    return -1;

  ret = p2G4_dev_handle_tx_resp_i(pb_dev_state, header,
                                  tx_done_s);
  return ret;
}

int p2G4_dev_get_rssi_resp_i(pb_dev_state_t *pb_dev_state,
                             p2G4_rssi_done_t *RSSI_done_s)
{
  pc_header_t header;
  int ret;

  ret = pb_dev_read(pb_dev_state, &header, sizeof(header));
  if (ret == -1)
      return -1;

  if (header == PB_MSG_DISCONNECT) {
    pb_dev_clean_up(pb_dev_state);
    return -1;
  } else if (header == P2G4_MSG_RSSI_END) {
    ret = pb_dev_read(pb_dev_state, RSSI_done_s, sizeof(p2G4_rssi_done_t));
    if (ret == -1)
      return -1;
    else
      return 0;
  } else {
    INVALID_RESP(header);
    return -1;
  }
}

int p2G4_rx_pick_packet(pb_dev_state_t *pb_dev_state, size_t rx_size,
                        uint8_t **rx_buf, size_t buf_size){
  if (rx_size > 0) {
    uint8_t buf_ok = 0;
    if (rx_size <= buf_size) {
      buf_ok = 1;
    } else if (buf_size == 0) {
      *rx_buf = bs_malloc(rx_size);
      buf_ok = 1;
    }
    if (buf_ok == 0) {
      bs_trace_warning_line("Too small buffer to pick incoming packet (%i < %i"
                            ") => Disconnecting\n", buf_size, rx_size);
      pb_dev_disconnect(pb_dev_state);
      return -1;
    }
    if (pb_dev_read(pb_dev_state, *rx_buf, rx_size) == -1) {
      return -1;
    }
  }
  return 0;
}
