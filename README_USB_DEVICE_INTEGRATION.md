# USB MIDI Device Integration Guide

## Overview

This guide explains how to integrate the custom 4-port USB MIDI Device class into MidiCore after CubeMX generates the USB Device core library.

## Custom MIDI Class Created

The following files have been created with full 4-port USB MIDI support:

- `Middlewares/ST/STM32_USB_Device_Library/Class/MIDI/Inc/usbd_midi.h`
- `Middlewares/ST/STM32_USB_Device_Library/Class/MIDI/Src/usbd_midi.c`

**Features:**
- 4 virtual MIDI ports (cables 0-3) over one USB connection
- USB MIDI 1.0 compliant descriptors
- Proper Code Index Number (CIN) encoding for all MIDI message types
- Bulk IN/OUT endpoints for bidirectional communication

## Step 1: CubeMX Configuration

### 1.1 Open MidiCore.ioc in STM32CubeMX

### 1.2 Configure USB OTG for Dual-Role

**Current Configuration:**
```
USB_OTG_FS.VirtualMode = Host_Only
```

**Change to:**
```
USB_OTG_FS.VirtualMode = OTG_FS
```

**Settings:**
- Mode: OTG_FS (not Host_Only, not Device_Only)
- Enable ID pin: PA10 (already configured)
- VBUS sensing: **Optional** (enable if available, disable if not supported)
- PHY Interface: Embedded

**Important**: VBUS sensing is optional. ID pin (PA10) is required for mode detection.

### 1.3 Add USB_DEVICE Middleware

