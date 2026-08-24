# Changelog

## 0.4.0

- Add versioned whole-settings JSON export and import without Wi-Fi credentials
- Add ChainOSC-compatible Key and Encoder preset export and import
- Validate JSON format, schema version, device identity, OSC fields, and size before saving
- Allow whole-settings JSON files up to 64 KiB so all 12 maximized Keys can be restored
- Restore previous input settings if a whole-settings storage write fails
- Stream whole-settings JSON export one device at a time
- Add English and Japanese Web UI with persistent language selection
- Select the browser language on the first Web UI visit
- Add localized import progress, confirmation, validation, and storage messages
- Combine network, JSON backup, Key, and Encoder settings into one Web UI page
- Update the firmware version to 0.4.0 and add ArduinoJson to both build workflows

## 0.3.0

- Add per-key OSC Address, Press value, Release value, and value type settings
- Add Float, Int, and String OSC output types
- Add Encoder Absolute and Increment modes
- Add configurable Encoder input range, scale, output range, and output type
- Add configurable Encoder Push messages
- Match ChainOSCmini Key and Encoder Click UI: Press / Release up to eight
  messages, ordering, deletion, and Sequence mode
- Match ChainOSCmini Encoder Rotation field names, ordering, and conditional
  Absolute fields
- Match ChainOSCmini Key and Encoder card styling, identity rows,
  tabs, message counters, byte counters, and responsive layout
- Add a sticky whole-settings save bar and remove per-device save buttons
- Show an unsaved-changes indicator beside the whole-settings save button
- Save all input settings with fetch without changing the current URL
- Use a pointer cursor for interactive Web UI controls
- Disable message-add buttons when Press / Release reaches eight messages
- Align OSC message type fields with the Address and Value fields
- Load input-page CSS and JavaScript before device cards so large settings do not disable UI controls
- Stream the input page in common, per-card, Encoder, and footer chunks instead of building one page-sized String
- Add String as a Sequence output type, matching ChainOSCmini
- Add editable Key and Encoder device names saved to LittleFS
- Validate and save all input settings as compact LittleFS blobs
- Use atomic temporary-file replacement and verify saved input settings
- Clear network and input settings together from the reset action

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
