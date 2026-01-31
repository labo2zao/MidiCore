# USB MIDI RX/TX Testing Guide

Complete guide for testing USB MIDI receive (RX) and transmit (TX) functionality on MidiCore.

## Overview

MidiCore implements a full USB MIDI 1.0 device with 4 virtual cables (ports). When `MODULE_TEST_USB_DEVICE_MIDI` is enabled, the firmware automatically:

- **TX**: Sends test MIDI messages every 2 seconds (Note On/Off)
- **RX**: Monitors and displays all incoming MIDI messages in the CDC terminal

## Quick Start

### 1. Build and Flash

```bash
# In Config/module_config.h, ensure:
#define MODULE_TEST_USB_DEVICE_MIDI 1
#define MODULE_ENABLE_USB_MIDI 1
#define MODULE_ENABLE_USB_CDC 1

# Build and flash firmware
# (Use STM32CubeIDE or your preferred tool)
```

### 2. Connect and Open Terminal

**Windows:**
```powershell
# Device Manager → Ports → Find COM port (e.g., COM11)
# Open with PuTTY, TeraTerm, or:
mode COM11:115200,n,8,1
type COM11:
```

**Linux:**
```bash
# Find device
ls /dev/ttyACM*

# Open terminal
screen /dev/ttyACM0
# or
minicom -D /dev/ttyACM0
```

### 3. Test TX (Device → Computer)

TX test runs automatically. You should see in terminal:

```
[TX] Cable:0 90 3C 64 (Note On)
[TX] Cable:0 80 3C 00 (Note Off)
[TX] Cable:0 90 3C 64 (Note On)
```

To verify in DAW/MIDI Monitor:
1. Open MIDI monitoring software (MIDI-OX, MIDI Monitor, DAW)
2. Select "MidiCore 4x4" as MIDI input
3. You should see Note On/Off messages every 2 seconds

### 4. Test RX (Computer → Device)

1. Keep CDC terminal open
2. Open DAW or MIDI controller software
3. Select "MidiCore 4x4" as MIDI **output** device
4. Send MIDI messages (play keyboard, send CCs, etc.)

You should see in terminal:

```
[RX] Cable:0 90 3C 64 (Note On Ch:1 Note:60 Vel:100)
[RX] Cable:0 B0 07 64 (CC Ch:1 CC:7 Val:100)
[RX] Cable:0 80 3C 00 (Note Off Ch:1 Note:60 Vel:0)
```

---

## Detailed Testing Procedures

### TX Testing (Device → Computer)

#### What Happens Automatically

The test firmware sends:
- **Note On** (90 3C 64) - Middle C, velocity 100
- **Note Off** (80 3C 00) - Middle C, velocity 0
- Interval: 2 seconds between messages
- Cable: 0 (USB MIDI Port 1)
- Channel: 1

#### Expected Terminal Output

```
USB MIDI Device Test
====================================
USB Device MIDI: Enabled
Test send interval: 2000 ms
Test channel: 1
Test note: 60
USB Cable: 0
====================================

[TX] Cable:0 90 3C 64 (Note On)
[TX] Cable:0 80 3C 00 (Note Off)
[TX] Cable:0 90 3C 64 (Note On)
```

#### How to Verify in MIDI Monitor

**Using MIDI-OX (Windows):**
1. Options → MIDI Devices → Select "MidiCore 4x4" as input
2. View → Input Monitor
3. You should see:
   ```
   00000000  90 3C 64  Note On  Ch:1  Note:60  Vel:100
   00002000  80 3C 00  Note Off Ch:1  Note:60  Vel:0
   ```

**Using MIDI Monitor (macOS):**
1. Select "MidiCore 4x4" in Sources
2. You should see Note On/Off messages every 2 seconds

**Using DAW (Any Platform):**
1. Create MIDI track
2. Set input to "MidiCore 4x4"
3. Enable MIDI input monitoring
4. You should see notes triggering every 2 seconds

### RX Testing (Computer → Device)

#### Using Virtual MIDI Keyboard

