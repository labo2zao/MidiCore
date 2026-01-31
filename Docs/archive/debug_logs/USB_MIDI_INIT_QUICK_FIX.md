# USB MIDI Init Issue - Quick Reference

## Problem
`(USBD_MIDI.DataOut != NULL && composite_class_data.midi_class_data != NULL)` is FALSE

## What the Debug Output Will Show

### Scenario 1: midi_data is NULL
```
[COMP-Init] MIDI class_data = 0x00000000  ← NULL!
[COMP-RX] EP:01 MIDI.DataOut=0x08001234 midi_data=0x00000000
[COMP-RX] EP:01 MIDI_SKIP (DataOut:0x08001234 data:0x00000000)
```
**Cause:** `USBD_MIDI_Init()` didn't set `pdev->pClassData`  
**Fix:** Check USBD_MIDI_Init() allocates class data properly

### Scenario 2: MIDI.DataOut is NULL
```
[COMP-Init] MIDI class_data = 0x20001234
[COMP-RX] EP:01 MIDI.DataOut=0x00000000 midi_data=0x20001234
[COMP-RX] EP:01 MIDI_SKIP (DataOut:0x00000000 data:0x20001234)
```
**Cause:** `USBD_MIDI` struct not properly initialized  
**Fix:** Check USBD_MIDI callbacks are set at compile time

### Scenario 3: Both NULL
```
[COMP-Init] MIDI class_data = 0x00000000
[COMP-RX] EP:01 MIDI.DataOut=0x00000000 midi_data=0x00000000
[COMP-RX] EP:01 MIDI_SKIP (DataOut:0x00000000 data:0x00000000)
```
**Cause:** Complete initialization failure  
**Fix:** Check USB device init order and class registration

### Scenario 4: Init Failed
```
[COMP-Init] Starting MIDI+CDC init
[COMP-Init] Calling USBD_MIDI.Init()
[COMP-Init] ERROR: USBD_MIDI.Init() FAILED!
```
**Cause:** USBD_MIDI_Init() returned error  
**Fix:** Check endpoint allocation, memory, HAL config

### Scenario 5: Init is NULL
```
[COMP-Init] Starting MIDI+CDC init
[COMP-Init] WARNING: USBD_MIDI.Init is NULL!
```
**Cause:** USBD_MIDI struct not linked properly  
**Fix:** Check USBD_MIDI definition in usbd_midi.c

## Quick Fixes

### Fix A: Check Class Data Allocation
**File:** `USB_DEVICE/Class/MIDI/Src/usbd_midi.c`  
**Function:** `USBD_MIDI_Init()`  
**Line:** ~425-450

Should have:
```c
pdev->pClassData = &midi_class_data;
hmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassData;
```

### Fix B: Check Composite Init
**File:** `USB_DEVICE/App/usbd_composite.c`  
**Function:** `USBD_COMPOSITE_Init()`  
**Line:** 133

Should have:
```c
composite_class_data.midi_class_data = pdev->pClassData;
```

### Fix C: Check Initialization Order
**File:** `Core/Src/main.c`  
**Lines:** 186-199

Should be:
```c
usb_midi_init();        // FIRST - register callbacks
usb_cdc_init();         // SECOND - register callbacks  
MX_USB_DEVICE_Init();   // THIRD - start enumeration
```

### Fix D: Check USBD_MIDI Definition
**File:** `USB_DEVICE/Class/MIDI/Src/usbd_midi.c`  
**Line:** 122-138

Should have:
```c
USBD_ClassTypeDef USBD_MIDI = {
  USBD_MIDI_Init,      // ← Must not be NULL
  USBD_MIDI_DeInit,
  USBD_MIDI_Setup,
  NULL,
  USBD_MIDI_EP0_RxReady,
  USBD_MIDI_DataIn,
  USBD_MIDI_DataOut,   // ← Must not be NULL
  // ...
};
```

## Breakpoints

Set these and check values:

1. `usbd_composite.c:133` - After MIDI init
   - Check: `composite_class_data.midi_class_data != NULL`

2. `usbd_composite.c:298` - Before DataOut check
   - Check: `USBD_MIDI.DataOut != NULL`
   - Check: `composite_class_data.midi_class_data != NULL`

3. `usbd_midi.c:425-450` - Inside USBD_MIDI_Init
   - Check: `pdev->pClassData` is set

## What to Report

When asking for help, provide:
1. Complete debug output from boot to MIDI data attempt
2. Which scenario matches your output
3. Pointer values shown in MIDI_SKIP message
4. Whether COMP-Init messages appear

## Full Documentation

See `USB_MIDI_INIT_DEBUG_GUIDE.md` for complete details.
