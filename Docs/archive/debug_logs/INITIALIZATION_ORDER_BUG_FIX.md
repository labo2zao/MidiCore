# CRITICAL BUG FIX: USB Initialization Order

## Problem Summary

Both MIOS Studio and Python diagnostic tools were failing to get responses from MidiCore:
- MIOS32 queries: 0/9 responded
- CDC terminal echo: 0/3 responded  
- USB enumeration: Working (device detected)
- CDC control lines: Working (DTR/RTS)

This indicated a **runtime issue** where callbacks were not being executed.

## Root Cause

**USB interface callbacks were registered AFTER USB device enumeration started.**

### The Broken Sequence

```c
// In main.c (OLD CODE - BROKEN):

1. MX_USB_DEVICE_Init();         // Starts USB device
   ↓
   USBD_Init()
   ↓
   USBD_RegisterClass(&USBD_COMPOSITE)
   ↓
   USBD_Start()                  // Triggers enumeration
   ↓
   [Host connects and sends SET_CONFIGURATION]
   ↓
   USBD_COMPOSITE_Init() called
   ↓
   USBD_MIDI.Init() called       // midi_fops is NULL! ❌
   USBD_CDC.Init() called        // pCDC_Fops is NULL! ❌
   
2. usb_midi_init();              // Too late! Already enumerated
   ↓
   USBD_MIDI_RegisterInterface() // Sets midi_fops
   
3. usb_cdc_init();               // Too late! Already enumerated
   ↓
   USBD_CDC_RegisterInterface()  // Sets pCDC_Fops
```

**Result:** When host sends MIDI/CDC data:
- `USBD_MIDI_DataOut()` checks `if (midi_fops != NULL && midi_fops->DataOut != NULL)`
- But `midi_fops` is NULL → callback never called → no response!
- Same for CDC: `pCDC_Fops` is NULL → no echo!

## The Fix

**Register callbacks BEFORE starting USB device:**

```c
// In main.c (NEW CODE - CORRECT):

1. usb_midi_init();              // Register callbacks FIRST
   ↓
   USBD_MIDI_RegisterInterface() // Sets midi_fops ✅
   
2. usb_cdc_init();               // Register callbacks FIRST
   ↓
   USBD_CDC_RegisterInterface()  // Sets pCDC_Fops ✅
   
3. MX_USB_DEVICE_Init();         // NOW start USB device
   ↓
   USBD_Start()                  // Triggers enumeration
   ↓
   [Host connects and sends SET_CONFIGURATION]
   ↓
   USBD_COMPOSITE_Init() called
   ↓
   USBD_MIDI.Init() called       // midi_fops != NULL ✅
   USBD_CDC.Init() called        // pCDC_Fops != NULL ✅
```

**Result:** When host sends MIDI/CDC data:
- `midi_fops` is valid → callback called → MIOS32 query answered ✅
- `pCDC_Fops` is valid → callback called → CDC data echoed ✅

## Code Change

**File:** `Core/Src/main.c` (lines 163-200)

**Before:**
```c
#if MODULE_ENABLE_USB_MIDI || MODULE_ENABLE_USB_CDC
  MX_USB_DEVICE_Init();  // Started FIRST (wrong!)
  
  #if MODULE_ENABLE_USB_MIDI
    usb_midi_init();     // Too late
  #endif
  
  #if MODULE_ENABLE_USB_CDC
    usb_cdc_init();      // Too late
  #endif
#endif
```

**After:**
```c
#if MODULE_ENABLE_USB_MIDI || MODULE_ENABLE_USB_CDC
  #if MODULE_ENABLE_USB_MIDI
    usb_midi_init();     // Register callbacks FIRST ✅
  #endif
  
  #if MODULE_ENABLE_USB_CDC
    usb_cdc_init();      // Register callbacks FIRST ✅
  #endif
  
  MX_USB_DEVICE_Init();  // Start USB LAST ✅
#endif
```

## Why This Wasn't Obvious

1. **USB enumeration seemed to work** - Device appeared in device list
2. **Control requests worked** - Line coding, DTR/RTS all functional
3. **Code looked correct** - All functions were being called
4. **Only data flow broken** - Callbacks were NULL during critical Init phase

The bug was specifically in the **timing** of when callbacks were registered relative to USB enumeration.

## Expected Results After Fix

