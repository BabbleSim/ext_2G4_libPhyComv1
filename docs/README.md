## Introduction

This library extends the common libPhyComv1 providing the low level
communications mechanism between the devices and the 2G4 phy

Note that this Phy only supports BLE like modulations, or interferers.
So it cannot be used for WLAN or Zigbee (or any other 802.15.4).

## 2G4 IPC protocol

You can find the sequence diagrams here:
[2G4 IPC sequence diagrams](2G4_libPhyComv1_protocol.svg)

![2G4 IPC sequence diagrams (for GitHub web rendering)](https://raw.githubusercontent.com/BabbleSim/ext_2G4_libPhyComv1/master/docs/2G4_libPhyComv1_protocol.svg?sanitize=true)
<!--The ?sanitize=true is an ugly thing for GitHub to enable the svg to be
rendered into the markdown preview-->

The content of these structures, which are sent between the devices and phy,
can be found in [bs_pc_2G4_types.h](../src/bs_pc_2G4_types.h)

## API

A description of the library API can be found in
[bs_pc_2G4.h](../src/bs_pc_2G4.h)
