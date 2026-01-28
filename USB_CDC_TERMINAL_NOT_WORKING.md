# USB CDC Terminal Not Showing Output - Diagnostic Guide

## Problem
MIOS Studio recognizes the device (USB MIDI works), but TeraTerm shows nothing (USB CDC terminal issue).

## Quick Diagnosis

### Step 1: Find the Correct COM Port

When you connect MidiCore to Windows, it creates **TWO COM ports**:

#### Windows Device Manager (Win+X → Device Manager)
```
Ports (COM & LPT)
  ├─ USB Serial Device (COM3)  ← MIOS Studio uses this
  └─ USB Serial Device (COM4)  ← TeraTerm connects here
```

**Problem:** You might be connected to the wrong COM port!

#### How to Identify
1. **Disconnect** MidiCore from USB
2. **Note** which COM ports exist
3. **Reconnect** MidiCore
4. **Check** which NEW COM ports appeared (usually 2)
5. **Try BOTH** ports in TeraTerm

### Step 2: TeraTerm Settings

**Correct Settings:**
```
Setup → Serial Port:
  Port: COM4 (or whichever is the CDC port)
  Speed: 115200 (doesn't matter for USB CDC, but use this)
  Data: 8 bit
  Parity: None
  Stop bits: 1
  Flow control: None
```

**Important:** USB CDC (Virtual COM) doesn't use actual baud rates, but TeraTerm requires a setting.

### Step 3: Check if Debug Output is Enabled

The firmware needs `MODULE_ENABLE_USB_CDC=1` defined at compile time.

#### Check in Your Build
Look in your IDE (STM32CubeIDE, Keil, etc.):

**Project Properties → C/C++ Build → Settings → Tool Settings**
- MCU GCC Compiler → Preprocessor
- Look for: `MODULE_ENABLE_USB_CDC=1`

**Or check module_config.h:**
```c
#define MODULE_ENABLE_USB_CDC 1
```

### Step 4: Test CDC Manually

#### Linux/Mac Test
```bash
# Find the CDC device
ls -la /dev/ttyACM*

# Connect and test
screen /dev/ttyACM0 115200
# Or
minicom -D /dev/ttyACM0

# Should see MIDI messages when you send MIDI data
```

#### Windows Test
1. Open TeraTerm
2. File → New Connection → Serial → Select COM port
3. Try EACH COM port that appeared
4. Send MIDI notes from MIOS Studio
5. Should see `[RX]` and `[TX]` messages

## Common Issues

### Issue 1: No Output at All

**Symptoms:** TeraTerm connected, but nothing appears even when sending MIDI

**Possible Causes:**
1. **Wrong COM port** → Try the OTHER COM port
2. **Debug disabled** → MODULE_ENABLE_USB_CDC=0 or not defined
3. **CDC not initialized** → Check main.c calls usb_cdc_init()

**Solution:**
```
1. Try BOTH COM ports
2. Check build defines
3. Verify initialization order
```

### Issue 2: Garbled Output

**Symptoms:** Characters appear but are unreadable

**Cause:** Wrong baud rate (shouldn't matter for USB CDC, but...)

**Solution:**
```
Set TeraTerm to 115200 baud
Setup → Serial Port → Speed: 115200
```

### Issue 3: TeraTerm Can't Open Port

**Symptoms:** "Cannot open COM port" error

**Causes:**
1. **Another app using it** - MIOS Studio might be using CDC port
2. **Driver issue** - USB CDC driver not installed
3. **Permissions** - User doesn't have COM port access

**Solution:**
```
1. Close MIOS Studio if it has CDC terminal open
2. Try different COM port
3. Check Device Manager for driver issues
```

### Issue 4: Port Opens but Immediate Disconnect

**Symptoms:** Port opens then closes immediately

**Cause:** USB enumeration issue or power problem

**Solution:**
```
1. Try different USB cable
2. Try different USB port (use USB 2.0, not 3.0)
3. Check if device re-enumerates
```

## Debug Steps

### Verify USB CDC is Working

#### Method 1: Check Device Manager
```
Windows Device Manager:
1. Expand "Ports (COM & LPT)"
2. Should see TWO "USB Serial Device" entries
3. Note the COM port numbers
4. Right-click → Properties → Details → Hardware IDs
   Should show: USB\VID_16C0&PID_0489&MI_02 (CDC interface)
```

#### Method 2: Check with MIOS Studio
```
MIOS Studio can show debug output too:
1. Open MIOS Studio
2. Connect to device
3. Open "Terminal" window
4. Should see debug messages there
```

#### Method 3: Add Test Output
If you can rebuild firmware, add this to main.c after initialization:

```c
// Test CDC output
for (int i = 0; i < 10; i++) {
  dbg_print("CDC Test Message ");
  dbg_print_uint(i);
  dbg_print("\r\n");
  HAL_Delay(1000);
}
```

This will send messages every second for 10 seconds after boot.

## Most Likely Causes (Ranked)

### 1. Wrong COM Port (80% probability)
**Symptom:** TeraTerm opens fine, just no data
**Fix:** Try the OTHER COM port that appeared when you plugged in device

### 2. Debug Output Disabled (15% probability)  
**Symptom:** TeraTerm works, but no messages
**Fix:** Ensure `MODULE_ENABLE_USB_CDC=1` and `MODULE_TEST_USB_DEVICE_MIDI=1`

### 3. Another App Using Port (4% probability)
**Symptom:** Can't open port or immediate disconnect
**Fix:** Close MIOS Studio terminal, try again

### 4. USB Cable/Port Issue (1% probability)
**Symptom:** Device works in MIOS Studio but CDC unreliable
**Fix:** Try different cable or USB port

## Quick Test Script

### Windows PowerShell
```powershell
# List all COM ports
Get-WmiObject Win32_SerialPort | Select-Object DeviceID, Description

# Or simpler:
mode
```

### Linux
```bash
# Show USB CDC devices
ls -la /dev/ttyACM*
dmesg | tail -20  # Shows recent USB events
```

## Expected Output

When CDC is working correctly, you should see:

```
[RX] Cable:0 90 3C 64 (Note On Ch:1 Note:60 Vel:100)
[RX] Cable:0 80 3C 00 (Note Off Ch:1 Note:60)
[TX] Cable:0 90 3C 64 (Note On)
```

And when MIOS Studio connects:
```
[MIOS32-Q] Received query len:9 cable:0
[MIOS32-Q] dev_id:00 cmd:00 type:01
[MIOS32-R] Sending type:01 "MIOS32" cable:0
[MIOS32-R] Sent 15 bytes
```

## Action Plan

1. **Reconnect TeraTerm**
   - Try BOTH COM ports that appear
   - Use 115200 baud, 8N1, no flow control

2. **Send MIDI Test**
   - Use MIOS Studio or DAW to send MIDI notes
   - Should see `[RX]` messages immediately

3. **If Still Nothing**
   - Check Device Manager for COM port numbers
   - Verify both ports exist
   - Try closing MIOS Studio first
   - Check build configuration for MODULE_ENABLE_USB_CDC

4. **Report Back**
   - Which COM ports do you see?
   - Which one are you connecting to?
   - Do you see ANYTHING (even garbage)?
   - Does Device Manager show any errors?

## Files to Check

If rebuilding:
- `Config/module_config.h` - Check MODULE_ENABLE_USB_CDC
- `Core/Src/main.c` - Verify usb_cdc_init() is called
- Build configuration - Check preprocessor defines

The most likely issue is connecting to the wrong COM port. Try the OTHER port!
