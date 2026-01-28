# MIOS Studio Terminal Not Working - But TeraTerm Works

## Problem Statement

- ✅ MIOS Studio recognizes device
- ✅ TeraTerm works and shows debug output
- ❌ MIOS Studio terminal shows nothing
- ⚠️ User reports: "there is only one Vcom port"

## Understanding MIOS Studio Terminal

### How MIOS Studio Terminal Works

MIOS Studio has two ways to show terminal/debug output:

#### Method 1: USB CDC (Virtual COM Port)
- Uses separate CDC interface (second COM port)
- Standard serial terminal protocol
- Most common method
- **Requires:** USB composite device with CDC interface

#### Method 2: MIDI SysEx Debug Messages
- Uses MIDI interface for debug data
- Debug messages sent as SysEx: `F0 00 00 7E 32 <dev> 0x0D <text> F7`
- Legacy MIOS32 protocol
- **Requires:** Special SysEx debug message implementation

## Current Status

Since TeraTerm works, one of these is true:

### Scenario A: CDC Works, MIOS Studio Misconfigured
**Symptoms:**
- TeraTerm shows debug output
- Only one COM port visible (or user didn't check carefully)
- MIOS Studio terminal blank

**Cause:**
- CDC is working (TeraTerm proves it)
- MIOS Studio not configured to use correct port
- Or MIOS Studio expects SysEx debug, not CDC

**Fix:**
Configure MIOS Studio terminal to use CDC port.

### Scenario B: MIDI-Based Terminal Expected
**Symptoms:**
- TeraTerm works via CDC
- MIOS Studio terminal blank
- MIOS Studio expects debug via MIDI SysEx

**Cause:**
- MidiCore sends debug to CDC only (dbg_print → USB CDC)
- MIOS Studio terminal expects MIDI SysEx debug messages
- No bridge between the two

**Fix:**
Implement MIOS32 debug message protocol (0x0D command).

## Solution: MIOS Studio Terminal Configuration

### Step 1: Verify COM Port Count

Open Device Manager (Win+X → Device Manager) and expand "Ports (COM & LPT)".

You should see:
```
Ports (COM & LPT)
  └─ USB Serial Device (COM3)    ← MidiCore CDC
```

And under "Sound, video and game controllers":
```
Sound, video and game controllers
  └─ MidiCore 4x4               ← MidiCore MIDI (not a COM port!)
```

**Note:** USB MIDI is NOT a COM port! It appears under audio/game devices.
So "one Vcom port" is CORRECT - that's the CDC interface.

### Step 2: Configure MIOS Studio Terminal

#### Option A: Use CDC Terminal (Recommended)

1. **Open MIOS Studio**
2. **Go to Menu**: View → Terminal
3. **Configure Terminal**:
   - Port: Select the COM port (same as TeraTerm uses)
   - Baud: 115200 (doesn't matter for USB CDC, but set it)
4. **Click Connect**

#### Option B: If Terminal Settings Not Available

MIOS Studio might auto-detect the terminal port. If it doesn't:

1. **Check MIOS Studio Preferences**:
   - Tools → Preferences
   - Look for "Terminal" or "Debug" section
   - Set COM port manually

2. **Restart MIOS Studio**:
   - Close completely
   - Reopen
   - Check terminal again

### Step 3: Verify Output

Send MIDI data (play notes in MIOS Studio or DAW).

TeraTerm shows:
```
[RX] Cable:0 90 3C 64 (Note On Ch:1 Note:60 Vel:100)
```

MIOS Studio terminal should show the SAME output.

## If MIOS Studio Terminal Still Blank

### Check 1: MIOS Studio Version

MIOS Studio v2.4+ required for proper CDC terminal support.

**Update MIOS Studio:**
- Download latest from: http://www.ucapps.de/mios_studio.html
- Install new version
- Test again

### Check 2: Terminal Not Opened

MIOS Studio doesn't always open terminal by default.

**Open Terminal Window:**
1. View → Terminal (or Window → Terminal)
2. Should see terminal panel appear
3. Click "Connect" if there's a connect button

### Check 3: Terminal Using Wrong Port

MIOS Studio might be trying to use a different port.

**Check Port Selection:**
1. In terminal panel, look for port dropdown
2. Select your MidiCore COM port
3. Click Connect/Apply

### Check 4: MIOS Studio Expects MIDI SysEx Debug

Some MIOS Studio versions expect debug via MIDI SysEx, not CDC.

**Test:** Does MIOS Studio have a separate "MIDI Terminal" option?

If yes, MidiCore needs MIOS32 debug message implementation (see below).

## Implementation: MIOS32 Debug Messages (If Needed)

If MIOS Studio requires MIDI SysEx debug messages, implement this:

### MIOS32 Debug Message Protocol

```
F0 00 00 7E 32 <dev_id> 0x0D <ascii_text> F7

Where:
  F0         = SysEx start
  00 00 7E   = MIOS32 manufacturer ID
  32         = MIOS32 device ID
  <dev_id>   = Device instance (0x00 for first device)
  0x0D       = Debug message command
  <text>     = ASCII text to display
  F7         = SysEx end
```

### Add to Services/usb_midi/usb_midi.c

```c
// MIOS32 debug message sender
void usb_midi_send_debug_message(const char* text, uint8_t cable)
{
  if (!text) return;
  
  size_t len = strlen(text);
  if (len == 0 || len > 200) return; // Sanity check
  
  // Build MIOS32 debug message SysEx
  uint8_t sysex[256];
  sysex[0] = 0xF0;           // SysEx start
  sysex[1] = 0x00;           // MIOS32 manufacturer
  sysex[2] = 0x00;
  sysex[3] = 0x7E;
  sysex[4] = 0x32;           // MIOS32 device ID
  sysex[5] = 0x00;           // Device instance
  sysex[6] = 0x0D;           // Debug message command
  
  // Copy text
  memcpy(&sysex[7], text, len);
  sysex[7 + len] = 0xF7;     // SysEx end
  
  // Send via USB MIDI
  usb_midi_send_sysex(sysex, 8 + len, cable);
}
```

### Modify Services/test/test.c

Dual output - both CDC and MIDI:

```c
void dbg_print(const char *str)
{
  #if MODULE_ENABLE_USB_CDC
    usb_cdc_send((const uint8_t*)str, strlen(str));
  #endif
  
  #if MODULE_ENABLE_USB_MIDI
    // Also send as MIOS32 debug message for MIOS Studio
    usb_midi_send_debug_message(str, 0);
  #endif
}
```

## Quick Test

### Test 1: Verify TeraTerm Port Number

1. **Close TeraTerm**
2. **Open Device Manager**
3. **Note COM port number**
4. **Open TeraTerm on that port**
5. **Verify output appears**

### Test 2: Try MIOS Studio Terminal on Same Port

1. **Keep TeraTerm CLOSED** (can't have both open)
2. **Open MIOS Studio**
3. **Open Terminal** (View → Terminal)
4. **Select SAME COM port** as TeraTerm used
5. **Send MIDI data**
6. **Check if output appears**

## Most Likely Solution

**MIOS Studio terminal is just not configured to use the CDC port.**

**Quick Fix:**
1. Open MIOS Studio
2. View → Terminal
3. Select your COM port (same as TeraTerm)
4. Done!

If that doesn't work, MIOS Studio might need MIDI SysEx debug messages instead of CDC.

## Summary

| Component | Status | Notes |
|-----------|--------|-------|
| USB CDC | ✅ Working | TeraTerm proves this |
| Debug Output | ✅ Working | Messages appear in TeraTerm |
| MIOS Studio Recognition | ✅ Working | Device detected |
| MIOS Studio Terminal | ❌ Not Working | Configuration issue |

**Action:** Configure MIOS Studio terminal to use the CDC COM port, or implement MIOS32 SysEx debug if required.

## Next Steps

1. **Tell me:** Can you see a "Terminal" option in MIOS Studio's View menu?
2. **Tell me:** Does the Terminal panel have a COM port selector?
3. **Tell me:** What MIOS Studio version are you using?
4. **Screenshot:** MIOS Studio terminal window would help diagnose

The fix is likely just a configuration issue, not a code problem!