1. Go to **Middleware** → **USB_DEVICE**
2. Enable USB_DEVICE
3. Class: Select "Custom HID" or "Audio" (temporary - we'll replace it)
4. Keep default settings

### 1.4 Generate Code

Click **Generate Code** button. CubeMX will create:

```
USB_DEVICE/
├── App/
│   ├── usb_device.c
│   └── usb_device.h
└── Target/
    ├── usbd_conf.c
    └── usbd_conf.h

Middlewares/ST/STM32_USB_Device_Library/
└── Core/
    ├── Inc/
    │   ├── usbd_core.h
    │   ├── usbd_ctlreq.h
    │   ├── usbd_def.h
    │   └── usbd_ioreq.h
    └── Src/
        ├── usbd_core.c
        ├── usbd_ctlreq.c
        └── usbd_ioreq.c
```

## Step 2: Integrate MIDI Class

### 2.1 Update USB_DEVICE/App/usb_device.c

Replace the HID/Audio class includes and registration:

**Remove:**
```c
#include "usbd_hid.h"  // or usbd_audio.h
```

**Add:**
```c
#include "usbd_midi.h"
```

**In MX_USB_DEVICE_Init():**

**Remove:**
```c
if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_HID) != USBD_OK)  // or USBD_AUDIO
```

**Replace with:**
```c
if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_MIDI) != USBD_OK)
```

### 2.2 Update USB_DEVICE/Target/usbd_conf.c

Add MIDI class include at the top:

```c
#include "usbd_midi.h"
```

Update `USBD_static_malloc()` function if needed:

```c
static uint32_t mem[(sizeof(USBD_MIDI_HandleTypeDef) / 4) + 1];
```

### 2.3 Update Include Paths

Add to project include paths:
```
Middlewares/ST/STM32_USB_Device_Library/Core/Inc
Middlewares/ST/STM32_USB_Device_Library/Class/MIDI/Inc
USB_DEVICE/App
USB_DEVICE/Target
```

### 2.4 Add Source Files to Build

Add to compilation:
```
Middlewares/ST/STM32_USB_Device_Library/Class/MIDI/Src/usbd_midi.c
```

## Step 3: Implement Application Layer

### 3.1 Update Services/usb_midi/usb_midi.c

Replace the stub implementation with actual USB Device calls:

```c
#include "Services/usb_midi/usb_midi.h"
#include "Services/router/router.h"
#include "usb_device.h"
#include "usbd_midi.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

/* MIDI Interface Callbacks */
static void USBD_MIDI_Init_Callback(void);
static void USBD_MIDI_DeInit_Callback(void);
static void USBD_MIDI_DataOut_Callback(USBD_MIDI_EventPacket_t *packet);

static USBD_MIDI_ItfTypeDef midi_fops = {
  USBD_MIDI_Init_Callback,
  USBD_MIDI_DeInit_Callback,
  USBD_MIDI_DataOut_Callback
};

void usb_midi_init(void) {
  /* Register interface callbacks */
  USBD_MIDI_RegisterInterface(&hUsbDeviceFS, &midi_fops);
}

void usb_midi_send_packet(uint8_t cin, uint8_t b0, uint8_t b1, uint8_t b2) {
  /* Extract cable number from CIN (upper 4 bits) */
  uint8_t cable = (cin >> 4) & 0x0F;
  
  /* Build MIDI message */
  uint8_t data[3] = {b0, b1, b2};
  uint16_t length = 3;
  
  /* Adjust length based on message type */
  if ((b0 & 0xF0) == 0xC0 || (b0 & 0xF0) == 0xD0) {
    length = 2; /* Program Change, Channel Pressure */
  }
  
  /* Send via USB */
  USBD_MIDI_SendData(&hUsbDeviceFS, cable, data, length);
}

void usb_midi_rx_packet(const uint8_t packet4[4]) {
  /* Extract cable number (upper 4 bits of header) */
  uint8_t cable = (packet4[0] >> 4) & 0x0F;
  
  /* Route to appropriate router node based on cable number */
  router_msg_t msg;
  msg.type = ROUTER_MSG_3B;
  msg.b0 = packet4[1];
  msg.b1 = packet4[2];
  msg.b2 = packet4[3];
  
  /* Cable 0-3 map to USB_PORT0-3 */
  router_node_t node = ROUTER_NODE_USB_PORT0 + cable;
  router_process(node, &msg);
}

/* Callbacks */
static void USBD_MIDI_Init_Callback(void) {
  /* USB MIDI initialized */
}

static void USBD_MIDI_DeInit_Callback(void) {
  /* USB MIDI de-initialized */
}

static void USBD_MIDI_DataOut_Callback(USBD_MIDI_EventPacket_t *packet) {
  /* Forward packet to application */
  usb_midi_rx_packet((uint8_t*)packet);
}
```

### 3.2 Update Router Nodes

Add USB Device port nodes to `Services/router/router.h`:

```c
typedef enum {
  // ... existing nodes ...
  ROUTER_NODE_USB_PORT0 = 14,  /* USB Device MIDI Port 1 (cable 0) */
  ROUTER_NODE_USB_PORT1 = 15,  /* USB Device MIDI Port 2 (cable 1) */
  ROUTER_NODE_USB_PORT2 = 16,  /* USB Device MIDI Port 3 (cable 2) */
  ROUTER_NODE_USB_PORT3 = 17,  /* USB Device MIDI Port 4 (cable 3) */
  ROUTER_NODE_MAX
} router_node_t;
```

## Step 4: Enable USB Device MIDI

### 4.1 Update Config/module_config.h

```c
/** @brief Enable USB Device MIDI (4 ports) */
#ifndef MODULE_ENABLE_USB_MIDI
#define MODULE_ENABLE_USB_MIDI 1  /* Enable for USB Device mode */
#endif

/** @brief Enable USB Host MIDI */
#ifndef MODULE_ENABLE_USBH_MIDI
#define MODULE_ENABLE_USBH_MIDI 1  /* Keep enabled for Host mode */
#endif
```

### 4.2 Update main.c

Ensure USB Device is initialized:

```c
/* USER CODE BEGIN 2 */
#if MODULE_ENABLE_USB_MIDI
  usb_midi_init();
#endif
/* USER CODE END 2 */
```

## Step 5: USB OTG Dual-Role Switching

### 5.1 Automatic Mode Detection

The STM32F407 USB OTG can automatically switch between Device and Host mode based on the ID pin:

- **ID pin LOW (grounded)**: Host mode → USB OTG adapter detected
- **ID pin HIGH (floating)**: Device mode → Standard USB cable detected

### 5.2 Implement Mode Switching

Add to `Core/Src/main.c`:

```c
/* Detect USB OTG mode */
void check_usb_mode(void) {
  GPIO_PinState id_pin = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10); // PA10 = ID pin
  
  if (id_pin == GPIO_PIN_RESET) {
    /* ID pin LOW = Host mode */
    /* Ensure USB Host is active */
  } else {
    /* ID pin HIGH = Device mode */
    /* Ensure USB Device is active */
  }
}
```

## Step 6: Testing

### 6.1 Test USB Device Mode (DAW Visibility)

1. Connect MidiCore to computer with **standard micro-USB cable**
2. Check device manager (Windows) or `lsusb` (Linux) or Audio MIDI Setup (Mac)
3. Should appear as "MidiCore" with 4 MIDI ports
4. Open DAW (Ableton, FL Studio, etc.)
5. Verify all 4 MIDI ports appear

### 6.2 Test USB Host Mode (Read MIDI Keyboard)

1. Connect **USB OTG adapter** to MidiCore
2. Connect **USB MIDI keyboard** to OTG adapter
3. Power MidiCore via USB Debug socket (important for Host mode!)
4. Keyboard should be detected
5. MIDI messages from keyboard should appear in router

### 6.3 Test 4-Port Routing

```c
/* Example: Route DIN1 → USB Port 2 (cable 1) */
router_add_route(ROUTER_NODE_DIN_IN1, ROUTER_NODE_USB_PORT1, 0xFFFF);

/* Example: Route USB Port 3 → DIN OUT3 */
router_add_route(ROUTER_NODE_USB_PORT2, ROUTER_NODE_DIN_OUT3, 0xFFFF);
```

## Troubleshooting

### Device Not Recognized

- Check USB_OTG_FS.VirtualMode = OTG_FS (not Host_Only)
- Verify VBUS sensing enabled
- Check cable (must be data cable, not charge-only)
- Verify endpoint addresses match in descriptors

### Host Mode Not Working

- Must power via USB Debug socket (not same USB port!)
- Check OTG adapter (must have ID pin grounded)
- Max current for connected device: ~300mA
- No USB hub support

### Only 1 Port Visible

- Check USB descriptors declare 4 embedded jacks
- Verify cable number in send/receive functions (0-3)
- Check router nodes are correctly configured

## Device Descriptor Customization

To rename the USB device, edit `USB_DEVICE/App/usbd_desc.c`:

```c
#define USBD_MANUFACTURER_STRING     "MidiCore"
#define USBD_PRODUCT_STRING_FS       "MidiCore 4x4 MIDI Interface"
#define USBD_CONFIGURATION_STRING_FS "MIDI Config"
#define USBD_INTERFACE_STRING_FS     "MIDI Interface"
```

## Summary

After following these steps, MidiCore will have:

- ✅ **4 USB MIDI ports** (cables 0-3) via one USB OTG cable
- ✅ **4 DIN MIDI ports** (USART1/2/3, UART5) via separate cables
- ✅ **Automatic mode switching** (Device/Host based on cable type)
- ✅ **Total 8 MIDI ports** - exactly like MIOS32

The USB OTG port can act as:
- **Device** with standard cable → Appears in DAW with 4 ports
- **Host** with OTG adapter → Reads USB MIDI keyboards
