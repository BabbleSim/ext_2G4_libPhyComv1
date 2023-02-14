/*
 * Copyright 2018 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "bs_pc_2G4_types.h"
#include "bs_pc_2G4.h"
#include "bs_pc_2G4_priv.h"
#include "bs_tracing.h"

int p2G4_dev_initcom_s_c(p2G4_dev_state_s_t *p2G4_dev_state, unsigned int dev_nbr,
                         const char* s, const char* p, dev_abort_reeval_f abort_fptr) {
  p2G4_dev_state->abort_f = abort_fptr;
  return pb_dev_init_com(&p2G4_dev_state->pb_dev_state, dev_nbr, s, p);
}

/**
 * Attempt to terminate the simulation
 */
void p2G4_dev_terminate_s_c(p2G4_dev_state_s_t *p2G4_dev_state){
  pb_dev_terminate(&p2G4_dev_state->pb_dev_state);
}

/**
 * Disconnect from the phy
 */
void p2G4_dev_disconnect_s_c(p2G4_dev_state_s_t *p2G4_dev_state){
  pb_dev_disconnect(&p2G4_dev_state->pb_dev_state);
}

/**
 * The phy has just asked us to reevaluate the abort => call the device
 * function which will give us a new abort structure and give it back to the
 * phy.
 * The device may have problems to do so, in that case we send a disconnect
 * to the phy and we return from this function -1
 */
static int p2G4_dev_do_abort_reeval_s(p2G4_dev_state_s_t *p2G4_dev_state, p2G4_abort_t *abort_s) {

  if (p2G4_dev_state->abort_f != NULL) {
    if (p2G4_dev_state->abort_f(abort_s) != 0) {
      bs_trace_warning_line("We (device) are dying in the middle of abort reevaluation!!\n");
      pb_dev_disconnect(&p2G4_dev_state->pb_dev_state);
      return -1;
    }
  } else {
    bs_trace_warning_line("The phy wants to reevaluate an abort but the "
       "device did not register an abort reevaluation call back => I try to "
       "handle it by responding with recheck set to infinite time and leaving "
       "abort as it was\n");
    abort_s->recheck_time = TIME_NEVER;
  }

  pb_send_msg(p2G4_dev_state->pb_dev_state.ff_dtp, P2G4_MSG_RERESP_ABORTREEVAL,
              (void *)abort_s, sizeof(p2G4_abort_t));

  return 0;
}

static pc_header_t get_resp_while_handling_abortreeval_s(p2G4_dev_state_s_t *p2G4_dev_state, p2G4_abort_t *abort) {
  pc_header_t header;
  while (1) {
    int ret;
    ret = pb_dev_read(&p2G4_dev_state->pb_dev_state, &header, sizeof(header));
    if (ret == -1)
        return -1;

    if (header == P2G4_MSG_ABORTREEVAL) { //while the phy answers us with abort reevaluations we handle them and wait for new responses
      ret = p2G4_dev_do_abort_reeval_s(p2G4_dev_state, abort);
      if (ret == -1)
          return -1;
    } else {
      break;
    }
  }
  return header;
}

/**
 * Request a transmissions to the phy
 *
 * returns -1 on error, 0 otherwise
 */
int p2G4_dev_req_tx_s_c_b(p2G4_dev_state_s_t *p2G4_dev_state, p2G4_tx_t *tx_s, uint8_t *packet, p2G4_tx_done_t *tx_done_s)
{
  CHECK_CONNECTED(p2G4_dev_state->pb_dev_state.connected);
  int ret;
  pc_header_t header;

  p2G4_dev_req_tx_i(&p2G4_dev_state->pb_dev_state, tx_s, packet);

  header = get_resp_while_handling_abortreeval_s(p2G4_dev_state, &tx_s->abort);

  ret = p2G4_dev_handle_tx_resp_i(&p2G4_dev_state->pb_dev_state, header,
                                  tx_done_s);
  return ret;
}

/**
 * Request a transmissions to the phy
 *
 * returns -1 on error, 0 otherwise
 */
int p2G4_dev_req_txv2_s_c_b(p2G4_dev_state_s_t *p2G4_dev_state, p2G4_txv2_t *tx_s, uint8_t *packet, p2G4_tx_done_t *tx_done_s)
{
  CHECK_CONNECTED(p2G4_dev_state->pb_dev_state.connected);
  int ret;
  pc_header_t header;

  p2G4_dev_req_txv2_i(&p2G4_dev_state->pb_dev_state, tx_s, packet);

  header = get_resp_while_handling_abortreeval_s(p2G4_dev_state, &tx_s->abort);

  ret = p2G4_dev_handle_tx_resp_i(&p2G4_dev_state->pb_dev_state, header,
                                  tx_done_s);
  return ret;
}

/**
 * Request a transmissions to the phy
 *
 * returns -1 on error, 0 otherwise
 *
 * It does not wait for a response, p2G4_dev_pick_txresp_s_c_b() should be called after
 */
