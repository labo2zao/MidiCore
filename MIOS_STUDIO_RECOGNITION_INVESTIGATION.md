# MIOS Studio Recognition - Deep Investigation Report

## Executive Summary

This document provides a comprehensive investigation into why MidiCore may not be recognized by MIOS Studio, covering both USB CDC (terminal) and USB MIDI (MIOS32 protocol) aspects.

**Status:** ✅ Code implementation is CORRECT - if device not recognized, likely a build/flash or configuration issue

---

## Table of Contents

1. [How MIOS Studio Discovers Devices](#how-mios-studio-discovers-devices)
2. [MidiCore Implementation Analysis](#midicore-implementation-analysis)
3. [Diagnostic Tools](#diagnostic-tools)
4. [Troubleshooting Guide](#troubleshooting-guide)
5. [Technical Deep Dive](#technical-deep-dive)

---

## How MIOS Studio Discovers Devices

### Two-Stage Discovery Process

MIOS Studio uses a two-stage handshake to discover and identify MIOS32-compatible devices:

#### Stage 1: USB Device Enumeration

```
1. Device connects via USB
   ↓
2. Operating System detects USB device
   ↓
3. OS reads USB descriptors (VID/PID/Class)
   ↓
4. OS loads appropriate drivers:
   - USB MIDI class driver (for MIDI interface)
   - USB CDC ACM driver (for Virtual COM port)
   ↓
5. Device appears in:
   - MIDI device list (e.g., "MidiCore 4x4")
   - Serial port list (e.g., "COM3" or "/dev/ttyACM0")
```

**Expected USB Configuration:**
- **Device Class:** 0xEF (Miscellaneous / IAD Composite)
- **Interface 0-1:** USB Audio (MIDI) - 2 endpoints
- **Interface 2-3:** USB CDC - 3 endpoints (control, data in, data out)

#### Stage 2: MIOS32 Protocol Handshake

MIOS Studio sends **MIOS32 Query messages** via USB MIDI:

```
Query Message Format:
F0 00 00 7E 32 <dev_id> <cmd> <query_type> F7

Where:
- F0           : SysEx start
- 00 00 7E     : MIOS manufacturer ID
- 32           : MIOS32 device ID (NOT application ID)
- <dev_id>     : Device instance (usually 0x00)
- <cmd>        : Command (0x00 = query)
- <query_type> : What to query (0x01-0x09)
- F7           : SysEx end
```

**Example Query:**
```
Query:    F0 00 00 7E 32 00 00 01 F7
Purpose:  Request operating system name
```

**Expected Response:**
```
Response: F0 00 00 7E 32 00 0F 4D 49 4F 53 33 32 F7
Decoded:  F0 00 00 7E 32 [dev] [ACK] "MIOS32" F7

Where:
- 0F      : ACK response code
- ASCII   : Response string ("MIOS32")
- NO null terminator in SysEx stream!
```

### Query Types (Per MIOS32 Specification)

| Type | Purpose | Expected Response Example |
|------|---------|---------------------------|
| 0x01 | Operating System | "MIOS32" |
| 0x02 | Board Name | "STM32F407VGT6" |
| 0x03 | Core Family | "STM32F4" |
| 0x04 | Chip ID | "12345678" (hex) |
| 0x05 | Serial Number | "000001" |
| 0x06 | Flash Size | "1048576" (bytes) |
| 0x07 | RAM Size | "131072" (bytes) |
| 0x08 | Application Name Line 1 | "MidiCore" |
| 0x09 | Application Version Line 2 | "1.0.0" |

**Recognition Logic:**
```
If device responds to query 0x01 with "MIOS32"
→ Device IS recognized as MIOS32-compatible
→ MIOS Studio will query types 0x08 and 0x09 for display name
→ CDC terminal becomes active for debug messages
```

---

## MidiCore Implementation Analysis

### ✅ USB CDC Implementation (CORRECT)

**Files Checked:**
- `USB_DEVICE/Class/CDC/Src/usbd_cdc.c` - CDC class driver
- `USB_DEVICE/Class/CDC/Src/usbd_cdc_if.c` - Interface layer
- `Services/usb_cdc/usb_cdc.c` - Service layer
- `Core/Src/main.c` - Initialization

**Implementation Status:**

| Component | Status | Notes |
|-----------|--------|-------|
| Interface Registration | ✅ CORRECT | `usb_cdc_init()` calls `USBD_CDC_RegisterInterface()` |
| Line Coding Handler | ✅ CORRECT | `SET_LINE_CODING`, `GET_LINE_CODING` properly handled |
| Control Line State | ✅ CORRECT | `SET_CONTROL_LINE_STATE` (DTR/RTS) working |
| Connection Detection | ✅ CORRECT | `USBD_CDC_IsConnected()` checks DTR bit |
| RX Data Path | ✅ CORRECT | Interrupt → Queue → Task context processing |
| TX Data Path | ✅ CORRECT | Non-blocking with busy check |
| Flow Control | ✅ CORRECT | RX queue prevents data loss |

**CDC Data Flow:**
```
1. USB RX Interrupt (EP_OUT)
   ↓
2. USBD_CDC_DataOut() callback
   ↓
3. CDC_Receive_FS() in usbd_cdc_if.c
   ↓
4. usb_cdc_rx_callback_internal() - enqueues data
   ↓
5. usb_cdc_process_rx_queue() - called from MIDI IO task (1ms)
   ↓
6. Application callback (e.g., terminal echo)
```

**Initialization Sequence:**
```c
// Core/Src/main.c
#if MODULE_ENABLE_USB_CDC
  extern void usb_cdc_init(void);
  usb_cdc_init();  // ← CRITICAL: Must be called!
#endif

// Services/usb_cdc/usb_cdc.c
void usb_cdc_init(void) {
  // Register CDC interface callbacks
  USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_CDC_fops);
}
```

### ✅ USB MIDI Implementation (CORRECT)

**Files Checked:**
- `Services/usb_midi/usb_midi.c` - MIDI service
- `Services/usb_midi/usb_midi_sysex.c` - SysEx transmission
- `USB_DEVICE/Class/MIDI/Src/usbd_midi.c` - MIDI class

**Implementation Status:**

| Component | Status | Notes |
|-----------|--------|-------|
| RX Queue | ✅ CORRECT | Deferred processing from interrupt |
| TX Queue | ✅ CORRECT | Flow control for multi-packet SysEx |
| SysEx Buffering | ✅ CORRECT | Per-cable buffers (4 cables) |
| CIN Handling | ✅ CORRECT | Proper USB MIDI packet encoding |

**MIDI SysEx Data Flow:**
```
1. USB RX Interrupt (EP_OUT)
   ↓
2. USBD_MIDI_DataOut() callback
   ↓
3. Enqueue in rx_queue[] (16 packets x 4 bytes)
   ↓
4. usb_midi_process_rx_queue() - called from MIDI IO task
   ↓
5. Parse CIN (Cable Index Number)
   ↓
6. Accumulate in per-cable sysex_buffer[]
   ↓
7. On SysEx end (CIN 0x5/0x6/0x7 with F7):
   - Check if MIOS32 query → process
   - Otherwise → route to MIDI router
```

### ✅ MIOS32 Query Handler (CORRECT)

**Files Checked:**
- `Services/mios32_query/mios32_query.c`
- `Services/mios32_query/mios32_query.h`

**Implementation Status:**

| Function | Status | Notes |
|----------|--------|-------|
| `mios32_query_is_query_message()` | ✅ CORRECT | Validates F0 00 00 7E 32 header |
| `mios32_query_process()` | ✅ CORRECT | Extracts query type, calls response handler |
| `mios32_query_send_response()` | ✅ CORRECT | Builds proper ACK response (0x0F + string) |
| Query Type Handling | ✅ CORRECT | All 9 types implemented with switch statement |
| Response Format | ✅ CORRECT | NO null terminator (matches MIOS32 spec) |
| Cable Awareness | ✅ CORRECT | Responds on same cable as query |

**Query Processing Code:**
```c
// Services/usb_midi/usb_midi.c (line 313-315)
if (mios32_query_is_query_message(buf->buffer, buf->pos)) {
  // Respond on the same cable, then swallow the query
  mios32_query_process(buf->buffer, buf->pos, cable);
  // Don't route query messages - handler already replied
}
```

**Response Generation:**
```c
// Services/mios32_query/mios32_query.c
void mios32_query_send_response(uint8_t query_type, uint8_t device_id, uint8_t cable) {
  // Build: F0 00 00 7E 32 <device_id> 0x0F <string> F7
  *p++ = 0xF0;  // SysEx start
  *p++ = 0x00;  // Manufacturer ID 1
  *p++ = 0x00;  // Manufacturer ID 2
  *p++ = 0x7E;  // Manufacturer ID 3
  *p++ = 0x32;  // MIOS32 device ID
  *p++ = device_id;  // Echo device instance
  *p++ = 0x0F;  // ACK code
  
  // Copy string (NO null terminator!)
  while (*response_str) {
    *p++ = *response_str++;
  }
  
  *p++ = 0xF7;  // SysEx end
  
  // Send via USB MIDI
  usb_midi_send_sysex(sysex_response_buffer, p - sysex_response_buffer, cable);
}
```

---

## Diagnostic Tools

### Python Test Scripts

Two diagnostic tools have been created in the `Tools/` directory:

#### 1. MIOS32 Recognition Test (`test_mios32_recognition.py`)

**Purpose:** Test if device responds to MIOS32 queries via USB MIDI

**Requirements:**
```bash
pip install python-rtmidi
```

**Usage:**
```bash
cd Tools
python3 test_mios32_recognition.py
```

**What it does:**
1. Lists available MIDI ports
2. Auto-detects MidiCore device (or prompts for selection)
3. Sends all 9 query types (0x01-0x09)
4. Validates responses
5. Reports pass/fail for each query

**Expected Output (if working):**
```
============================================================
                 MIOS32 Device Recognition Diagnostic
============================================================

Testing Query Type 0x01: Operating System
  Sending: f0 00 00 7e 32 00 00 01 f7
  Received: f0 00 00 7e 32 00 0f 4d 49 4f 53 33 32 f7
✓ Valid response: "MIOS32"

Testing Query Type 0x08: Application Name
  Sending: f0 00 00 7e 32 00 00 08 f7
  Received: f0 00 00 7e 32 00 0f 4d 69 64 69 43 6f 72 65 f7
✓ Valid response: "MidiCore"

...

============================================================
                      Test Summary
============================================================
Results:
  Total queries: 9
  Passed: 9
  Failed: 0
  
✓ All tests passed! Device should be recognized by MIOS Studio.
```

#### 2. CDC Terminal Test (`test_cdc_terminal.py`)

**Purpose:** Test USB CDC (Virtual COM port) communication

**Requirements:**
```bash
pip install pyserial
```

**Usage:**
```bash
cd Tools
python3 test_cdc_terminal.py
```

**What it does:**
1. Lists available serial ports
2. Auto-detects MidiCore COM port
3. Tests line coding (baud rate, data bits, etc.)
4. Tests control lines (DTR/RTS)
5. Tests echo functionality
6. Tests sustained data transfer
7. Offers interactive terminal mode

**Expected Output (if working):**
```
============================================================
                  USB CDC Terminal Test Tool
============================================================

Available Serial Ports:
  0: COM3
     Description: STMicroelectronics Virtual COM Port
     VID:PID = 16C0:0489

✓ Auto-detected MidiCore port: COM3

============================================================
                     Line Coding Test
============================================================
ℹ Testing CDC line coding (baud rate, data bits, etc.)...

  Current settings:
    Baud rate: 115200
    Data bits: 8
    Parity: N
    Stop bits: 1
✓ Line coding parameters accessible

...

============================================================
                      Test Summary
============================================================
  Line Coding          PASS
  Control Lines        PASS
  Echo                 PASS
  Data Transfer        PASS

Overall: 4/4 tests passed

✓ All tests passed! CDC terminal should work with MIOS Studio.
```

---

## Troubleshooting Guide

### Problem: MIOS Studio Doesn't Detect Device

**Symptoms:**
- Device appears in Windows Device Manager / Linux lsusb
- MIDI ports are visible
- COM port is visible
- But MIOS Studio shows "No devices detected"

**Diagnostic Steps:**

#### Step 1: Verify USB Enumeration

**Windows:**
```powershell
# Check if device appears
devmgmt.msc
# Look for:
# - Sound, video and game controllers → "MidiCore 4x4"
# - Ports (COM & LPT) → "STMicroelectronics Virtual COM Port (COMx)"
```

**Linux:**
```bash
# Check USB device
lsusb | grep 16C0:0489

# Check MIDI
aconnect -l | grep -i midi

# Check serial
ls /dev/ttyACM*

# Check kernel messages
dmesg | tail -20
```

**Expected:**
- ✅ USB device appears (VID:PID = 16C0:0489)
- ✅ MIDI interface enumerated
- ✅ CDC serial port created

#### Step 2: Test MIOS32 Query Response

Run the Python test script:
```bash
cd Tools
python3 test_mios32_recognition.py
```

**If all queries pass:**
→ Firmware is correct, likely MIOS Studio configuration issue

**If queries fail:**
→ Firmware issue - see [Firmware Issues](#firmware-issues) below

#### Step 3: Test CDC Terminal

Run the CDC test script:
```bash
cd Tools
python3 test_cdc_terminal.py
```

**If terminal works:**
→ CDC is functional, issue is with MIDI queries

**If terminal doesn't work:**
→ CDC initialization issue - see [CDC Issues](#cdc-issues) below

### Firmware Issues

#### Issue: Device Not Responding to Queries

**Possible Causes:**
1. **Firmware not flashed** - Old firmware without MIOS32 query handler
2. **Module disabled** - `MODULE_ENABLE_USB_MIDI` is 0
3. **Not compiled in** - `mios32_query.o` missing from build

**Solution:**
```bash
# 1. Verify module is enabled
grep "MODULE_ENABLE_USB_MIDI" Config/module_config.h
# Should show: #define MODULE_ENABLE_USB_MIDI 1

# 2. Clean rebuild
# In STM32CubeIDE: Project → Clean, then Build

# 3. Verify object files
ls -la Debug/Services/mios32_query/
# Should see: mios32_query.o

# 4. Reflash firmware
# Run → Debug or Run → Run
```

#### Issue: Queries Timeout (No Response)

**Possible Causes:**
1. **TX queue full** - Too many messages, USB can't keep up
2. **USB MIDI not initialized** - `usb_midi_init()` not called
3. **Cable mismatch** - Response sent on wrong cable

**Solution:**
```c
// Check Core/Src/main.c initialization sequence
#if MODULE_ENABLE_USB_MIDI
  extern void usb_midi_init(void);
  usb_midi_init();  // ← Must be called!
#endif
```

#### Issue: Invalid Response Format

**Symptoms:**
- Response received but parse error
- Contains null bytes or garbage

**Possible Causes:**
1. **Old firmware** - Response format changed in recent commits
2. **Buffer overflow** - SysEx buffer too small

**Solution:**
```bash
# Check firmware version
git log --oneline Services/mios32_query/ | head -5

# Should see recent commits with query handler fixes
# If not, pull latest code and rebuild
```

### CDC Issues

#### Issue: COM Port Not Appearing

**Windows:**
```powershell
# Check if driver loaded
pnputil /enum-devices /class Ports

# Manually install driver if needed
# Use STM32 Virtual COM Port Driver from ST website
```

**Linux:**
```bash
# Check if cdc_acm driver loaded
lsmod | grep cdc_acm

# Check permissions
ls -l /dev/ttyACM0
# Should be: crw-rw---- ... dialout

# Add user to dialout group if needed
sudo usermod -a -G dialout $USER
# Logout and login again
```

#### Issue: COM Port Opens But No Data

**Possible Causes:**
1. **CDC not initialized** - `usb_cdc_init()` not called
2. **Interface not registered** - `USBD_CDC_RegisterInterface()` skipped
3. **RX queue not processed** - `usb_cdc_process_rx_queue()` not called

**Solution:**
```c
// Check App/app_init.c
#if MODULE_ENABLE_USB_CDC
  usb_cdc_init();  // ← Must be called
  usb_cdc_register_receive_callback(cdc_terminal_echo);
#endif

// Check App/midi_io_task.c
usb_cdc_process_rx_queue();  // ← Must be called periodically
```

---

## Technical Deep Dive

### USB CDC Packet Flow

```
Host → Device (RX):
1. OS writes to COM port
   ↓
2. USB driver sends packet to EP_OUT (0x02)
   ↓
3. STM32 USB peripheral generates interrupt
   ↓
4. HAL_PCD_DataOutStageCallback()
   ↓
5. USBD_LL_DataOutStage()
   ↓
6. USBD_CDC_DataOut() in usbd_cdc.c
   ↓
7. CDC_Receive_FS() in usbd_cdc_if.c
   ↓
8. usb_cdc_rx_callback_internal() - enqueues data
   ↓
9. usb_cdc_process_rx_queue() - processes in task context
   ↓
10. Application callback (echo, CLI, etc.)

Device → Host (TX):
1. Application calls usb_cdc_send()
   ↓
2. Checks if CDC connected (DTR bit)
   ↓
3. USBD_CDC_TransmitData() copies to TX buffer
   ↓
4. USBD_CDC_TransmitPacket() sends to EP_IN (0x82)
   ↓
5. USB hardware sends packet
   ↓
6. HAL_PCD_DataInStageCallback() on completion
   ↓
7. USBD_CDC_DataIn() callback
   ↓
8. CDC_TransmitCplt_FS() (optional notification)
```

### USB MIDI SysEx Flow

```
Host → Device (Query):
1. MIOS Studio sends MIDI SysEx via MIDI OUT port
   ↓
2. OS sends USB MIDI packets to EP_OUT (0x01)
   Format: [Cable+CIN] [byte1] [byte2] [byte3]
   ↓
3. STM32 USB peripheral generates interrupt
   ↓
4. USBD_MIDI_DataOut() in usbd_midi.c
   ↓
5. usb_midi_rx_callback_internal() - enqueues packet
   ↓
6. usb_midi_process_rx_queue() - task context
   ↓
7. Parse CIN, accumulate in sysex_buffer[]
   ↓
8. On SysEx end (F7):
   - mios32_query_is_query_message() → true
   - mios32_query_process() → generates response
   ↓
9. mios32_query_send_response()
   ↓
10. usb_midi_send_sysex() - queues response packets

Device → Host (Response):
1. usb_midi_send_sysex() splits SysEx into USB MIDI packets:
   - CIN 0x4: SysEx continue (3 bytes, no F7)
   - CIN 0x5: SysEx end with 1 byte (contains F7)
   - CIN 0x6: SysEx end with 2 bytes (last is F7)
   - CIN 0x7: SysEx end with 3 bytes (last is F7)
   ↓
2. usb_midi_send_packet() enqueues in tx_queue[]
   ↓
3. tx_queue_send_next() sends packets with flow control
   ↓
4. USBD_MIDI_TransmitPacket() sends to EP_IN (0x81)
   ↓
5. USB hardware sends packet
   ↓
6. USBD_MIDI_DataIn() on completion
   ↓
7. Send next packet from queue (if any)
   ↓
8. MIOS Studio receives response via MIDI IN port
   ↓
9. Parses response, extracts string, displays device name
```

### MIOS32 Protocol State Machine

```
MIOS Studio:
    |
    | 1. Send Query (type 0x01)
    |    F0 00 00 7E 32 00 00 01 F7
    v
MidiCore:
    |
    | 2. Detect MIOS32 header
    |    Check: F0 00 00 7E 32
    v
    | 3. Parse query type (0x01)
    v
    | 4. Build response
    |    F0 00 00 7E 32 00 0F "MIOS32" F7
    v
    | 5. Send via USB MIDI
    |    (queued, flow-controlled)
    v
MIOS Studio:
    |
    | 6. Receive response
    v
    | 7. Parse ACK (0x0F)
    v
    | 8. Extract string ("MIOS32")
    v
    | 9. Device RECOGNIZED!
    v
    | 10. Query app name (types 0x08, 0x09)
    |     Display as "MidiCore 1.0.0"
    v
    | 11. Enable terminal, show in device list
```

---

## Conclusion

### What We Know

1. ✅ **USB CDC implementation is CORRECT**
   - All control requests properly handled
   - RX/TX flow control working
   - Interface registration correct

2. ✅ **USB MIDI implementation is CORRECT**
   - SysEx buffering per-cable
   - TX queue prevents packet dropping
   - RX queue prevents interrupt blocking

3. ✅ **MIOS32 query handler is CORRECT**
   - All 9 query types implemented
   - Response format matches MIOS32 spec exactly
   - Cable-aware responses

### Most Likely Causes of Non-Recognition

If device is NOT recognized after verifying code is correct:

1. **Firmware not flashed** (most common)
   - Old firmware without query handler
   - Solution: Clean rebuild and reflash

2. **Module disabled in config**
   - `MODULE_ENABLE_USB_MIDI` or `MODULE_ENABLE_USB_CDC` = 0
   - Solution: Enable in `Config/module_config.h`

3. **USB cable issue**
   - Charge-only cable (no data lines)
   - Solution: Try different USB cable

4. **Driver issue** (Windows)
   - CDC driver not loaded
   - Solution: Install STM32 Virtual COM Port Driver

5. **Permissions issue** (Linux)
   - User not in `dialout` group
   - Solution: `sudo usermod -a -G dialout $USER`

### Verification Steps

1. **Build firmware:**
   ```bash
   # Clean build in STM32CubeIDE
   # Verify mios32_query.o is created
   ```

2. **Flash firmware:**
   ```bash
   # Run → Debug or Run → Run
   # Verify "Download verified successfully"
   ```

3. **Test with Python scripts:**
   ```bash
   cd Tools
   python3 test_mios32_recognition.py  # Test MIDI queries
   python3 test_cdc_terminal.py         # Test CDC terminal
   ```

4. **Test with MIOS Studio:**
   - Open MIOS Studio
   - Select MIDI IN/OUT ports
   - Device should appear automatically
   - Terminal should work

### If Still Not Working

Contact the development team with:
- Output of both Python test scripts
- Screenshot of Windows Device Manager / Linux lsusb output
- Build log showing compiled object files
- MIOS Studio version and OS version

---

## Appendix: Reference Documentation

### MIOS32 Protocol References

- **MIOS32 MIDI Documentation:** http://midibox.org/mios32/manual/group___m_i_o_s32___m_i_d_i.html
- **MIOS Studio Page:** http://www.ucapps.de/mios_studio.html
- **MIDIbox NG SysEx:** http://www.midibox.org/mios32/midibox_ng/group___m_b_n_g___s_y_s_e_x.html
- **USB MIDI Specification:** USB Device Class Definition for MIDI Devices, v1.0

### MidiCore Source Files

**USB CDC:**
- `USB_DEVICE/Class/CDC/Src/usbd_cdc.c` - CDC class driver
- `USB_DEVICE/Class/CDC/Src/usbd_cdc_if.c` - Interface callbacks
- `Services/usb_cdc/usb_cdc.c` - Service layer
- `Services/usb_cdc/usb_cdc.h` - API

**USB MIDI:**
- `USB_DEVICE/Class/MIDI/Src/usbd_midi.c` - MIDI class driver
- `Services/usb_midi/usb_midi.c` - MIDI service
- `Services/usb_midi/usb_midi_sysex.c` - SysEx handling

**MIOS32 Query:**
- `Services/mios32_query/mios32_query.c` - Query handler
- `Services/mios32_query/mios32_query.h` - API

**Initialization:**
- `Core/Src/main.c` - Hardware and USB init
- `App/app_init.c` - Service init
- `App/midi_io_task.c` - Queue processing task

---

**Document Version:** 1.0  
**Last Updated:** 2026-01-28  
**Author:** MidiCore Investigation Team
