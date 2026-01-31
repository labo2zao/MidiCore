# MIOS Studio Detection Debug Guide

## Current Status

**User reports:** MIOS Studio not detecting MidiCore despite working MIDI RX/TX

**Code Status:**
- ✅ MIOS32 query handler enabled in usb_midi.c
- ✅ MIOS32 response generator implemented
- ✅ USB MIDI RX/TX working with clean output
- ✅ Debug traces added to MIOS32 query handling

## Debug Traces Added

### What to Look For

With `MODULE_TEST_USB_DEVICE_MIDI=1`, you should see these messages when MIOS Studio connects:

#### When MIOS Studio Sends Query
```
[MIOS32-Q] Received query len:9 cable:0
[MIOS32-Q] dev_id:00 cmd:00 type:01
[MIOS32-R] Sending type:01 "MIOS32" cable:0
[MIOS32-R] Sent 15 bytes
```

#### Query/Response Cycle
MIOS Studio will send multiple queries (types 0x01-0x09):
```
[MIOS32-Q] Received query len:9 cable:0
[MIOS32-Q] dev_id:00 cmd:00 type:01
[MIOS32-R] Sending type:01 "MIOS32" cable:0
[MIOS32-R] Sent 15 bytes

[MIOS32-Q] Received query len:9 cable:0
[MIOS32-Q] dev_id:00 cmd:00 type:08
[MIOS32-R] Sending type:08 "MidiCore" cable:0
[MIOS32-R] Sent 16 bytes
```

## Diagnostic Steps

### Step 1: Check if MIOS Studio is Sending Queries

**Expected:** When you open MIOS Studio, it should immediately send MIOS32 queries

**If you see `[MIOS32-Q]` messages:**
- ✅ MIOS Studio is sending queries
- ✅ MidiCore is receiving them
- → Continue to Step 2

**If you DON'T see `[MIOS32-Q]` messages:**
- ❌ MIOS Studio not sending queries OR
- ❌ USB MIDI SysEx not being received
- → Check USB connection and device enumeration

### Step 2: Check if Responses are Being Sent

**Expected:** After each query, you should see `[MIOS32-R]` messages

**If you see `[MIOS32-R]` messages:**
- ✅ Responses being sent
- → Continue to Step 3

**If you DON'T see `[MIOS32-R]` messages:**
- ❌ Response generation failing
- → Check if MODULE_ENABLE_USB_MIDI is defined
- → Check if usb_midi_send_sysex() is working

### Step 3: Check Response Content

**Expected responses for each query type:**

| Type | Expected Response |
|------|-------------------|
| 0x01 | "MIOS32" |
| 0x02 | "STM32F407VGT6" |
| 0x03 | "STM32F4" |
| 0x08 | "MidiCore" |
| 0x09 | "1.0.0" |

**If responses are correct:**
- ✅ Query handler working perfectly
- → Continue to Step 4

### Step 4: Check if MIOS Studio is Receiving Responses

**Problem scenarios:**

#### A. MIOS Studio Not Receiving ANY Responses
**Symptoms:**
- Device not appearing in MIOS Studio
- MIOS Studio shows "Searching..." indefinitely

**Possible causes:**
1. USB MIDI TX path broken
2. SysEx not being transmitted properly
3. MIOS Studio looking at wrong MIDI port

**Debug:**
```
- Check [TX SysEx] messages appear after [MIOS32-R]
- Use MIDI monitor tool to verify SysEx transmission
- Ensure MIOS Studio is set to correct MIDI port
```

#### B. MIOS Studio Receiving Partial Responses
**Symptoms:**
- Device appears briefly then disappears
- MIOS Studio shows errors

**Possible causes:**
1. SysEx fragmentation issues
2. Response format incorrect
3. Timing issues

#### C. MIOS Studio Receiving All Responses but Not Recognizing
**Symptoms:**
- All 9 query/response cycles complete
- Device still not recognized

**Possible causes:**
1. USB device descriptor strings don't match expectations
2. MIOS Studio version compatibility
3. Device already registered with wrong settings

## Complete Debug Output Example

### Successful Recognition
```
[MIOS32-Q] Received query len:9 cable:0
[MIOS32-Q] dev_id:00 cmd:00 type:01
[MIOS32-R] Sending type:01 "MIOS32" cable:0
[MIOS32-R] Sent 15 bytes
[TX SysEx] Cable:0 CIN:0x4 Data: F0 00 00
[TX SysEx] Cable:0 CIN:0x4 Data: 7E 32 00
[TX SysEx] Cable:0 CIN:0x4 Data: 0F 4D 49
[TX SysEx] Cable:0 CIN:0x6 Data: 4F 53 F7

[MIOS32-Q] Received query len:9 cable:0
[MIOS32-Q] dev_id:00 cmd:00 type:08
[MIOS32-R] Sending type:08 "MidiCore" cable:0
[MIOS32-R] Sent 16 bytes
[TX SysEx] Cable:0 CIN:0x4 Data: F0 00 00
[TX SysEx] Cable:0 CIN:0x4 Data: 7E 32 00
[TX SysEx] Cable:0 CIN:0x4 Data: 0F 4D 69
[TX SysEx] Cable:0 CIN:0x4 Data: 64 69 43
[TX SysEx] Cable:0 CIN:0x5 Data: 6F 72 65
[TX SysEx] Cable:0 CIN:0x5 Data: F7 00 00
```

## Testing Without MIOS Studio

You can test MIOS32 query handling using the diagnostic tool:

```bash
cd Tools
python3 test_mios32_recognition.py
```

This will send the same queries MIOS Studio sends and verify responses.

## Common Issues and Solutions

### Issue 1: No Debug Output at All
**Cause:** `MODULE_TEST_USB_DEVICE_MIDI` not defined
**Solution:** Rebuild with debug define enabled

### Issue 2: Query Received but No Response
**Cause:** `MODULE_ENABLE_USB_MIDI` not defined
**Solution:** Check module_config.h, rebuild

### Issue 3: Response Sent but Not Transmitted
**Cause:** USB MIDI TX path issue
**Solution:** Check usb_midi_send_sysex() implementation

### Issue 4: All Working but MIOS Studio Still Doesn't Detect
**Possible causes:**
1. **USB descriptor strings** - Check device shows as "MidiCore" in Device Manager
2. **MIOS Studio cache** - Close MIOS Studio, delete cache, restart
3. **MIDI port conflict** - Another application using the MIDI port
4. **Windows MIDI driver** - Device not properly registered
5. **MIOS Studio version** - Try latest version

## Next Steps

1. **Rebuild firmware** with debug traces
2. **Flash to device**
3. **Open terminal** to see debug output
4. **Open MIOS Studio** and observe what happens
5. **Report** what debug messages you see (or don't see)

The debug traces will immediately show:
- ✅ Is MIOS Studio sending queries?
- ✅ Is MidiCore receiving them?
- ✅ Is MidiCore sending responses?
- ✅ What are the response contents?

This will pinpoint exactly where the issue is!

## Files Modified

- `Services/mios32_query/mios32_query.c` - Added comprehensive debug traces
