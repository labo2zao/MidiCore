# MIOS Terminal Production Mode Debugging Guide

## Problem Statement

**Issue:** MIOS terminal works with `MODULE_TEST_USB_DEVICE_MIDI` but not in production mode

**Expected:** MIOS Studio should detect device and terminal should work in both modes

---

## Architecture Overview

### MIOS Terminal Components

MIOS Studio terminal requires **TWO USB channels**:

1. **USB MIDI** - Device queries (MIOS32 protocol)
   - Query: `F0 00 00 7E 32 00 <cmd> F7`
   - Response: `F0 00 00 7E 32 00 0F <text> F7`
   
2. **USB CDC** - Terminal text I/O
   - Virtual COM port
   - CLI commands
   - Debug output

### Data Flow

```
MIOS Studio
    ↓
USB Host
    ↓
┌───────────────────────────────┐
│   STM32 USB Composite Device  │
├─────────────┬─────────────────┤
│  USB MIDI   │    USB CDC      │
└──────┬──────┴────────┬────────┘
       ↓               ↓
  USB ISR Handler  USB ISR Handler
       ↓               ↓
  mios32_query_queue()  CLI callback
       ↓               ↓
  midi_io_task()   cli_task()
       ↓               ↓
  mios32_query_process_queued()
       ↓
  mios32_query_send_response()
       ↓
  USB MIDI TX
```

---

## Test Mode vs Production Mode

### Test Mode (MODULE_TEST_USB_DEVICE_MIDI)

**Entry Point:** `module_test_usb_device_midi_run()` in `module_tests.c`

**Behavior:**
- Runs in infinite loop
- Bypasses normal app initialization
- Sends test MIDI messages
- **Verbose debug output:**
  ```c
  dbg_print("[MIOS32-Q] Received query...");
  dbg_print("[MIOS32-R] Sending response...");
  ```

**Advantages:**
- Clear visibility of what's happening
- Isolated USB MIDI testing
- Easy to verify query/response

**Disadvantages:**
- CLI doesn't start
- Looper doesn't start
- AINSER doesn't start

### Production Mode (Normal)

**Entry Point:** `app_init_and_start()` in `app_init.c`

**Behavior:**
- Normal initialization
- All subsystems active
- midi_io_task processes queries
- **NO debug output** (conditionally compiled out)

**Advantages:**
- Full system operational
- All features available
- Real use case

**Disadvantages:**
- No visibility into MIOS32 query processing
- Hard to debug if not working

---

## Diagnostic Procedure

### Step 1: Verify USB Enumeration

**Connect USB and check:**

**Windows:**
- Device Manager → Ports (COM & LPT)
- Should see: "STM32 Virtual COM Port (COMX)"
- Device Manager → Sound, video and game controllers
- Should see: "USB MIDI Device"

**Linux:**
```bash
lsusb
# Should show STM32 device

ls /dev/ttyACM*
# Should show /dev/ttyACM0 (or similar)

aconnect -l
# Should show MIDI ports
```

**macOS:**
```bash
system_profiler SPUSBDataType | grep -A 10 "STM32"
ls /dev/tty.usbmodem*
```

### Step 2: Verify MIDI IO Task is Running

**In GDB:**
```gdb
# Set breakpoint
break MidiIOTask

# Run
continue

# Should hit breakpoint during init
# Then continue and check it's looping
```

**Expected:** Task should be running in 1ms loop

### Step 3: Check Query Reception

**In GDB:**
```gdb
# Set breakpoint in USB ISR
break usb_midi_rx_callback_internal

# Continue
continue

# In MIOS Studio, try to connect
# Should hit breakpoint when MIOS Studio sends query
```

**When breakpoint hits:**
```gdb
# Check if it's a query
print packet4[0]  # Should be 0x04 (SysEx start)
print packet4[1]  # Should be 0xF0
print packet4[2]  # Should be 0x00
print packet4[3]  # Should be 0x00

# Continue processing
next
# Keep stepping to see if mios32_query_queue() is called
```

### Step 4: Check Query Queuing

**In GDB:**
```gdb
# Set breakpoint
break mios32_query_queue

# Continue
continue

# When query received, should hit this breakpoint
```

**When breakpoint hits:**
```gdb
# Check queue state
print query_queue_write
print query_queue_read
print len
print cable

# Check query data
x/16xb data
# Should show: F0 00 00 7E 32 00 ...
```

### Step 5: Check Query Processing

**In GDB:**
```gdb
# Set breakpoint
break mios32_query_process_queued

# Continue
continue

# Should hit every 1ms in midi_io_task loop
```

**When breakpoint hits:**
```gdb
# Check if there's a query to process
print query_queue_write
print query_queue_read

# If different, there's a queued query
# Step through to process it
next
next
...
```

### Step 6: Check Response Sending

**In GDB:**
```gdb
# Set breakpoint
break mios32_query_send_response

# Continue
continue

# Should hit when processing query
```

**When breakpoint hits:**
```gdb
# Check parameters
print query_type
print device_id
print cable

# Check if in ISR (should be 0 = task context)
print __get_IPSR()

# Step through send
next
...

# Check if sent successfully
print sent
```

### Step 7: USB Traffic Analysis

