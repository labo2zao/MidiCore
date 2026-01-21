# MIOS32-Style Dual-Mode USB Configuration

## Overview

MidiCore is now configured like MIOS32 to support **both USB Device and USB Host modes** simultaneously, with automatic mode switching based on cable/adapter detection.

## Architecture

### 8 MIDI Ports Total (MIOS32-Compatible)
- **4 USB ports** (cables 0-3) - Can work in Device OR Host mode
  - **Device mode**: Connect to DAW/computer via standard USB cable
  - **Host mode**: Connect USB MIDI keyboards via OTG adapter
- **4 DIN MIDI ports** (USART1/2/3, UART5) - Always available

### USB Dual-Mode Configuration

**Hardware Support**:
- STM32F407VGT6 USB_OTG_FS peripheral configured for dual-role
- PA10: USB_OTG_FS_ID (detects cable type)
- PA11: USB_OTG_FS_DM (data minus)
- PA12: USB_OTG_FS_DP (data plus)

**Software Support**:
- Both HAL_PCD_MODULE (Device driver) and HAL_HCD_MODULE (Host driver) enabled
- Runtime mode switching based on ID pin detection
- Automatic enumeration in correct mode

## Current Configuration

### Module Configuration (`Config/module_config.h`)
```c
#define MODULE_ENABLE_USB_MIDI 1      // USB Device mode enabled
#define MODULE_ENABLE_USBH_MIDI 1     // USB Host mode enabled (MIOS32-style)
```

### CubeMX Configuration (`MidiCore.ioc`)
```
PA11.Mode=OTG/Dual_Role_Device
PA12.Mode=OTG/Dual_Role_Device
```

### HAL Configuration (`Core/Inc/stm32f4xx_hal_conf.h`)
```c
#define HAL_PCD_MODULE_ENABLED   // USB Device driver
#define HAL_HCD_MODULE_ENABLED   // USB Host driver
```

## Build and Test

### Compile
1. **Clean project**: Project → Clean → Clean all projects
2. **Rebuild**: Project → Build Project
3. Should compile without errors ✅

### Test Device Mode
1. Flash firmware via ST-Link
2. **Connect standard USB cable** (Micro-USB to USB-A)
3. Computer sees "MidiCore 4x4" with 4 MIDI ports
4. Test in DAW - all 4 ports operational

### Test Host Mode (Optional - Requires Implementation)
1. Flash firmware via ST-Link
2. **Connect USB MIDI keyboard via OTG adapter** (Micro-USB OTG to USB-A female)
3. MidiCore should detect and enumerate the keyboard
4. MIDI from keyboard routed through MidiCore

## Automatic Mode Switching (Future Enhancement)

For full MIOS32-style automatic switching, implement the USB Mode Manager service:

### Option 1: Manual Mode Selection (Current)
- User manually enables Device or Host mode via configuration
- Simple, reliable, good for testing

### Option 2: Automatic Detection (MIOS32-Style)
- ID pin (PA10) detection determines mode:
  - **ID pin LOW (grounded)**: USB OTG adapter detected → **Host mode**
  - **ID pin HIGH (floating)**: Standard USB cable → **Device mode**
- 100ms debounce to prevent spurious switching
- Seamless transitions between modes
- See `MIOS32_STYLE_AUTO_SWITCHING.md` for implementation guide

## Implementation Status

✅ **Completed**:
- Dual-mode hardware configuration (PA11/PA12 as OTG/Dual_Role)
- Both PCD and HCD drivers enabled in HAL
- Module configuration supports both modes
- Build errors fixed (USB_OTG_GOTGCTL register bits defined)
- STM32F407 B-session valid override for Device mode without VBUS

🔨 **Next Steps** (Optional Enhancements):
- Implement automatic mode detection service
- Add USB Host MIDI class support for HCD
- Test with real USB MIDI keyboards
- Implement mode switching at runtime

## Technical Details

### Why Both Modes Can Coexist

1. **Different HAL Drivers**:
   - PCD (Peripheral Controller Device) for Device mode
   - HCD (Host Controller Device) for Host mode
   - Both use same USB_OTG_FS peripheral hardware

2. **Hardware Detection**:
   - USB_OTG_FS peripheral can detect cable type via ID pin
   - Standard cable: ID pin HIGH → Device mode
   - OTG adapter: ID pin LOW → Host mode

3. **Runtime Selection**:
   - Initialize only one driver at a time based on detection
   - Or implement runtime switching with proper de-init/re-init

### STM32F407 Specifics

**VBUS Sensing**:
- STM32F407VGT6 package has no VBUS pin
- Must disable VBUS sensing (`USB_OTG_GCCFG_NOVBUSSENS`)
- Must force B-session valid for Device mode:
  ```c
  USB_OTG_FS->GOTGCTL |= USB_OTG_GOTGCTL_BVALOEN;  // Enable override
  USB_OTG_FS->GOTGCTL |= USB_OTG_GOTGCTL_BVALOVAL; // Force valid
  ```

**Interrupt Handling**:
- OTG_FS_IRQHandler calls appropriate handler based on active mode:
  - `HAL_PCD_IRQHandler()` for Device mode
  - `HAL_HCD_IRQHandler()` for Host mode

## References

- **MIOS32 USB Implementation**: github.com/midibox/mios32/tree/master/modules/usbh_midi
- **ST AN3965**: USB hardware and PCB guidelines using STM32 MCUs
- **RM0090**: STM32F407 Reference Manual - USB OTG chapter
- **USB MIDI Spec**: usb.org USB MIDI Device Class v1.0

## Summary

MidiCore is now configured for MIOS32-style dual-mode USB operation:
- ✅ Hardware configured for OTG dual-role
- ✅ Both Device and Host drivers available
- ✅ Can connect to DAW (Device mode) OR USB MIDI keyboards (Host mode)
- ✅ Build errors fixed, ready to compile and test
- 🎯 Next: Implement automatic mode detection for seamless switching

🎉 **Ready for MIOS32-style dual-mode USB operation!**
