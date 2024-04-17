/*
 * Copyright 2018 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef _BS_COM_2G4_H
#define _BS_COM_2G4_H

#include "bs_pc_2G4_types.h"
#include "bs_pc_base.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C"{
#endif

/**
 * Note: 4 sets of functions are provided for devices to interact with the phy
 * Although the provided functionality is very similar, each of these sets allow
 * to quickly build different types of device HW models.
 * Note that this functions do not change the underlying API/ABI to the phy.
 *
 * These sets are divided as follows:
 *   With vs without callbacks:
 *     With callbacks: (_c)
 *       The set with callbacks allows for simpler devices models.
 *       All phy-device protocol handshakes are hidden inside libCom calls.
 *       When a device decision is needed in the middle of a handshake, a
 *       callback in the device is called.
 *       But, registering the callbacks is optional, in which case either that
 *       device decision is assumed always positive (evaluation of an ongoing
 *       reception) or the functionality is just not used (abort reevaluation)
 *
 *     Without callbacks: (_nc):
 *       The device must handle on its own all possible responses from the
 *       phy and call the appropriate function afterwards to continue the
 *       handshakes.
 *
 *   State-less vs with memory:
 *     State-less (_s) calls rely on the device keeping and owning the
 *     structure with the link state.
 *     The API "with memory" is just a convenience, where the state is kept
 *     internally in libCom
 *
 * Note that calls to these 4 sets canNOT be mixed
 *
 * Note: Not all version have functions for the whole device-phy API.
 *
 * The functions which are blocking until a phy response is received are suffixed with _b.
 * Except the initcom functions which are always blocking
 */

/*
 * API with call-backs and memory
 */

/* Function prototype for the device callback mechanism to reevaluate the abort
 * during Tx or Rx abort_s is the original one sent with the Tx or Rx request.
 * Therefore the current time == abort_s.recheck_time
 *
 * Returns 0 if everything went ok, -1 otherwise (=> disconnect)
 */
typedef int (*dev_abort_reeval_f)(p2G4_abort_t* abort_s);

/* Function prototype for the device to evaluate if it wants to accept an
 * incoming packet or not (doesn't like the sync word and or header)
 * v1 API
 * This function shall return 1 if it accepts the packet, 0 otherwise
 */
typedef int (*device_eval_rx_f)(p2G4_rx_done_t* rx_done, uint8_t *buff);

/* Function prototype for the device to evaluate if it wants to accept an
 * incoming packet or not (doesn't like the sync word and or header)
 * v2 API
 * This function shall return 1 if it accepts the packet, 0 otherwise
 */
typedef int (*device_eval_rxv2_f)(p2G4_rxv2_done_t* rx_done, uint8_t *buff);

int p2G4_dev_initcom_c(uint d, const char* s, const char* p, dev_abort_reeval_f abort_f);
int p2G4_dev_req_rx_c_b(p2G4_rx_t *rx_s, p2G4_rx_done_t *rx_done_s, uint8_t **rx_buf, size_t buf_size, device_eval_rx_f fptr);
int p2G4_dev_req_rxv2_c_b(p2G4_rxv2_t *rx_s, p2G4_address_t *phy_addr, p2G4_rxv2_done_t *rx_done_s, uint8_t **buf, size_t size,
                          device_eval_rxv2_f eval_f);
int p2G4_dev_req_RSSI_c_b(p2G4_rssi_t *RSSI_s, p2G4_rssi_done_t *RSSI_done_s);
int p2G4_dev_req_cca_c_b(p2G4_cca_t *cca_s, p2G4_cca_done_t *cca_done_s);
int p2G4_dev_req_tx_c_b(p2G4_tx_t *tx_s, uint8_t *buf, p2G4_tx_done_t *tx_done_s);
int p2G4_dev_req_txv2_c_b(p2G4_txv2_t *tx_s, uint8_t *packet, p2G4_tx_done_t *tx_done_s);
int p2G4_dev_req_wait_c_b(pb_wait_t *wait_s);
void p2G4_dev_disconnect_c(void);
void p2G4_dev_terminate_c(void);

/*
 * API without call-backs and memory
 */
