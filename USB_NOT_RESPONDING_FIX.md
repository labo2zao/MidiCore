# USB Not Responding - Critical Fix

## Problem: USB Device Completely Non-Responsive

After fixing the descriptor freeze issue, the device still didn't work properly. It would appear to enumerate but not respond to any USB requests.

## Root Cause: pClassData Pointer Conflict

### The Critical Bug

Both MIDI and CDC classes use the **same** `pdev->pClassData` pointer to store their class-specific data:

```c
// In USBD_MIDI_Init():
static USBD_MIDI_HandleTypeDef midi_class_data;
pdev->pClassData = &midi_class_data;  // MIDI sets the pointer

// In USBD_CDC_Init() (called after MIDI):
USBD_CDC_HandleTypeDef *hcdc = USBD_malloc(sizeof(...));
pdev->pClassData = hcdc;  // ❌ CDC OVERWRITES MIDI pointer!
```

### What Went Wrong

1. **Composite Init** calls MIDI Init first
   - MIDI sets `pdev->pClassData = &midi_class_data`
   - MIDI stores its state there

2. **Composite Init** then calls CDC Init
   - CDC allocates new memory for its data
   - CDC sets `pdev->pClassData = hcdc`
   - **MIDI pointer is lost!**

3. **USB callbacks are invoked** (Setup, DataIn, DataOut)
   - MIDI tries to access `pdev->pClassData` → Gets CDC data!
   - Type mismatch: expects `USBD_MIDI_HandleTypeDef`, gets `USBD_CDC_HandleTypeDef`
   - **Result**: Crashes, hangs, or incorrect behavior

### Why Device Appeared Non-Responsive

- **Setup requests**: Failed because classes accessed wrong data
- **Data transfers**: Didn't work because endpoint state was corrupted
- **Control transfers**: Crashed or timed out
- **Host perspective**: Device enumerated but doesn't respond to any requests

## Solution: Composite Class Data Manager

### Architecture

Created a composite class that manages separate data pointers for each class:

```c
/* Composite class data structure */
typedef struct {
  void *midi_class_data;  /* Pointer to MIDI class data */
  void *cdc_class_data;   /* Pointer to CDC class data */
} USBD_COMPOSITE_HandleTypeDef;

/* Static storage */
static USBD_COMPOSITE_HandleTypeDef composite_class_data;
```

### How It Works

#### Phase 1: Initialization

```c
static uint8_t USBD_COMPOSITE_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  /* Set pClassData to composite storage */
  pdev->pClassData = &composite_class_data;
  void *original = pdev->pClassData;
  
  /* Initialize MIDI */
  USBD_MIDI.Init(pdev, cfgidx);
  /* MIDI has set pdev->pClassData to its own data */
  composite_class_data.midi_class_data = pdev->pClassData;  // Save it!
  
  /* Restore composite pointer */
  pdev->pClassData = original;
  
  /* Initialize CDC */
  USBD_CDC.Init(pdev, cfgidx);
  /* CDC has set pdev->pClassData to its own data */
  composite_class_data.cdc_class_data = pdev->pClassData;  // Save it!
  
  /* Restore composite pointer */
  pdev->pClassData = original;
  
  return USBD_OK;
}
```

**Result**: Both class data pointers are safely stored in composite structure.

#### Phase 2: Callback Routing

Every callback (Setup, DataIn, DataOut, EP0_RxReady) follows this pattern:

```c
static uint8_t USBD_COMPOSITE_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  void *saved_class_data = pdev->pClassData;  // Save composite pointer
  uint8_t ret = USBD_OK;
  
  uint8_t interface = LOBYTE(req->wIndex);
  
  if (interface <= 1) {  /* MIDI interfaces */
    /* Restore MIDI class data pointer */
    pdev->pClassData = composite_class_data.midi_class_data;
    
    /* Call MIDI handler - it now sees correct data! */
    ret = USBD_MIDI.Setup(pdev, req);
    
    /* Restore composite pointer */
    pdev->pClassData = saved_class_data;
  }
  else if (interface >= 2 && interface <= 3) {  /* CDC interfaces */
    /* Restore CDC class data pointer */
    pdev->pClassData = composite_class_data.cdc_class_data;
    
    /* Call CDC handler - it now sees correct data! */
    ret = USBD_CDC.Setup(pdev, req);
    
    /* Restore composite pointer */
    pdev->pClassData = saved_class_data;
  }
  
  return ret;
}
```

**Result**: Each class handler sees its own data, exactly as if it were the only class.

### Example: CDC Setup Request

```
1. Host sends CDC_SET_LINE_CODING request for interface 2
2. USBD_COMPOSITE_Setup() is called
3. Detects interface 2 → CDC
4. Saves composite pointer
5. Restores CDC class data pointer
6. Calls USBD_CDC_Setup():
   - Accesses pdev->pClassData (now pointing to CDC data)
   - Casts to USBD_CDC_HandleTypeDef* (correct type!)
   - Processes line coding request
   - Updates CDC state (tx_state, rx_state, etc.)
7. Returns to composite
8. Composite restores its own pointer
9. Success!
```

