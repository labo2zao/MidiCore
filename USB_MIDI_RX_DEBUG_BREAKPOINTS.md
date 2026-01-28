# USB MIDI RX Debug - Breakpoint Locations

This document tells you **exactly where** each debug message is printed and where to set breakpoints for debugging USB MIDI RX issues.

## Overview: Message Flow

When MIDI data is received via USB:
```
[COMP-RX] EP:01 MIDI_OK       ← USB Composite layer (ISR context)
[COMP] Calling MIDI.DataOut   ← USB Composite layer (ISR context)
[COMP] MIDI.DataOut returned  ← USB Composite layer (ISR context)
[RX-ISR] Cable:0 CIN:09       ← USB MIDI queue (ISR context)
[RX-TASK] Processing 1 packet ← USB MIDI processor (Task context)
[RX] Cable:0 90 3C 64 (...)   ← Debug hook (Task context)
```

---

## 1. [COMP-RX] EP:01 MIDI_OK

### Location
**File:** `USB_DEVICE/App/usbd_composite.c`  
**Function:** `USBD_COMPOSITE_DataOut()`  
**Line:** 282

### Code
```c
#ifdef MODULE_TEST_USB_DEVICE_MIDI
  char buf[40];
  snprintf(buf, sizeof(buf), "[COMP-RX] EP:%02X MIDI_OK\r\n", epnum);
  dbg_print(buf);
#endif
```

### Breakpoint
Set breakpoint at **line 282** in `usbd_composite.c`

### Context
- **Called from:** USB interrupt (ISR context)
- **When:** USB hardware receives data on endpoint 0x01 (MIDI OUT)
- **What it means:** USB composite layer received data and is about to route it to MIDI class

### What to Check
- `epnum` should be `0x01`
- If this prints, USB hardware is receiving data correctly
- If this doesn't print, check:
  - USB cable connected
  - USB enumeration successful
  - MIDI device selected in DAW/host

---

## 2. [COMP] Calling MIDI.DataOut

### Location
**File:** `USB_DEVICE/App/usbd_composite.c`  
**Function:** `USBD_COMPOSITE_DataOut()`  
**Line:** 288

### Code
```c
#ifdef MODULE_TEST_USB_DEVICE_MIDI
  dbg_print("[COMP] Calling MIDI.DataOut\r\n");
#endif
```

### Breakpoint
Set breakpoint at **line 288** in `usbd_composite.c`

### Context
- **Called from:** USB interrupt (ISR context)
- **When:** Right before calling MIDI class DataOut handler
- **What it means:** About to forward packet to MIDI class

### What to Check
- If `[COMP-RX]` printed but this doesn't:
  - Check `USBD_MIDI.DataOut != NULL`
  - Check `composite_class_data.midi_class_data != NULL`
  - Problem with MIDI class initialization

---

## 3. [RX-ISR] Cable:X CIN:XX

### Location
**File:** `Services/usb_midi/usb_midi.c`  
**Function:** `usb_midi_rx_packet()`  
**Line:** 249

### Code
```c
#ifdef MODULE_TEST_USB_DEVICE_MIDI
  char buf[40];
  uint8_t cable = (packet4[0] >> 4) & 0x0F;
  uint8_t cin = packet4[0] & 0x0F;
  snprintf(buf, sizeof(buf), "[RX-ISR] Cable:%d CIN:%02X\r\n", cable, cin);
  dbg_print(buf);
#endif
```

### Breakpoint
Set breakpoint at **line 249** in `Services/usb_midi/usb_midi.c`

### Context
- **Called from:** USB MIDI class callback (ISR context)
- **When:** Packet received and about to be queued
- **What it means:** MIDI class received packet, will queue it for task processing

### What to Check
- `cable` should be 0-3 (cable/port number)
- `cin` (Code Index Number) indicates message type:
  - `0x08` = Note Off
  - `0x09` = Note On
  - `0x0B` = Control Change
  - `0x04-0x07` = SysEx
- If this doesn't print but `[COMP]` does:
  - MIDI class callback not registered
  - Check `usb_midi_init()` was called
  - Check callback chain

