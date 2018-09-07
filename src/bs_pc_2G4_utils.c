/*
 * Copyright 2018 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <string.h>
#include "bs_pc_2G4_utils.h"
#include "bs_tracing.h"
#include "bs_oswrap.h"

/**
 * Return a power level as usable by libCom from a double value (dBm)
 */
p2G4_power_t p2G4_power_from_d(double power){
  if ( power < -128 ) {//we need to ensure we fit into p2G4_power_t (range -128dBm..127dBm)
    power = -128;
  } else if ( power > 127 ) {
    power = 127;
  }
  return (p2G4_power_t)(power*256);
}

/**
 * Convert a libCom power level (p2G4_power_t) into a double float (dBm)
 */
double p2G4_power_to_d(p2G4_power_t power){
  return ((double)power)/256.0;
}

/**
 * Convert a libCom RSSI level (p2G4_rssi_power_t) into a double float (dBm)
 */
double p2G4_RSSI_value_to_dBm(p2G4_rssi_power_t value){
  return value/65536.0;
}

/**
 * Convert a power level in dBm (Represented as double) into P2G4Com RSSI level format
 */
p2G4_rssi_power_t p2G4_RSSI_value_from_dBm(double value){
  if (value < -32768) {//staturate to the type limit
    value = -32768;
  } else if (value > 32767) {
    value = 32767;
  }
  return (p2G4_rssi_power_t)(value*65536);
}

/**
 * Return a center frequency as usable by P2G4Com from a ble channel number
 */
p2G4_freq_t p2G4_center_freq_from_ble_ch_nbr(uint ch_idx){
  double freq;
  if (ch_idx < 37) {
    freq = 2404 + ch_idx*2 + (ch_idx > 10)*2;
  } else {
    if (ch_idx == 37) {
      freq = 2402;
    } else if (ch_idx == 38) {
      freq = 2426;
    } else if (ch_idx == 39) {
      freq = 2480;
    }
  }
  p2G4_freq_t center_freq;
  if ( p2G4_freq_from_d( freq, 1, &center_freq  ) != 0 ){
    bs_trace_error_line("Got wrong channel index (%i)\n", ch_idx);
  }
  return center_freq;
}

/**
 * Convert a strings into a low level modulation
 * The strings may be either straight the number (decimal or hexadecimal) (which is a inadvisable way of using it)
 * or a human readable string (see below in the code)
 */
