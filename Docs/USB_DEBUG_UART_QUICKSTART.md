# USB MIDI Debug - UART Quick Start Guide

## Simple UART Debug (No Build Flags Needed!)

This guide shows you how to enable USB debug output via UART - it's very simple!

## Step 1: Enable Debug Output (Choose ONE Method)

### Method 1: Via Module Config (Recommended for Project Integration)

Open the file: `Config/module_config.h`

Find this section:
```c
/** @brief Enable USB MIDI debug output via UART */
#ifndef MODULE_ENABLE_USB_MIDI_DEBUG
#define MODULE_ENABLE_USB_MIDI_DEBUG 0  // Disabled by default
#endif
```

**Change the 0 to 1**:
```c
#ifndef MODULE_ENABLE_USB_MIDI_DEBUG
#define MODULE_ENABLE_USB_MIDI_DEBUG 1  // ENABLED for debugging
#endif
```

### Method 2: Directly in Debug Header (Quick Testing)

Open the file: `USB_DEVICE/Class/MIDI/Inc/usbd_midi_debug.h`

Find this section near the top:
```c
/* ============================================ */
/* ENABLE DEBUG HERE - Just uncomment this line */
/* ============================================ */
// #define USBD_MIDI_DEBUG
/* ============================================ */
```

**Uncomment the line** so it looks like this:
```c
/* ============================================ */
/* ENABLE DEBUG HERE - Just uncomment this line */
/* ============================================ */
#define USBD_MIDI_DEBUG
/* ============================================ */
```

**Note:** Method 1 is recommended for integration with your project. Method 2 is quicker for one-off testing.

## Step 2: Add Debug Calls to USB Code

Open: `USB_DEVICE/Class/MIDI/Src/usbd_midi.c`

At the top, add:
```c
#include "USB_DEVICE/Class/MIDI/Inc/usbd_midi_debug.h"
```

In the `USBD_MIDI_Setup()` function, add this at the beginning:
```c
static uint8_t USBD_MIDI_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    /* Add debug logging */
    DEBUG_SETUP(req->bmRequest, req->bRequest, req->wValue, req->wIndex, req->wLength);
    
    /* ... rest of function ... */
```

When the configuration descriptor is returned, add:
```c
/* Inside USBD_MIDI_GetCfgDesc() or where descriptor is sent */
DEBUG_DESCRIPTOR("Config", USBD_MIDI_CfgDesc, USB_MIDI_CONFIG_DESC_SIZ);
```

## Step 3: Make Sure UART Printf Works

Your project needs printf redirected to UART. If you don't have this, add to your main.c or syscalls.c:

```c
#include "usart.h"  /* Or whatever your UART header is called */

int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, HAL_MAX_DELAY);  /* Change huart2 to your UART */
    return len;
}
```

## Step 4: Connect UART Terminal

1. Connect USB-to-UART adapter to your board's UART pins
2. Open a terminal program (PuTTY, TeraTerm, etc.)
3. Set baud rate: **115200** (or whatever your UART is configured for)
4. Set: 8-N-1, no flow control

## Step 5: Rebuild and Flash

1. Build your project
2. Flash to the board
3. Reset the board
4. **Plug in USB cable to PC**

## What You'll See

When you plug in USB, you'll see output like:

```
USB State: RESET
USB Setup: bmReq=0x80 bReq=0x06 wVal=0x0100 wIdx=0x0000 wLen=64
  -> GET_DESCRIPTOR: type=1 index=0
     (DEVICE descriptor)
Descriptor [Device]: 18 bytes (0x12)
  0000: 12 01 00 02 00 00 00 40 C0 16 89 04 00 02 01 02 
  0010: 03 01 

USB Setup: bmReq=0x80 bReq=0x06 wVal=0x0200 wIdx=0x0000 wLen=9
  -> GET_DESCRIPTOR: type=2 index=0
     (CONFIGURATION descriptor)
Descriptor [Config]: 207 bytes (0xCF)
  0000: 09 02 CF 00 02 01 00 80 FA 09 04 00 00 00 01 01 
  0010: 00 00 09 24 01 00 01 09 00 01 01 09 04 01 00 02 
  0020: 01 03 00 00 07 24 01 00 01 A4 00 06 24 02 01 01 
  ... (more bytes)
```

## What to Look For

### If you see "GET_DESCRIPTOR type=2" but then ERROR:
- Windows requested the configuration descriptor
- But rejected it
- The debug dump shows what bytes you sent
- Compare with MIOS32 or look for invalid values

### If you see "SET_CONFIGURATION":
- Device enumerated successfully!
- But maybe there's an error later
- Keep watching for more requests

### If you see nothing:
- UART printf not working - check Step 3
- Or USB not even detecting device - hardware issue

## Disable Debug Later

When done debugging, just comment out the line again:
```c
// #define USBD_MIDI_DEBUG
```

Rebuild and the debug code disappears (no performance impact).

## Common Issues

**Q: I see garbled characters**
- Check UART baud rate matches (115200)
- Check terminal settings (8-N-1)

**Q: Nothing appears on UART**
- Check printf redirection (Step 3)
- Check UART pins are correct
- Try sending a test printf from main() first

**Q: Too much output, can't read it**
- Add delays between prints
- Or use a terminal that can capture to file
- Or only enable specific DEBUG_xxx macros you need

## Quick Test

To test if UART printf works before enabling USB debug:

In your `main()` function, add:
```c
printf("UART Test - Hello World!\r\n");
HAL_Delay(1000);
```

If you see this on your terminal, UART printf is working!

---

**Next**: Once you have debug output, look for which descriptor Windows rejects and what bytes are being sent.
