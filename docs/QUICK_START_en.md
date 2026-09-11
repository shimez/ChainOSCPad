---
layout: default
title: ChainOSCPad Quick Start
permalink: /en/quick-start/
---

# ChainOSCPad Quick Start

[日本語版](../../quick-start/)

This guide shows how to enter a Key setting manually without using a Device Preset, send an OSC message from ChainOSCPad to VRChat, and verify the result. See the [English User Guide](../user-guide/) for details.

## Requirements

- An assembled ChainOSCPad with XIAO ESP32S3, ESP32C3, ESP32C5, or ESP32C6
- A data-capable USB Type-C cable
- A supported Wi-Fi network (2.4/5 GHz on ESP32C5; 2.4 GHz on ESP32S3/C3/C6)
- Desktop Chrome or Edge
- A computer running VRChat

## 1. Install the firmware

1. Open the [ChainOSCPad Web Installer](../../installer/) in Chrome or Edge.
2. Connect ChainOSCPad to your PC with USB.
3. Select `Install ChainOSCPad` and choose the XIAO serial port.
4. Follow the on-screen instructions.

If the port is not listed, hold the XIAO BOOT button while reconnecting USB.

## 2. Configure Wi-Fi

Connect to `ChainOSCPad-Setup` with password `12345678`. If the captive portal does not open, visit `http://192.168.4.1/`, then save your Wi-Fi credentials. ESP32C5 supports 2.4 GHz and 5 GHz; ESP32S3/C3/C6 use 2.4 GHz.

## 3. Enable OSC in VRChat

Start VRChat and select **Action Menu → Options → OSC → Enabled**.

## 4. Find the IPv4 address of the VRChat PC

In Windows PowerShell or Command Prompt, run `ipconfig`. Find the IPv4 address of the Wi-Fi or Ethernet adapter connected to the same network as ChainOSCPad. Do not use a VPN or virtual adapter address.

## 5. Open the settings page

Open `http://chainoscpad.local/` from the same network. If mDNS is unavailable, open the IP address shown in the serial log.

## 6. Configure the OSC destination

1. In **OSC Destination**, enter the IPv4 address of the PC running VRChat in **IP Address**.
2. Enter `9000` in **UDP Port**.

## 7. Configure a Voice action on a Key

Choose one of the 12 Keys and enter the following values under **Press**:

- OSC Address: `/input/Voice`
- Type: `Int`
- Value: `1`

Switch to **Release** and enter:

- OSC Address: `/input/Voice`
- Type: `Int`
- Value: `0`

## 8. Save and verify the action

1. Select **Save All Settings**.
2. With VRChat running and OSC enabled, operate the configured Key.
3. Confirm that VRChat's Voice input state changes. This confirms that ChainOSCPad sent an OSC message.

ChainOSCPad is not limited to VRChat. For another OSC-compatible application, set the destination, OSC Address, type, and value for that application. Device Preset can be used to reuse and share frequently used settings; see the [English User Guide](../user-guide/) for details.

[Back to the ChainOSCPad portal](../../)
