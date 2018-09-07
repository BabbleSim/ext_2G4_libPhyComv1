/*
 * Copyright 2018 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "bs_pc_2G4_types.h"
#include "bs_pc_2G4.h"
#include "bs_pc_2G4_priv.h"
#include "bs_tracing.h"

int p2G4_dev_initCom_s_nc(p2G4_dev_state_nc_t *p2G4_dev_state, uint d,
                          const char* s, const char* p) {
  return pb_dev_init_com(&p2G4_dev_state->pb_dev_state, d, s, p);
}

void p2G4_dev_terminate_s_nc(p2G4_dev_state_nc_t *p2G4_dev_state){
  pb_dev_terminate(&p2G4_dev_state->pb_dev_state);
}

void p2G4_dev_disconnect_s_nc(p2G4_dev_state_nc_t *p2G4_dev_state){
  pb_dev_disconnect(&p2G4_dev_state->pb_dev_state);
}

static int p2G4_dev_get_tx_resp_nc(p2G4_dev_state_nc_t *c2G4_dev_st) {
  pc_header_t header;
  int ret;

  ret = pb_dev_read(&c2G4_dev_st->pb_dev_state, &header, sizeof(pc_header_t));
  if (ret == -1)
    return -1;

  if (header == P2G4_MSG_ABORTREEVAL) {
    c2G4_dev_st->ongoing = Tx_Abort_Reeval_2G4;
    return header;
  }

  c2G4_dev_st->ongoing = Nothing_2G4;

  ret = p2G4_dev_handle_tx_resp_i(&c2G4_dev_st->pb_dev_state, header, c2G4_dev_st->tx_done_s);
  if (ret == -1)
    return -1;
  else
    return header;
}

/**
 * Request a transmissions to the phy
 *
 * tx_done_s needs to point to an allocated structure. Its content will be overwritten
 *
 * returns -1 on error, otherwise the response from the phy.
 * Possible phy responses are:
 *   * P2G4_MSG_TX_END : (updates the tx_done_s)
 *        The transaction has terminated, the device may start a new transaction
 *   * P2G4_MSG_ABORTREEVAL
 *        The device shall call p2G4_dev_provide_new_tx_abort_s_nc_b() with a new abort structure
 */
int p2G4_dev_req_tx_s_nc_b(p2G4_dev_state_nc_t *c2G4_dev_st, p2G4_tx_t *tx_s, uint8_t *packet, p2G4_tx_done_t *tx_done_s) {

  CHECK_CONNECTED(c2G4_dev_st->pb_dev_state.connected);
  c2G4_dev_st->tx_done_s = tx_done_s;

  if ( c2G4_dev_st->ongoing != Nothing_2G4 ) {
    bs_trace_error_line("Tried to request a new tx while some other transaction was ongoing\n");
  }

  p2G4_dev_req_tx_i(&c2G4_dev_st->pb_dev_state, tx_s, packet);

  return p2G4_dev_get_tx_resp_nc(c2G4_dev_st);
}

/**
 * Provide the phy a new abort struct (during a Tx transaction)
 *
 * returns -1 on error, otherwise the response from the phy.
 * The possible responses depend on the transaction which is ongoing
 *
 * Note that the response structures will come thru the pointers
 * which were provided with the call which initiated the transaction
 */
int p2G4_dev_provide_new_tx_abort_s_nc_b(p2G4_dev_state_nc_t *c2G4_dev_st, p2G4_abort_t * abort){
  CHECK_CONNECTED(c2G4_dev_st->pb_dev_state.connected);

  if ( c2G4_dev_st->ongoing != Tx_Abort_Reeval_2G4 ) {
    bs_trace_error_line("Tried to send a new Tx Abort substruct but we are not in a Tx transaction abort reevaluation!\n");
  }

  pb_send_msg(c2G4_dev_st->pb_dev_state.ff_dtp, P2G4_MSG_RERESP_ABORTREEVAL,
              (void *)abort, sizeof(p2G4_abort_t));

  return p2G4_dev_get_tx_resp_nc(c2G4_dev_st);
}

/**
 * Request a wait to the phy and block until receiving the response
 * If everything goes ok 0 is returned
 *
 * Otherwise, we should disconnect (-1 will be returned)
 */
int p2G4_dev_req_wait_s_nc_b(p2G4_dev_state_nc_t *p2G4_dev_state, pb_wait_t *wait_s){
  return pb_dev_request_wait_block(&p2G4_dev_state->pb_dev_state, wait_s);
}

static int c2G4_handle_rx_responses_s_nc(p2G4_dev_state_nc_t *c2G4_dev_st, pc_header_t header){
  int ret;
  p2G4_rx_done_t *rx_done_s = c2G4_dev_st->rx_done_s;

  if (header == P2G4_MSG_ABORTREEVAL) {
    c2G4_dev_st->ongoing = Rx_Abort_Reeval_2G4;

  } else if ((header == P2G4_MSG_RX_ADDRESSFOUND) && (c2G4_dev_st->WeGotAddress == false )) {
    ret = pb_dev_read(&c2G4_dev_st->pb_dev_state, rx_done_s, sizeof(p2G4_rx_done_t));
    if (ret == -1)
      return -1;

    ret = p2G4_rx_pick_packet(&c2G4_dev_st->pb_dev_state, rx_done_s->packet_size,
                              c2G4_dev_st->rxbuf, c2G4_dev_st->bufsize);
    if (ret == -1)
      return ret;

    c2G4_dev_st->WeGotAddress = true;
    c2G4_dev_st->ongoing = Rx_Header_Eval_2G4;

  } else if (header == PB_MSG_DISCONNECT) {
    c2G4_dev_st->ongoing = Nothing_2G4;
    pb_dev_clean_up(&c2G4_dev_st->pb_dev_state);
    return -1;
  } else if (header == P2G4_MSG_RX_END) {
    c2G4_dev_st->ongoing = Nothing_2G4;
    ret = pb_dev_read(&c2G4_dev_st->pb_dev_state, rx_done_s, sizeof(p2G4_rx_done_t));
    if (ret == 1)
      return -1;
  } else {
    INVALID_RESP(header);
  }
  return header;
}

