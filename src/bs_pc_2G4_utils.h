/*
 * Copyright 2018 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef BS_P2G4_UTILS_H
#define BS_P2G4_UTILS_H

/**
 * Utility functions related to the 2.4G PhyCom IF
 * Mostly, conversion functions from the 2.4G PhyCom types into more general types
 */

#include "bs_pc_2G4_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {Mod_None, OnlyReceivable, OnlyNonReceivable} p2G4_mod_limits_t;

int p2G4_modulation_from_string(const char* name, p2G4_modulation_t *modulation, p2G4_mod_limits_t limit, int verb);

double p2G4_freq_to_d(p2G4_freq_t freq);
int p2G4_freq_from_d(double center_freq, int prevent_OOB, p2G4_freq_t *result);
p2G4_freq_t p2G4_center_freq_from_ble_ch_nbr(uint channel_idx);

p2G4_power_t p2G4_power_from_d(double power);
double p2G4_power_to_d(p2G4_power_t power);

double p2G4_RSSI_value_to_dBm(p2G4_rssi_power_t value);
p2G4_rssi_power_t p2G4_RSSI_value_from_dBm(double value);

#ifdef __cplusplus
}
#endif

#endif
