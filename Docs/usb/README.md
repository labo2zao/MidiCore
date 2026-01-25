# USB Documentation

> 🇫🇷 [Version française disponible](README_FR.md)

USB MIDI implementation and debugging guides for MidiCore.

## Available Guides

### Configuration & Setup
- **[USB Configuration Guide](USB_CONFIGURATION_GUIDE.md)** - Complete USB setup guide
- **[USB Device and Host Guide](USB_DEVICE_AND_HOST_GUIDE.md)** - Comprehensive device and host guide
- **[USB Host and Device Explained](USB_HOST_AND_DEVICE_EXPLAINED.md)** - Architecture explanation

### Debugging
- **[USB Debug Guide](USB_DEBUG_GUIDE.md)** - Complete debugging guide for USB issues
- **[USB Debug UART Quickstart](USB_DEBUG_UART_QUICKSTART.md)** - Quick debug setup via UART

### CubeMX Integration
- **[CubeMX OTG Issue Explained](CUBEMX_OTG_ISSUE_EXPLAINED.md)** - OTG problem resolution
- **[Protect USB Host from CubeMX](PROTECT_USB_HOST_FROM_CUBEMX.md)** - Protect from CubeMX regeneration
- **[USB MIDI CubeMX Protection](USB_MIDI_CUBEMX_PROTECTION.md)** - MIDI-specific protection guide

### USB MIDI Jacks
- **[USB MIDI Jacks Explained](USB_MIDI_JACKS_EXPLAINED.md)** - Jack configuration (English)
- **[USB MIDI Jacks Explications](USB_MIDI_JACKS_EXPLICATIONS_FR.md)** - Configuration des jacks (Français)

### Technical Documentation
- **[USB Descriptor Structure](USB_DESCRIPTOR_STRUCTURE.txt)** - Descriptor structure details
- **[USB MIDI Descriptor Analysis](USB_MIDI_DESCRIPTOR_ANALYSIS.md)** - Detailed descriptor analysis
- **[USB MIDI Protocol Audit](USB_MIDI_PROTOCOL_AUDIT.md)** - Protocol implementation audit
- **[USB MIDI SysEx Engine](USB_MIDI_SYSEX_ENGINE.md)** - SysEx implementation guide

### Known Issues & Fixes
- **[STM32 USB Library Bugs](STM32_USB_LIBRARY_BUGS.md)** - Known STM32 library issues
- **[USB Bulk Endpoint Bug Fix](USB_BULK_ENDPOINT_BUG_FIX.md)** - Bulk endpoint fix
- **[USB Descriptor Size Bug Fix](USB_DESCRIPTOR_SIZE_BUG_FIX.md)** - Descriptor size fix
- **[USB Device Descriptor Failure Analysis](USB_DEVICE_DESCRIPTOR_FAILURE_ANALYSIS.md)** - Device descriptor failures
- **[USB IAD Fix](USB_IAD_FIX.md)** - Interface Association Descriptor fix
- **[USB IAD Removal Fix](USB_IAD_REMOVAL_FIX.md)** - IAD removal fix
- **[USB MIDI MS Header Bug Fix](USB_MIDI_MS_HEADER_BUG_FIX.md)** - MS header bug fix

## USB Overview

MidiCore implements comprehensive USB MIDI support:
- **USB MIDI Device**: STM32 appears as MIDI device to computer
- **USB MIDI Host**: Connect external USB MIDI controllers/keyboards
- **Dual Mode**: Device and Host can operate simultaneously (with hardware support)
- **MIOS32 Compatible**: Uses standard MIOS32 USB MIDI protocol

## Quick Start

1. **Configure**: Follow [USB Configuration Guide](USB_CONFIGURATION_GUIDE.md)
2. **Protect**: Use [CubeMX Protection](USB_MIDI_CUBEMX_PROTECTION.md) before regeneration
3. **Debug**: Check [USB Debug Guide](USB_DEBUG_GUIDE.md) if issues occur
4. **Fix**: Reference known bug fixes if needed

## Related Documentation

- **[Development](../development/)** - USB implementation details
- **[MIOS32](../mios32/)** - MIOS32 USB compatibility
- **[Configuration](../configuration/)** - System configuration
- **[Testing](../testing/)** - USB testing procedures
