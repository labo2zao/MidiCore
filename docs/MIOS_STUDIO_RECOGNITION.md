# MIOS Studio Recognition Troubleshooting Guide

## System Architecture

### MidiCore Query Hook System

The MidiCore query system is **properly hooked** into the USB MIDI processing chain:

```
┌─────────────────────────────────────────────────────────────┐
│                    USB MIDI RX ISR                           │
│  (Services/usb_midi/usb_midi.c:389-391)                     │
│                                                               │
│  1. Receives USB MIDI packet                                 │
│  2. Assembles SysEx message                                  │
│  3. Checks: midicore_query_is_query_message()              │
│  4. If query: midicore_query_queue() [ISR-SAFE]            │
│                                                               │
└─────────────────────┬─────────────────────────────────────────┘
                      │
                      ▼
            ┌─────────────────┐
            │  Query Queue    │
            │ (Circular Buffer)│
            │   Max 4 queries │
            └─────────┬────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────┐
│              MidiIOTask (Every 1ms)                          │
│  (App/midi_io_task.c:65-69)                                 │
│                                                               │
│  1. usb_midi_process_rx_queue()                             │
│  2. midicore_query_process_queued() ← Processes queue      │
│  3. Sends responses via USB MIDI                            │
│                                                               │
└─────────────────────────────────────────────────────────────┘
```

### Query Detection Flow

```c
// In USB MIDI ISR (usb_midi.c:389-391)
if (midicore_query_is_query_message(buf->buffer, buf->pos)) {
    // Queue for processing from task context (ISR-safe)
    midicore_query_queue(buf->buffer, buf->pos, cable);
    // Don't route query messages - they'll be processed from queue
}
```

### Query Processing Flow

```c
// In MidiIOTask (midi_io_task.c:69)
midicore_query_process_queued();
```

## Comparison: Test Mode vs Normal Mode

### MODULE_TEST_USB_DEVICE_MIDI (Works!)

```c
for (;;) {
    usb_midi_process_rx_queue();       // Process USB RX
    midicore_query_process_queued();   // Process queries
    osDelay(10);                        // 10ms delay
}
```

### Normal Mode (Should work!)

```c
for (;;) {
    usb_midi_process_rx_queue();       // Process USB RX
    midicore_query_process_queued();   // Process queries
    usb_cdc_process_rx_queue();        // Process CDC
    midi_din_tick();                    // Other tasks
    looper_tick_1ms();
    osDelay(1);                         // 1ms delay (FASTER!)
}
```

**Both call the SAME functions!** Normal mode is actually FASTER (1ms vs 10ms).

## Diagnostic Procedures

### Step 1: Enable Debug Output

In `Config/module_config.h`:

```c
#define MODULE_DEBUG_MIDICORE_QUERIES 1
```

This enables query reception/response debug messages.

### Step 2: Flash Firmware and Connect UART

1. Flash firmware with debug enabled
2. Connect UART terminal (115200 baud, PC12/PD2)
3. Reset device and capture startup output

### Step 3: Expected UART Output

```
[MIDI-TASK] MidiIOTask started
[MIDI-TASK] USB MIDI Status:
  Ready: YES
  TX Queue Size: 16 packets
[MIDI-TASK] MidiCore query processing enabled
[MIDI-TASK] Sending test message to MIOS Studio terminal...
[MIDI-TASK] Test message sent successfully
```

### Step 4: Open MIOS Studio and Monitor

1. Open MIOS Studio application
2. Look for "MidiCore" in device list
3. Watch UART terminal for query messages

**Expected when MIOS Studio queries:**

```
[MIDICORE-Q] Received query len:8 cable:0
[MIDICORE-Q] dev_id:00 cmd:00 type:01
[MIDICORE-R] Sending type:01 "MidiCore" cable:0
[MIDICORE-R] Sent 15 bytes (success=1)
```

### Step 5: Troubleshooting Matrix

| UART Shows | Device Appears | Diagnosis |
|------------|----------------|-----------|
| ✅ Query received | ❌ No device | Response not sent properly |
| ❌ No query | ❌ No device | MIOS Studio not sending queries |
| ✅ Query + Response | ❌ No device | Protocol mismatch |
| ✅ Query + Response | ✅ Device shown | **WORKING!** |

## Common Issues and Solutions

### Issue 1: USB MIDI Not Ready

**Symptom:**
```
[MIDI-TASK] USB MIDI Status:
  Ready: NO
```

**Solution:**
- Check USB cable connection
- Verify USB enumeration completed
- Wait longer after boot before testing

### Issue 2: No Queries Received

**Symptom:**
- UART shows no `[MIDICORE-Q]` messages
- MIOS Studio doesn't show device