int p2G4_modulation_from_string(const char* name, p2G4_modulation_t *modulation, p2G4_mod_limits_t limit, int verb){
  uint i = 0;

  while(1) {
    if ( name[i] != 0 ) {
      if ( name[i] == ' ' ) {
        i++;
      } else {
        break;
      }
    } else {
      bs_trace_warning_line("No modulation type found in string %s\n",name);
      return -1;
    }
  }

  if ( name[i] >= '0' && name[i] <='9') { //it's a number
    bs_trace_warning_line("Converting modulation from numerical string not yet allowed %s\n",name);
    return -1;
    //TOLOW: handle this
  } else { //It's a string
    if ( strncmp(&name[i], "BLI",3)==0 ){
      *modulation = P2G4_MOD_BLEINTER;
      if (verb) bs_trace_raw(9,"Modulation set to BLE interferent\n");
    } else if ( strncmp(&name[i], "BL",2)==0 ){
      *modulation = P2G4_MOD_BLE;
      if (verb) bs_trace_raw(9,"Modulation set to BLE\n");
    } else if ( strncmp(&name[i], "CW",2)==0 ) {
      *modulation = P2G4_MOD_CWINTER;
      if (verb) bs_trace_raw(9,"Modulation set to CW\n");
    } else if ( strncmp(&name[i], "Prop2M",6)==0 ) {
      *modulation = P2G4_MOD_PROP2M;
      if (verb) bs_trace_raw(9,"Modulation set to 2Mbit propietary\n");
    } else if ( strncmp(&name[i], "Prop3M",6)==0 ) {
      *modulation = P2G4_MOD_PROP3M;
      if (verb) bs_trace_raw(9,"Modulation set to 3Mbit propietary\n");
    } else if ( strncmp(&name[i], "Prop4M",6)==0 ) {
      *modulation = P2G4_MOD_PROP4M;
      if (verb) bs_trace_raw(9,"Modulation set to 4Mbit propietary\n");
    } else if ( strncmp(&name[i], "WLAN",4)==0 ){
      *modulation = P2G4_MOD_WLANINTER;
      if (verb) bs_trace_raw(9,"Modulation set to WLAN\n");
    } else if ( strcmp(&name[i], "WN16")==0 ){
      *modulation = P2G4_MOD_WHITENOISE16MHz;
      if (verb) bs_trace_raw(9,"Modulation set to 16MHz White noise\n");
    } else if ( strcmp(&name[i], "WN1")==0 ){
      *modulation = P2G4_MOD_WHITENOISE1MHz;
      if (verb) bs_trace_raw(9,"Modulation set to 1MHz White noise\n");
    } else if ( strcmp(&name[i], "WN2")==0 ){
      *modulation = P2G4_MOD_WHITENOISE2MHz;
      if (verb) bs_trace_raw(9,"Modulation set to 2MHz White noise\n");
    } else if ( strcmp(&name[i], "WN4")==0 ){
      *modulation = P2G4_MOD_WHITENOISE4MHz;
      if (verb) bs_trace_raw(9,"Modulation set to 4MHz White noise\n");
    } else if ( strcmp(&name[i], "WN8")==0 ){
      *modulation = P2G4_MOD_WHITENOISE8MHz;
      if (verb) bs_trace_raw(9,"Modulation set to 8MHz White noise\n");
    } else if ( strcmp(&name[i], "WN20")==0 ){
      *modulation = P2G4_MOD_WHITENOISE20MHz;
      if (verb) bs_trace_raw(9,"Modulation set to 20MHz White noise\n");
    } else if ( strcmp(&name[i], "WN40")==0 ){
      *modulation = P2G4_MOD_WHITENOISE40MHz;
      if (verb) bs_trace_raw(9,"Modulation set to 40MHz White noise\n");
    } else if ( strcmp(&name[i], "WN80")==0 ){
      *modulation = P2G4_MOD_WHITENOISE80MHz;
      if (verb) bs_trace_raw(9,"Modulation set to 80MHz White noise\n");
    } else {
      bs_trace_warning_line("Unknown modulation type %s\n",name);
      return -1;
    }
  }

  return 0;
}

/**
 * Convert a p2G4_freq_t frequency into a double
 *
 * Note that the frequency is returned in MHz, as an offset relative to 2400 MHz
 */
double p2G4_freq_to_d(p2G4_freq_t freq){
  return ((double)freq)/(1<<P2G4_freq_FRACB);
}

/**
 * Convert a frequency in double float to p2G4_freq_t
 *
 * If prevent_OOB is set, frequencies out of the 2400..2480MHz band wont be allowed
 */
int p2G4_freq_from_d(double center_freq, int prevent_OOB, p2G4_freq_t *result){
  double i_center_freq = center_freq;
  if ( center_freq > 1e9 ){ //they provided it in Hz
    center_freq = center_freq/1e6;
  } else if ( center_freq > 1e6 ) { //they provided it in KHz..
    center_freq = center_freq/1e3;
  }
  //We expect MHz

  if ( ( center_freq >= 2400-128 ) && ( center_freq <= 2400+128 ) ) //we offset it down to 0 = 2400MHz
    center_freq = center_freq - 2400;
  //it could have been provided directly as an offset relative to 2400MHz and we would accept it

  if ( ( center_freq >= 127 ) || ( center_freq <= -127 ) ) {
    bs_trace_error_line("center_frequency seems to be more than 127MHz apart from 2400MHz (%e, %e+2400MHz)\n",i_center_freq, center_freq);
  }

  if ( ( prevent_OOB == 1 ) && ( ( center_freq > 80) || (center_freq < 0 ) ) ){
    bs_trace_warning_line("center_frequency out of ISM band\n");
    return -1;
  }

  center_freq *= (1<<P2G4_freq_FRACB);

  *result = (p2G4_freq_t)(center_freq+0.5);
  return 0;
}
