# USB MIDI Debug Guide

## Problem

Windows error 0xC00000E5 (CONFIGURATION_DESCRIPTOR_VALIDATION_FAILURE) persists despite multiple descriptor fixes. We need to debug what's actually happening during USB enumeration.

## Debug Strategy

Since descriptor fixes haven't worked, we need to:
1. Capture what Windows is requesting during enumeration
2. See what data is actually being sent
3. Identify where the validation failure occurs

## Method 1: USB Analyzer Hardware

**Best option if available:**
- Use a USB protocol analyzer (e.g., Beagle USB 480, Total Phase)
- Capture the enumeration sequence
- Compare with working MIOS32 capture
- See exact point of failure

## Method 2: Windows USB Log

**Enable Windows USB tracing:**

```cmd
# As Administrator
logman create trace usbtrace -o usbtrace.etl -nb 128 640 -bs 128
logman update usbtrace -p Microsoft-Windows-USB-USBPORT
logman update usbtrace -p Microsoft-Windows-USB-USBHUB
logman start usbtrace

# Plug in device, reproduce error

logman stop usbtrace
```

View the trace:
- Use Microsoft Message Analyzer or Wireshark with USB plugin
- Look for descriptor GET requests and responses
- Check for validation errors

## Method 3: STM32 Debug Output

**Enable debug instrumentation in firmware:**

### Option A: ITM/SWO Trace

1. Enable ITM in your debugger (ST-Link Utility or OpenOCD)
2. Add debug defines to your build:
   ```c
   // In your IDE or Makefile
   -DUSBD_MIDI_DEBUG
   ```

3. Use the provided debug functions:
   ```c
   #include "USB_DEVICE/Class/MIDI/Inc/usbd_midi_debug.h"
   
   // In USBD_MIDI_Setup()
   DEBUG_SETUP(req->bmRequest, req->bRequest, req->wValue, req->wIndex, req->wLength);
   
   // When returning descriptor
   DEBUG_DESCRIPTOR("Config", USBD_MIDI_CfgDesc, *length);
   ```

4. View output in ST-Link Utility SWV console

### Option B: UART Debug

If ITM not available, add UART output:

```c
// In USBD_MIDI_Setup()
printf("USB Setup: bmRequest=0x%02X bRequest=0x%02X wValue=0x%04X\r\n",
       req->bmRequest, req->bRequest, req->wValue);

// When GET_DESCRIPTOR
if (req->bRequest == USB_REQ_GET_DESCRIPTOR) {
    uint8_t desc_type = (req->wValue >> 8) & 0xFF;
    uint8_t desc_index = req->wValue & 0xFF;
    printf("GET_DESCRIPTOR: type=%u index=%u length=%u\r\n",
           desc_type, desc_index, req->wLength);
}
```

## Method 4: Simplify Descriptor

**Create minimal test descriptor:**

Start with 1-port MIDI interface instead of 4-port:
1. Set `MIDI_NUM_PORTS = 1` 
2. Rebuild and test
3. If it works, gradually add ports

This isolates whether the issue is:
- Fundamental descriptor structure problem
- Size/complexity problem with 4 ports

## Method 5: Compare with Known Working Device

**Use USBTreeView or USBDeview:**

1. Install MIOS32 firmware (which works)
2. Capture descriptor with USBTreeView
3. Save the exact byte values
4. Compare byte-by-byte with MidiCore descriptor

Look for:
- Any field value differences
- Order of descriptors
- Padding or alignment issues

## What to Look For

### Common Issues:

1. **Descriptor order wrong**
   - IAD must be before first interface it describes
   - Jacks must be in correct order

2. **Jack ID mismatch**
   - Jack IDs must be sequential 1-16 for 4 ports
   - baSourceID must reference valid jack IDs

3. **Endpoint address mismatch**
   - Descriptor says 0x01 (OUT) and 0x81 (IN)
   - But Init function configures different addresses

4. **Size mismatch in nested structures**
   - AC Header wTotalLength
   - MS Header wTotalLength
   - Config wTotalLength
   - All must match actual bytes

5. **String descriptor failure**
   - Serial number callback returns NULL
   - Windows can't enumerate without serial

## Next Steps

Based on debug output:

### If GET_DESCRIPTOR never arrives:
- Device detection problem
- VBUS sensing issue
- Clock configuration problem

### If GET_DESCRIPTOR arrives but fails:
- Capture the exact wValue, wIndex, wLength
- Check what descriptor type Windows wants
- Verify we're returning correct descriptor

### If CONFIGURATION descriptor is rejected:
- Dump the exact bytes being sent
- Compare size field vs actual data
- Check for any invalid field values

### If DEVICE descriptor is OK but CONFIG fails:
- The issue is definitely in configuration descriptor
- Use hex dump to find exact byte causing failure

## Files for Debug Instrumentation

- **usbd_midi_debug.h**: Debug macros and function declarations
- **usbd_midi_debug.c**: Debug implementation (ITM output)

Enable with: `-DUSBD_MIDI_DEBUG` in build flags

## Success Criteria

Debug is successful when you can answer:
1. What descriptor request causes the failure?
2. What exact bytes are being sent?
3. Which byte or field does Windows reject?
4. How does it differ from working MIOS32?

Once you know the EXACT problem, the fix becomes obvious.

---

**Current Status**: Multiple descriptor fixes attempted, all unsuccessful. Debug instrumentation needed to identify actual root cause.
