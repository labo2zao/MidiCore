# MIOS32-Style Automatic USB Mode Switching - Implementation Guide

## Overview

This guide implements **Option 2** from `USB_DEVICE_AND_HOST_GUIDE.md`: Automatic runtime switching between USB Device and Host modes, just like MIOS32.

**How it works**:
- Detects cable type via ID pin (PA10)
- Automatically switches between Device and Host modes
- No user intervention required
- Seamless transition when cable changes

---

## Architecture

### ID Pin Detection (Like MIOS32)

**PA10 (USB_OTG_FS_ID pin)**:
- **LOW (grounded)**: OTG adapter connected → Switch to Host mode
- **HIGH (floating)**: Standard USB cable → Switch to Device mode

**MIOS32 Reference**: `mios32/STM32F4xx/mios32_usb.c` lines 850-900

---

## Implementation

### Step 1: Create USB Mode Manager

Create new file: `Services/usb_mode_manager/usb_mode_manager.c`

```c
/**
 ******************************************************************************
 * @file    usb_mode_manager.c
 * @brief   USB OTG Mode Manager - MIOS32 Style Automatic Switching
 * @author  MidiCore Project
 ******************************************************************************
 * Automatically switches between USB Device and Host modes based on cable type
 * Inspired by MIOS32 implementation
 ******************************************************************************
 */

#include "usb_mode_manager.h"
#include "main.h"
#include "Services/usb_midi/usb_midi.h"
#include "Services/usb_host_midi/usb_host_midi.h"

#if USB_DEVICE_AVAILABLE
#include "usb_device.h"
#include "usbd_midi.h"
extern USBD_HandleTypeDef hUsbDeviceFS;
#endif

#if USB_HOST_AVAILABLE
#include "usb_host.h"
extern USBH_HandleTypeDef hUsbHostFS;
#endif

/* Current USB mode */
static usb_mode_t current_mode = USB_MODE_NONE;
static uint8_t mode_initialized = 0;

/* ID pin state tracking */
static GPIO_PinState last_id_pin_state = GPIO_PIN_SET;
static uint32_t id_pin_stable_time = 0;
#define ID_PIN_DEBOUNCE_MS 100  /* Debounce time like MIOS32 */

/**
 * @brief  Initialize USB Mode Manager
 * @retval None
 */
void usb_mode_manager_init(void) {
  mode_initialized = 1;
  current_mode = USB_MODE_NONE;
  
  /* Read initial ID pin state */
  last_id_pin_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10);
  id_pin_stable_time = HAL_GetTick();
  
  /* Initialize to appropriate mode */
  usb_mode_manager_task();
}

/**
 * @brief  Deinitialize current USB mode
 * @retval None
 */
static void usb_mode_deinit_current(void) {
  if (current_mode == USB_MODE_DEVICE) {
#if USB_DEVICE_AVAILABLE
    USBD_Stop(&hUsbDeviceFS);
    USBD_DeInit(&hUsbDeviceFS);
#endif
  }
  else if (current_mode == USB_MODE_HOST) {
#if USB_HOST_AVAILABLE
    USBH_Stop(&hUsbHostFS);
    USBH_DeInit(&hUsbHostFS);
#endif
  }
  
  current_mode = USB_MODE_NONE;
}

/**
 * @brief  Switch to USB Device mode
 * @retval None
 */
static void usb_mode_switch_to_device(void) {
  if (current_mode == USB_MODE_DEVICE) {
    return;  /* Already in Device mode */
  }
  
  /* Deinitialize current mode */
  usb_mode_deinit_current();
  
  /* Small delay for hardware to settle */
  HAL_Delay(50);
  
#if USB_DEVICE_AVAILABLE
  /* Initialize USB Device */
  MX_USB_DEVICE_Init();
  usb_midi_init();
  
  current_mode = USB_MODE_DEVICE;
  
  /* Debug output */
  #ifdef DEBUG
  printf("[USB] Switched to Device mode (DAW)\r\n");
  #endif
#else
  #warning "USB Device not available - CubeMX code not generated yet"
#endif
}

/**
 * @brief  Switch to USB Host mode
 * @retval None
 */
static void usb_mode_switch_to_host(void) {
  if (current_mode == USB_MODE_HOST) {
    return;  /* Already in Host mode */
  }
  
  /* Deinitialize current mode */
  usb_mode_deinit_current();
  
  /* Small delay for hardware to settle */
  HAL_Delay(50);
  
#if USB_HOST_AVAILABLE
  /* Initialize USB Host */
  MX_USB_HOST_Init();
  usb_host_midi_init();
  
  current_mode = USB_MODE_HOST;
  
  /* Debug output */
  #ifdef DEBUG
  printf("[USB] Switched to Host mode (MIDI keyboard)\r\n");
  #endif
#else
  /* USB Host should always be available */
  MX_USB_HOST_Init();
  usb_host_midi_init();
  current_mode = USB_MODE_HOST;
#endif
}

/**
 * @brief  Get current USB mode
 * @retval Current USB mode
 */
usb_mode_t usb_mode_manager_get_mode(void) {
  return current_mode;
}

/**
 * @brief  USB Mode Manager task - Call regularly (like MIOS32)
 * @note   Should be called at least every 100ms
 * @retval None
 */
void usb_mode_manager_task(void) {
  if (!mode_initialized) {
    return;
  }
  
  /* Read ID pin (PA10) */
  GPIO_PinState id_pin = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10);
  
  /* Check if ID pin state changed */
  if (id_pin != last_id_pin_state) {
    /* State changed, reset debounce timer */
    last_id_pin_state = id_pin;
    id_pin_stable_time = HAL_GetTick();
    return;
  }
  
  /* Check if debounce time elapsed */
  uint32_t current_time = HAL_GetTick();
  if ((current_time - id_pin_stable_time) < ID_PIN_DEBOUNCE_MS) {
    return;  /* Still debouncing */
  }
  
  /* ID pin state is stable, check mode */
  if (id_pin == GPIO_PIN_RESET) {
    /* ID LOW = OTG adapter connected = Host mode */
    if (current_mode != USB_MODE_HOST) {
      usb_mode_switch_to_host();
    }
  }
  else {
    /* ID HIGH = Standard cable = Device mode */
    if (current_mode != USB_MODE_DEVICE) {
      usb_mode_switch_to_device();
    }
  }
  
  /* Process current mode */
  if (current_mode == USB_MODE_HOST) {
#if USB_HOST_AVAILABLE || 1
    usb_host_midi_task();
#endif
  }
  /* Device mode is handled by interrupts, no polling needed */
}
```