---

## 4. [RX-TASK] Processing N packet(s)

### Location
**File:** `Services/usb_midi/usb_midi.c`  
**Function:** `usb_midi_process_rx_queue()`  
**Line:** 293

### Code
```c
#ifdef MODULE_TEST_USB_DEVICE_MIDI
  if (!rx_queue_is_empty()) {
    uint8_t count = ((rx_queue_head - rx_queue_tail) & (USB_MIDI_RX_QUEUE_SIZE - 1));
    char buf[50];
    snprintf(buf, sizeof(buf), "[RX-TASK] Processing %d packet(s)\r\n", count);
    dbg_print(buf);
  }
#endif
```

### Breakpoint
Set breakpoint at **line 293** in `Services/usb_midi/usb_midi.c`

### Context
- **Called from:** Application task context (main loop)
- **When:** `usb_midi_process_rx_queue()` is called and queue has packets
- **What it means:** Task is processing queued packets

### What to Check
- If `[RX-ISR]` printed but this doesn't:
  - **CRITICAL:** `usb_midi_process_rx_queue()` is NOT being called!
  - Check test loop calls `usb_midi_process_rx_queue()`
  - This is the most common bug!
- `count` shows how many packets are queued

---

## 5. [RX] Cable:X YY YY YY (decoded message)

### Location
**File:** `App/tests/module_tests.c`  
**Function:** `module_test_usb_midi_print_packet()` called from `usb_midi_rx_debug_hook()`  
**Line:** 7073 (for regular MIDI), 7105-7114 (for SysEx)

### Code for Regular MIDI
```c
static void module_test_usb_midi_print_packet(const uint8_t packet4[4])
{
  uint8_t cable = (packet4[0] >> 4) & 0x0F;
  uint8_t status = packet4[1];
  uint8_t data1 = packet4[2];
  uint8_t data2 = packet4[3];
  
  dbg_printf("[RX] Cable:%d %02X %02X %02X", cable, status, data1, data2);
  
  // ... decoding logic ...
  
  dbg_print("\r\n");
}
```

### Called From
**Function:** `usb_midi_rx_debug_hook()` at line 7098  
**Called from:** `usb_midi_process_rx_queue()` at line 318

### Breakpoint Options
1. **Line 7098** - `usb_midi_rx_debug_hook()` entry (for all messages)
2. **Line 7073** - Regular MIDI print (for non-SysEx)
3. **Line 7105** - SysEx print (for SysEx messages)

### Context
- **Called from:** Task context via `usb_midi_process_rx_queue()`
- **When:** Packet is being decoded for display
- **What it means:** Packet successfully queued, dequeued, and is being displayed

### What to Check
- If `[RX-TASK]` printed but this doesn't:
  - Check `cable > 3` (invalid cable rejected at line 315)
  - Debug hook might not be compiled
  - Check `MODULE_TEST_USB_DEVICE_MIDI` is defined

---

## Complete Call Chain for RX

