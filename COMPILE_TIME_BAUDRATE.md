# Compile-Time Baudrate Configuration

If you're experiencing persistent baudrate issues with runtime reconfiguration, use this compile-time approach instead.

## Method 1: Compile-Time Flag (RECOMMENDED)

Add this to your build configuration:

### STM32CubeIDE
1. Right-click project → Properties
2. C/C++ Build → Settings → MCU GCC Compiler → Preprocessor
3. Add to "Defined symbols": `TEST_MODE_DEBUG_UART=1`
4. Click Apply and Close
5. Clean and rebuild project

### Makefile
Add to CFLAGS:
```makefile
CFLAGS += -DTEST_MODE_DEBUG_UART=1
```

### Command Line
```bash
arm-none-eabi-gcc ... -DTEST_MODE_DEBUG_UART=1 ...
```

## Method 2: Edit main.c Directly

Open `Core/Src/main.c` and find the `MX_UART5_Init()` function (around line 607).

Change:
```c
#ifdef TEST_MODE_DEBUG_UART
  huart5.Init.BaudRate = 115200;  // Test mode: 115200 for debug terminal
#else
  huart5.Init.BaudRate = 31250;   // Production mode: MIDI baudrate
#endif
```

To:
```c
  huart5.Init.BaudRate = 115200;  // Always 115200 for test mode
```

## Verification

After compiling, the UART will be initialized to 115200 from the start.
Terminal should show clean output immediately.

## Switching Back to Production

Remove the `-DTEST_MODE_DEBUG_UART=1` flag and rebuild.
UART5 will return to 31250 baud for MIDI.

---

**Why This Works:**
- Baudrate set at compile-time, before any code runs
- No runtime reconfiguration needed
- No timing issues or initialization race conditions
- 100% reliable
