# DEFINITIVE DEBUG TEST - Prove What's Actually Happening

## Problem
- MidiCore not recognized by MIOS Studio
- Terminal not working
- But works fine with real MIOS32 device

## This Test Will Prove
1. If firmware is actually flashed
2. If CDC init is called
3. If MIOS32 query is received
4. If response is sent

## Add This Code

### Test 1: Startup Blink (Proves Firmware is Running)

Add to `App/app_init.c` in `app_init_and_start()` AFTER all init:

```c
// DEFINITIVE TEST: Blink LED on startup
for (int i = 0; i < 10; i++) {
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);  // LED on
  HAL_Delay(100);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);  // LED off
  HAL_Delay(100);
}
```

**Expected:** LED blinks 10 times at startup
**If this fails:** Firmware not flashed or not running

### Test 2: CDC Startup Message (Proves CDC Works)

Add to `App/app_init.c` AFTER usb_cdc_init():

```c
// DEFINITIVE TEST: Send startup message
osDelay(3000);  // Wait for USB enum
const char *msg = "\r\n\r\n=== MIDICORE STARTUP ===\r\n";
usb_cdc_send((const uint8_t*)msg, strlen(msg));
```

**Expected:** Terminal shows "=== MIDICORE STARTUP ==="
**If this fails:** CDC not working or not enumerated

### Test 3: MIOS32 Query Detection (Proves Query Received)

Add to `Services/mios32_query/mios32_query.c` at START of `mios32_query_process()`:

```c
bool mios32_query_process(const uint8_t *data, uint32_t len, uint8_t cable) {
  // DEFINITIVE TEST: Blink LED when query received
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);  // Orange LED
  
  // DEFINITIVE TEST: Log to CDC
  const char *msg = "[QUERY RX]\r\n";
  usb_cdc_send((const uint8_t*)msg, strlen(msg));
  
  // ... rest of function
```

**Expected:** Orange LED stays on, terminal shows "[QUERY RX]"
**If this fails:** Query not reaching mios32_query_process()

### Test 4: Response Sent (Proves TX Works)

Add to `Services/mios32_query/mios32_query.c` AFTER usb_midi_send_sysex():

```c
// DEFINITIVE TEST: Confirm response sent
HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);  // Red LED
const char *msg = "[RESP TX]\r\n";
usb_cdc_send((const uint8_t*)msg, strlen(msg));
```

**Expected:** Red LED on, terminal shows "[RESP TX]"
**If this fails:** Response not being sent

## Test Procedure

1. **Add all test code above**
2. **Clean rebuild** (Project → Clean, then Build All)
3. **Flash to device**
4. **Watch LEDs at startup** - should blink 10 times
5. **Open MIOS Studio terminal**
6. **Look for startup message** - should see "=== MIDICORE STARTUP ==="
7. **Connect MIOS Studio MIDI** - triggers query
8. **Watch LEDs** - Orange should light (query), then Red (response)
9. **Check terminal** - should see "[QUERY RX]" and "[RESP TX]"

## Interpreting Results

### If LED doesn't blink at startup:
❌ Firmware not flashed or not running
→ Solution: Verify flash, check power, check ST-Link

### If LED blinks but no terminal message:
❌ CDC not working
→ Solution: Check USB enumeration, verify MODULE_ENABLE_USB_CDC=1

### If terminal works but no "[QUERY RX]":
❌ Query not reaching mios32_query_process()
→ Solution: Check usb_midi_process_rx_queue() is being called

### If "[QUERY RX]" but no "[RESP TX]":
❌ Response not being sent
→ Solution: Check usb_midi_send_sysex(), check TX queue

### If all messages appear but device not recognized:
❌ Response format wrong or timing issue
→ Solution: Capture USB traffic, compare with real MIOS32

## USB Traffic Capture (Advanced)

If all LEDs/messages appear but still not recognized:

### Windows:
```
1. Download USBPcap or Wireshark with USB capture
2. Start capture on MidiCore USB device
3. Connect MIOS Studio
4. Save capture as midicore.pcap
5. Repeat with real MIOS32 device
6. Compare query/response packets byte-by-byte
```

### Linux:
```bash
# Capture MidiCore
sudo usbmon -f -i 1 | tee midicore.log

# Repeat with MIOS32
sudo usbmon -f -i 1 | tee mios32.log

# Compare
diff midicore.log mios32.log
```

## Most Likely Issues (Based on Tests)

1. **CDC not initialized** - Test 2 will fail
2. **Query not processed** - Test 3 will fail  
3. **Response not sent** - Test 4 will fail
4. **Response format wrong** - All tests pass but device not recognized

## Next Steps

After running these tests, report:
1. ✅/❌ LED blinks at startup?
2. ✅/❌ Terminal startup message?
3. ✅/❌ "[QUERY RX]" appears?
4. ✅/❌ "[RESP TX]" appears?

This will pinpoint EXACTLY where the failure occurs!
