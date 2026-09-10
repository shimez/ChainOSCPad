# ChainOSCPad Status / Activity LED Human Acceptance

This procedure verifies the single-color LED behavior on XIAO `D10`. Physical
Human Acceptance confirmed that the firmware's Active HIGH polarity matches
the ChainOSCPad PCB.

## LED-01: Boot / initialization

- Preconditions: Device powered off; LED connected to `D10` through the PCB resistor.
- Operation: Apply power.
- Expected LED behavior: The LED starts OFF, then changes to the applicable network pattern.
- Expected system behavior: Normal boot continues without an LED-related delay.
- PASS: No unintended startup flash or boot regression is observed.

## LED-02: AP Mode slow blink

- Preconditions: No usable saved Wi-Fi credentials.
- Operation: Boot and wait for Setup AP mode.
- Expected LED behavior: 500 ms ON / 500 ms OFF, repeating.
- Expected system behavior: `ChainOSCPad-Setup` and its captive portal remain available.
- PASS: The slow blink and AP functions are both stable.

## LED-03: STA connecting double blink

- Preconditions: Saved Wi-Fi credentials exist; keep the access point unavailable long enough to observe connection attempts.
- Operation: Boot the device.
- Expected LED behavior: 100 ms ON, 100 ms OFF, 100 ms ON, 700 ms OFF, repeating.
- Expected system behavior: Existing connection timeout and AP fallback remain unchanged.
- PASS: The double blink is recognizable during the connection attempt.

## LED-04: Wi-Fi ready solid ON

- Preconditions: Saved credentials refer to an available Wi-Fi network.
- Operation: Boot and wait for STA connection.
- Expected LED behavior: Solid ON after connection.
- Expected system behavior: WebUI and OSC operation remain available.
- PASS: The LED remains steadily ON when idle and connected.

## LED-05: Key Press OSC activity

- Preconditions: Wi-Fi ready; a Key Press message is configured.
- Operation: Press the Key.
- Expected LED behavior: One short, approximately 50 ms OFF pulse over the solid-ON base.
- Expected OSC behavior: The configured Press message is sent normally.
- PASS: Activity is visible and Key scanning is not delayed.

## LED-06: Key Release OSC activity

- Preconditions: Wi-Fi ready; a Key Release message is configured.
- Operation: Press and release the Key.
- Expected LED behavior: Release produces a short OFF pulse.
- Expected OSC behavior: The configured Release message is sent normally.
- PASS: Release activity is visible without changing message behavior.

## LED-07: Key Sequence OSC activity

- Preconditions: Wi-Fi ready; a Key is in Sequence mode.
- Operation: Press the Key repeatedly.
- Expected LED behavior: Each accepted transmission can produce a short OFF pulse.
- Expected OSC behavior: Sequence values and progression remain unchanged.
- PASS: Activity is visible and Sequence semantics are unchanged.

## LED-08: Encoder Rotation OSC activity

- Preconditions: Wi-Fi ready; Encoder Rotation is configured.
- Operation: Rotate one detent in each direction, then rotate continuously.
- Expected LED behavior: Short OFF activity pulses are visible.
- Expected OSC behavior: Rotation values and direction remain unchanged.
- PASS: Rotation remains responsive and the LED does not block input processing.

## LED-09: Encoder Push OSC activity

- Preconditions: Wi-Fi ready; test Press/Release and Sequence push modes.
- Operation: Press and release the Encoder in each mode.
- Expected LED behavior: A short OFF pulse appears for each actual transmission.
- Expected OSC behavior: Push Press, Release, and Sequence messages remain unchanged.
- PASS: All three paths show activity without behavioral regression.

## LED-10: OSC skip while disconnected

- Preconditions: Start connected, then make Wi-Fi unavailable.
- Operation: Operate Keys and Encoder while `WiFi disconnected` skips appear in Serial output.
- Expected LED behavior: No OSC Activity pulse is produced; the network base pattern applies.
- Expected OSC behavior: Existing disconnected-send skip behavior is unchanged.
- PASS: Skipped operations do not create Activity indications.

## LED-11: High-speed Encoder rotation

- Preconditions: Wi-Fi ready; Encoder Rotation is configured.
- Operation: Rotate the Encoder rapidly for at least five seconds.
- Expected LED behavior: Repeated activity remains visible, with base-status ON intervals between pulses; the LED does not remain continuously OFF.
- Expected OSC behavior: Rotation scanning and OSC transmission remain responsive.
- PASS: No blocking, unbounded pulse extension, or permanent OFF state occurs.

## LED-12: Network state transitions

- Preconditions: Ability to make the configured access point available/unavailable and to clear Wi-Fi settings.
- Operation: Exercise Connecting to Ready and boot/fallback to AP paths.
- Expected LED behavior: The latest state selects double blink, solid ON, or slow blink respectively; Activity never restores a stale base state.
- Expected network behavior: Connection, fallback, reconnect, WebUI, and captive portal semantics remain unchanged.
- PASS: Every transition displays the current network state.

## Human Acceptance Result

Hardware:

- Physical ChainOSCPad
- Status / Activity LED connected to XIAO D10 through the PCB resistor

Result:

- LED-01: PASS
- LED-02: PASS
- LED-03: PASS
- LED-04: PASS
- LED-05: PASS
- LED-06: PASS
- LED-07: PASS
- LED-08: PASS
- LED-09: PASS
- LED-10: PASS
- LED-11: PASS
- LED-12: PASS

LED polarity:

- Active HIGH: Physical PASS
- Further polarity confirmation required: No

Overall: **PASS**
