# Debug Output Configuration Guide

## Choosing Debug Output Method

MidiCore supports multiple debug output methods. Choose the one that best fits your workflow.

---

## Quick Selection

Edit `Config/module_config.h` and set `MODULE_DEBUG_OUTPUT`:

```c
// OPTION 1: SWV via ST-Link (RECOMMENDED for USB MIDI devices)
#define MODULE_DEBUG_OUTPUT DEBUG_OUTPUT_SWV

// OPTION 2: USB CDC Virtual COM port (MIOS Studio compatible)
#define MODULE_DEBUG_OUTPUT DEBUG_OUTPUT_USB_CDC

// OPTION 3: Hardware UART (fallback)
#define MODULE_DEBUG_OUTPUT DEBUG_OUTPUT_UART

// OPTION 4: Disabled (production)
#define MODULE_DEBUG_OUTPUT DEBUG_OUTPUT_NONE
```

---

## Option 1: SWV (Serial Wire Viewer) ⭐ RECOMMENDED

### Advantages
- ✅ **No USB conflicts** - Uses ST-Link debug interface, not USB
- ✅ **Always available** - Works even if USB enumeration fails
- ✅ **No driver issues** - Built into ST-Link
- ✅ **High bandwidth** - Up to 2 MHz SWO clock
- ✅ **Real-time** - Low latency debug traces
- ✅ **Best for USB MIDI** - Debug without interfering with USB MIDI/CDC

### Disadvantages
- ⚠️ Requires ST-Link connected
- ⚠️ Only works in debug session (not standalone)
- ⚠️ Requires STM32CubeIDE or OpenOCD

### Setup in STM32CubeIDE

1. **Enable SWV in Debug Configuration:**
   - Right-click project → Debug As → Debug Configurations
   - Select your debug configuration
   - Go to **Debugger** tab
   - Scroll down to **Serial Wire Viewer (SWV)** section
   - ☑ **Enable**
   - Set **Core Clock:** 168000000 (168 MHz for STM32F407)
   - Set **SWO Clock:** 2000000 (2 MHz recommended)

2. **Configure ITM Port:**
   - In SWV section, under **ITM Stimulus Ports**
   - Port 0: ☑ **Enabled** (used for debug output)
   - Ports 1-31: Leave disabled

3. **View Output:**
   - Start debug session (F11)
   - Window → Show View → SWV → **SWV ITM Data Console**
   - Click **Configure** button (gear icon)
   - Port 0: ☑ Enabled
   - Click **Start Trace** (red button)
   - Debug output will appear in real-time

### Configuration

```c
// In Config/module_config.h:
#define MODULE_DEBUG_OUTPUT DEBUG_OUTPUT_SWV
```

That's it! No other changes needed.

### Troubleshooting

**Problem:** No output in SWV console

**Solutions:**
1. Check SWV is enabled in debug config (Debugger tab)
2. Check Port 0 is enabled in ITM Data Console config
3. Click "Start Trace" button (red button)
4. Verify core clock is correct (168 MHz for STM32F407)
5. Try lower SWO clock (500000 or 1000000)

**Problem:** Garbled output

**Solutions:**
1. Core clock setting doesn't match actual clock
2. SWO clock too high (max = Core Clock / 4)
3. Try 2000000 Hz (2 MHz) - most reliable

---

## Option 2: USB CDC (Virtual COM Port)

### Advantages
- ✅ **MIOS Studio compatible** - Works with MIOS terminal
- ✅ **Standalone operation** - No debugger needed
- ✅ **Standard serial port** - Use any terminal software
- ✅ **Same as MIOS32** - Familiar workflow

### Disadvantages
- ⚠️ **USB conflicts** - Shares USB with MIDI (can interfere during debugging)
- ⚠️ **Driver dependent** - Requires USB CDC driver
- ⚠️ **Enumeration required** - Won't work if USB fails
- ⚠️ **May affect timing** - USB interrupts can cause jitter

### Setup

1. **Enable in configuration:**
```c
// In Config/module_config.h:
#define MODULE_DEBUG_OUTPUT DEBUG_OUTPUT_USB_CDC
#define MODULE_ENABLE_USB_CDC 1  // Must also be enabled
```

2. **Connect USB cable** to STM32F407

3. **Open terminal:**
   - **MIOS Studio:** Should auto-detect
   - **Windows:** Device Manager → Ports (COM & LPT) → Find COM port
   - **macOS:** `/dev/tty.usbmodem*`
   - **Linux:** `/dev/ttyACM*`
   - **PuTTY/screen/minicom:** Connect to COM port at any baud rate

4. **View output** in terminal

### When to Use
- Using MIOS Studio for development
- Need standalone debug (no ST-Link)
- Debugging USB CDC functionality itself
- Want MIOS32-compatible workflow

---

## Option 3: Hardware UART

### Advantages
- ✅ **Always available** - No USB or debugger needed
- ✅ **Independent** - Doesn't interfere with USB or debugging
- ✅ **Standard** - Works with any UART adapter

### Disadvantages
- ⚠️ **Extra hardware** - Requires UART-to-USB adapter
- ⚠️ **Wiring** - Need to connect TX/RX/GND
- ⚠️ **Port conflict** - Uses one MIDI DIN port

### Setup