```
┌─────────────────────────────────────────────────────────────────┐
│ USB Hardware receives data on endpoint 0x01                     │
└────────────────────────┬────────────────────────────────────────┘
                         ↓
┌─────────────────────────────────────────────────────────────────┐
│ USB HAL / Drivers                                               │
│ (USB interrupt context)                                         │
└────────────────────────┬────────────────────────────────────────┘
                         ↓
┌─────────────────────────────────────────────────────────────────┐
│ USBD_COMPOSITE_DataOut()                                        │
│ File: USB_DEVICE/App/usbd_composite.c                           │
│ Line: 271                                                       │
│                                                                 │
│ → Prints: [COMP-RX] EP:01 MIDI_OK (line 282)                   │
│ → Prints: [COMP] Calling MIDI.DataOut (line 288)               │
│ → Calls: USBD_MIDI.DataOut() (line 291)                        │
│ → Prints: [COMP] MIDI.DataOut returned (line 294)              │
└────────────────────────┬────────────────────────────────────────┘
                         ↓
┌─────────────────────────────────────────────────────────────────┐
│ USBD_MIDI_DataOut()                                             │
│ File: USB_DEVICE/Class/MIDI/Src/usbd_midi.c                    │
│ (calls interface callback)                                     │
└────────────────────────┬────────────────────────────────────────┘
                         ↓
┌─────────────────────────────────────────────────────────────────┐
│ USBD_MIDI_DataOut_Callback()                                   │
│ File: Services/usb_midi/usb_midi.c                             │
│ Line: 518                                                       │
│ → Calls: usb_midi_rx_packet()                                  │
└────────────────────────┬────────────────────────────────────────┘
                         ↓
┌─────────────────────────────────────────────────────────────────┐
│ usb_midi_rx_packet()                     ** ISR ENDS HERE **   │
│ File: Services/usb_midi/usb_midi.c                             │
│ Line: 240                                                       │
│                                                                 │
│ → Prints: [RX-ISR] Cable:X CIN:XX (line 249)                   │
│ → Queues packet to rx_queue[] (lines 263-271)                  │
│ → Returns (ISR complete)                                       │
└─────────────────────────────────────────────────────────────────┘
                         ↓
                   [ISR Complete]
                         ↓
       ┌─────────────────────────────────┐
       │  Packet sits in queue until... │
       └─────────────────────────────────┘
                         ↓
┌─────────────────────────────────────────────────────────────────┐
│ Application Task Loop                                           │
│ File: App/tests/module_tests.c or app_test_usb_midi.c          │
│                                                                 │
│ for (;;) {                                                      │
│   usb_midi_process_rx_queue(); ← MUST BE CALLED!               │
│   // ... other code ...                                        │
│   osDelay(10);                                                  │
│ }                                                               │
└────────────────────────┬────────────────────────────────────────┘
                         ↓
┌─────────────────────────────────────────────────────────────────┐
│ usb_midi_process_rx_queue()              ** TASK CONTEXT **    │
│ File: Services/usb_midi/usb_midi.c                             │
│ Line: 285                                                       │
│                                                                 │
│ → Prints: [RX-TASK] Processing N packet(s) (line 293)          │
│ → Dequeues packets (line 301)                                  │
│ → Calls: usb_midi_rx_debug_hook() (line 318)                   │
└────────────────────────┬────────────────────────────────────────┘
                         ↓
┌─────────────────────────────────────────────────────────────────┐
│ usb_midi_rx_debug_hook()                                        │
│ File: App/tests/module_tests.c                                 │
│ Line: 7098                                                      │
│                                                                 │
│ → Calls: module_test_usb_midi_print_packet() (line 7119)       │
└────────────────────────┬────────────────────────────────────────┘
                         ↓
┌─────────────────────────────────────────────────────────────────┐
│ module_test_usb_midi_print_packet()                             │
│ File: App/tests/module_tests.c                                 │
│ Line: 7066                                                      │
│                                                                 │
│ → Prints: [RX] Cable:X YY YY YY (decoded) (line 7073)          │
└─────────────────────────────────────────────────────────────────┘
```

---

## Debugging Strategy

### If you see ONLY `[COMP-RX] EP:01 MIDI_OK`:

**Problem:** Either MIDI class not getting data OR queue not being processed.

**Debug steps:**
1. Check if `[COMP] Calling MIDI.DataOut` appears
   - **YES:** MIDI class is being called, go to step 2
   - **NO:** MIDI class callback not registered
     - Set breakpoint at line 277 in `usbd_composite.c`
     - Check `USBD_MIDI.DataOut != NULL`
     - Check `usb_midi_init()` was called

2. Check if `[RX-ISR]` appears
   - **YES:** Packets are being queued, go to step 3
   - **NO:** MIDI callback not forwarding to queue
     - Set breakpoint at line 240 in `usb_midi.c`
     - Check function is being called

3. Check if `[RX-TASK]` appears
   - **YES:** Queue is being processed, go to step 4
   - **NO:** **MOST COMMON BUG** - `usb_midi_process_rx_queue()` not called!
     - Find your test loop (module_tests.c or app_test_usb_midi.c)
     - Add: `usb_midi_process_rx_queue();` at top of loop
     - Rebuild and reflash