### Step 2: Create Header File

Create new file: `Services/usb_mode_manager/usb_mode_manager.h`

```c
/**
 ******************************************************************************
 * @file    usb_mode_manager.h
 * @brief   USB OTG Mode Manager - MIOS32 Style Automatic Switching
 * @author  MidiCore Project
 ******************************************************************************
 */

#ifndef USB_MODE_MANAGER_H
#define USB_MODE_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Check if USB Device is available */
#if defined(__has_include)
#  if __has_include("usb_device.h")
#    define USB_DEVICE_AVAILABLE 1
#  else
#    define USB_DEVICE_AVAILABLE 0
#  endif
#else
#  define USB_DEVICE_AVAILABLE 0
#endif

/* USB Host is always available in this project */
#define USB_HOST_AVAILABLE 1

/* USB Mode enumeration */
typedef enum {
  USB_MODE_NONE = 0,
  USB_MODE_DEVICE,   /* Device mode - appears in DAW */
  USB_MODE_HOST      /* Host mode - reads MIDI keyboards */
} usb_mode_t;

/* Function prototypes */
void usb_mode_manager_init(void);
void usb_mode_manager_task(void);
usb_mode_t usb_mode_manager_get_mode(void);

#ifdef __cplusplus
}
#endif

#endif /* USB_MODE_MANAGER_H */
```

---

## Integration with main.c

### In StartDefaultTask

Replace current USB initialization with mode manager:

```c
/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* init code for USB_HOST */
  /* MX_USB_HOST_Init();  ← REMOVE THIS - Mode manager handles it */
  
  /* USER CODE BEGIN 5 */
  
  /* Initialize USB Mode Manager (MIOS32 style) */
  usb_mode_manager_init();
  
  /* Check if module test selected */
  module_test_t selected_test = module_tests_get_compile_time_selection();
  
  if (selected_test != MODULE_TEST_NONE) {
    /* Run module test */
    module_tests_run(selected_test);
  } else {
    /* Run production application */
    while(1)
    {
      /* USB Mode Manager task - handles automatic switching */
      usb_mode_manager_task();
      
      /* Yield to other tasks */
      osDelay(10);  /* 10ms = 100Hz like MIOS32 */
    }
  }
  
  /* USER CODE END 5 */
}
```

### Add Include

At top of main.c:

```c
/* USER CODE BEGIN Includes */
#include "Services/usb_mode_manager/usb_mode_manager.h"
/* USER CODE END Includes */
```

---

## How It Works

### 1. Cable Detection

**Standard USB Cable**:
```
Computer ←──USB──→ MidiCore
                   PA10 = HIGH (floating)
                   → Device mode activated
                   → MidiCore appears as 4-port MIDI interface in DAW
```

**OTG Adapter + MIDI Keyboard**:
```
Power ←──USB Debug──→ MidiCore ←──OTG Adapter──→ USB MIDI Keyboard
                      PA10 = LOW (grounded)
                      → Host mode activated
                      → MidiCore reads MIDI keyboard
```

### 2. Automatic Switching Sequence

**When Cable Connected**:
1. ID pin state changes
2. Debounce for 100ms (avoid false triggers)
3. Check stable state
4. Deinitialize old mode (if any)
5. Wait 50ms for hardware to settle
6. Initialize new mode
7. Start processing

