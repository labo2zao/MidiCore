# USB MIDI Implementation - Final Configuration Guide

## Summary

This guide provides all necessary steps to enable USB MIDI Device (4 ports) and USB Host MIDI in MidiCore, inspired by MIOS32 architecture.

## Error Fix: Missing Include Paths

**Error**: `fatal error: usbd_midi.h: No such file or directory`

**Cause**: The custom MIDI class include paths are not in the build configuration.

**Solution**: Add these include paths to your STM32CubeIDE project:

### Option 1: Automatic (Recommended - via CubeMX)

1. Open `MidiCore.ioc` in STM32CubeMX
2. Configure USB as described below (see "CubeMX Configuration")
3. **Generate Code** - This will automatically add include paths

### Option 2: Manual (Add to .cproject)

Add these paths to your project settings:

**Project → Properties → C/C++ Build → Settings → Tool Settings → MCU GCC Compiler → Include paths**

Add:
```
../Middlewares/ST/STM32_USB_Device_Library/Class/MIDI/Inc
../Middlewares/ST/STM32_USB_Device_Library/Core/Inc
```

Or edit `.cproject` file and add to the include paths list (line 47):
```xml
<listOptionValue builtIn="false" value="../Middlewares/ST/STM32_USB_Device_Library/Class/MIDI/Inc"/>
```

## Complete Implementation Steps

### Step 1: CubeMX Configuration

#### 1.1 Configure USB OTG Mode

**Current** (Host only):
```
USB_OTG_FS.VirtualMode = Host_Only
```

**Change to** (Dual-role OTG):
```
USB_OTG_FS.VirtualMode = OTG_FS
```

**Settings**:
- Connectivity → USB_OTG_FS → Mode: **OTG_FS**
- ID pin: PA10 (already configured)
- VBUS sensing: **Optional** (enable if available, disable if not)
- SOF output: Disable
- Low power mode: Disabled

**Note**: VBUS sensing is optional. The ID pin handles Device/Host mode detection.

#### 1.2 Add USB_DEVICE Middleware

1. Middleware → USB_DEVICE → **Enable**
2. Class for FS IP: Select **"Custom HID"** (temporary)
3. Parameter Settings: Leave defaults

#### 1.3 Generate Code

Click **Generate Code**. CubeMX will create:
- `USB_DEVICE/App/` directory
- `USB_DEVICE/Target/` directory  
- `Middlewares/ST/STM32_USB_Device_Library/Core/` (if not exists)

### Step 2: Replace HID with MIDI Class

After code generation:

#### 2.1 Update USB_DEVICE/App/usb_device.c

**Find**:
```c
#include "usbd_hid.h"
```

**Replace with**:
```c
#include "usbd_midi.h"
```

**Find**:
```c
if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_HID) != USBD_OK)
```

**Replace with**:
```c
if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_MIDI) != USBD_OK)
```

#### 2.2 Update USB_DEVICE/Target/usbd_conf.c

**Add at top**:
```c
#include "usbd_midi.h"
```

**Find** (in `USBD_static_malloc`):
```c
static uint32_t mem[(sizeof(USBD_HID_HandleTypeDef) / 4) + 1];
```

**Replace with**:
```c
static uint32_t mem[(sizeof(USBD_MIDI_HandleTypeDef) / 4) + 1];
```

#### 2.3 Add Include Paths (if not automatic)

**Project → Properties → C/C++ Build → Settings → MCU GCC Compiler → Include paths**

Add:
```
../Middlewares/ST/STM32_USB_Device_Library/Class/MIDI/Inc
${workspace_loc:/${ProjName}}
${workspace_loc:/${ProjName}}/Services
${workspace_loc:/${ProjName}}/Config
```

#### 2.4 Add Source Files to Build

**Project → Properties → C/C++ Build → Settings → MCU GCC Compiler → Source Location**

Verify these are included:
```
../Middlewares/ST/STM32_USB_Device_Library/Class/MIDI/Src
../Services/usb_midi
../Services/usb_host_midi
```

### Step 3: Enable Modules

Edit `Config/module_config.h`:

```c
/** @brief Enable USB Device MIDI (4 ports) */
#ifndef MODULE_ENABLE_USB_MIDI
#define MODULE_ENABLE_USB_MIDI 1  // Enable for Device mode
#endif

/** @brief Enable USB Host MIDI */
#ifndef MODULE_ENABLE_USBH_MIDI
#define MODULE_ENABLE_USBH_MIDI 1  // Enable for Host mode
#endif
```

