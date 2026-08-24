# Changelog

## 0.2.0

- Add captive setup AP (`ChainOSCPad-Setup`)
- Add browser configuration for Wi-Fi and OSC destination
- Save configuration to ESP32-S3 NVS
- Add `chainoscpad.local` mDNS access while connected to Wi-Fi
- Fall back to setup AP when no credentials exist or connection times out
- Add configuration reset and automatic restart
- Keep the 12-key matrix and ChainOSC-compatible absolute encoder behavior

## 0.1.0

- Add the initial XIAO ESP32S3 hardware test firmware
- Add 3x4 key matrix scanning and per-key debounce
- Add rotary encoder absolute output and encoder push input
- Add Arduino IDE and PlatformIO project entry points

