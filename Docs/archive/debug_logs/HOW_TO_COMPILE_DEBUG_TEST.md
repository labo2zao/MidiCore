# How to Compile Debug Test

## Quick Answer
"how i compile the test?" → Add code to existing file, press Ctrl+B, flash!

## Quick Start (3 Steps)

1. **Open** STM32CubeIDE with MidiCore project
2. **Add** test code to `App/app_init.c` (see below)
3. **Build** with Ctrl+B, then **Flash** with F11

## Detailed Instructions

### Step 1: Add Test Code

**File:** `App/app_init.c`
**Location:** After `usb_midi_init()` call (around line 145)

**Add this code:**

```c
// ========== DEBUG TEST CODE - ADD AFTER usb_midi_init() ==========
#if MODULE_ENABLE_USB_MIDI

// TEST 1: Startup LED Blink (Green LED - proves firmware running)
for (int i = 0; i < 10; i++) {
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);   // LED on
  osDelay(100);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET); // LED off
  osDelay(100);
}

// TEST 2: CDC Startup Message (proves CDC working)
osDelay(2000);  // Wait for USB enumeration
const char* startup_msg = "=== MIDICORE STARTUP ===\r\n";
usb_cdc_send((const uint8_t*)startup_msg, strlen(startup_msg));

#endif
// ========== END DEBUG TEST CODE ==========
```

### Step 2: Compile in STM32CubeIDE

**Method 1: Menu**
1. Click `Project` → `Clean...`
2. Select "Clean all projects"
3. Click OK
4. Click `Project` → `Build All`
5. Watch console - should see "Build Finished"

**Method 2: Keyboard Shortcuts**
1. Press `Ctrl+Shift+C` (Clean)
2. Press `Ctrl+B` (Build All)
3. Check console for success

**What to Look For:**
```
Building file: ../App/app_init.c
Arm assembler/compiler/linker...
Finished building: App/app_init.o
...
Build Finished. 0 errors, X warnings.
```

### Step 3: Flash to Device

**Method 1: Debug Mode**
1. Connect ST-Link to PC and board
2. Click `Run` → `Debug` (or press F11)
3. Wait for "Programming successful"
4. Click Continue or press F8

**Method 2: Run Mode**
1. Connect ST-Link
2. Click `Run` → `Run` (or press Ctrl+F11)
3. Wait for "Programming successful"
4. Device resets automatically

### Step 4: Observe Results

**Immediately after reset:**
- ✅ **Green LED blinks 10 times** (2 seconds) → Firmware running!
- ✅ **Terminal shows "=== MIDICORE STARTUP ==="** → CDC working!

**When MIOS Studio sends query:**
- ✅ **Orange LED lights** → Query received!
- ✅ **Terminal shows "[QUERY RX]"** → Handler working!
- ✅ **Red LED lights** → Response sent!
- ✅ **Terminal shows "[RESP TX]"** → TX working!

## Adding Query Detection Test

To test if queries are being received, add to `Services/mios32_query/mios32_query.c`:

**At start of `mios32_query_process()` function (around line 55):**

```c
// TEST 3: Query Detection (Orange LED)
HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
const char* query_msg = "[QUERY RX]\r\n";
usb_cdc_send((const uint8_t*)query_msg, strlen(query_msg));
```

## Adding Response Test

**At end of `mios32_query_send_response()` function (after usb_midi_send_sysex, around line 135):**

```c
// TEST 4: Response Sent (Red LED)
HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
const char* resp_msg = "[RESP TX]\r\n";
usb_cdc_send((const uint8_t*)resp_msg, strlen(resp_msg));
```

## Troubleshooting

### Compilation Errors

**Error: `strlen` undeclared**
```c
// Add at top of app_init.c:
#include <string.h>
```

**Error: `HAL_GPIO_WritePin` undeclared**
```c
// Add at top of app_init.c:
#include "stm32f4xx_hal.h"
```

### Linking Errors

**Error: undefined reference to `usb_cdc_send`**
- Make sure `#include "Services/usb_cdc/usb_cdc.h"` is at top of file
- Check that MODULE_ENABLE_USB_CDC = 1 in Config/module_config.h

### No LEDs Blink

**Check LED GPIO Pins:**
- STM32F4-Discovery uses GPIOD pins 12-15
- Green LED: PD12
- Orange LED: PD13  
- Red LED: PD14
- Blue LED: PD15

**If using different board:**
Replace `GPIOD, GPIO_PIN_12` with your board's LED pins.

### No Terminal Output

**Check:**
1. MIOS Studio terminal is open (not just device window)
2. Correct COM port selected
3. `usb_cdc_init()` was called in app_init.c
4. MODULE_ENABLE_USB_CDC = 1 in module_config.h

## Expected Test Results

### Success Case (Everything Works)
```
Terminal output:
=== MIDICORE STARTUP ===
[QUERY RX]
[RESP TX]

LEDs:
- Green blinks 10 times at startup
- Orange lights when query received
- Red lights when response sent

MIOS Studio:
- Device recognized
- Appears in device list
```

### Failure Case 1: No Startup Message
```
Terminal: (nothing)
LEDs: Green blinks

Diagnosis: CDC not initialized or not enumerating
Solution: Check usb_cdc_init() call, check USB cable
```

### Failure Case 2: No Query Detection
```
Terminal: === MIDICORE STARTUP ===
LEDs: Green blinks, then nothing

Diagnosis: Queries not reaching mios32_query_process()
Solution: Check usb_midi_process_rx_queue() is called in task
```

### Failure Case 3: No Response
```
Terminal: 
=== MIDICORE STARTUP ===
[QUERY RX]

LEDs: Green + Orange, but no Red

Diagnosis: Response not being transmitted
Solution: Check usb_midi_send_sysex(), check TX queue
```

## Summary

**To compile and test:**
1. Add test code to app_init.c (copy-paste from above)
2. Press Ctrl+B to build
3. Press F11 to flash
4. Watch LEDs and terminal

**Each test proves one component:**
- Green LED → Firmware running
- Startup message → CDC working
- Orange LED → Queries received
- Red LED → Responses sent

**After testing, report which LEDs light up and what terminal shows!**

This will reveal the exact problem location.