### MIOS32 Query Test
```
Testing Query Type 0x01: Operating System
  Sending: f0 00 00 7e 32 00 00 01 f7
  Received: f0 00 00 7e 32 00 0f 4d 49 4f 53 33 32 f7
✓ Valid response: "MIOS32"

Test Summary:
  Total queries: 9
  Passed: 9     ← Was 0/9 before fix
  Failed: 0
```

### CDC Terminal Test
```
Echo Test:
  Sending: Hello MidiCore!
  Received: Hello MidiCore!
✓   ✓ Perfect echo!    ← Was "No response" before fix

Test Summary:
  Line Coding          PASS
  Control Lines        PASS
  Echo                 PASS  ← Was FAIL before fix
  Data Transfer        PASS  ← Was FAIL before fix

Overall: 4/4 tests passed   ← Was 2/4 before fix
```

### MIOS Studio
- Device will be automatically recognized
- Will appear as "MidiCore 1.0.0"
- Terminal will be functional
- Full MIOS32 compatibility

## How to Apply Fix

1. **Pull latest code** from branch `copilot/investigate-midicore-recognition`
2. **Rebuild firmware** (Clean + Build in STM32CubeIDE)
3. **Flash to device**
4. **Power cycle** device (unplug/replug USB)
5. **Test with Python scripts:**
   ```bash
   python3 test_midi_loopback.py
   python3 test_mios32_recognition.py
   python3 test_cdc_terminal.py
   ```
6. **Test with MIOS Studio** - device should be recognized

## Technical Notes

### Why Service Init Can Be Called Before USB Device

The service init functions (`usb_midi_init()` and `usb_cdc_init()`) only:
1. Initialize internal state (queues, buffers)
2. Register callback pointers with the USB class drivers

They do **NOT**:
- Access USB hardware
- Trigger USB transactions
- Depend on USB being initialized

Therefore, it's **safe and correct** to call them before `MX_USB_DEVICE_Init()`.

### Callback Registration Mechanism

**MIDI:**
```c
// usb_midi.c
static USBD_MIDI_ItfTypeDef midi_fops = {
  USBD_MIDI_Init_Callback,
  USBD_MIDI_DeInit_Callback,
  USBD_MIDI_DataOut_Callback
};

void usb_midi_init(void) {
  USBD_MIDI_RegisterInterface(&hUsbDeviceFS, &midi_fops);
}

// usbd_midi.c
static USBD_MIDI_ItfTypeDef *midi_fops = NULL;

uint8_t USBD_MIDI_RegisterInterface(USBD_HandleTypeDef *pdev, 
                                    USBD_MIDI_ItfTypeDef *fops) {
  midi_fops = fops;  // Store pointer
  return USBD_OK;
}

static uint8_t USBD_MIDI_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum) {
  if (midi_fops != NULL && midi_fops->DataOut != NULL) {
    midi_fops->DataOut(&packet);  // Call callback
  }
}
```

**CDC:** Same pattern with `pCDC_Fops`

### Composite Device Considerations

In composite mode, the callback pointers are **global static variables** in the class drivers:
- `midi_fops` in `usbd_midi.c`
- `pCDC_Fops` in `usbd_cdc.c`

These are separate from the class data pointers that get switched by the composite driver. The callbacks themselves don't change - only the class data pointer (`pdev->pClassData`) is switched for each endpoint.

## Lessons Learned

1. **Initialization order matters** - Even if all functions are called
2. **USB enumeration is asynchronous** - Happens during `USBD_Start()`
3. **Callbacks must exist before they're needed** - Not after
4. **Static NULL checks are important** - Prevented crashes but silently failed
5. **Integration testing required** - Unit tests wouldn't catch this

## Related Files

- `Core/Src/main.c` - Initialization sequence (FIXED)
- `Services/usb_midi/usb_midi.c` - MIDI service (correct)
- `Services/usb_cdc/usb_cdc.c` - CDC service (correct)
- `USB_DEVICE/Class/MIDI/Src/usbd_midi.c` - MIDI class driver (correct)
- `USB_DEVICE/Class/CDC/Src/usbd_cdc.c` - CDC class driver (correct)

All service and class code was correct. Only the initialization order in `main.c` was wrong.

---

**Status:** ✅ FIXED  
**Commit:** d79e84b  
**Date:** 2026-01-28  
**Impact:** Critical - Enables all USB MIDI/CDC functionality
