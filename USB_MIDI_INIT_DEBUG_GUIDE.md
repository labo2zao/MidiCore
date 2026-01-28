# USB MIDI Initialization Debug Guide

## Problem Statement

The condition `(USBD_MIDI.DataOut != NULL && composite_class_data.midi_class_data != NULL)` is evaluating to false, preventing USB MIDI from working.

## Debug Changes Added

### Location: `USB_DEVICE/App/usbd_composite.c`

#### 1. Composite Init Debug (lines 124-154)

Added debug output during `USBD_COMPOSITE_Init()` to trace:
- When MIDI+CDC initialization starts
- When `USBD_MIDI.Init()` is called
- If `USBD_MIDI.Init()` fails
- The value of `composite_class_data.midi_class_data` after init
- Warning if `USBD_MIDI.Init` is NULL

**Expected Output:**
```
[COMP-Init] Starting MIDI+CDC init
[COMP-Init] Calling USBD_MIDI.Init()
[COMP-Init] MIDI class_data = 0x20001234
```

#### 2. DataOut Check Debug (lines 298-330)

Added debug output when USB MIDI data is received to show:
- The values of both pointers being checked
- Whether the check passes (MIDI_OK) or fails (MIDI_SKIP)

**Expected Output if OK:**
```
[COMP-RX] EP:01 MIDI.DataOut=0x08001234 midi_data=0x20001234
[COMP-RX] EP:01 MIDI_OK
[COMP] Calling MIDI.DataOut
[COMP] MIDI.DataOut returned
```

**Expected Output if FAILED:**
```
[COMP-RX] EP:01 MIDI.DataOut=0x08001234 midi_data=0x00000000
[COMP-RX] EP:01 MIDI_SKIP (DataOut:0x08001234 data:0x00000000)
```

## How to Use These Debug Messages

### Step 1: Check Initialization

When the device boots, you should see:
```
[COMP-Init] Starting MIDI+CDC init
[COMP-Init] Calling USBD_MIDI.Init()
[COMP-Init] MIDI class_data = 0x20001234
```

**If you see:**
- `[COMP-Init] WARNING: USBD_MIDI.Init is NULL!` → USBD_MIDI struct is not properly linked
- `[COMP-Init] ERROR: USBD_MIDI.Init() FAILED!` → MIDI init failed, check MIDI class implementation
- No `[COMP-Init]` messages at all → USB enumeration never happened or debug not enabled

### Step 2: Check Data Reception

When MIDI data is sent from host, you should see:
```
[COMP-RX] EP:01 MIDI.DataOut=0x08001234 midi_data=0x20001234
[COMP-RX] EP:01 MIDI_OK
```

**If you see MIDI_SKIP:**
- Check which pointer is NULL (0x00000000)
- If `MIDI.DataOut` is NULL → USBD_MIDI class callbacks not properly set
- If `midi_data` is NULL → Composite init didn't save the MIDI class data pointer

### Step 3: Diagnose the Issue

**Scenario A: `USBD_MIDI.Init is NULL`**
- **Cause:** The USBD_MIDI struct is not properly defined or linked
- **Check:** `USB_DEVICE/Class/MIDI/Src/usbd_midi.c` line 122-138
- **Verify:** `USBD_MIDI` is declared as `USBD_ClassTypeDef` with callbacks

**Scenario B: `USBD_MIDI.Init() FAILED`**
- **Cause:** MIDI class initialization returned error
- **Check:** Look at USBD_MIDI_Init() implementation
- **Common issues:** 
  - Endpoint allocation failed
  - Memory allocation failed
  - HAL configuration issue

**Scenario C: `midi_data` is NULL after init**
- **Cause:** `pdev->pClassData` was not set by USBD_MIDI_Init()
- **Check:** USBD_MIDI_Init() should allocate and set pClassData
- **Verify:** Line 133 copies pdev->pClassData to composite_class_data.midi_class_data

**Scenario D: `midi_data` is NULL during DataOut but was set during init**
- **Cause:** Memory corruption or composite_class_data was cleared
- **Check:** Look for code that might clear or overwrite composite_class_data
- **Verify:** composite_class_data is static and should persist

## Initialization Order (Correct)

According to the repository memory and main.c:

```c
// 1. Register MIDI interface callbacks FIRST
usb_midi_init();  // Calls USBD_MIDI_RegisterInterface()

// 2. Register CDC interface callbacks FIRST (if enabled)
usb_cdc_init();   // Calls USBD_CDC_RegisterInterface()

// 3. NOW start USB device with callbacks ready
MX_USB_DEVICE_Init();
  └─> USBD_Init()
      └─> USBD_RegisterClass(&USBD_COMPOSITE)
          └─> USB Enumeration starts
              └─> USBD_COMPOSITE_Init()  ← This is where we set midi_class_data
                  ├─> USBD_MIDI.Init()
                  │   └─> Sets pdev->pClassData
                  └─> Saves to composite_class_data.midi_class_data
```

## Required Compiler Define

For all debug messages to appear:
```c
#define MODULE_TEST_USB_DEVICE_MIDI 1
```

Set in CubeIDE: Project Properties → Preprocessor → Defined symbols

## Breakpoint Locations

For debugging initialization:
- `USB_DEVICE/App/usbd_composite.c:127` - Before MIDI init
- `USB_DEVICE/App/usbd_composite.c:133` - After MIDI init, check midi_class_data

For debugging data reception:
- `USB_DEVICE/App/usbd_composite.c:298` - Start of MIDI DataOut check
- `USB_DEVICE/App/usbd_composite.c:306` - Inside successful path

## Common Fixes

### Fix 1: Initialization Order
If initialization happens in wrong order, move `usb_midi_init()` BEFORE `MX_USB_DEVICE_Init()` in main.c.

### Fix 2: Missing Interface Registration
If `midi_fops` is NULL, ensure `usb_midi_init()` calls:
```c
USBD_MIDI_RegisterInterface(&hUsbDeviceFS, &midi_fops);
```

### Fix 3: Class Data Not Saved
If `composite_class_data.midi_class_data` is NULL after init, check that:
```c
// In USBD_COMPOSITE_Init() after USBD_MIDI.Init():
composite_class_data.midi_class_data = pdev->pClassData;
```

## Next Steps After Getting Debug Output

1. Save the complete debug output from boot to MIDI reception attempt
2. Identify which scenario matches your output
3. Follow the specific fix for that scenario
4. If issue persists, check for:
   - Multiple USB device instances
   - Race conditions during enumeration
   - Memory corruption
   - Incorrect linker configuration
