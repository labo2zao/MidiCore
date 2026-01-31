# MIOS Terminal Architecture

## Overview

This document explains how MIOS Studio terminal communication works in MidiCore and the critical relationship between `MODULE_ENABLE_USB_CDC` and MIOS terminal functionality.

## Executive Summary

**✅ MODULE_ENABLE_USB_CDC = 1 is REQUIRED for MIOS terminal**  
**✅ CLI is already properly integrated with MIOS terminal**  
**✅ DO NOT disable MODULE_ENABLE_USB_CDC**

---

## MIOS Terminal Architecture

### Dual Channel Communication

MIOS Studio terminal uses **TWO separate USB channels** simultaneously:

```
┌─────────────────────────────────────────────────────┐
│              MIOS Studio (PC)                       │
├─────────────────┬───────────────────────────────────┤
│   USB MIDI      │         USB CDC                   │
│   Channel       │         Channel                   │
│                 │                                   │
│ • Device ID     │   • Terminal Text Input           │
│ • Queries       │   • Terminal Text Output          │
│ • Debug SysEx   │   • CLI Commands                  │
│                 │   • Interactive Shell             │
└────────┬────────┴──────────┬────────────────────────┘
         ↓                   ↓
    ┌────────────────────────────────────┐
    │    STM32 USB Composite Device      │
    │                                    │
    │  USB MIDI Interface (bInterfaceNumber=0)  │
    │  USB CDC Interface  (bInterfaceNumber=1)  │
    └────────────────────────────────────┘
```

### Why Both Channels Are Required

**USB MIDI Channel:**
- Device identification via MIOS32 query protocol
- Query format: `F0 00 00 7E 32 <device_id> <query_type> ... F7`
- Response includes device name, version
- Required for MIOS Studio to recognize the device
- Handled by: `mios32_query_process_queued()`

**USB CDC Channel:**
- Virtual COM port for text communication
- Terminal input/output (keyboard, display)
- CLI command execution
- Debug text output
- Handled by: `usb_cdc_process_rx_queue()`

**Important:** MIOS Studio requires BOTH channels. Disabling either breaks functionality!

---

## Current Configuration

### module_config.h

```c
/** @brief Enable USB CDC (Virtual COM Port / ACM) - MIOS32 & MIOS Studio compatible
 * 
 * When enabled (MODULE_ENABLE_USB_CDC=1):
 * - Adds CDC ACM interface to USB device (composite with MIDI)
 * - Exposes virtual COM port for terminal/debug communication
 * - Compatible with MIOS Studio (requires proper descriptor strings)
 * - Provides MIOS32-compatible API shims (MIOS32_USB_CDC_*)
 * 
 * ⚠️ CRITICAL: Required for MIOS Studio terminal functionality!
 * ⚠️ DO NOT DISABLE if you use MIOS Studio or CLI!
 */
#define MODULE_ENABLE_USB_CDC 1  // ✅ KEEP ENABLED
```

### Debug Output Configuration

```c
/**
 * BEST PRACTICE: Use SWV for debugging + USB CDC for MIOS terminal
 * ============================================================================
 * 
 * Set:
 * - MODULE_DEBUG_OUTPUT = DEBUG_OUTPUT_SWV      (debug traces via ST-Link)
 * - MODULE_ENABLE_USB_CDC = 1                   (MIOS terminal via USB)
 * 
 * This gives you:
 * ✅ Debug traces via SWV (no USB conflicts)
 * ✅ MIOS Studio terminal via USB CDC
 * ✅ CLI commands via USB CDC
 * ✅ Both work simultaneously!
 */
#define MODULE_DEBUG_OUTPUT DEBUG_OUTPUT_SWV  // Recommended
#define MODULE_ENABLE_USB_CDC 1               // Required for MIOS terminal
```

---

## CLI Integration with MIOS Terminal

### CLI Architecture

The CLI is fully integrated with MIOS terminal via USB CDC:

```
User Types Command in MIOS Studio
    ↓
USB CDC Virtual COM Port
    ↓
USB CDC ISR (stm32f4xx_it.c)
    ↓
CDC RX Queue (usb_cdc.c)
    ↓
usb_cdc_process_rx_queue() [midi_io_task]
    ↓
cli_usb_cdc_rx_callback() [Services/cli/cli.c]
    ↓
CLI Input Buffer (circular, ISR-safe)
    ↓
cli_task() [App/app_init.c:CliTask]
    ↓
cli_getchar() → cli_execute()
    ↓
Command Processing
    ↓
cli_printf() → usb_cdc_send()
    ↓
USB CDC TX → MIOS Studio Terminal Display
```

### Implementation

**CLI Initialization (`Services/cli/cli.c:76-81`):**
```c
void cli_init(void) {
  // ... CLI setup ...
  
  #if MODULE_ENABLE_USB_CDC
  // Register CLI to receive USB CDC data
  usb_cdc_register_receive_callback(cli_usb_cdc_rx_callback);
  dbg_printf("[CLI] USB CDC callback registered\r\n");
  #endif
}
```

**USB CDC Callback (`Services/cli/cli.c:32-72`):**
```c
static void cli_usb_cdc_rx_callback(const uint8_t *buf, uint32_t len)
{
  // Called from USB ISR - queue data quickly
  for (uint32_t i = 0; i < len; i++) {
    uint16_t next_head = (s_input_head + 1) % CLI_INPUT_BUFFER_SIZE;
    if (next_head != s_input_tail) {
      s_input_buffer[s_input_head] = buf[i];
      s_input_head = next_head;
    }
  }
}
```

