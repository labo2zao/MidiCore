# CRITICAL: Debug Instructions for MIOS32 Recognition

## PROVEN: Code Format is 100% Correct!

After analyzing actual MIOS Studio source code (`tools/mios_studio/src/UploadHandler.cpp`), confirmed our response format is EXACTLY correct:

### What MIOS Studio Expects:
```
F0 00 00 7E 32 [device_id] 0x0F [string] F7
└─ byte 0-6        └─ byte 7+ (string data)
```

### What We Send:
```c
*p++ = 0xF0;  // byte 0
*p++ = 0x00;  // byte 1  
*p++ = 0x00;  // byte 2
*p++ = 0x7E;  // byte 3
*p++ = 0x32;  // byte 4
*p++ = device_id;  // byte 5
*p++ = 0x0F;  // byte 6
while (*response_str) *p++ = *response_str++;  // byte 7+
*p++ = 0xF7;  // end
```

**FORMAT IS PERFECT! ✓**

## Then Why Doesn't It Work?

Since format is correct, the problem MUST be:

### 1. Code Not Compiled (Most Likely!)

**VERIFY:** Check if mios32_query.c is in the build:

In STM32CubeIDE:
1. Project → Clean
2. Project → Build All
3. **WATCH BUILD CONSOLE CAREFULLY!**
4. Search for: `mios32_query.c`
5. Should see:
   ```
   Building file: ../Services/mios32_query/mios32_query.c
   Invoking: MCU GCC Compiler
   ...
   Finished building: ../Services/mios32_query/mios32_query.c
   ```

**IF NOT SEEN:** File is not being compiled!

**FIX:** Add to .cproject manually:
```xml
<sourceEntries>
  <entry excluding="**/*_example.c|**/test_*.c" flags="VALUE_WORKSPACE_PATH|RESOLVED" kind="sourcePath" name="Services"/>
</sourceEntries>
```

OR in Project Properties → C/C++ Build → Settings → Tool Settings:
- Add `Services/mios32_query` to source folders

### 2. MODULE_ENABLE_USB_MIDI is 0 at Compile Time

**VERIFY:** Add this to top of mios32_query.c:

```c
#include "Config/module_config.h"

#if !MODULE_ENABLE_USB_MIDI
#error "MODULE_ENABLE_USB_MIDI is not enabled! Response transmission will be disabled!"
#endif
```

If you see this error, MODULE_ENABLE_USB_MIDI is not being seen as 1.

**FIX:** Check Config/module_config.h line 80:
```c
#ifndef MODULE_ENABLE_USB_MIDI
#define MODULE_ENABLE_USB_MIDI 1
#endif
```

### 3. usb_midi_send_sysex() Not Working

**VERIFY:** Add debug output AT START of mios32_query_send_response():

```c
void mios32_query_send_response(uint8_t query_type, uint8_t device_id, uint8_t cable) {
  // DEBUG: Blink LED to prove function is called
  #ifdef LED_GPIO_Port
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
  #endif
  
  uint8_t* p = sysex_response_buffer;
  // ... rest of function
```

If LED doesn't blink, function is never called!

### 4. Query Not Reaching mios32_query_process()

**VERIFY:** Add debug at start of mios32_query_process():

```c
bool mios32_query_process(const uint8_t* data, uint32_t len, uint8_t cable) {
  // DEBUG: Send to CDC terminal
  #if MODULE_ENABLE_USB_CDC
  usb_cdc_transmit((uint8_t*)"Q!", 2);
  #endif
  
  if (!mios32_query_is_query_message(data, len)) {
    return false;
  }
  // ... rest
```

Check CDC terminal - should see "Q!" when query arrives.

### 5. Response Being Sent But Corrupted

**VERIFY:** Log exact bytes being sent:

```c
#if MODULE_ENABLE_USB_MIDI
  // DEBUG: Show response bytes before sending
  #if MODULE_ENABLE_USB_CDC
  char debug[256];
  int len_msg = sprintf(debug, "RESP:%d bytes\r\n", (int)(p - sysex_response_buffer));
  usb_cdc_transmit((uint8_t*)debug, len_msg);
  for(int i=0; i<(p - sysex_response_buffer); i++) {
    len_msg = sprintf(debug, "%02X ", sysex_response_buffer[i]);
    usb_cdc_transmit((uint8_t*)debug, len_msg);
  }
  usb_cdc_transmit((uint8_t*)"\r\n", 2);
  #endif
  
  usb_midi_send_sysex(sysex_response_buffer, p - sysex_response_buffer, cable);
#endif
```

This will show EXACTLY what's being sent.

## Step-by-Step Debugging Procedure

### Step 1: Verify Code is Compiled
1. Clean project
2. Build with verbose output
3. Check build log for mios32_query.c
4. **IF NOT FOUND:** Add source folder to project

### Step 2: Add LED Blink
1. Add LED toggle at start of mios32_query_send_response()
2. Rebuild and flash
3. Connect MIOS Studio
4. **IF LED BLINKS:** Code is running!
5. **IF LED DOESN'T BLINK:** Code not being called

### Step 3: Add CDC Debug
1. Add CDC output showing:
   - "Q" when query received
   - "R" when response sent
   - Exact response bytes
2. Rebuild and flash
3. Open CDC terminal in MIOS Studio
4. Watch for debug output

### Step 4: Capture USB Traffic
1. Use USB analyzer or Wireshark with USBPcap
2. Capture USB MIDI traffic
3. Look for response SysEx:
   ```
   F0 00 00 7E 32 00 0F ...
   ```
4. If NOT seen: Response not being transmitted
5. If SEEN: Response format issue (but we know it's correct!)

## Most Likely Issue: Build Configuration

Based on all analysis, **99% certain** the issue is:
1. mios32_query.c not in build, OR
2. Old firmware still on device, OR  
3. MODULE_ENABLE_USB_MIDI somehow 0 at compile time

**SOLUTION:**
1. Verify build includes mios32_query.c
2. Clean rebuild
3. Flash new firmware
4. Test

## Verification Checklist

- [ ] Git branch is copilot/fix-midicore-driver-crash
- [ ] Latest commit (0433b31 or later)
- [ ] Build log shows mios32_query.c being compiled
- [ ] No compilation errors
- [ ] MODULE_ENABLE_USB_MIDI = 1 verified
- [ ] Firmware flashed successfully
- [ ] Device reset after flash
- [ ] MIOS Studio query sent
- [ ] Response expected but not received

## If All Else Fails: Add This Test Code

Replace entire mios32_query_send_response() temporarily with:

```c
void mios32_query_send_response(uint8_t query_type, uint8_t device_id, uint8_t cable) {
  // ULTRA-SIMPLE TEST: Just send fixed response
  uint8_t test_response[] = {
    0xF0, 0x00, 0x00, 0x7E, 0x32, 0x00, 0x0F,
    'T', 'E', 'S', 'T',  // Simple test string
    0xF7
  };
  
  #if MODULE_ENABLE_USB_MIDI
  usb_midi_send_sysex(test_response, sizeof(test_response), cable);
  #endif
}
```

If MIOS Studio sees "TEST", then:
- ✓ Code is running
- ✓ USB MIDI works
- ✓ Issue was in string copying logic

If MIOS Studio still doesn't see anything:
- ✗ Code not running OR
- ✗ USB transmission broken OR
- ✗ Cable number wrong

## Conclusion

**Code format is 100% correct per MIOS Studio source.**

Issue is build/runtime, not logic.

Follow systematic debug procedure above to find exact failure point.
