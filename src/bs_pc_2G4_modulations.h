/*
 * Copyright 2018 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef BS_P2G4_MODULATIONS_H
#define BS_P2G4_MODULATIONS_H

#ifdef __cplusplus
extern "C"{
#endif

#define P2G4_MOD_SIMILAR_MASK 0xFFF0
/*
 * Note: All modems for this phy would need to understand these modulations
 * to be able to, at least, account for their interference.
 * To ease this process, modulations which have similar enough spectral
 * characteristics should have the same upper 12 bits.
 * In this manner a modem model can tell that an unknown modulation is close
 * to another known one and just treat it as if it were the same
 */

#define P2G4_MOD_BLE             0x10 //Standard 1Mbps BLE modulation
#define P2G4_MOD_BLE2M           0x20 //Standard 2Mbps BLE
#define P2G4_MOD_PROP2M          0x21 //Proprietary 2Mbps
#define P2G4_MOD_PROP3M          0x31 //Proprietary 3Mbps
#define P2G4_MOD_PROP4M          0x41 //Proprietary 4Mbps

/* Non receivable modulations (interferers) */
#define P2G4_MOD_BLEINTER        0x8000  //BLE shaped interference
#define P2G4_MOD_WLANINTER       0x8010  //WLAN shaped interference (for all WLAN modulations)
#define P2G4_MOD_CWINTER         0x8020  //CW interference
/* Non receivable white noise of a given bandwidth: */
#define P2G4_MOD_WHITENOISE1MHz  0x8030
#define P2G4_MOD_WHITENOISE2MHz  0x8040
#define P2G4_MOD_WHITENOISE4MHz  0x8050
#define P2G4_MOD_WHITENOISE8MHz  0x8060
#define P2G4_MOD_WHITENOISE16MHz 0x8070
#define P2G4_MOD_WHITENOISE20MHz 0x8080
#define P2G4_MOD_WHITENOISE40MHz 0x8090
#define P2G4_MOD_WHITENOISE80MHz 0x80A0

#ifdef __cplusplus
}
#endif

#endif