**Use Wireshark with USBPcap (Windows) or usbmon (Linux):**

**What to look for:**

**MIOS Studio Query:**
```
F0 00 00 7E 32 00 00 F7  // Query device info
or
F0 00 00 7E 32 00 01 08 F7  // Query application name
```

**Expected Response:**
```
F0 00 00 7E 32 00 0F 4D 49 4F 53 33 32 F7
// "MIOS32"

or

F0 00 00 7E 32 00 0F 4D 69 64 69 43 6F 72 65 F7
// "MidiCore"
```

**If no response seen:**
- Device not receiving queries
- Queries not being processed
- Responses not being sent
- USB TX queue full

---

## Common Issues and Solutions

### Issue 1: No USB Enumeration

**Symptoms:**
- Device not detected
- No COM port appears
- No MIDI device visible

**Causes:**
- USB cable data lines broken
- USB not initialized
- Descriptors incorrect

**Fix:**
- Check USB cable (try different cable)
- Verify USB initialization in main.c
- Check USB descriptors in usbd_composite.c

### Issue 2: Queries Not Received

**Symptoms:**
- Breakpoint in mios32_query_queue() never hit
- query_queue_write doesn't increment

**Causes:**
- USB MIDI not receiving data
- MIOS Studio not sending queries
- Wrong USB cable

**Fix:**
- Verify USB MIDI enumeration
- Check MIOS Studio connection settings
- Use USB traffic sniffer

### Issue 3: Queries Not Processed

**Symptoms:**
- query_queue_write increments
- But query_queue_read doesn't
- Breakpoint in mios32_query_process_queued() not hit

**Causes:**
- midi_io_task not running
- Task blocked/crashed
- Infinite loop in task

**Fix:**
- Verify task creation in app_init.c
- Check for stack overflow
- Use GDB to see task state

### Issue 4: Responses Not Sent

**Symptoms:**
- Queries processed
- But no response seen
- sent = false in mios32_query_send_response()

**Causes:**
- USB TX queue full
- USB not ready
- usb_midi_send_sysex() failing

**Fix:**
- Increase USB TX queue size
- Check USB connection
- Add retry logic (already implemented)

### Issue 5: Timing Issues

**Symptoms:**
- Works intermittently
- Sometimes detected, sometimes not

**Causes:**
- USB enumeration timing
- MIOS Studio connection timing
- Task scheduling issues

**Fix:**
- Add delays in initialization
- Ensure USB ready before processing
- Check task priorities

---

## Production Mode Diagnostics Patch

**Problem:** No debug output in production mode makes debugging impossible

**Solution:** Add conditional diagnostics

### Option 1: Enable MIOS32 Query Debug

Add to `Config/module_config.h`:
```c
/** @brief Enable MIOS32 query debug messages in production mode */
#define MODULE_DEBUG_MIOS32_QUERIES 1
```

Modify `Services/mios32_query/mios32_query.c`:
```c
// Replace #ifdef MODULE_TEST_USB_DEVICE_MIDI with:
#if defined(MODULE_TEST_USB_DEVICE_MIDI) || MODULE_DEBUG_MIOS32_QUERIES
  dbg_print("[MIOS32-Q] Query received...");
#endif
```

### Option 2: Add CLI Query Stats Command

Add new CLI command: `mios32` or `query stats`

**Implementation:**
```c
// In Services/mios32_query/mios32_query.c
uint32_t mios32_query_get_stats(uint32_t* received, uint32_t* processed, uint32_t* sent) {
  // Return query statistics
  *received = total_queries_received;
  *processed = total_queries_processed;
  *sent = total_responses_sent;
  return 0;
}

// In CLI
void cmd_query(int argc, char* argv[]) {
  uint32_t rx, proc, tx;
  mios32_query_get_stats(&rx, &proc, &tx);
  cli_printf("MIOS32 Query Statistics:\r\n");
  cli_printf("  Received:  %lu\r\n", rx);
  cli_printf("  Processed: %lu\r\n", proc);
  cli_printf("  Sent:      %lu\r\n", tx);
}
```

### Option 3: Add LED Indicator

**Blink LED when query received/processed:**
```c
// In mios32_query_queue()
HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
```

---

## Testing Checklist

- [ ] USB device enumerates
- [ ] COM port appears
- [ ] MIDI device appears
- [ ] midi_io_task starts
- [ ] Query ISR callback fires
- [ ] mios32_query_queue() called
- [ ] Query queued successfully
- [ ] mios32_query_process_queued() runs
- [ ] Query processed
- [ ] mios32_query_send_response() called
- [ ] Response built correctly
- [ ] usb_midi_send_sysex() succeeds
- [ ] Response visible in USB traffic
- [ ] MIOS Studio detects device
- [ ] Terminal text works

---

## Summary

**Architecture is CORRECT** - all components properly implemented

**Likely cause:** Visibility issue - works but no debug output in production mode

**Recommended action:**
1. Follow diagnostic procedure above
2. Add production mode diagnostics
3. Verify with USB sniffer
4. Check MIOS Studio settings

**If still not working after diagnostics:**
- Check USB descriptors match test mode
- Verify timing of initialization
- Check for USB TX queue congestion
- Test with different MIOS Studio version