**Windows - VMPK (Virtual MIDI Piano Keyboard):**
```
1. Download VMPK from vmpk.sourceforge.net
2. Edit → MIDI Connections
3. MIDI OUT: Select "MidiCore 4x4"
4. Play notes on virtual keyboard
```

**macOS - Virtual MIDI Keyboard:**
```
1. Download from sourceforge
2. Preferences → MIDI Connections
3. Output: "MidiCore 4x4"
4. Play notes
```

#### Using DAW

**Any DAW (Ableton, Logic, Reaper, etc.):**
```
1. Create MIDI track
2. Set MIDI output to "MidiCore 4x4"
3. Draw notes or use MIDI controller
4. Watch CDC terminal for RX messages
```

#### Expected Terminal Output

**Note On/Off:**
```
[RX] Cable:0 90 3C 64 (Note On Ch:1 Note:60 Vel:100)
[RX] Cable:0 80 3C 00 (Note Off Ch:1 Note:60 Vel:0)
```

**Control Changes:**
```
[RX] Cable:0 B0 07 64 (CC Ch:1 CC:7 Val:100)
[RX] Cable:0 B0 0A 40 (CC Ch:1 CC:10 Val:64)
```

**Program Changes:**
```
[RX] Cable:0 C0 05 00 (Program Change Ch:1 Prog:5)
```

**Pitch Bend:**
```
[RX] Cable:0 E0 00 40 (Pitch Bend Ch:1 Bend:8192)
```

**SysEx (special format):**
```
[RX SysEx] Cable:0 CIN:0x4 Data: F0 7E 7F
[RX SysEx] Cable:0 CIN:0x7 Data: 06 01 F7
```

---

## Test Scenarios

### Basic Functionality Test

**Goal:** Verify both RX and TX work

1. Flash firmware with `MODULE_TEST_USB_DEVICE_MIDI=1`
2. Open CDC terminal
3. Verify TX messages appear every 2 seconds
4. Send notes from MIDI keyboard/DAW
5. Verify RX messages appear in terminal

**Success criteria:**
- ✅ TX messages appear in terminal
- ✅ TX messages received by MIDI monitor
- ✅ RX messages appear when sending from DAW
- ✅ All messages properly formatted

### Latency Test

**Goal:** Measure round-trip latency

1. Use MIDI feedback loop tool (MIDI-OX "MIDI Thru")
2. Send Note On from computer
3. Measure time until echo received back
4. Expected: <10ms round-trip latency

### Stress Test

**Goal:** Verify stability under heavy load

1. Send rapid MIDI messages (arpeggios, drum rolls)
2. Monitor for dropped messages
3. Check terminal for overflow/error messages

**Success criteria:**
- ✅ All messages received
- ✅ No "Queue FULL" errors
- ✅ No crashes or hangs

### Multi-Cable Test

**Goal:** Verify all 4 USB MIDI ports work

MidiCore exposes 4 virtual cables (ports):
- Cable 0: "MidiCore 4x4" (main port)
- Cable 1: "MIDIOUT2 (MidiCore 4x4)"
- Cable 2: "MIDIOUT3 (MidiCore 4x4)"
- Cable 3: "MIDIOUT4 (MidiCore 4x4)"

Test each port independently by selecting different MIDI ports in your DAW.

---

## Automated Testing with Python

### Test Script