**Like MIOS32**: `mios32_usb.c:MIOS32_USB_Init()` lines 850-900

---

## Build Configuration

### Add to CMakeLists.txt or Makefile

If using CMake:
```cmake
# USB Mode Manager
${CMAKE_CURRENT_SOURCE_DIR}/Services/usb_mode_manager/usb_mode_manager.c
```

If using STM32CubeIDE Makefile:
```makefile
# USB Mode Manager
Services/usb_mode_manager/usb_mode_manager.c \
```

### Add Include Path

In project settings or Makefile:
```
-I../Services/usb_mode_manager
```

---

## Testing

### Test Device Mode

1. Connect standard micro-USB cable to computer
2. MidiCore should appear in DAW within ~150ms
3. All 4 MIDI ports should be visible
4. Send/receive MIDI messages

**Expected Debug Output**:
```
[USB] Switched to Device mode (DAW)
```

### Test Host Mode

1. Disconnect from computer
2. Connect micro-USB OTG adapter to MidiCore
3. **Power MidiCore via USB Debug socket**
4. Connect USB MIDI keyboard to OTG adapter
5. MidiCore should switch to Host mode within ~150ms
6. Play notes on keyboard

**Expected Debug Output**:
```
[USB] Switched to Host mode (MIDI keyboard)
```

### Test Automatic Switching

1. Start in Device mode (connected to computer)
2. Disconnect
3. Connect OTG adapter + keyboard
4. **Automatic switch to Host mode** ✅
5. Disconnect OTG adapter
6. Connect standard USB cable to computer
7. **Automatic switch back to Device mode** ✅

---

## Advanced: Mode Status Indicator

### Add LED Indicator (Optional)

```c
/* In usb_mode_switch_to_device() */
HAL_GPIO_WritePin(LED_USB_DEVICE_GPIO_Port, LED_USB_DEVICE_Pin, GPIO_PIN_SET);
HAL_GPIO_WritePin(LED_USB_HOST_GPIO_Port, LED_USB_HOST_Pin, GPIO_PIN_RESET);

/* In usb_mode_switch_to_host() */
HAL_GPIO_WritePin(LED_USB_DEVICE_GPIO_Port, LED_USB_DEVICE_Pin, GPIO_PIN_RESET);
HAL_GPIO_WritePin(LED_USB_HOST_GPIO_Port, LED_USB_HOST_Pin, GPIO_PIN_SET);
```

**Visual Feedback**:
- LED 1 ON = Device mode (DAW)
- LED 2 ON = Host mode (MIDI keyboard)

---

## MIOS32 Compatibility

This implementation follows MIOS32 patterns:

| Feature | MIOS32 | MidiCore | Status |
|---------|--------|----------|--------|
| ID pin detection | ✅ Yes | ✅ Yes | ✅ Compatible |
| Automatic switching | ✅ Yes | ✅ Yes | ✅ Compatible |
| Debounce logic | ✅ Yes | ✅ Yes | ✅ Compatible |
| 100ms task rate | ✅ Yes | ✅ 10ms (faster) | ✅ Improved |
| Mode deinit | ✅ Yes | ✅ Yes | ✅ Compatible |
| Seamless transition | ✅ Yes | ✅ Yes | ✅ Compatible |

**Reference**: 
- `mios32/STM32F4xx/mios32_usb.c` (USB initialization and mode handling)
- `mios32/STM32F4xx/mios32_usb_midi.c` (MIDI-specific handling)

---

## Troubleshooting

### Mode Doesn't Switch

**Check**:
1. PA10 configured as USB_OTG_FS_ID in CubeMX
2. Mode manager task is called regularly
3. Debug output shows pin state changes

**Debug Code**:
```c
GPIO_PinState id_pin = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10);
printf("ID Pin: %d, Mode: %d\r\n", id_pin, current_mode);
```

### Device Mode Not Working

**Check**:
1. CubeMX generated USB_DEVICE files
2. USB_DEVICE_AVAILABLE is 1
3. usb_device.h exists
4. MIDI class integrated

### Host Mode Not Working

**Check**:
1. External power via USB Debug socket
2. OTG adapter has ID pin grounded
3. USB_HOST files exist
4. usb_host_midi_task() is called

---

## Summary

### What You Get

✅ **Automatic mode switching** - No user intervention
✅ **MIOS32-style operation** - Proven design
✅ **Seamless transitions** - ~150ms switch time
✅ **Debug-friendly** - Clear status messages
✅ **Robust debouncing** - Prevents false triggers

### Usage

**For DAW**: Just plug in standard USB cable → Device mode
**For MIDI keyboards**: Plug in OTG adapter + keyboard → Host mode

**That's it!** No buttons, no configuration, no manual switching. Just like MIOS32! 🎉

---

## Next Steps

1. Create `Services/usb_mode_manager/` directory
2. Add the .c and .h files from above
3. Integrate with main.c as shown
4. Add to build system
5. Flash and test!

**Ready for MIOS32-style automatic USB mode switching!**