### Step 4: Initialize in main.c

**Find** `/* USER CODE BEGIN 2 */` section in `main.c`:

```c
/* USER CODE BEGIN 2 */

#if MODULE_ENABLE_USB_MIDI
  usb_midi_init();
#endif

#if MODULE_ENABLE_USBH_MIDI
  usb_host_midi_init();
#endif

/* USER CODE END 2 */
```

**In the main loop or FreeRTOS task**:

```c
/* USER CODE BEGIN WHILE */
while (1)
{
  #if MODULE_ENABLE_USBH_MIDI
    usb_host_midi_task();
  #endif
  
  /* USER CODE END WHILE */
  /* USER CODE BEGIN 3 */
}
/* USER CODE END 3 */
```

### Step 5: Configure Router (Already Done)

The router is already configured with 4 USB Device ports:

- `ROUTER_NODE_USB_PORT0` (Cable 0) - Node 8
- `ROUTER_NODE_USB_PORT1` (Cable 1) - Node 9
- `ROUTER_NODE_USB_PORT2` (Cable 2) - Node 10
- `ROUTER_NODE_USB_PORT3` (Cable 3) - Node 11

**Example routing**:

```c
// Route DIN MIDI IN 1 to USB Device Port 2 (cable 1)
router_set_route(ROUTER_NODE_DIN_IN1, ROUTER_NODE_USB_PORT1, 1);

// Route USB Device Port 3 to DIN MIDI OUT 3
router_set_route(ROUTER_NODE_USB_PORT2, ROUTER_NODE_DIN_OUT3, 1);
```

## Build and Test

### Build

1. **Clean project**: Project → Clean
2. **Build**: Project → Build All (Ctrl+B)
3. Check for no errors

### Test USB Device Mode (Appear in DAW)

1. **Connect** standard micro-USB cable to computer
2. **Check Windows Device Manager** / **Mac Audio MIDI Setup** / **Linux `lsusb`**
3. Should appear as **"MidiCore"** or **"STM32 Audio Device"** with **4 MIDI ports**
4. **Open DAW** (Ableton, FL Studio, Reaper, etc.)
5. **Verify** all 4 MIDI ports visible

### Test USB Host Mode (Read MIDI Keyboard)

1. **Disconnect** from computer
2. **Connect** micro-USB OTG adapter to MidiCore
3. **Power** via USB Debug socket (IMPORTANT - MidiCore must power the OTG device!)
4. **Connect** USB MIDI keyboard to OTG adapter
5. **Verify** MIDI messages received (check debug output or LED indicators)

### Test Mode Switching

The system should **automatically switch** between modes:
- **ID pin HIGH** (floating) → Device mode
- **ID pin LOW** (grounded in OTG adapter) → Host mode

## Customization

### Change Device Name

Edit `USB_DEVICE/App/usbd_desc.c`:

```c
#define USBD_MANUFACTURER_STRING     "MidiCore"
#define USBD_PRODUCT_STRING_FS       "MidiCore 4x4 MIDI Interface"
#define USBD_CONFIGURATION_STRING_FS "MIDI Config"
#define USBD_INTERFACE_STRING_FS     "MIDI Interface"
```

### Change Number of Ports

Edit `Middlewares/ST/STM32_USB_Device_Library/Class/MIDI/Inc/usbd_midi.h`:

```c
/* Number of MIDI Ports (Cables) - 4x4 interface like MIOS32 */
#define MIDI_NUM_PORTS  4  // Can be 1-8 like MIOS32
```

**Note**: If changing port count:
1. Update router nodes in `Config/router_config.h`
2. Rebuild descriptors (automatic via MIDI_NUM_PORTS)
3. Update routing configuration

## Architecture Summary

### 8 MIDI Ports Total (Like MIOS32)

**USB Device (4 ports via ONE cable)**:
- Port 1 (Cable 0) → ROUTER_NODE_USB_PORT0
- Port 2 (Cable 1) → ROUTER_NODE_USB_PORT1
- Port 3 (Cable 2) → ROUTER_NODE_USB_PORT2
- Port 4 (Cable 3) → ROUTER_NODE_USB_PORT3

**DIN MIDI (4 ports via separate cables)**:
- DIN 1 → USART1 → ROUTER_NODE_DIN_IN1/OUT1
- DIN 2 → USART2 → ROUTER_NODE_DIN_IN2/OUT2
- DIN 3 → USART3 → ROUTER_NODE_DIN_IN3/OUT3
- DIN 4 → UART5 → ROUTER_NODE_DIN_IN4/OUT4

