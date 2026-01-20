# Using Both USB Device and USB Host - Complete Guide

## Your Question

**"If I select USB Device, how do I keep USB Host?"**

## Short Answer

✅ **Both are kept in your .ioc file!**
- USB_DEVICE middleware: Added (for DAW)
- USB_HOST middleware: Still there (for MIDI keyboards)
- Both can coexist, but not simultaneously on same hardware

## Current .ioc Configuration

Your .ioc file contains:
```
Mcu.IP18=USB_DEVICE      ← For appearing in DAW
Mcu.IP19=USB_HOST        ← For reading MIDI keyboards
Mcu.IP20=USB_OTG_FS      ← Hardware peripheral

USB_OTG_FS.VirtualMode=Device_Only
USB_DEVICE.CLASS_NAME_FS=MIDI
USB_HOST.VirtualModeFS=Hid
```

**Result**: Both middlewares are configured! ✅

## Why Both Can't Be Active Simultaneously

**Hardware Limitation**: STM32F407 has one USB OTG peripheral
- Can be **Device** (slave to computer)
- Can be **Host** (master to MIDI keyboard)
- **Cannot be both at the same time** on same port

This is a hardware limitation, not a software one.

## How to Use Both - Three Approaches

### Approach 1: Primary Device, Manual Host Switch (Recommended)

**Best for**: Using MidiCore in DAW most of the time, occasionally reading MIDI keyboards

**Setup**:
1. Keep current .ioc (Device_Only mode)
2. Generate code → Creates USB_DEVICE files
3. USB_HOST code stays in `USB_HOST/` directory

**Runtime**:
- Normal operation: USB Device mode (visible in DAW)
- When needed: Disconnect, switch to Host mode in code, reconnect with OTG adapter

**Advantage**: Simple, clear mode separation

---

### Approach 2: Automatic Runtime Switching (MIOS32 Style)

**Best for**: Frequently switching between DAW and MIDI keyboard modes

**How it works**:
1. Detect cable type via ID pin (PA10)
2. Initialize appropriate USB mode based on detection
3. Switch modes when cable changes

**Implementation**:

```c
typedef enum {
  USB_MODE_NONE,
  USB_MODE_DEVICE,
  USB_MODE_HOST
} usb_mode_t;

static usb_mode_t current_usb_mode = USB_MODE_NONE;

void usb_check_and_switch_mode(void) {
  // Read ID pin (PA10)
  GPIO_PinState id_pin = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10);
  
  if (id_pin == GPIO_PIN_RESET) {
    // ID LOW = Host mode (OTG adapter connected)
    if (current_usb_mode != USB_MODE_HOST) {
      usb_switch_to_host();
    }
  } else {
    // ID HIGH = Device mode (standard cable)
    if (current_usb_mode != USB_MODE_DEVICE) {
      usb_switch_to_device();
    }
  }
}

void usb_switch_to_device(void) {
  // Deinitialize Host if active
  if (current_usb_mode == USB_MODE_HOST) {
    USBH_Stop(&hUsbHostFS);
    USBH_DeInit(&hUsbHostFS);
  }
  
  // Initialize Device
  MX_USB_DEVICE_Init();
  usb_midi_init();
  
  current_usb_mode = USB_MODE_DEVICE;
}

void usb_switch_to_host(void) {
  // Deinitialize Device if active
  if (current_usb_mode == USB_MODE_DEVICE) {
    USBD_Stop(&hUsbDeviceFS);
    USBD_DeInit(&hUsbDeviceFS);
  }
  
  // Initialize Host
  MX_USB_HOST_Init();
  usb_host_midi_init();
  
  current_usb_mode = USB_MODE_HOST;
}
```

**Advantage**: Automatic switching, like MIOS32

---

### Approach 3: Two USB Ports (Hardware Solution)

**If your hardware has two USB ports**:
- USB_OTG_FS: Device mode (DAW)
- USB_OTG_HS: Host mode (MIDI keyboards)

**Setup**: Configure both in CubeMX, no switching needed

**Advantage**: Both active simultaneously

**Note**: STM32F407VG has only one USB OTG FS port, so this requires hardware modification