```python
#!/usr/bin/env python3
"""
USB MIDI RX/TX Test Script
Requires: python-rtmidi, pyserial
"""

import rtmidi
import serial
import time
import sys

def find_midicore_ports():
    """Find MidiCore MIDI ports"""
    midi_out = rtmidi.MidiOut()
    midi_in = rtmidi.MidiIn()
    
    out_ports = midi_out.get_ports()
    in_ports = midi_in.get_ports()
    
    # Find MidiCore ports
    midicore_out = None
    midicore_in = None
    
    for i, port in enumerate(out_ports):
        if 'MidiCore' in port:
            midicore_out = i
            break
    
    for i, port in enumerate(in_ports):
        if 'MidiCore' in port:
            midicore_in = i
            break
    
    return midicore_out, midicore_in

def test_rx():
    """Test MIDI RX (send to device)"""
    print("Testing USB MIDI RX (Computer → Device)...")
    
    midi_out_port, _ = find_midicore_ports()
    if midi_out_port is None:
        print("ERROR: MidiCore MIDI OUT port not found!")
        return False
    
    midi_out = rtmidi.MidiOut()
    midi_out.open_port(midi_out_port)
    
    print("Sending test messages to MidiCore...")
    print("Check CDC terminal for RX output!")
    
    # Send Note On
    midi_out.send_message([0x90, 60, 100])
    print("  Sent: Note On (90 3C 64)")
    time.sleep(0.5)
    
    # Send Note Off
    midi_out.send_message([0x80, 60, 0])
    print("  Sent: Note Off (80 3C 00)")
    time.sleep(0.5)
    
    # Send CC
    midi_out.send_message([0xB0, 7, 100])
    print("  Sent: CC (B0 07 64)")
    
    midi_out.close_port()
    print("✓ RX test complete. Check terminal for messages.")
    return True

def test_tx():
    """Test MIDI TX (receive from device)"""
    print("\nTesting USB MIDI TX (Device → Computer)...")
    
    _, midi_in_port = find_midicore_ports()
    if midi_in_port is None:
        print("ERROR: MidiCore MIDI IN port not found!")
        return False
    
    midi_in = rtmidi.MidiIn()
    midi_in.open_port(midi_in_port)
    
    print("Listening for messages from MidiCore...")
    print("Waiting up to 5 seconds...")
    
    received = []
    start_time = time.time()
    
    while time.time() - start_time < 5:
        msg = midi_in.get_message()
        if msg:
            data, timestamp = msg
            received.append(data)
            print(f"  Received: {' '.join(f'{b:02X}' for b in data)}")
        time.sleep(0.01)
    
    midi_in.close_port()
    
    if received:
        print(f"✓ TX test complete. Received {len(received)} messages.")
        return True
    else:
        print("✗ TX test failed. No messages received.")
        return False

def main():
    print("=" * 60)
    print("MidiCore USB MIDI RX/TX Test")
    print("=" * 60)
    
    rx_ok = test_rx()
    tx_ok = test_tx()
    
    print("\n" + "=" * 60)
    print("Test Results:")
    print(f"  RX (Computer → Device): {'✓ PASS' if rx_ok else '✗ FAIL'}")
    print(f"  TX (Device → Computer): {'✓ PASS' if tx_ok else '✗ FAIL'}")
    print("=" * 60)
    
    if rx_ok and tx_ok:
        print("✓ All tests PASSED!")
        return 0
    else:
        print("✗ Some tests FAILED!")
        return 1

if __name__ == '__main__':
    sys.exit(main())
```

### Running the Test

```bash
pip install python-rtmidi pyserial
python3 test_usb_midi_rx_tx.py
```

---

## Troubleshooting

### No RX Output in Terminal

**Problem:** Sending MIDI from DAW but no [RX] messages appear

**Solutions:**
1. **Check MIDI routing in DAW**
   - Ensure MIDI output is set to "MidiCore 4x4"
   - Verify MIDI track is armed/enabled
   
2. **Verify USB enumeration**
   ```bash
   # Windows Device Manager
   # Should see "MidiCore 4x4" under Sound, video and game controllers
   
   # Linux
   lsusb | grep 16C0:0489
   # Should show VOTI device
   
   amidi -l
   # Should list MidiCore ports
   ```

3. **Check firmware initialization**
   - Verify `usb_midi_init()` called before `MX_USB_DEVICE_Init()`
   - See INITIALIZATION_ORDER_BUG_FIX.md

4. **Check debug hook is active**
   - `MODULE_TEST_USB_DEVICE_MIDI` must be defined
   - `usb_midi_rx_debug_hook()` in module_tests.c should be compiled

### No TX Output in Terminal

**Problem:** No [TX] messages appearing

