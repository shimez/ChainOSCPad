# ChainOSCPad Key v1 Physical E2E Evidence Record

## 1. Purpose

This document records the physical end-to-end evidence collected for ChainOSCPad conformance with the maintained Device Preset v1 Key contract. It is an evidence record, not a new specification.

## 2. Baseline

### Common Source of Truth

- Repository: `shimez/ChainOSC`
- Key v1 Specification Closure: `b57494280b9f55c1b68aa12b390713dc5bb18005`
- Canonical Key Device Preset format: **v1**

### ChainOSCPad

Relevant fixes:

- `fc6102324071831c73686a8c16f59f7ca2245bb7` — `Fix Device Preset v1 Key validation`
- `6791758a9e722ad0bc563a5c044ee9c7a512bc7c` — `Fix Key v1 validation priority and test cleanup`

### Physical hardware

- Product: ChainOSCPad
- MCU: **Seeed Studio XIAO ESP32C5**
- KEY-06 used a Wi-Fi AP that could be stopped/restored without rebooting the board.

The procedure originally named XIAO ESP32S3 as reference hardware. This execution was on ESP32C5 and must not be reported as an S3 physical result.

## 3. Overall Result

```text
HTTP-01           PASS
KEY-01            PASS
KEY-02            PASS
KEY-03            PASS
KEY-04            PASS
KEY-05            PASS
KEY-06            PASS
Final Restoration PASS

Overall Key v1 Physical E2E: PASS
Open physical findings: None
```

## 4. HTTP-01 — Product-level validation

Environment:

- PowerShell 7.6.5
- BaseUri `http://192.168.0.12`
- Key index `0`

Authoritative output:

```text
PASS deviceTypeName canonical status=200
PASS unknown root field status=400 E_PRESET_DEVICE_SETTING_INVALID
PASS unknown Key field status=400 E_PRESET_DEVICE_SETTING_INVALID
PASS unknown OSC Message field status=400 E_PRESET_DEVICE_SETTING_INVALID
PASS unknown Sequence field status=400 E_PRESET_DEVICE_SETTING_INVALID
PASS deviceTypeName wrong literal status=400 E_PRESET_DEVICE_SETTING_INVALID
PASS deviceTypeName missing status=400 E_PRESET_REQUIRED_FIELD_MISSING
PASS deviceTypeName wrong type status=400 E_PRESET_FIELD_TYPE_INVALID
PASS required field precedes wrong deviceTypeName literal status=400 E_PRESET_REQUIRED_FIELD_MISSING
PASS canonical OSC Address status=200
PASS OSC Address leading whitespace status=400 E_OSC_ADDRESS_INVALID
PASS OSC Address trailing whitespace status=400 E_OSC_ADDRESS_INVALID
Restoring original Key preset...
PASS restore original preset status=200
PASS ChainOSCPad Device Preset v1 Key validation regression suite
```

Pre-test and post-restore exported JSON were completely identical.

Result: **PASS — 12/12 plus restoration/integrity.**

## 5. KEY-01 — Press/Release order and duplicate preservation

Covered vector: `KEY-PRESS-RELEASE-ORDER`

Observed:

```text
/chainosc/e2e/key/order  ('first',)
/chainosc/e2e/key/order  ('duplicate',)
/chainosc/e2e/key/order  ('duplicate',)
/chainosc/e2e/key/order  ('last',)
/chainosc/e2e/key/order  ('release-first',)
/chainosc/e2e/key/order  ('release-last',)
```

Order was preserved and both duplicate messages were emitted.

Result: **PASS**

## 6. KEY-02 — Start First / reachable End inclusive

Covered:

- `KEY-SEQUENCE-START-FIRST`
- `KEY-SEQUENCE-ASCENDING-END-INCLUSIVE`
- `KEY-SEQUENCE-DESCENDING-END-INCLUSIVE`

Test address used: `/avatar/parameters/KeySeq`.

Ascending:

```text
0, 1, 2, 3, 0
```

Descending:

```text
3, 2, 1, 0, 3
```

Start was emitted first and reachable End was emitted before returning to Start.

Result: **PASS**

## 7. KEY-03 — Unreachable End / Start equals End

Covered:

- `KEY-SEQUENCE-UNREACHABLE-END-NO-SNAP`
- `KEY-SEQUENCE-START-EQUALS-END`

Unreachable End observed:

```text
0.0
0.30000001192092896
0.6000000238418579
0.9000000357627869
0.0
```

These are ordinary binary32 display differences for 0.3/0.6/0.9. No synthesized `1.0` was emitted.

Start == End observed:

```text
2, 2, 2
```