**USB Host (OTG mode)**:
- Host IN → ROUTER_NODE_USBH_IN
- Host OUT → ROUTER_NODE_USBH_OUT

### Automatic Mode Switching

**Device Mode** (standard micro-USB cable):
- MidiCore appears as **4-port MIDI interface** in DAW
- Sends/receives MIDI to/from computer
- All 4 ports independently routable

**Host Mode** (OTG adapter + USB MIDI device):
- MidiCore reads external **USB MIDI keyboards/controllers**
- Routes messages to internal processing or DIN MIDI outputs
- Requires external power via USB Debug socket

## Troubleshooting

### Compilation Error: "usbd_midi.h: No such file or directory"

**Solution**: Add include path as described in "Error Fix" section above.

### Device Not Recognized by Computer

1. Check USB cable (must be data cable, not charge-only)
2. Verify `USB_OTG_FS.VirtualMode = OTG_FS` in .ioc
3. Check VBUS sensing enabled
4. Verify USB_DEVICE middleware is enabled

### Host Mode Not Working

1. Must power via **USB Debug socket** (not same USB port!)
2. OTG adapter must have **ID pin grounded**
3. Check connected device is class-compliant
4. Max current: ~300mA for connected devices
5. No USB hub support

### Only 1 Port Visible Instead of 4

1. Check `MIDI_NUM_PORTS` = 4 in usbd_midi.h
2. Verify all 16 jack descriptors (4 IN + 4 OUT, Embedded + External)
3. Check cable number routing in usb_midi.c
4. Verify router nodes ROUTER_NODE_USB_PORT0-3 configured

### Build Errors

1. **Clean project** first
2. Check all include paths added
3. Verify source files in build configuration
4. Check USB_DEVICE and Middlewares folders exist

## Files Modified/Created

### Created Files:
- `Middlewares/ST/STM32_USB_Device_Library/Class/MIDI/Inc/usbd_midi.h`
- `Middlewares/ST/STM32_USB_Device_Library/Class/MIDI/Src/usbd_midi.c`
- `README_USB_DEVICE_INTEGRATION.md`
- `README_IMPLEMENTATION_FINAL.md` (this file)

### Modified Files:
- `Config/router_config.h` - Added USB_PORT0-3 nodes
- `Services/usb_midi/usb_midi.c` - Implemented 4-port support
- `Services/usb_midi/usb_midi.h` - Updated documentation
- `Services/usb_host_midi/usb_host_midi.c` - Added router integration
- `Services/usb_host_midi/usb_host_midi.h` - Updated documentation

### To Be Generated by CubeMX:
- `USB_DEVICE/App/usb_device.c`
- `USB_DEVICE/App/usb_device.h`
- `USB_DEVICE/Target/usbd_conf.c`
- `USB_DEVICE/Target/usbd_conf.h`
- `Middlewares/ST/STM32_USB_Device_Library/Core/` (if not exists)

## MIOS32 Compatibility

This implementation is inspired by and compatible with MIOS32:

| Feature | MIOS32 | MidiCore | Status |
|---------|--------|----------|--------|
| USB Device MIDI | ✅ Yes | ✅ Yes | ✅ Compatible |
| 4 virtual ports (cables) | ✅ Yes | ✅ Yes | ✅ Compatible |
| USB Host MIDI | ✅ Yes | ✅ Yes | ✅ Compatible |
| OTG mode switching | ✅ Yes | ✅ Yes | ✅ Compatible |
| Cable number routing | ✅ Yes | ✅ Yes | ✅ Compatible |
| DIN MIDI (4 ports) | ✅ Yes | ✅ Yes | ✅ Compatible |
| Router system | ✅ Yes | ✅ Yes | ✅ Compatible |
| Configurable port count (1-8) | ✅ Yes | ✅ Yes | ✅ Compatible |

## References

- **MIOS32**: https://github.com/midibox/mios32
- **STM32F4 USB MIDI**: mios32/STM32F4xx/mios32_usb_midi.c
- **MIOS32 Descriptors**: mios32/STM32F4xx/mios32_usb.c
- **USB MIDI Spec**: https://www.usb.org/sites/default/files/midi10.pdf

## Support

For issues or questions, refer to:
1. This complete guide
2. `README_USB_DEVICE_INTEGRATION.md` for detailed steps
3. MIOS32 source code for reference implementation
4. USB MIDI specification for protocol details
