## Introduction

This library extends the common libPhyComv1 providing the low level
communications mechanism between the devices and the 2G4 phy

Note that this Phy only supports BLE or 2006 802.15.4 DSS like modulations,
or interferers.
So it cannot be used for WLAN (or other OFDMA like)

## 2G4 IPC protocol

You can find the sequence diagrams here:
[2G4 IPC sequence diagrams](2G4_libPhyComv1_protocol.svg)

![2G4 IPC sequence diagrams (for GitHub web rendering)](https://raw.githubusercontent.com/BabbleSim/ext_2G4_libPhyComv1/master/docs/2G4_libPhyComv1_protocol.svg?sanitize=true)
<!--The ?sanitize=true is an ugly thing for GitHub to enable the svg to be
rendered into the markdown preview-->

The content of these structures, which are sent between the devices and phy,
can be found in [bs_pc_2G4_types.h](../src/bs_pc_2G4_types.h)

### v2 API Updates

Note: The old API is still supported, and remains ABI compatible
The v2 API is a superset of the old API aiming at providing support for
different protocols, and more features.

If desired, devices which supported the old API can be mapped to the new API
without loss of functionality (see instructions on each structure in the header
file). But at this point it is not recommended to do this, as the new API is
still in alpha state.

#### Txv2:
A p2G4_txv2_t structure from the device, is followed by a byte array of
packet_size bytes.
A p2G4_txv2_t is followed by a p2G4_tx_done_t from the Phy.

#### Rxv2:
A p2G4_rxv2_t structure from the device is followed by a p2G4_rxv2_addr_t,
containing p2G4_rxv2_t.n_addr elements.

When p2G4_rxv2_t.resp_type = 0, the Phy responds with a p2G4_rx_done_t,
and depending on status, a bytearray of p2G4_rx_done_t.packet_size bytes
with the possibly received packet.

#### RSSI measurements during abort reevaluations
During an Rx abort reevaluation, the device may now both send back a new abort
structure (after a RERESP_ABORTREEVAL) or also request an immediate RSSI
measurement (and follow it with another abort structure)

#### CCA procedure
To better support systems with CCA, a new API is introduced.
The device may do a search for compatible modulations and/or an (average) energy
measurement.
This is done periodically by looking for a compatible transmitter
and simultaneously measuring the RRSI level.
The device may also select to stop the measurement as soon as either the RSSI
measurement is over a predefined threshold, or, the RSSI measurement is over
another threshold and a compatible modulation is found in the air.

#### Coded Phy and other multi-modulation and/or multi-payload packets
Coded Phy packets shall be handled as two back to back transmissions.
Where the 1st transmission covers the {preamble + address + CI + term}
and the 2nd transmissions covers the {PDU (header + payload + ..) + header + CRC}.
For each consecutive transmission the following must be true:

* {1st tx}.end_tx_time = {1st tx}.end_packet_time
* {2nd tx}.start_tx_time = {1st tx}.end_tx_time + 1
* {2nd tx}.start_tx_time = {2nd tx}.start_packet_time

In the receive side, similarly 2 receptions will be performed.
Where the 1st reception will attempt to receive only the 1st transmission,
and the 2nd reception must be configured with prelocked_tx set to true.
In this case, the receiver will continue automatically with the same
transmitter it sync'ed to last time.
It is the responsibility of the device to properly set the modulation of the
2nd reception to match the transmitter 2nd transmission.

For coded phy, the 2nd/piggybacking Rx pream_and_addr_duration should be set to
0, acceptable_pre_truncation = 0, scan_duration = 1,
sync_threshold = UINT16_MAX.

This can be expanded to any number of sub-payloads with the same or different
modulations.

## API

A description of the library API can be found in
[bs_pc_2G4.h](../src/bs_pc_2G4.h)
