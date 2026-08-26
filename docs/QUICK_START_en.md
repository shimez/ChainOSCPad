---
layout: default
title: ChainOSCPad Quick Start
permalink: /en/quick-start/
---

# ChainOSCPad Quick Start

[日本語版](../../quick-start/)

This guide covers firmware installation, Wi-Fi setup and a basic OSC transmission test. See the [English User Guide](../user-guide/) for details.

## Requirements

- An assembled ChainOSCPad with XIAO ESP32S3 or XIAO ESP32C6
- A data-capable USB Type-C cable
- A 2.4 GHz Wi-Fi network
- Desktop Chrome or Edge
- An OSC receiver

## 1. Install the firmware

1. Open the [ChainOSCPad Web Installer](../../installer/) in Chrome or Edge.
2. Connect ChainOSCPad to your PC with USB.
3. Select `Install ChainOSCPad` and choose the XIAO serial port.
4. Follow the on-screen instructions.

If the port is not listed, hold the XIAO BOOT button while reconnecting USB.

## 2. Configure Wi-Fi

Connect to `ChainOSCPad-Setup` with password `12345678`. If the captive portal does not open, visit `http://192.168.4.1/`, then save your 2.4 GHz Wi-Fi credentials.

## 3. Configure OSC and test

Open `http://chainoscpad.local/` from the same network. Set the OSC destination IP address and UDP port, save all settings, then operate a Key or Encoder and verify the message at your receiver.

[Back to the ChainOSCPad portal](../../)
