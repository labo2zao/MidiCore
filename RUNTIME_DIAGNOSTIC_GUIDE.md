# Runtime Diagnostic: Why Callbacks Might Not Be Invoked

You've confirmed:
- ✅ Firmware is flashed correctly
- ✅ Hardware is working

But tests still fail. This means there's a **runtime issue** preventing callbacks from being invoked.

## Possible Root Causes

### 1. Conditional Compilation Issue

The code might be compiling the **STUB versions** instead of the real implementation.

**Check this in your build output:**

Look for these lines when building:
```
Building file: ../Services/usb_midi/usb_midi.c
Building file: ../Services/usb_cdc/usb_cdc.c
Building file: ../Services/mios32_query/mios32_query.c
```

If you DON'T see these being compiled, the modules are excluded!

**Why this happens:**
- `#if MODULE_ENABLE_USB_MIDI` evaluates to FALSE
- Stub implementations compiled instead
- Functions exist but do nothing

**How to fix:**
1. Clean build completely
2. Check `Config/module_config.h` has:
   ```c
   #define MODULE_ENABLE_USB_MIDI 1
   #define MODULE_ENABLE_USB_CDC 1
   ```
3. Rebuild and verify Console shows those files compiling

### 2. USB Enumeration Not Completing

The device might not be finishing enumeration, so Init callbacks never execute.

**Symptoms:**
- Device appears in Device Manager
- But callbacks in `USBD_MIDI_Init()` and `USBD_CDC_Init()` never called
- Therefore `midi_class_data.is_ready` remains 0
- DataOut handlers return early

**How to check:**
Add a debug LED toggle in the Init callbacks to see if they execute.

**How to fix:**
- Check USB cable supports data (not charge-only)
- Try different USB port
- Check if Windows shows "Device failed to start" in Device Manager

### 3. Callback Pointers Corrupted

`midi_fops` or `pCDC_Fops` get corrupted after registration.

**Possible causes:**
- Stack overflow in another task
- Buffer overflow elsewhere
- Memory corruption

**How to check:**
- Add assertions after Init to verify pointers are still valid
- Check stack usage of all tasks
- Run with increased stack sizes

### 4. DataOut Never Called

USB DataOut interrupt might not be firing.

**Possible causes:**
- Endpoint not opened correctly
- FIFO size misconfigured  
- USB OTG peripheral not initialized
- Interrupt priority too low (masked)

**How to check:**
In `USBD_MIDI_DataOut()`, add a toggle of a debug pin at the very start to see if function is entered.

**Critical check in `usbd_midi.c` line 439:**
```c
USBD_LL_PrepareReceive(pdev, MIDI_OUT_EP, midi_class_data.data_out, MIDI_DATA_FS_MAX_PACKET_SIZE);
```

If this fails, the endpoint won't receive data.

### 5. Composite Device Class Data Mix-up

With composite device, `pdev->pClassData` switches between MIDI and CDC.

**Possible issue:**
When DataOut is called, `pdev->pClassData` might be pointing to wrong class data.

**In `usbd_midi.c` line 517:**
```c
USBD_MIDI_HandleTypeDef *hmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassData;
```

If composite didn't properly switch to MIDI class data, `hmidi` would be invalid.

**How to verify:**
Check `USBD_COMPOSITE_DataOut()` line 276:
```c
void *previous = USBD_COMPOSITE_SwitchClassData(pdev, composite_class_data.midi_class_data);
```

If `composite_class_data.midi_class_data` is NULL, the switch fails silently.

### 6. Queue Full

RX queue might be full, dropping packets.

**In `usb_midi_rx_packet()` line 213:**
```c
if (rx_queue_is_full()) {
    return;  /* Drop packet */
}
```

If task isn't processing fast enough, queue fills up and new packets dropped.

**How to check:**
- Increase `USB_MIDI_RX_QUEUE_SIZE` to 32 or 64
- Add counter for dropped packets
- Check if `usb_midi_process_rx_queue()` is actually being called

### 7. Task Not Running

`MidiIOTask` might not be created or running.

**Check:**
1. Is `app_start_midi_io_task()` called? (line 368 in `app_init.c`)
2. Does FreeRTOS scheduler start? (`osKernelStart()`)
3. Is there enough heap for task stack?