int p2G4_dev_initcom_nc(uint d, const char* s, const char* p);
int p2G4_dev_req_tx_nc_b(p2G4_tx_t *tx_s, uint8_t *buf, p2G4_tx_done_t *tx_done_s);
int p2G4_dev_req_txv2_nc_b(p2G4_txv2_t *tx_s, uint8_t *packet, p2G4_tx_done_t *tx_done_s);
int p2G4_dev_provide_new_tx_abort_nc_b(p2G4_abort_t * abort);
int p2G4_dev_req_rx_nc_b(p2G4_rx_t *rx_s, p2G4_rx_done_t *rx_done_s, uint8_t **rx_buf, size_t buf_size);
int p2G4_dev_req_rxv2_nc_b(p2G4_rxv2_t *rx_s, p2G4_address_t *phy_addr, p2G4_rxv2_done_t *rx_done_s, uint8_t **buf, size_t size);
int p2G4_dev_rx_cont_after_addr_nc_b(bool accept);
int p2G4_dev_rxv2_cont_after_addr_nc_b(bool accept_rx, p2G4_abort_t *abort);
int p2G4_dev_provide_new_rx_abort_nc_b(p2G4_abort_t * abort);
int p2G4_dev_provide_new_rxv2_abort_nc_b(p2G4_abort_t * abort);
int p2G4_dev_req_RSSI_nc_b(p2G4_rssi_t *RSSI_s, p2G4_rssi_done_t *RSSI_done_s);
int p2G4_dev_req_wait_nc_b(pb_wait_t *wait_s);
int p2G4_dev_req_cca_nc_b(p2G4_cca_t *cca_s, p2G4_cca_done_t *cca_done_s);
int p2G4_dev_provide_new_cca_abort_nc_b(p2G4_abort_t * abort);
void p2G4_dev_terminate_nc(void);
void p2G4_dev_disconnect_nc(void);

/*
 * API without call-backs and without memory
 */
//in the communication with the device, are we in the middle of a transaction (!Nothing_2G4), and if so, what
typedef enum { Nothing_2G4 = 0, Tx_Abort_Reeval_2G4 , Rx_Abort_Reeval_2G4 , Rx_Header_Eval_2G4, CCA_Abort_Reeval_2G4 ,  } p2G4_t_ongoing_transaction_t;

typedef struct {
  pb_dev_state_t pb_dev_state;
  p2G4_t_ongoing_transaction_t ongoing; //just as a safety check against bugy devices (only used in the version without callbacks)
  p2G4_tx_done_t   *tx_done_s;
  p2G4_rx_done_t *rx_done_s;
  p2G4_rxv2_done_t *rxv2_done_s;
  p2G4_cca_done_t *cca_done_s;
  uint8_t **rxbuf;
  size_t bufsize;
  bool WeGotAddress;
} p2G4_dev_state_nc_t;

int p2G4_dev_initCom_s_nc(p2G4_dev_state_nc_t *p2G4_dev_st, uint d, const char* s, const char* p);
int p2G4_dev_req_tx_s_nc_b(p2G4_dev_state_nc_t *p2G4_dev_st, p2G4_tx_t *tx_s, uint8_t *buf, p2G4_tx_done_t *tx_done_s);
int p2G4_dev_req_txv2_s_nc_b(p2G4_dev_state_nc_t *c2G4_dev_st, p2G4_txv2_t *tx_s, uint8_t *packet, p2G4_tx_done_t *tx_done_s);
int p2G4_dev_provide_new_tx_abort_s_nc_b(p2G4_dev_state_nc_t *p2G4_dev_st, p2G4_abort_t * abort);
int p2G4_dev_req_cca_s_nc_b(p2G4_dev_state_nc_t *c2G4_dev_st, p2G4_cca_t *cca_s, p2G4_cca_done_t *cca_done_s);
int p2G4_dev_provide_new_cca_abort_s_nc_b(p2G4_dev_state_nc_t *c2G4_dev_st, p2G4_abort_t * abort);
int p2G4_dev_req_rx_s_nc_b(p2G4_dev_state_nc_t *p2G4_dev_st, p2G4_rx_t *rx_s, p2G4_rx_done_t *rx_done_s, uint8_t **rx_buf, size_t bus_size);
int p2G4_dev_req_rxv2_s_nc_b(p2G4_dev_state_nc_t *p2G4_dev_state, p2G4_rxv2_t *rx_s, p2G4_address_t *phy_addr, p2G4_rxv2_done_t *rx_done_s, uint8_t **rx_buf, size_t buf_size);
int p2G4_dev_rx_cont_after_addr_s_nc_b(p2G4_dev_state_nc_t *p2G4_dev_st, bool accept);
int p2G4_dev_rxv2_cont_after_addr_s_nc_b(p2G4_dev_state_nc_t *p2G4_dev_state, bool dev_accepts, p2G4_abort_t * abort);
int p2G4_dev_provide_new_rx_abort_s_nc_b(p2G4_dev_state_nc_t *p2G4_dev_st, p2G4_abort_t * abort);
int p2G4_dev_provide_new_rxv2_abort_s_nc_b(p2G4_dev_state_nc_t *p2G4_dev_state, p2G4_abort_t * abort);
int p2G4_dev_req_RSSI_s_nc_b(p2G4_dev_state_nc_t *p2G4_dev_st, p2G4_rssi_t *RSSI_s, p2G4_rssi_done_t *RSSI_done_s);
int p2G4_dev_req_wait_s_nc_b(p2G4_dev_state_nc_t *p2G4_dev_st, pb_wait_t *wait_s);
void p2G4_dev_terminate_s_nc(p2G4_dev_state_nc_t *p2G4_dev_st);
void p2G4_dev_disconnect_s_nc(p2G4_dev_state_nc_t *p2G4_dev_st);


