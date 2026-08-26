---
layout: default
title: ChainOSCPad English User Guide
permalink: /en/user-guide/
---

# ChainOSCPad English User Guide

[日本語版](../../user-guide/)

ChainOSCPad sends OSC from 12 Keys and a rotary Encoder with rotation and click. It supports XIAO ESP32S3 and XIAO ESP32C6.

> ChainOSCPad is an independent, unofficial project and is not an official product of Seeed Studio or any other hardware manufacturer.

## 1. Initial Wi-Fi setup

Power the device, connect to `ChainOSCPad-Setup` with password `12345678`, then select and save a 2.4 GHz Wi-Fi network. If the captive portal does not open, visit `http://192.168.4.1/`. The captive portal only configures Wi-Fi.

## 2. Open the settings page

From the same network, open `http://chainoscpad.local/`. If mDNS is unavailable, use the IP address shown in the serial log.

The page contains Language, System, Wi-Fi, Backup and Restore, OSC Destination, Key 1–12 and Encoder sections. The System card shows the product, XIAO model, firmware version, IP address and mDNS name. Use the floating save button to save all changes together.

## 3. OSC destination

Set the receiver IPv4 address and a UDP port from `1` to `65535`. Messages are not sent while Wi-Fi is disconnected. A Sequence advances only after a successful transmission.

## 4. Keys and Encoder Click

Press / Release mode supports up to eight messages in total. Each message has an OSC Address, a Float, Int or String type, and a value.

Sequence mode advances from Start by Step and wraps after End. Step cannot be zero. Use a positive Step when Start is below End and a negative Step when Start is above End.

## 5. Encoder rotation

Configure OSC Address, type and value for clockwise and counterclockwise rotation in the Encoder card.

## 6. Presets and backup

Use each card's `…` menu to export or import a Key or Encoder preset as JSON. Use Backup and Restore for the complete device settings. Wi-Fi credentials are not included in the full-settings JSON.

Export important settings before reinstalling or erasing firmware. The same `…` menu can reset an individual Key or Encoder; the Wi-Fi section can erase stored Wi-Fi settings.

[Back to the ChainOSCPad portal](../../)