int p2G4_dev_req_tx_s_c(p2G4_dev_state_s_t *p2G4_dev_state, p2G4_tx_t *tx_s, uint8_t *packet) {
  CHECK_CONNECTED(p2G4_dev_state->pb_dev_state.connected);
  p2G4_dev_req_tx_i(&p2G4_dev_state->pb_dev_state, tx_s, packet);
  return 0;
}

/**
 * Request a transmissions to the phy
 *
 * returns -1 on error, 0 otherwise
 *
 * It does not wait for a response, p2G4_dev_pick_txresp_s_c_b() should be called after
 */
int p2G4_dev_req_txv2_s_c(p2G4_dev_state_s_t *p2G4_dev_state, p2G4_txv2_t *tx_s, uint8_t *packet) {
  CHECK_CONNECTED(p2G4_dev_state->pb_dev_state.connected);
  p2G4_dev_req_txv2_i(&p2G4_dev_state->pb_dev_state, tx_s, packet);
  return 0;
}

/**
 * Pickup a tx response (This function is meant only for devices which use p2G4_dev_req_tx_s_c()
 * or p2G4_dev_req_txv2_s_c()
 * Note that this function canNOT handle abort reevaluations.
 * (if those are expected they should have been handled before, or with a different function)
 *
 * returns -1 on error, 0 otherwise
 */
int p2G4_dev_pick_txresp_s_c_b(p2G4_dev_state_s_t *p2G4_dev_state, p2G4_tx_done_t *tx_done_s) {
  CHECK_CONNECTED(p2G4_dev_state->pb_dev_state.connected);
  int ret = p2G4_dev_get_tx_resp_i(&p2G4_dev_state->pb_dev_state, tx_done_s);
  return ret;
}

/**
 * Request a (v1) reception to the phy
 *
 * rx_done_s needs to be allocated by the caller
 *
 * rx_buf is a pointer to a buffer in which the packet will be copied.
 * This buffer shall have buf_size bytes.
 * If buf_size is 0, this function will allocate a new buffer and point
 *  *rx_buf to it (the application must free it afterwards).
 * Otherwise this function will fail if the buffer is too small to fit
 * the incoming packet
 *
 * dev_rxeval_f is a function which will be called when receiving the packet.
 * If the device will accept any packet (quite normal behavior), set this to
 * NULL.
 * dev_rxeval_f() shall return 1, if it accepts the packet, 0 otherwise
 *
 * returns -1 on error, the received response header >=0 otherwise
 */
int p2G4_dev_req_rx_s_c_b(p2G4_dev_state_s_t *p2G4_dev_state, p2G4_rx_t *rx_s,
                          p2G4_rx_done_t *rx_done_s, uint8_t **rx_buf,
                          size_t buf_size, device_eval_rx_f dev_rxeval_f) {

  CHECK_CONNECTED(p2G4_dev_state->pb_dev_state.connected);

  pb_send_msg(p2G4_dev_state->pb_dev_state.ff_dtp,
              P2G4_MSG_RX, (void *)rx_s, sizeof(p2G4_rx_t));

  pc_header_t r_header;
  r_header = get_resp_while_handling_abortreeval_s(p2G4_dev_state, &rx_s->abort);

  if (r_header == P2G4_MSG_RX_ADDRESSFOUND) {
    int ret;

    ret = pb_dev_read(&p2G4_dev_state->pb_dev_state,
                      rx_done_s, sizeof(p2G4_rx_done_t));
    if (ret == -1)
      return -1;

    ret = p2G4_rx_pick_packet(&p2G4_dev_state->pb_dev_state,
                              rx_done_s->packet_size, rx_buf, buf_size);
    if (ret)
      return ret;

    int accept_packet = true;
    if (dev_rxeval_f != NULL) {
      accept_packet = dev_rxeval_f(rx_done_s, *rx_buf);
    }
    pc_header_t header;
    if (accept_packet == true) {
      header = P2G4_MSG_RXCONT;
    } else {
      header = P2G4_MSG_RXSTOP;
    }
    write(p2G4_dev_state->pb_dev_state.ff_dtp, &header, sizeof(header));

    if (accept_packet != true) {
      return r_header;
    }

    r_header = get_resp_while_handling_abortreeval_s(p2G4_dev_state, &rx_s->abort);
  }

  if (r_header == PB_MSG_DISCONNECT) {
    pb_dev_clean_up(&p2G4_dev_state->pb_dev_state);
    return -1;
  } else if (r_header == P2G4_MSG_RX_END) {
    if (pb_dev_read(&p2G4_dev_state->pb_dev_state, rx_done_s, sizeof(p2G4_rx_done_t)) == -1) {
      return -1;
    }
  } else {
    INVALID_RESP(r_header);
  }
  return r_header;
}

