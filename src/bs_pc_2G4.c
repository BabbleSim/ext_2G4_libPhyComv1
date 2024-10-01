/*
 * Copyright 2018 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "bs_pc_2G4_types.h"
#include "bs_pc_2G4.h"

/**
 * Family of functions with callbacks:
 * (just a call thru to the stateless version library)
 */
//State for the family of functions with callbacks
static p2G4_dev_state_s_t C2G4_dev_st = {0};

int p2G4_dev_initcom_c(uint d, const char* s, const char* p, dev_abort_reeval_f abort_f) {
  return p2G4_dev_initcom_s_c(&C2G4_dev_st, d, s, p, abort_f);
}

void p2G4_dev_terminate_c(){
  p2G4_dev_terminate_s_c(&C2G4_dev_st);
}

void p2G4_dev_disconnect_c(){
  p2G4_dev_disconnect_s_c(&C2G4_dev_st);
}

int p2G4_dev_req_tx_c_b(p2G4_tx_t *tx_s, uint8_t *packet, p2G4_tx_done_t *tx_done_s) {
  return p2G4_dev_req_tx_s_c_b(&C2G4_dev_st, tx_s, packet, tx_done_s);
}

int p2G4_dev_req_txv2_c_b(p2G4_txv2_t *tx_s, uint8_t *packet, p2G4_tx_done_t *tx_done_s) {
  return p2G4_dev_req_txv2_s_c_b(&C2G4_dev_st, tx_s, packet, tx_done_s);
}

int p2G4_dev_req_rx_c_b(p2G4_rx_t *rx_s, p2G4_rx_done_t *rx_done_s, uint8_t **buf, size_t size,
                        device_eval_rx_f eval_f){
  return p2G4_dev_req_rx_s_c_b(&C2G4_dev_st, rx_s, rx_done_s, buf, size, eval_f);
}

int p2G4_dev_req_rxv2_c_b(p2G4_rxv2_t *rx_s, p2G4_address_t *phy_addr, p2G4_rxv2_done_t *rx_done_s, uint8_t **buf, size_t size,
                          device_eval_rxv2_f eval_f){
  return p2G4_dev_req_rxv2_s_c_b(&C2G4_dev_st, rx_s, phy_addr, rx_done_s, buf, size, eval_f);
}

int p2G4_dev_req_RSSI_c_b(p2G4_rssi_t *RSSI_s, p2G4_rssi_done_t *RSSI_done_s) {
  return p2G4_dev_req_RSSI_s_c_b(&C2G4_dev_st, RSSI_s, RSSI_done_s);
}

int p2G4_dev_req_cca_c_b(p2G4_cca_t *cca_s, p2G4_cca_done_t *cca_done_s) {
  return p2G4_dev_req_cca_s_c_b(&C2G4_dev_st, cca_s, cca_done_s);
}

int p2G4_dev_req_wait_c_b(pb_wait_t *wait_s){
  return p2G4_dev_req_wait_s_c_b(&C2G4_dev_st, wait_s);
}


/*
 * Set of functions without callbacks:
 * (call thru to stateless version library)
 */
//State for the family of functons without callbacks
static p2G4_dev_state_nc_t C2G4_dev_st_nc = {{0}};

int p2G4_dev_initcom_nc(uint d, const char* s, const char* p) {
  C2G4_dev_st_nc.ongoing = Nothing_2G4;
  return p2G4_dev_initCom_s_nc(&C2G4_dev_st_nc, d, s, p);
}

void p2G4_dev_terminate_nc(){
  p2G4_dev_terminate_s_nc(&C2G4_dev_st_nc);
}
void p2G4_dev_disconnect_nc(){
  p2G4_dev_disconnect_s_nc(&C2G4_dev_st_nc);
}

int p2G4_dev_req_tx_nc_b(p2G4_tx_t *tx_s, uint8_t *packet, p2G4_tx_done_t *tx_done_s) {
  return p2G4_dev_req_tx_s_nc_b(&C2G4_dev_st_nc, tx_s, packet, tx_done_s);
}

int p2G4_dev_req_txv2_nc_b(p2G4_txv2_t *tx_s, uint8_t *packet, p2G4_tx_done_t *tx_done_s) {
  return p2G4_dev_req_txv2_s_nc_b(&C2G4_dev_st_nc, tx_s, packet, tx_done_s);
}

int p2G4_dev_provide_new_tx_abort_nc_b(p2G4_abort_t * abort){
  return p2G4_dev_provide_new_tx_abort_s_nc_b(&C2G4_dev_st_nc, abort);
}

int p2G4_dev_req_rx_nc_b(p2G4_rx_t *rx_s, p2G4_rx_done_t *rx_done_s, uint8_t **buf, size_t size){
  return p2G4_dev_req_rx_s_nc_b(&C2G4_dev_st_nc, rx_s, rx_done_s, buf, size);
}

int p2G4_dev_req_rxv2_nc_b(p2G4_rxv2_t *rx_s, p2G4_address_t *phy_addr, p2G4_rxv2_done_t *rx_done_s, uint8_t **buf, size_t size){
  return p2G4_dev_req_rxv2_s_nc_b(&C2G4_dev_st_nc, rx_s, phy_addr, rx_done_s, buf, size);
}

int p2G4_dev_rx_cont_after_addr_nc_b(bool accept_rx){
  return p2G4_dev_rx_cont_after_addr_s_nc_b(&C2G4_dev_st_nc, accept_rx);
}

int p2G4_dev_rxv2_cont_after_addr_nc_b(bool accept_rx, p2G4_abort_t *abort){
  return p2G4_dev_rxv2_cont_after_addr_s_nc_b(&C2G4_dev_st_nc, accept_rx, abort);
}

int p2G4_dev_provide_new_rx_abort_nc_b(p2G4_abort_t * abort){
  return p2G4_dev_provide_new_rx_abort_s_nc_b(&C2G4_dev_st_nc, abort);
}

int p2G4_dev_provide_new_rxv2_abort_nc_b(p2G4_abort_t * abort){
  return p2G4_dev_provide_new_rxv2_abort_s_nc_b(&C2G4_dev_st_nc, abort);
}

int p2G4_dev_req_imm_RSSI_nc_b(p2G4_rssi_t *RSSI_s, p2G4_rssi_done_t *RSSI_done_s){
  return p2G4_dev_req_imm_RSSI_s_nc_b(&C2G4_dev_st_nc, RSSI_s, RSSI_done_s);
}

int p2G4_dev_req_RSSI_nc_b(p2G4_rssi_t *RSSI_s, p2G4_rssi_done_t *RSSI_done_s) {
  return p2G4_dev_req_RSSI_s_nc_b(&C2G4_dev_st_nc, RSSI_s, RSSI_done_s);
}

int p2G4_dev_req_wait_nc_b(pb_wait_t *wait_s){
  return p2G4_dev_req_wait_s_nc_b(&C2G4_dev_st_nc, wait_s);
}

int p2G4_dev_req_cca_nc_b(p2G4_cca_t *cca_s, p2G4_cca_done_t *cca_done_s) {
  return p2G4_dev_req_cca_s_nc_b(&C2G4_dev_st_nc, cca_s, cca_done_s);
}

int p2G4_dev_provide_new_cca_abort_nc_b(p2G4_abort_t * abort){
  return p2G4_dev_provide_new_cca_abort_s_nc_b(&C2G4_dev_st_nc, abort);
}