### Example: MIDI Data Transfer

```
1. Host sends MIDI data on endpoint 0x01
2. USBD_COMPOSITE_DataOut() is called with epnum=0x01
3. Detects EP1 → MIDI
4. Saves composite pointer
5. Restores MIDI class data pointer
6. Calls USBD_MIDI_DataOut():
   - Accesses pdev->pClassData (now pointing to MIDI data)
   - Casts to USBD_MIDI_HandleTypeDef* (correct type!)
   - Processes MIDI packet
   - Updates MIDI state
7. Returns to composite
8. Composite restores its own pointer
9. Success!
```

## Comparison: Before vs After

### Before Fix

```
Init:
  MIDI.Init() → pdev->pClassData = &midi_data
  CDC.Init()  → pdev->pClassData = &cdc_data  ❌ Overwrites MIDI!

Setup (interface 0):
  MIDI.Setup() tries to access pdev->pClassData
    → Gets CDC data instead of MIDI data
    → Type mismatch
    → Crash or wrong behavior

DataOut (EP1):
  MIDI.DataOut() tries to access pdev->pClassData
    → Gets CDC data instead of MIDI data
    → Corrupted state
    → Device hangs
```

### After Fix

```
Init:
  pdev->pClassData = &composite_data
  MIDI.Init() → sets pdev->pClassData = &midi_data
    → composite saves it: composite_data.midi_class_data = &midi_data
  CDC.Init()  → sets pdev->pClassData = &cdc_data
    → composite saves it: composite_data.cdc_class_data = &cdc_data
  pdev->pClassData = &composite_data (restored)

Setup (interface 0):
  Composite.Setup():
    → Detects MIDI interface
    → pdev->pClassData = composite_data.midi_class_data
    → MIDI.Setup() sees correct MIDI data ✓
    → pdev->pClassData = &composite_data (restored)

DataOut (EP1):
  Composite.DataOut():
    → Detects MIDI endpoint
    → pdev->pClassData = composite_data.midi_class_data
    → MIDI.DataOut() sees correct MIDI data ✓
    → pdev->pClassData = &composite_data (restored)
```

## Why This is a Standard Pattern

This is the **correct architecture** for USB composite devices:

1. **Single Device Handle**: USB stack provides one `USBD_HandleTypeDef`
2. **Single pClassData**: Only one class data pointer available
3. **Multiple Classes**: Need separate data for each
4. **Composite Layer**: Acts as a **multiplexer**
   - Stores all class data pointers
   - Routes callbacks to correct class
   - Temporarily restores correct pointer
   - Ensures each class sees its own data

This pattern is used in professional USB stacks (e.g., STM32 HAL, USB IF examples).

## Testing Validation

### What Should Work Now

1. **Enumeration**
   - Device appears in Device Manager
   - Both MIDI and COM port show up
   - No error codes

2. **MIDI Operations**
   - Send MIDI notes → Device receives correctly
   - Device sends MIDI → Host receives correctly
   - No data corruption

3. **CDC Operations**
   - Open COM port → Success
   - Send data → Device receives correctly
   - Device sends data → Host receives correctly
   - Set line coding → Works

4. **Simultaneous Use**
   - MIDI and CDC can be used at same time
   - No interference
   - No crashes

### What Would Fail Without This Fix

- ❌ Setup requests fail or return wrong data
- ❌ Data transfers corrupt or don't work
- ❌ Device appears to hang after enumeration
- ❌ MIDI or CDC (or both) completely non-functional
- ❌ Crashes during USB operations
- ❌ Device must be unplugged/replugged repeatedly

## Technical Details

### Memory Layout

```
Before:
  midi_class_data: static storage (stack or data section)
  ↓
  [LOST when CDC overwrites pointer]

After:
  composite_class_data: {
    .midi_class_data = &midi_class_data (safe)
    .cdc_class_data  = &cdc_data (safe)
  }
  
  Both pointers preserved!
```

### Callback Flow

```
USB Core
  ↓
USBD_COMPOSITE callbacks
  ↓
Route based on interface/endpoint
  ↓
Temporarily restore class pointer
  ↓
Call class callback (MIDI or CDC)
  ↓
Class sees its own data ✓
  ↓
Return to composite
  ↓
Restore composite pointer
  ↓
Return to USB Core
```

## Summary

**Problem**: Classes fought over single `pClassData` pointer → corruption → non-responsive device

**Solution**: Composite layer manages separate pointers, restores correct one for each callback

**Result**: Each class accesses its own data correctly, device works properly

This was a **critical architectural bug** that would prevent any composite USB device from working. The fix implements the standard composite device pattern used in all professional USB implementations.

---

**Status**: ✅ Fixed
**Commit**: d500552
**Risk**: Very Low (standard pattern)
**Testing**: Device should now be fully functional