---

## What Happens in CubeMX

### When You Open .ioc:

**You'll see**:
- ✅ Connectivity → USB_OTG_FS: Configured as Device_Only
- ✅ Middleware → USB_DEVICE: Enabled with MIDI class
- ✅ Middleware → USB_HOST: Still present but grayed out

**Why USB_HOST is grayed out**:
- CubeMX allows only one USB middleware active at a time
- Hardware can only be one mode
- This is expected and correct!

**USB_HOST code is NOT deleted**:
- Files in `USB_HOST/` directory remain
- Initialization code `MX_USB_HOST_Init()` stays in main.c
- You can call it when needed

### When You Generate Code:

**Generated**:
- `USB_DEVICE/App/usb_device.c` and `.h`
- `USB_DEVICE/Target/usbd_conf.c` and `.h`

**Preserved**:
- `USB_HOST/App/usb_host.c` and `.h`
- `USB_HOST/Target/usbh_conf.c` and `.h`
- All USB Host library files
- `MX_USB_HOST_Init()` function

**Result**: You have both sets of files! ✅

---

## Recommended Configuration for Your Use Case

Based on your requirements:

### Primary: USB Device Mode
**For**: Appearing in DAW with 4 MIDI ports

**Configuration**:
- Current .ioc setup ✅
- Generate code with USB_DEVICE
- Use `usb_midi_init()` in main.c

**Result**: MidiCore appears as 4-port MIDI interface

### Secondary: USB Host Mode (When Needed)
**For**: Reading USB MIDI keyboards

**How to activate**:
```c
// In your code, when you want Host mode:
void switch_to_midi_keyboard_mode(void) {
  // Stop Device mode
  USBD_Stop(&hUsbDeviceFS);
  USBD_DeInit(&hUsbDeviceFS);
  
  // Start Host mode
  MX_USB_HOST_Init();
  usb_host_midi_init();
  
  // Now you can read MIDI keyboards
  while(1) {
    usb_host_midi_task();
    osDelay(1);
  }
}
```

---

## Practical Example: Mode Switching

### Scenario: DAW by Default, Keyboard on Button Press

```c
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  
  // Start in Device mode (for DAW)
  usb_midi_init();
  
  while(1)
  {
    // Check if user button pressed (for mode switch)
    if (HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin) == GPIO_PIN_RESET)
    {
      // Switch to Host mode
      usb_switch_to_host();
      
      // Stay in Host mode until button pressed again
      while(HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin) == GPIO_PIN_RESET)
      {
        usb_host_midi_task();
        osDelay(1);
      }
      
      // Switch back to Device mode
      usb_switch_to_device();
    }
    
    // Process Device mode MIDI
    // (router already handles this)
    
    osDelay(10);
  }
  
  /* USER CODE END 5 */
}
```

---

## Summary

### You DON'T Lose USB Host! ✅

**What you have**:
- ✅ USB_DEVICE configured (for DAW)
- ✅ USB_HOST configured (for MIDI keyboards)
- ✅ Both code sets in project
- ✅ Both initialization functions available

**How to use**:
- **Device mode**: Active by default (current .ioc)
- **Host mode**: Available, switch to it when needed
- **Both**: Use runtime switching code (see examples above)

### Your .ioc is Correct

The current configuration is good:
- Device_Only mode makes USB_DEVICE selectable
- USB_HOST code is preserved
- You can use both, just not simultaneously

### Next Steps

1. **Generate code** from CubeMX
2. **Verify both directories exist**:
   - `USB_DEVICE/` → New files
   - `USB_HOST/` → Existing files (preserved)
3. **Integrate MIDI class** (follow `WHAT_TO_DO_NOW.md`)
4. **Implement mode switching** if needed (use examples above)

---

## Files Reference

All files you need:
- `WHAT_TO_DO_NOW.md` - Integration steps
- `CUBEMX_OTG_ISSUE_EXPLAINED.md` - Why Device_Only mode
- `FREERTOS_PROTECTION_GUIDE.md` - How to add mode switching code
- This guide - How to use both Device and Host

You're all set! 🎉