/**
 * Request a (v2) reception to the phy
 *
 * rx_done_s needs to be allocated by the caller
 *
 * rx_buf is a pointer to a buffer in which the packet will be copied.
 * This buffer shall have buf_size bytes.
 * If buf_size is 0, this function will allocate a new buffer and point
 *  *rx_buf to it (the application must free it afterwards).
 * Otherwise this function will fail if the buffer is too small to fit
 * the incoming packet
 *
 * dev_rxeval_f is a function which will be called when receiving the packet.
 * If the device will accept any packet (quite normal behavior), set this to
 * NULL.
 * dev_rxeval_f() shall return 1, if it accepts the packet, 0 otherwise
 *
 * returns -1 on error, the received response header >=0 otherwise
 */
int p2G4_dev_req_rxv2_s_c_b(p2G4_dev_state_s_t *p2G4_dev_state, p2G4_rxv2_t *rx_s,
                          p2G4_rxv2_done_t *rx_done_s, uint8_t **rx_buf,
                          size_t buf_size, device_eval_rxv2_f dev_rxeval_f) {

  CHECK_CONNECTED(p2G4_dev_state->pb_dev_state.connected);

  pb_send_msg(p2G4_dev_state->pb_dev_state.ff_dtp,
              P2G4_MSG_RXV2, (void *)rx_s, sizeof(p2G4_rxv2_t));

  pc_header_t r_header;
  r_header = get_resp_while_handling_abortreeval_s(p2G4_dev_state, &rx_s->abort);

  if (r_header == P2G4_MSG_RX_ADDRESSFOUND) {
    int ret;

    ret = pb_dev_read(&p2G4_dev_state->pb_dev_state,
                      rx_done_s, sizeof(p2G4_rx_done_t));
    if (ret == -1)
      return -1;

    ret = p2G4_rx_pick_packet(&p2G4_dev_state->pb_dev_state,
                              rx_done_s->packet_size, rx_buf, buf_size);
    if (ret)
      return ret;

    int accept_packet = true;
    if (dev_rxeval_f != NULL) {
      accept_packet = dev_rxeval_f(rx_done_s, *rx_buf);
    }
    pc_header_t header;
    if (accept_packet == true) {
      header = P2G4_MSG_RXCONT;
    } else {
      header = P2G4_MSG_RXSTOP;
    }
    write(p2G4_dev_state->pb_dev_state.ff_dtp, &header, sizeof(header));

    if (accept_packet != true) {
      return r_header;
    }

    r_header = get_resp_while_handling_abortreeval_s(p2G4_dev_state, &rx_s->abort);
  }

  if (r_header == PB_MSG_DISCONNECT) {
    pb_dev_clean_up(&p2G4_dev_state->pb_dev_state);
    return -1;
  } else if (r_header == P2G4_MSG_RXV2_END) {
    if (pb_dev_read(&p2G4_dev_state->pb_dev_state, rx_done_s, sizeof(p2G4_rxv2_done_t)) == -1) {
      return -1;
    }
  } else {
    INVALID_RESP(r_header);
  }
  return r_header;
}

/**
 * Request a RSSI measurement to the phy
 * RSSI_done_s needs to be allocated by the caller
 *
 * returns -1 if disconnected, 0 otherwise
 */
int p2G4_dev_req_RSSI_s_c_b(p2G4_dev_state_s_t *p2G4_dev_state,
                            p2G4_rssi_t *RSSI_s, p2G4_rssi_done_t *RSSI_done_s)
{
  CHECK_CONNECTED(p2G4_dev_state->pb_dev_state.connected);
  pb_send_msg(p2G4_dev_state->pb_dev_state.ff_dtp,
              P2G4_MSG_RSSIMEAS, (void *)RSSI_s, sizeof(p2G4_rssi_t));
  return p2G4_dev_get_rssi_resp_i(&p2G4_dev_state->pb_dev_state, RSSI_done_s);
}

/**
 * Request a wait to the phy and block until receiving the response
 * If everything goes ok 0 is returned
 *
 * Otherwise, we should disconnect (-1 will be returned)
 */
int p2G4_dev_req_wait_s_c_b(p2G4_dev_state_s_t *p2G4_dev_state, pb_wait_t *wait_s){
  return pb_dev_request_wait_block(&p2G4_dev_state->pb_dev_state, wait_s);
}


/**
 * Request a non blocking wait to the phy
 * Note that eventually the caller needs to pick the wait response
 * from the phy with p2G4_dev_pick_wait_resp_s_c_b()
 */
int p2G4_dev_req_wait_s_c(p2G4_dev_state_s_t *p2G4_dev_state, pb_wait_t *wait_s){
  return pb_dev_request_wait_nonblock(&p2G4_dev_state->pb_dev_state, wait_s);
}

/**
 * Block until getting a wait response from the phy
 * If everything goes ok, the phy has just reached the
 * requested end time and 0 is returned
 * Otherwise, we should disconnect (-1 will be returned)
 */
int p2G4_dev_pick_wait_resp_s_c_b(p2G4_dev_state_s_t *p2G4_dev_state){
  return pb_dev_pick_wait_resp(&p2G4_dev_state->pb_dev_state);
}