**Solutions:**
1. **Check CDC terminal connection**
   - Terminal must be connected to correct COM port
   - Baud rate doesn't matter (CDC handles it)

2. **Check test is running**
   - Should see "USB MIDI Device Test" header
   - Should see "Test started" message

3. **Check TX path**
   - Look for error messages like "[TX-DBG] ERROR: ..."
   - If you see "Queue FULL", reduce message rate

### Messages Not Received by DAW

**Problem:** Terminal shows [TX] but DAW doesn't receive

**Solutions:**
1. **Check DAW MIDI input**
   - Select "MidiCore 4x4" as MIDI input device
   - Enable MIDI input monitoring on track

2. **Check MIDI driver**
   - Windows: Reinstall USB driver
   - Linux: Check permissions (`groups | grep audio`)

3. **Try MIDI Monitor**
   - Use dedicated MIDI monitor app (MIDI-OX, MIDI Monitor)
   - Eliminates DAW configuration issues

### Garbled/Fragmented Messages

**Problem:** Messages appear but are incomplete or corrupted

**Solution:** This was fixed in commit 9d91da9. Debug messages from interrupt context now buffer complete messages before sending to CDC. If still seeing issues, verify you have the latest code.

### High Latency

**Problem:** Messages delayed >50ms

**Solutions:**
1. **Check USB polling interval** - Should be 1ms (HS)
2. **Check FreeRTOS task priorities** - MIDI task should be high priority
3. **Disable USB suspend** - Some systems suspend inactive USB devices

---

## Configuration Options

### Test Parameters

In `app_test_usb_midi.c`:

```c
#define APP_TEST_USB_MIDI_SEND_INTERVAL 2000  // ms between test messages
#define APP_TEST_USB_MIDI_BASE_NOTE 60        // Middle C
#define APP_TEST_USB_MIDI_CHANNEL 0           // Channel 1 (0-based)
#define APP_TEST_USB_MIDI_VELOCITY 100        // Velocity
#define APP_TEST_USB_MIDI_CABLE 0             // USB Port 1
```

### Module Enable Flags

In `Config/module_config.h`:

```c
#define MODULE_ENABLE_USB_MIDI 1       // Enable USB MIDI
#define MODULE_ENABLE_USB_CDC 1        // Enable USB CDC (terminal)
#define MODULE_TEST_USB_DEVICE_MIDI 1  // Enable test mode
```

---

## Expected Behavior Summary

### Normal Operation

✅ **TX working:** Terminal shows [TX] messages every 2s, MIDI monitor receives them  
✅ **RX working:** Sending from DAW shows [RX] messages in terminal immediately  
✅ **Both working:** Bi-directional MIDI communication confirmed  
✅ **Clean output:** Messages complete and properly formatted  
✅ **No errors:** No "Queue FULL" or "Endpoint BUSY" messages  

### What's Normal

- "Queue empty" debug message (now suppressed) is normal - queue processed fast
- Short delays (<10ms) between send and receive are normal
- USB re-enumeration when flashing firmware is normal

### What's Not Normal

- No RX output when sending from DAW
- TX messages in terminal but not received by MIDI monitor
- "Queue FULL" errors (indicates overflow)
- Garbled/incomplete messages
- Crashes or hangs when sending MIDI

---

## Next Steps

After confirming RX/TX work:

1. **Test with MIOS Studio** - Should auto-recognize device
2. **Test MIOS32 queries** - See test_mios32_recognition.py
3. **Test CDC terminal** - See test_cdc_terminal.py
4. **Integrate with MIDI router** - Route USB MIDI to DIN/looper
5. **Test with real application** - Disable test mode, use production code

---

## References

- **USB MIDI 1.0 Spec:** https://www.usb.org/sites/default/files/midi10.pdf
- **MIOS32 USB MIDI:** http://www.ucapps.de/mios32_c.html#usb_midi
- **Python MIDI tools:** https://github.com/SpotlightKid/python-rtmidi

---

**Status:** Complete USB MIDI RX/TX implementation with full debug instrumentation
**Last Updated:** 2026-01-28