Result: **PASS**

## 8. KEY-04 — Cold-start Sequence position

Covered: `KEY-SEQUENCE-COLD-START`

Configuration: Int, Start=-2, End=2, Step=1.

Pre-reboot:

```text
-2, -1, 0
```

Reboot itself emitted **no Sequence OSC**.

First post-reboot physical press:

```text
-2
```

Runtime current reset to Start and was not persisted.

Result: **PASS**

## 9. KEY-05 — Invalid Import atomicity

Covered: `KEY-INVALID-IMPORT-PRESERVES-STATE`

Valid setup:

```text
Address: /chainosc/e2e/key/atomicity
Type: Int
Start: 0
End: 3
Step: 1
```

Two physical presses emitted:

```text
0, 1
```

so next runtime position was `2`.

A copy of the exported valid preset was modified **only** by changing `key.sequence.step` from `1` to `0`.

Import was rejected with:

```text
E_SEQUENCE_STEP_ZERO:
SequenceのStepには0を指定できません。StartからEndへ進む0以外の値を指定してください。
```

The failed Import emitted no OSC. Before-invalid and after-invalid exported JSON were completely identical.

Without rebooting or saving, the next physical press emitted:

```text
2
```

Complete receiver progression:

```text
0, 1, 2
```

Thus persisted settings and runtime Sequence position were preserved.

Result: **PASS**

## 10. KEY-06 — Detected send failure holds Sequence position

Covered: `KEY-SEQUENCE-DETECTED-FAILURE-HOLDS-POSITION`

Configuration:

```text
Address: /chainosc/e2e/key/detected-failure
Type: Int
Start: 0
End: 2
Step: 1
```

The AP was stopped without rebooting ChainOSCPad.

Serial evidence during disconnected physical press:

```text
[Key] 1 pressed
[OSC] skipped (WiFi disconnected): /chainosc/e2e/key/detected-failure 0
[Key] 1 released
[WiFi diag] STA disconnected reason=201
[WiFi diag] STA disconnected reason=36
```

No corresponding OSC packet was received during the failure.

After the same AP was restored, without reboot or Save:

```text
[Key] 1 pressed
[OSC] /chainosc/e2e/key/detected-failure 0 type=1 -> 192.168.8.120:9000
[Key] 1 released
[Key] 1 pressed
[OSC] /chainosc/e2e/key/detected-failure 1 type=1 -> 192.168.8.120:9000
[Key] 1 released
```

Receiver:

```text
/chainosc/e2e/key/detected-failure  (0,)
/chainosc/e2e/key/detected-failure  (1,)
```

Verified:

- detectable Wi-Fi-disconnected failure occurred;
- failed send did not advance Sequence position;
- first successful post-recovery value was `0`, then `1`;
- board reboot: **NO**;
- settings Save during interval: **NO**.

Result: **PASS**

## 11. Final Restoration and Integrity

The original Key Device Preset was restored.

- immediate re-export: **binary-identical** to original JSON;
- after reboot: re-export **binary-identical** to original JSON;
- original physical Key behavior: **normal**.

Result: **PASS**

## 12. Coverage Matrix

| Requirement / Vector | Evidence | Result |
|---|---|---|
| Product HTTP validation, 12 cases | HTTP-01 | PASS |
| `KEY-PRESS-RELEASE-ORDER` | KEY-01 | PASS |
| `KEY-SEQUENCE-START-FIRST` | KEY-02 | PASS |
| `KEY-SEQUENCE-ASCENDING-END-INCLUSIVE` | KEY-02 | PASS |
| `KEY-SEQUENCE-DESCENDING-END-INCLUSIVE` | KEY-02 | PASS |
| `KEY-SEQUENCE-UNREACHABLE-END-NO-SNAP` | KEY-03 | PASS |
| `KEY-SEQUENCE-START-EQUALS-END` | KEY-03 | PASS |
| `KEY-SEQUENCE-COLD-START` | KEY-04 | PASS |
| `KEY-INVALID-IMPORT-PRESERVES-STATE` | KEY-05 | PASS |
| `KEY-SEQUENCE-DETECTED-FAILURE-HOLDS-POSITION` | KEY-06 | PASS |
| Final restoration / integrity | Final Restoration | PASS |

## 13. Conclusion

For the physical scope exercised on **Seeed Studio XIAO ESP32C5**, all required ChainOSCPad Device Preset v1 Key Physical E2E scenarios passed.

```text
Overall Key v1 Physical E2E: PASS
Open physical findings: None
```

This does not claim that the same physical execution was performed on ESP32S3/C3/C6. Build and inspection evidence for other targets remain separately classified.
