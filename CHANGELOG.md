# Changelog

## Unreleased

## 0.6.0

- Add XIAO ESP32C6 support alongside ESP32S3, including board-specific build environments, GPIO documentation, and hardware tests
- Add GitHub Actions firmware builds, downloadable S3/C6 artifacts, and an ESP Web Tools installer deployed through GitHub Pages
- Add a ChainOSC-series product portal linking to the Web Installer, with red ChainOSCPad product assets and social sharing metadata
- Save and validate the OSC target together with all Key and Encoder settings, replacing the separate OSC target save action

## 0.5.1

- Add a sticky Key Layout Guide showing the 3 x 4 key matrix and Encoder position
- Highlight every Key and Encoder card currently visible in the viewport
- Highlight only the corresponding Key or Encoder when its settings card is operated
- Scroll to a settings card by selecting its position in the guide
- Add a left-aligned control that collapses and expands the entire guide content
- Add hardware test checklist items for the Key Layout Guide behavior

## 0.5.0

- Reject a Sequence with a zero Step
- Reject a Sequence whose Step moves away from End
- Stop automatically correcting Sequence values during preset import, whole-settings import, Web UI save, and storage load
- Use the same Sequence validation rules for Key and Encoder Click settings
- Separate the captive portal into a Wi-Fi-only setup page
- Reorganize the normal settings page to match the ChainOSC series: Language, System, WiFi, backup and restore, OSC target, then input settings
- Add Product, Version, IP address, and mDNS information to the System card
- Replace editable Wi-Fi credentials on the normal page with a Wi-Fi settings delete action
- Preserve OSC and input settings when Wi-Fi credentials are deleted
- Save OSC target settings independently without restarting or changing the browser URL
- Add comfortable margins on wide browser windows
- Match the ChainOSC Sequence editor layout with a full-width OSC Address and four parameter tiles
- Update the hardware test checklist for the new validation and Web UI behavior

## 0.4.2

- Match ChainOSCmini Sequence behavior while Wi-Fi is disconnected
- Keep Key and Encoder Click Sequence values unchanged when OSC cannot be sent
- Add hardware tests for Sequence behavior across Wi-Fi disconnection and reconnection
- License the original software, Web UI, and documentation under the MIT License
- Document third-party software licenses and copyright notices

## 0.4.1

- Unify whole-settings JSON import limits at 64 KiB in the Web UI and firmware
- Remove the legacy split Network / Inputs Web UI implementation
- Keep non-AJAX save and reset responses on the unified settings page
- Update the hardware test checklist for the unified root URL
- Complete the version 0.4.1 hardware test checklist

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
- Add per-Key and Encoder default-reset actions to each device menu
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
