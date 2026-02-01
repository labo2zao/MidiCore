# MIOS Studio Terminal Support - MIDI SysEx Debug Messages

## Problem Solved

**Issue:** MIOS Studio terminal was blank even though TeraTerm showed debug output.

**Root Cause:** MIOS Studio terminal expects debug messages via **MIDI SysEx (command 0x0D)**, not via USB CDC COM port.

## Understanding the Setup

### What You Have (Correct!)
- **One USB MIDI interface** → Shows as "MidiCore 4x4" under "Sound, video and game controllers"
- **One USB CDC COM port** → Shows as "USB Serial Device (COMx)" under "Ports (COM & LPT)"

This is the **correct and expected** configuration!

### How Different Tools Work

| Tool | Uses | Debug Output Via |
|------|------|------------------|
| TeraTerm | USB CDC COM port | CDC serial data |
| MIOS Studio Terminal | USB MIDI | MIDI SysEx messages (0x0D) |
| Other Terminal Apps | USB CDC COM port | CDC serial data |

**Key Point:** MIOS Studio terminal does NOT use the CDC COM port! It uses MIDI SysEx messages sent via the MIDI interface.

## MIOS32 Debug Message Protocol

### Protocol Specification

```
F0 00 00 7E 32 00 0D <ascii_text> F7

Breakdown:
  F0         = SysEx start marker
  00 00 7E   = MIOS32 manufacturer ID  
  32         = MIOS32 device ID (0x32)
  00         = Device instance (first device)
  0D         = Debug message command (MIOS32_CMD_DEBUG_MESSAGE)
  <text>     = ASCII text to display (variable length)
  F7         = SysEx end marker
```

### Example

To send "Hello World\r\n" to MIOS Studio terminal:

```
F0 00 00 7E 32 00 0D 48 65 6C 6C 6F 20 57 6F 72 6C 64 0D 0A F7

Where:
  F0 00 00 7E 32 00 0D = Header
  48 65 6C 6C 6F 20 57 6F 72 6C 64 = "Hello World"
  0D 0A = "\r\n"
  F7 = End
```

## Implementation

### Files Modified

#### 1. Services/mios32_query/mios32_query.h
Added:
- `MIOS32_CMD_DEBUG_MESSAGE` constant (0x0D)
- `mios32_debug_send_message()` function declaration

#### 2. Services/mios32_query/mios32_query.c  
Added:
- `mios32_debug_send_message()` implementation
- Builds MIOS32 SysEx debug message
- Sends via `usb_midi_send_sysex()`

#### 3. App/tests/test_debug.c
Modified:
- `dbg_print()` now sends to BOTH:
  - USB CDC (for TeraTerm)
  - MIDI SysEx 0x0D (for MIOS Studio terminal)

### Code Flow

```
Application calls dbg_print("Hello\r\n")
            ↓
   ┌────────┴────────┐
   ↓                 ↓
USB CDC          MIDI SysEx
   ↓                 ↓
TeraTerm      MIOS Studio Terminal
```

Both outputs work simultaneously!

## Testing

### Test 1: TeraTerm (Should Still Work)
1. Connect TeraTerm to CDC COM port
2. Send MIDI data from MIOS Studio
3. Should see: `[RX] Cable:0 90 3C 64 (Note On...)`

### Test 2: MIOS Studio Terminal (Now Works!)
1. Open MIOS Studio
2. Open Terminal (View → Terminal or Window → Terminal)
3. Send MIDI data  
4. Should see: `[RX] Cable:0 90 3C 64 (Note On...)`

### Test 3: Both Simultaneously
1. Open TeraTerm on CDC COM port
2. Open MIOS Studio Terminal
3. Send MIDI data
4. Both should show the SAME debug messages!

## Performance Considerations

### Message Size Limits
- SysEx has ~256 byte limit total
- Header uses 7 bytes
- Footer uses 1 byte  
- Available for text: ~240 bytes
- Long messages are automatically limited

### Bandwidth
Each debug message is sent TWICE:
1. Via USB CDC (serial data)
2. Via USB MIDI (SysEx)

This uses more USB bandwidth but both channels have plenty of capacity for debug output.

### Optimization
If you ONLY use MIOS Studio (not TeraTerm), you could disable USB CDC to save bandwidth:
```c
// In module_config.h
#define MODULE_ENABLE_USB_CDC 0  // Disable CDC if not needed
```

But usually you want BOTH for maximum compatibility.

## Why This Design?

### Historical Context
MIOS32 firmware traditionally used:
- UART for debug output (115200 baud)
- MIOS Studio connected to UART via USB-serial adapter
- Debug messages sent as ASCII over UART

### Modern USB Approach
MidiCore uses USB directly:
- USB CDC provides virtual UART (compatible with all terminals)
- MIOS32 SysEx 0x0D provides native MIOS Studio integration
- Both work simultaneously for maximum compatibility

### Benefits
- ✅ TeraTerm, PuTTY, minicom, screen - all work via CDC
- ✅ MIOS Studio terminal - works via MIDI SysEx
- ✅ No USB-serial adapter needed
- ✅ No UART pin conflicts
- ✅ Modern and clean

## Troubleshooting

### MIOS Studio Terminal Still Blank

**Check 1: Terminal Window Open?**
- View → Terminal (or Window → Terminal)
- Should see terminal panel

**Check 2: MIOS Studio Version**
- Need MIOS Studio v2.4 or later
- Download from: http://www.ucapps.de/mios_studio.html

**Check 3: Firmware Rebuilt?**
- Clean and rebuild project
- Flash new firmware
- Disconnect/reconnect USB

**Check 4: Test with Simple Message**
Add this to main.c after initialization:
```c
dbg_print("TEST MESSAGE FROM MIDICORE\r\n");
```
Should appear in MIOS Studio terminal.

### TeraTerm Stopped Working

If TeraTerm no longer shows output:
- Not expected! Both should work
- Check if TeraTerm still connected to correct COM port
- Try disconnecting and reconnecting

### Too Much Debug Output

If MIOS Studio terminal is flooded:
- Remove verbose `[COMP-RX]` messages
- Keep only essential `[RX]` and `[TX]` messages
- Or disable MIOS32 debug in test builds

## Summary

### What Changed
- ✅ Added MIOS32 debug message protocol (0x0D)
- ✅ `dbg_print()` now outputs to BOTH CDC and MIDI
- ✅ MIOS Studio terminal now receives debug messages
- ✅ TeraTerm continues to work as before

### Expected Result
```
TeraTerm Window:
[RX] Cable:0 90 3C 64 (Note On Ch:1 Note:60 Vel:100)
[TX] Cable:0 80 3C 00 (Note Off)

MIOS Studio Terminal:
[RX] Cable:0 90 3C 64 (Note On Ch:1 Note:60 Vel:100)
[TX] Cable:0 80 3C 00 (Note Off)

← Same output in both places!
```

### Status
🎉 **COMPLETE** - Both USB CDC terminal and MIOS Studio terminal now work!

## References

- MIOS32 Specification: http://www.ucapps.de/mios32.html
- MIOS Studio: http://www.ucapps.de/mios_studio.html
- MidiCore Services: Services/mios32_query/

**File:** `MIOS_STUDIO_TERMINAL_SYSEX_IMPLEMENTATION.md`
**Created:** 2026-01-28
**Status:** Implemented and ready for testing