/**
 * Continue reception after an address evaluation request
 *  bool dev_accepts defines if the device accepts the packet or not
 *
 * if dev_accepts is false, the reception ends and this function returns 0
 * otherwise, the function will eventually return
 *  P2G4_MSG_RX_END ( and update rx_done_s )
 *  or a new P2G4_MSG_ABORTREEVAL
 *
 * Note that the response structures will come thru the pointers which
 * were provided with the call which initiated the transaction
 */
int p2G4_dev_rx_cont_after_addr_s_nc_b(p2G4_dev_state_nc_t *p2G4_dev_state, bool dev_accepts){
  CHECK_CONNECTED(p2G4_dev_state->pb_dev_state.connected);

  if ( p2G4_dev_state->ongoing != Rx_Header_Eval_2G4 ) {
    bs_trace_error_line("Tried to continue from an Rx Header eval, but we are not doing that now..\n");
  }
  pc_header_t header;

  if (dev_accepts) {
    header = P2G4_MSG_RXCONT;
  } else {
    header = P2G4_MSG_RXSTOP;
  }
  write(p2G4_dev_state->pb_dev_state.ff_dtp, &header, sizeof(header));

  if (!dev_accepts) {
    p2G4_dev_state->ongoing = Nothing_2G4;
    return 0;
  }

  if (pb_dev_read(&p2G4_dev_state->pb_dev_state, &header, sizeof(header)) == -1) {
    return -1;
  }

  return c2G4_handle_rx_responses_s_nc(p2G4_dev_state, header);
}

/**
 * Provide the phy a new abort struct (during an Rx transaction)
 *
 * returns -1 on error, otherwise the response from the phy.
 * The possible responses depend on the step in the ongoing transaction
 *
 * Note that the response structures will come thru the pointers which
 * were provided with the call which initiated the transaction
 */
int p2G4_dev_provide_new_rx_abort_s_nc_b(p2G4_dev_state_nc_t *p2G4_dev_state, p2G4_abort_t * abort){
  CHECK_CONNECTED(p2G4_dev_state->pb_dev_state.connected);
  if ( p2G4_dev_state->ongoing != Rx_Abort_Reeval_2G4 ) {
    bs_trace_error_line("Tried to send a new Rx Abort substruct but we are not in a Rx transaction abort reevaluation!\n");
  }
  pc_header_t header;

  pb_send_msg(p2G4_dev_state->pb_dev_state.ff_dtp, P2G4_MSG_RERESP_ABORTREEVAL,
      (void *)abort,  sizeof(p2G4_abort_t));

  if (pb_dev_read(&p2G4_dev_state->pb_dev_state, &header, sizeof(header)) == -1) {
    return -1;
  }

  return c2G4_handle_rx_responses_s_nc(p2G4_dev_state, header);
}

int p2G4_dev_req_RSSI_s_nc_b(p2G4_dev_state_nc_t *p2G4_dev_st, p2G4_rssi_t *RSSI_s, p2G4_rssi_done_t *RSSI_done_s){
  CHECK_CONNECTED(p2G4_dev_st->pb_dev_state.connected);
  pb_send_msg(p2G4_dev_st->pb_dev_state.ff_dtp,
              P2G4_MSG_RSSIMEAS, (void *)RSSI_s, sizeof(p2G4_rssi_t));
  return p2G4_dev_get_rssi_resp_i(&p2G4_dev_st->pb_dev_state, RSSI_done_s);
}

/**
 * Request a reception to the phy
 *
 * rx_buf is a pointer to a buffer in which the packet will be copied.
 * The buffer shall have buf_size bytes.
 * If buf_size is 0, this function will allocate a new buffer and point
 *  *RxBuf to it (the application must free it afterwards).
 * Otherwise this function will fail if the buffer is to small to read the
 * incoming packet
 *
 * returns -1 on error, the received response header >=0 otherwise
 */
int p2G4_dev_req_rx_s_nc_b(p2G4_dev_state_nc_t *p2G4_dev_state, p2G4_rx_t *rx_s, p2G4_rx_done_t *rx_done_s, uint8_t **rx_buf, size_t buf_size) {
  CHECK_CONNECTED(p2G4_dev_state->pb_dev_state.connected);

  if ( p2G4_dev_state->ongoing != Nothing_2G4 ) {
    bs_trace_error_line("Tried to request a new Rx while another transaction was ongoing\n");
  }

  pb_send_msg(p2G4_dev_state->pb_dev_state.ff_dtp, P2G4_MSG_RX,
              (void *)rx_s,  sizeof(p2G4_rx_t));

  p2G4_dev_state->bufsize = buf_size;
  p2G4_dev_state->rxbuf   = rx_buf;
  p2G4_dev_state->rx_done_s = rx_done_s;
  p2G4_dev_state->WeGotAddress = false;

  pc_header_t header;
  int ret;

  ret = pb_dev_read(&p2G4_dev_state->pb_dev_state, &header, sizeof(header));
  if (ret==-1)
    return -1;

  return c2G4_handle_rx_responses_s_nc(p2G4_dev_state, header);
}