1. **Configure port:**
```c
// In Config/module_config.h:
#define MODULE_DEBUG_OUTPUT DEBUG_OUTPUT_UART

// In App/tests/test_debug.h:
#define TEST_DEBUG_UART_PORT 1  // USART3 (PD8/PD9)
#define TEST_DEBUG_UART_BAUD 115200
```

2. **Connect UART adapter:**
   - TX (STM32) → RX (Adapter)
   - RX (STM32) → TX (Adapter)  
   - GND → GND

3. **Open terminal** at 115200 baud

### Default Port
- Port 1 = USART3 (PD8/PD9) - Recommended
- Port 0 = USART2 (PA2/PA3)
- Port 3 = UART5 (PC12/PD2)

---

## Option 4: Disabled (Production)

### Configuration
```c
#define MODULE_DEBUG_OUTPUT DEBUG_OUTPUT_NONE
```

### Effects
- No debug output compiled
- Saves Flash/RAM
- Minimal overhead
- Production builds

---

## Comparison Table

| Feature | SWV | USB CDC | UART | None |
|---------|-----|---------|------|------|
| **No USB conflicts** | ✅ | ❌ | ✅ | ✅ |
| **Standalone operation** | ❌ | ✅ | ✅ | ✅ |
| **MIOS Studio compatible** | ❌ | ✅ | ❌ | ❌ |
| **Works during USB issues** | ✅ | ❌ | ✅ | N/A |
| **Extra hardware needed** | ST-Link | USB | UART adapter | None |
| **Best for debugging** | ✅ | ⚠️ | ⚠️ | ❌ |
| **Best for production** | ❌ | ✅ | ❌ | ✅ |
| **Bandwidth** | High | Medium | Low | N/A |

---

## Recommendations

### For Development/Debugging
**Use SWV** - Most reliable, no USB conflicts

```c
#define MODULE_DEBUG_OUTPUT DEBUG_OUTPUT_SWV
```

### For MIOS Studio Users
**Use USB CDC** - Compatible with MIOS terminal

```c
#define MODULE_DEBUG_OUTPUT DEBUG_OUTPUT_USB_CDC
#define MODULE_ENABLE_USB_CDC 1
```

### For Production
**Disable or use USB CDC** - For user terminal access

```c
#define MODULE_DEBUG_OUTPUT DEBUG_OUTPUT_NONE  // Or DEBUG_OUTPUT_USB_CDC for terminal
```

---

## Switching Between Methods

You can switch at any time by:

1. Edit `Config/module_config.h`
2. Change `MODULE_DEBUG_OUTPUT` value
3. Rebuild project (Clean + Build)
4. Flash to device
5. Configure viewer (SWV console or terminal)

No other code changes needed!

---

## Using Both SWV and USB CDC

You can have USB CDC enabled for MIOS terminal while using SWV for debug traces:

```c
// Debug traces go to SWV
#define MODULE_DEBUG_OUTPUT DEBUG_OUTPUT_SWV

// USB CDC still available for MIOS terminal and CLI
#define MODULE_ENABLE_USB_CDC 1
```

This is the **BEST** configuration:
- Debug traces via SWV (no conflicts)
- MIOS Studio terminal via USB CDC
- CLI commands via USB CDC
- Both working simultaneously!

---

## Code Examples

All debug functions work with any output method:

```c
// Initialize (call once at startup)
test_debug_init();

// Simple output
dbg_print("Hello World\r\n");
dbg_putc('A');

// Formatted output
dbg_printf("Value: %d, Hex: 0x%02X\r\n", 123, 0xAB);

// Hex dump
uint8_t data[] = {0x00, 0x11, 0x22};
dbg_hex_dump(data, sizeof(data));
```

Output appears in:
- **SWV**: STM32CubeIDE → SWV ITM Data Console
- **USB CDC**: MIOS Studio or serial terminal
- **UART**: Serial terminal at 115200 baud

---

## Performance Impact

| Method | CPU Usage | RAM | Flash | Latency |
|--------|-----------|-----|-------|---------|
| SWV | ~0.1% | ~0 bytes | ~200 bytes | <1 ms |
| USB CDC | ~0.5% | ~512 bytes | ~8 KB | 1-10 ms |
| UART | ~0.2% | ~0 bytes | ~100 bytes | <1 ms |
| None | 0% | 0 bytes | 0 bytes | N/A |

---

## Troubleshooting

### SWV Issues

**Q:** No output in SWV console  
**A:** Enable SWV in debug config, enable Port 0, click Start Trace

**Q:** Garbled characters  
**A:** Check core clock setting (168 MHz), try lower SWO clock (500 kHz)

**Q:** Output stops after a while  
**A:** SWV buffer overflow - reduce output rate or increase buffer

### USB CDC Issues

**Q:** COM port not appearing  
**A:** Check USB cable, verify USB CDC enabled, reinstall driver

**Q:** Data corruption  
**A:** USB enumeration issue - try different USB port or cable

**Q:** Conflicts with MIDI  
**A:** Switch to SWV for debugging, keep CDC for production

### UART Issues

**Q:** No output  
**A:** Check baud rate (115200), verify wiring, check UART port number

**Q:** Corrupted data  
**A:** Baud rate mismatch, wrong UART port configured

---

## Summary

- **Development:** Use **SWV** (best debugging experience)
- **MIOS Studio:** Use **USB CDC** (terminal compatibility)
- **Production:** Use **USB CDC** or **NONE**
- **Best of both:** **SWV** for debug + **USB CDC** enabled for terminal

Choose what works best for your workflow!