4. Check if `[RX]` appears
   - **YES:** Everything working!
   - **NO:** Debug hook not printing
     - Set breakpoint at line 7098 in `module_tests.c`
     - Check if `usb_midi_rx_debug_hook()` is called
     - Check `MODULE_TEST_USB_DEVICE_MIDI` is defined

---

## Required Compiler Defines

For all debug messages to appear, you MUST have:
```c
#define MODULE_TEST_USB_DEVICE_MIDI 1
```

This can be set in:
- **CubeIDE:** Project Properties → C/C++ Build → Settings → MCU GCC Compiler → Preprocessor → Defined symbols
- **Makefile:** `CFLAGS += -DMODULE_TEST_USB_DEVICE_MIDI`

Without this define, debug messages will NOT be compiled in!

---

## Quick Breakpoint List

Copy these into your debugger:

```
USB_DEVICE/App/usbd_composite.c:282    [COMP-RX]
USB_DEVICE/App/usbd_composite.c:288    [COMP] Calling
Services/usb_midi/usb_midi.c:249       [RX-ISR]
Services/usb_midi/usb_midi.c:293       [RX-TASK]
App/tests/module_tests.c:7098          [RX] hook
App/tests/module_tests.c:7073          [RX] print
```

---

## Most Common Issues

### 1. Only `[COMP-RX]` appears, nothing else
**Cause:** `usb_midi_process_rx_queue()` not called in test loop  
**Fix:** Add `usb_midi_process_rx_queue();` at top of test loop  
**Files:** `App/tests/module_tests.c` line 7164 OR `App/tests/app_test_usb_midi.c` line 339

### 2. None of the debug messages appear
**Cause:** `MODULE_TEST_USB_DEVICE_MIDI` not defined  
**Fix:** Add to compiler defines in project settings

### 3. `[RX-ISR]` appears but not `[RX-TASK]`
**Cause:** Queue not empty but `usb_midi_process_rx_queue()` not called often enough  
**Fix:** Ensure `usb_midi_process_rx_queue()` called in main loop (every 10ms or faster)

---

## Additional Debug Options

### Enable More Detailed MIDI Class Debug

In `USB_DEVICE/Class/MIDI/Src/usbd_midi.c`, add debug prints to `USBD_MIDI_DataOut()`:

```c
uint8_t USBD_MIDI_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
#ifdef MODULE_TEST_USB_DEVICE_MIDI
  dbg_print("[MIDI-DataOut] ENTRY\r\n");
#endif
  
  // ... existing code ...
  
#ifdef MODULE_TEST_USB_DEVICE_MIDI
  char buf[40];
  snprintf(buf, sizeof(buf), "[MIDI-RX] Len:%d\r\n", hmidi->RxLength);
  dbg_print(buf);
#endif
  
  // ... callback call ...
}
```

This will show:
```
[COMP-RX] EP:01 MIDI_OK
[COMP] Calling MIDI.DataOut
[MIDI-DataOut] ENTRY
[MIDI-RX] Len:4
[MIDI-RX] Calling callback
[COMP] MIDI.DataOut returned
[RX-ISR] Cable:0 CIN:09
```

---

## Summary

| Message | File | Line | Function | Context |
|---------|------|------|----------|---------|
| `[COMP-RX] EP:01 MIDI_OK` | usbd_composite.c | 282 | USBD_COMPOSITE_DataOut | ISR |
| `[COMP] Calling MIDI.DataOut` | usbd_composite.c | 288 | USBD_COMPOSITE_DataOut | ISR |
| `[RX-ISR] Cable:X CIN:XX` | usb_midi.c | 249 | usb_midi_rx_packet | ISR |
| `[RX-TASK] Processing N` | usb_midi.c | 293 | usb_midi_process_rx_queue | Task |
| `[RX] Cable:X YY YY YY` | module_tests.c | 7073 | module_test_usb_midi_print_packet | Task |

**Critical:** The transition from ISR context to Task context happens between `[RX-ISR]` and `[RX-TASK]`. This requires explicit call to `usb_midi_process_rx_queue()` in your application task loop!
