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

### v2.1 API Updates

Note: The old API is still supported, and remains ABI compatible
The v2.1 API is a superset of the old API aiming at providing support for
different protocols, and more features.

If desired, devices which supported the old API can be mapped to the new API
without loss of functionality (see instructions on each structure in the header
file).

#### Txv2.1:
A p2G4_tx2v1_t structure from the device, is followed by a byte array of
packet_size bytes.
A p2G4_tx2v1_t is followed by a p2G4_tx_done_t from the Phy.

#### RSSIv2:
A p2G4_rssiv2_t structure from the device, which gets as response from the Phy
a p2G4_rssi_done_t.

#### Rxv2.1:
A p2G4_rx2v1_t structure from the device is followed by a p2G4_rxv2_addr_t,
containing p2G4_rx2v1_t.n_addr elements.

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
and the 2nd transmissions covers the {PDU (header + payload + ..) + CRC}.
For each consecutive transmission the following must be true:

* {1st tx}.end_tx_time = {1st tx}.end_packet_time
* {2nd tx}.start_tx_time = {1st tx}.end_tx_time + 1
* {2nd tx}.start_tx_time = {2nd tx}.start_packet_time

In the receive side, similarly 2 receptions will be performed.
Where the 1st reception will attempt to receive only the 1st transmission,
and the 2nd reception must be configured with prelocked_tx set to true.
In this case, the receiver will continue automatically with the same
transmitter it sync'ed to last time.
It is the responsibility of the device to properly set the coding_rate of the
2nd reception to match the transmitter 2nd transmission.

The first transmission payload shall contain at least 1 byte, in which the
lowest 2 bits are the CI.
The FEC2 transmission address should be an invalid one (for ex. 0x0), to ensure
no other Receiver matches it as if it were a FEC1.
The FEC2 reception:
 * should be set to be piggybacking, and should preferably have n_addr = 0
   (to ensure nothing else is locked if the Tx FEC2 disapears).
 * pream_and_addr_duration should be set to 0, acceptable_pre_truncation = 0, scan_duration = 1,
   sync_threshold = UINT16_MAX.

This can be expanded to any number of sub-payloads with the same or different
modulations.

##### HDT

For HDT, packets shall be transmitted as follows:

* For all packets parts
    * phy_address shall be the 40 bit PCA.
    * radio_params.modulation shall be set to P2G4_MOD_BLE_HDT*.

* Short format packets shall be 1 single transmission, consisting of the Premable(s) (STS & LTS),
  and the control header.
    * The coding_rate shall be HDT2.
    * The packet content shall include the 57 Control header bits, including the PCA-A, including its HEC-C CRC bits.
    * These 57 control header shall be packed in 8 bytes
        * The first 5 bytes shall contain the actual 33bits of control header information (zero padded with 7 zeroes in the last byte MSBits).
        * The following 3 bytes shall contain the HEC-C CRC
    * And therefore packet_size shall be 8 bytes.
    * duration (end_packet_time - start_packet_time + 1) shall include the termination (termination
      bits, symbol zero padding and termination symbols), that is 37+32us

* Format 0 packets shall be 2 separate transmissions.
    * Where the first transmission shall be equal to a short format transmission.
    * And the 2nd transmission shall include the PDU (PDU Header, data, CRC and termination).
        * coding_rate shall correspond to the configured rate (HDT2, HDT3,..,HDT7p5)
        * The packet content shall include the PDU header, payload (incl. possible MIC) and CRC
        * packet_size shall match that 2nd "transmission" packet content
        * duration shall include the termination.

* Format 1 packets shall be 3 or more separate transmissions.
    * Where the first transmission shall be equal to a short format transmission.
    * Where the 2nd transmission includes the PDU header
        * coding_rate shall correspond to the configured header rate (P2G4_CODRATE_BLE_HEADHDT*)
        * The packet content shall include the PDU header, and HEC-P
        * packet_size shall match this packet content
        * duration shall include the termination.
    * The remaining transmissions will be one per payload or payload block (if the payload is
      divided into blocks).
        * coding_rate shall correspond to the configured rate (HDT2, HDT3,..,HDT7p5)
        * The packet content shall include the payload (incl. possible MIC) or payload block
          and CRC
        * packet_size shall match this packet content
        * The end_packet_time shall always match:
            * The last symbol on air that transmitted the last bit of information of that
              payload or block CRC, + 1 us (the timing of 2 possible termination symbols).
              But not including PITS even if the block or payload finished just before
              a Phy interval border.
            * This means that if a payload or payload block would have been broken in
              several Phy intervals the extra time used in between those Phy intervals
              (for the termination and PITS) is to be accounted into the duration.
          This way, the end of the last block will match the actual end of the packet.

TODO: Make drawing for format 1 with phy interval in the middle of the block, just at the end and no phyint.

HDT is only supported in >=v2 Tx and Rx APIs

## API

A description of the library API can be found in
[bs_pc_2G4.h](../src/bs_pc_2G4.h)