/*
 * API with call-backs and without memory
 */
typedef struct {
  dev_abort_reeval_f abort_f;
  pb_dev_state_t pb_dev_state;
} p2G4_dev_state_s_t;

int p2G4_dev_initcom_s_c(p2G4_dev_state_s_t *p2G4_dev_st, uint d, const char* s, const char* p, dev_abort_reeval_f fptr);
int p2G4_dev_req_tx_s_c_b(p2G4_dev_state_s_t *p2G4_dev_st, p2G4_tx_t *tx_s, uint8_t *buf, p2G4_tx_done_t *tx_done_s);
int p2G4_dev_req_txv2_s_c_b(p2G4_dev_state_s_t *p2G4_dev_state, p2G4_txv2_t *tx_s, uint8_t *packet, p2G4_tx_done_t *tx_done_s);
int p2G4_dev_req_tx_s_c(p2G4_dev_state_s_t *p2G4_dev_st, p2G4_tx_t *tx_s, uint8_t *buf);
int p2G4_dev_pick_txresp_s_c_b(p2G4_dev_state_s_t *p2G4_dev_st, p2G4_tx_done_t *tx_done_s);
int p2G4_dev_req_rx_s_c_b(p2G4_dev_state_s_t *p2G4_dev_st, p2G4_rx_t *rx_s, p2G4_rx_done_t *rx_done_s, uint8_t **rx_buf, size_t buf_size, device_eval_rx_f fptr);
int p2G4_dev_req_rxv2_s_c_b(p2G4_dev_state_s_t *p2G4_dev_state, p2G4_rxv2_t *rx_s, p2G4_address_t *phy_addr, p2G4_rxv2_done_t *rx_done_s, uint8_t **rx_buf, size_t buf_size, device_eval_rxv2_f dev_rxeval_f);
int p2G4_dev_req_RSSI_s_c_b(p2G4_dev_state_s_t *p2G4_dev_st, p2G4_rssi_t *RSSI_s, p2G4_rssi_done_t *RSSI_done_s);
int p2G4_dev_req_cca_s_c_b(p2G4_dev_state_s_t *p2G4_dev_state, p2G4_cca_t *cca_s, p2G4_cca_done_t *cca_done_s);
int p2G4_dev_req_wait_s_c_b(p2G4_dev_state_s_t *p2G4_dev_st, pb_wait_t *wait_s);
int p2G4_dev_req_wait_s_c(p2G4_dev_state_s_t *p2G4_dev_st, pb_wait_t *wait_s);
int p2G4_dev_pick_wait_resp_s_c_b(p2G4_dev_state_s_t *p2G4_dev_st);
void p2G4_dev_disconnect_s_c(p2G4_dev_state_s_t *p2G4_dev_st);
void p2G4_dev_terminate_s_c(p2G4_dev_state_s_t *p2G4_dev_st);

#ifdef __cplusplus
}
#endif

#endif
