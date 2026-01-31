# Debug Output Configuration - Quick Start

## TL;DR - How to Choose and Configure Debug Output

### Step 1: Choose Your Debug Method

Edit **`Config/module_config.h`** around line 93:

```c
// Choose ONE:
#define MODULE_DEBUG_OUTPUT DEBUG_OUTPUT_SWV      // ⭐ RECOMMENDED
// #define MODULE_DEBUG_OUTPUT DEBUG_OUTPUT_USB_CDC  // Alternative
// #define MODULE_DEBUG_OUTPUT DEBUG_OUTPUT_UART     // Alternative
// #define MODULE_DEBUG_OUTPUT DEBUG_OUTPUT_NONE     // Disabled
```

### Step 2: Configure and Use

#### If You Chose SWV (Recommended):

1. **In CubeIDE Debug Configuration:**
   - Run → Debug Configurations
   - Debugger tab → SWV section
   - ☑ Enable
   - Core Clock: 168000000
   - SWO Clock: 2000000  
   - Port 0: ☑ Enabled
   - Apply → Close

2. **Every Debug Session:**
   - F11 (Start Debug)
   - Window → Show View → SWV → SWV ITM Data Console
   - Configure (⚙️) → Port 0: ☑ → OK
   - Start Trace (⏺️)
   - Resume (▶️)

**See:** docs/SWV_CUBEIDE_SETUP.md for detailed instructions

#### If You Chose USB CDC:

1. **In Config:**
```c
#define MODULE_DEBUG_OUTPUT DEBUG_OUTPUT_USB_CDC
#define MODULE_ENABLE_USB_CDC 1
```

2. **Connect USB and open terminal:**
   - MIOS Studio (auto-detects), OR
   - Serial terminal on COM port

**See:** docs/DEBUG_OUTPUT_GUIDE.md for complete guide

---

## Complete Documentation

| Document | What It Covers |
|----------|----------------|
| **SWV_CUBEIDE_SETUP.md** | Step-by-step CubeIDE configuration for SWV |
| **DEBUG_OUTPUT_GUIDE.md** | Comparison of all debug methods, when to use each |
| **Config/module_config.h** | Quick selection (one line change) |

---

## Why SWV is Recommended

✅ No USB conflicts (uses ST-Link)  
✅ Always works (even if USB fails)  
✅ High bandwidth (2 MHz)  
✅ Real-time traces  
✅ Best for USB MIDI devices  

---

## Best Configuration: Both!

```c
#define MODULE_DEBUG_OUTPUT DEBUG_OUTPUT_SWV  // Debug via ST-Link
#define MODULE_ENABLE_USB_CDC 1               // MIOS terminal via USB
```

- Debug traces → SWV (no conflicts)
- MIOS terminal → USB CDC  
- CLI commands → USB CDC
- Both work simultaneously! ✅

---

## Need Help?

1. **Quick start:** This file
2. **SWV setup:** docs/SWV_CUBEIDE_SETUP.md
3. **Method comparison:** docs/DEBUG_OUTPUT_GUIDE.md
4. **Configuration:** Config/module_config.h (well documented)