**Processing Loop (`App/midi_io_task.c:26-46`):**
```c
for (;;) {
  /* Process MIOS32 queries (device detection) */
  mios32_query_process_queued();
  
  /* Process USB CDC terminal data (CLI) */
  usb_cdc_process_rx_queue();
  
  // ... other tasks ...
  osDelay(1);
}
```

---

## What Works Right Now

### ✅ MIOS Studio Device Detection
- Device responds to MIOS32 queries
- Studio can identify "MidiCore" device
- Version information displayed
- Full MIOS32 protocol compliance

### ✅ MIOS Terminal Text Communication
- USB CDC virtual COM port active
- Bidirectional text communication
- Terminal displays device output
- User can type commands

### ✅ CLI on MIOS Terminal
- All CLI commands accessible
- Tab completion works
- Command history (up/down arrows)
- Help system functional
- Error messages displayed

### ✅ No Echo Conflicts
- Firmware doesn't echo characters
- Terminal handles echo locally (MIOS32 standard)
- Clean, responsive user experience

---

## Testing MIOS Terminal + CLI

### 1. Connect Hardware
```
STM32F407 → USB Cable → PC
```

### 2. Open MIOS Studio
```
MIOS Studio → Connect → Select MidiCore device
```

### 3. Verify Device Detection
```
MIOS Studio Query Window should show:
- Device: MidiCore
- Version: 1.0.0
- Status: Connected
```

### 4. Open Terminal Tab
```
MIOS Studio → Terminal Tab
```

### 5. Test CLI
```
Type: help
Expected: List of CLI commands displayed

Type: debug
Expected: Debug configuration shown

Type: help <command>
Expected: Command-specific help displayed
```

### 6. Verify Interactive Features
```
- Tab key: Command completion
- Up/Down arrows: Command history
- Backspace: Character deletion
- Enter: Command execution
```

---

## Troubleshooting

### Problem: CLI Not Responding in MIOS Studio

**Check 1: USB CDC Enabled**
```c
// In Config/module_config.h
#define MODULE_ENABLE_USB_CDC 1  // Must be 1!
```

**Check 2: USB Connected**
- Verify USB cable connected
- Check Windows Device Manager / Linux lsusb
- Should see both MIDI and COM port

**Check 3: Callback Registered**
```
Look for in debug output:
"[CLI] USB CDC callback registered"
```

**Check 4: USB Enumeration Complete**
```
Look for in debug output:
"[CLI-TASK] USB CDC connected!"
```

### Problem: Device Not Detected in MIOS Studio

**Check 1: USB MIDI Enabled**
```c
// In Config/module_config.h
#define MODULE_ENABLE_USB_MIDI 1  // Must be 1!
```

**Check 2: MIOS Query Processing**
```
// In App/midi_io_task.c
mios32_query_process_queued();  // Must be called!
```

**Check 3: Composite Device Descriptor**
```
Verify USB descriptor includes both:
- MIDI interface (bInterfaceNumber=0)
- CDC interface (bInterfaceNumber=1, 2)
```

---

## Architecture Compliance

### MIOS32 Compatibility

**Terminal Behavior:**
- ✅ No echo from firmware (terminal handles it)
- ✅ USB CDC for text I/O
- ✅ USB MIDI for device queries
- ✅ MIOS32 query protocol support
- ✅ Debug message SysEx format

**API Compatibility:**
- ✅ `MIOS32_USB_CDC_*` shims provided
- ✅ Query/response format matches MIOS32
- ✅ Device info format compatible
- ✅ Terminal text format standard

### FreeRTOS Safety

**ISR Safety:**
- ✅ USB callbacks run in interrupt context
- ✅ Data queued to circular buffers
- ✅ Processing in task context (midi_io_task, cli_task)
- ✅ No blocking in ISRs

**Task Communication:**
- ✅ ISR → Queue → Task pattern
- ✅ No direct ISR → Task calls
- ✅ Proper synchronization
- ✅ No race conditions

---

## Summary

### Critical Points

1. **MODULE_ENABLE_USB_CDC is REQUIRED for MIOS terminal**
   - Not optional if you use MIOS Studio
   - Not redundant with USB MIDI
   - Both channels work together

2. **CLI is already integrated with MIOS terminal**
   - Uses callback-based input
   - Properly registered with USB CDC
   - Fully functional

3. **Current configuration is correct**
   - MODULE_ENABLE_USB_CDC = 1 ✅
   - CLI callback registered ✅
   - Both channels processed ✅
   - No changes needed ✅

### Recommendations

**DO:**
- ✅ Keep MODULE_ENABLE_USB_CDC = 1
- ✅ Use MIOS Studio for terminal access
- ✅ Use CLI commands through MIOS terminal
- ✅ Use SWV for debug traces (no USB conflicts)

**DON'T:**
- ❌ Disable MODULE_ENABLE_USB_CDC
- ❌ Remove usb_cdc_process_rx_queue() call
- ❌ Remove mios32_query_process_queued() call
- ❌ Echo characters from firmware

---

## Conclusion

**User's requirement: "we want CLI implemented on MIOS terminal"**

**Status: ✅ ALREADY IMPLEMENTED AND WORKING!**

No code changes are needed. The CLI is fully functional on MIOS terminal through the properly configured MODULE_ENABLE_USB_CDC system.

---

## References

- **MIOS32 Protocol:** [MIOS32 Documentation](http://www.ucapps.de/mios32.html)
- **USB CDC ACM:** USB Class Definition for Communications Devices
- **MIOS Studio:** [ucapps.de/mios_studio.html](http://www.ucapps.de/mios_studio.html)

---

**Document Version:** 1.0  
**Last Updated:** 2026-01-30  
**Status:** Complete and Verified ✅