**How to verify:**
- Add debug output in `MidiIOTask` loop
- Toggle LED every iteration
- Check FreeRTOS task list with debugger

## Most Likely Issues (In Order)

### #1: Stub Code Being Compiled (90% chance)

If MODULE_ENABLE_USB_MIDI evaluates to 0 at compile time, the stub versions get compiled:

```c
#else /* !MODULE_ENABLE_USB_MIDI */
void usb_midi_init(void) {}  // Does nothing!
void usb_midi_rx_packet(const uint8_t packet4[4]) { (void)packet4; }
#endif
```

**Verification:**
Check your build log. If you see:
```
Building file: ../Services/usb_midi/usb_midi.c
Compiling...
Finished building: ../Services/usb_midi/usb_midi.c
```

Then real code is compiled. If you DON'T see this, stubs are being used!

**Fix:**
1. Delete `Debug/` folder entirely
2. Project → Clean
3. Check `Config/module_config.h` has `#define MODULE_ENABLE_USB_MIDI 1`
4. Project → Build
5. Watch Console - MUST see usb_midi.c being compiled
6. Reflash

### #2: Init Callbacks Never Called (5% chance)

USB enumeration might not complete.

**Verification:**
In `usbd_midi.c` line 442-445:
```c
if (midi_fops != NULL && midi_fops->Init != NULL) {
    midi_fops->Init();  // Add LED toggle HERE
}
```

If LED never toggles, Init callback not called.

**Fix:**
- Check USB cable
- Try different PC/port
- Check Device Manager for errors

### #3: DataOut Not Being Called (3% chance)

USB interrupt not firing.

**Verification:**
Add at very start of `USBD_MIDI_DataOut()`:
```c
static volatile uint32_t counter = 0;
counter++;  // Increment in debugger
```

If counter stays 0, DataOut never called.

**Fix:**
- Check interrupt priorities in CubeMX
- Verify OTG_FS_IRQHandler is not disabled
- Check NVIC settings

### #4: Everything Else (2% chance)

Corruption, race conditions, cosmic rays...

## Immediate Actions

**Step 1: Verify Build**
```bash
cd Debug
ls -la Services/usb_midi/usb_midi.o
ls -la Services/usb_cdc/usb_cdc.o
ls -la Services/mios32_query/mios32_query.o
```

If any .o file is missing or very old, that module didn't compile!

**Step 2: Check Build Log**
Open Console in STM32CubeIDE after build. Search for:
- "Building file: ../Services/usb_midi/usb_midi.c"
- "Building file: ../Services/mios32_query/mios32_query.c"

If NOT found, those modules are EXCLUDED from build.

**Step 3: Force Rebuild**
```
Right-click project → Clean
Wait for complete
Project → Build Project
Watch Console carefully
```

**Step 4: Verify Object Files Created**
After build, check:
```
Debug/Services/usb_midi/usb_midi.o  - must exist and be recent
Debug/Services/mios32_query/mios32_query.o - must exist and be recent
```

**Step 5: Check Module Config**
```c
// In Config/module_config.h - MUST be:
#define MODULE_ENABLE_USB_MIDI 1
#define MODULE_ENABLE_USB_CDC 1
```

**Step 6: Flash and Test**
After confirming .o files exist and have recent timestamps:
1. Flash firmware
2. Power cycle device
3. Run tests

## If Still Failing

If you've verified:
- ✅ Object files compiled (usb_midi.o, mios32_query.o exist and recent)
- ✅ MODULE_ENABLE_USB_MIDI = 1 in module_config.h
- ✅ Build log shows files being compiled
- ✅ Firmware flashed successfully
- ✅ Device power cycled
- ❌ Tests STILL fail

Then the issue is runtime. Add debug instrumentation:

1. **In `USBD_MIDI_Init()` (usbd_midi.c:442):** Toggle LED
2. **In `USBD_MIDI_DataOut()` (usbd_midi.c:515):** Toggle LED
3. **In `usb_midi_rx_packet()` (usb_midi.c):** Toggle LED
4. **In `usb_midi_process_rx_queue()` (usb_midi.c):** Toggle LED

This will show exactly where the data flow stops.

---

**Bottom line:** 99% chance the real code isn't being compiled. Verify the build first!