**Possible Causes:**
1. **MIOS Studio not sending queries**
   - Check MIOS Studio is running
   - Try "Rescan" button in MIOS Studio

2. **USB MIDI port not visible**
   - Check Windows Device Manager
   - Verify "USB MIDI Device" appears

3. **Wrong USB port**
   - Try different USB port
   - Disconnect/reconnect

### Issue 3: Queries Received But No Device

**Symptom:**
```
[MIDICORE-Q] Received query len:8 cable:0
[MIDICORE-R] Sending type:01 "MidiCore"
```
But device doesn't appear in MIOS Studio.

**Possible Causes:**
1. **Response format mismatch**
   - Check SysEx response format
   - Verify device ID (0x32)

2. **USB TX queue full**
   - Check queue drops in diagnostics
   - Reduce other MIDI traffic

3. **Timing issue**
   - Response sent too late
   - Try reducing MidiIOTask delay

### Issue 4: Task Not Running

**Symptom:**
- No heartbeat messages
- No query processing

**Solution:**
```
[MIDI-TASK] Heartbeat: Task running OK (loops: 10000)
```

If missing:
- Check heap exhaustion (need 36KB+)
- Verify MidiIOTask created
- Check for task starvation

## Debug Commands

### Check USB MIDI Status

Look for periodic heartbeat (every 10 seconds):

```
[MIDI-TASK] Heartbeat: Task running OK (loops: 9876)
[MIDI-TASK] USB MIDI: READY, Queue: 0/16, Drops: 0
```

### Manual Query Test

From UART debug prompt, you can manually test:

```
# Send test query (if debug CLI available)
query_test
```

### Check Task Status

```
# Stack monitor shows task is running
[STACK] MidiIO: 1234/2048 bytes free (60% free)
```

## Protocol Reference

### MidiCore Query Format

**Query (from MIOS Studio):**
```
F0 00 00 7E 32 00 00 01 F7
│  │  │  │  │  │  │  │  │
│  │  │  │  │  │  │  │  └─ End of SysEx
│  │  │  │  │  │  │  └──── Query type (0x01 = device name)
│  │  │  │  │  │  └─────── Command (0x00 = query)
│  │  │  │  │  └────────── Device instance (0x00)
│  │  │  │  └───────────── Device ID (0x32 = MidiCore)
│  │  │  └──────────────── Manufacturer ID (7E = Universal)
│  │  └─────────────────── Manufacturer ID
│  └────────────────────── Manufacturer ID
└───────────────────────── Start of SysEx
```

**Response (from MidiCore):**
```
F0 00 00 7E 32 00 0F 01 4D 69 64 69 43 6F 72 65 F7
│  │  │  │  │  │  │  │  │                         │
│  │  │  │  │  │  │  │  └─ "MidiCore" in ASCII   │
│  │  │  │  │  │  │  └──── Query type (0x01)     │
│  │  │  │  │  │  └─────── Response (0x0F)       │
│  │  │  │  │  └────────── Device instance        │
│  │  │  │  └───────────── Device ID (0x32)       │
│  │  │  └──────────────── Manufacturer ID        │
│  │  └─────────────────── Manufacturer ID        │
│  └────────────────────── Manufacturer ID        │
└───────────────────────── Start of SysEx         │
                                        End of SysEx ┘
```

## System Requirements

### USB Configuration

```c
// Required in module_config.h
#define MODULE_ENABLE_USB_MIDI 1
#define MODULE_ENABLE_USB_CDC 1  // For terminal (optional)
```

### Task Configuration

```c
// MidiIOTask priority
.priority = osPriorityAboveNormal  // Preempts other tasks
.stack_size = 2048                  // 2KB for query processing
```

### Heap Configuration

```c
// In FreeRTOSConfig.h
#define configTOTAL_HEAP_SIZE (36 * 1024)  // 36KB minimum
```

## Next Steps for User

If MIOS Studio still doesn't recognize MidiCore:

1. ✅ **Verify architecture** - System is correctly hooked
2. 🔍 **Enable debug** - Set MODULE_DEBUG_MIDICORE_QUERIES=1
3. 📊 **Capture logs** - Save UART output during connection attempt
4. 📝 **Report findings:**
   - Are queries received? (Look for `[MIDICORE-Q]`)
   - Are responses sent? (Look for `[MIDICORE-R]`)
   - Any USB errors or warnings?
   - USB MIDI ready status?

5. 🐛 **Possible issues to check:**
   - USB enumeration timing
   - Response format compatibility
   - Cable number mismatch
   - Task scheduling delays

---

**The hook system is correct - runtime debugging will find the issue!**
