# MIOS32 Compatibility

> 🇫🇷 [Version française disponible](README_FR.md)

MIOS32 compatibility and migration guides for MidiCore.

## Available Guides

### Compatibility Documentation
- **[MIOS32 Compatibility](MIOS32_COMPATIBILITY.md)** - Overall compatibility assessment (98.95% compatible)
- **[MIOS32 Deep Comparison](MIOS32_DEEP_COMPARISON.md)** - Detailed feature-by-feature comparison
- **[MIOS32 Descriptor Analysis](MIOS32_DESCRIPTOR_ANALYSIS.md)** - USB descriptor compatibility analysis

### Implementation Guides
- **[MIOS32 USB Implementation Guide](MIOS32_USB_IMPLEMENTATION_GUIDE.md)** - USB MIDI implementation guide
- **[MIOS32 Dual Mode Guide](MIOS32_DUAL_MODE_GUIDE.md)** - Dual-mode USB operation
- **[MIOS32 Style Auto Switching](MIOS32_STYLE_AUTO_SWITCHING.md)** - Automatic mode switching

## Compatibility Overview

MidiCore maintains high compatibility with MIOS32/MIDIbox ecosystem:

### Hardware Compatibility (100%)
- **AINSER64**: 100% compatible hardware and protocol
- **SRIO**: 100% compatible (MBHP naming conventions)
- **MIDI DIN**: 100% compatible (31.25k baud, running status)
- **USB MIDI Bootloader**: MIOS32-compatible SysEx protocol

### Software Compatibility (98.95%)
- **Patch Format**: Compatible TXT key=value format
- **Looper**: Based on LoopA (96 PPQN, quantization)
- **API**: Similar function signatures and behavior
- **Configuration**: Compatible module enable/disable system

### Known Differences
- FreeRTOS vs MIOS32 task scheduler (different APIs)
- HAL abstraction layer (MidiCore-specific)
- Some peripheral implementations optimized for STM32F4

## Migration from MIOS32

1. **Read** [MIOS32 Compatibility](MIOS32_COMPATIBILITY.md) for overview
2. **Compare** features in [Deep Comparison](MIOS32_DEEP_COMPARISON.md)
3. **Review** USB implementation in [USB Implementation Guide](MIOS32_USB_IMPLEMENTATION_GUIDE.md)
4. **Check** descriptor compatibility in [Descriptor Analysis](MIOS32_DESCRIPTOR_ANALYSIS.md)
5. **Test** your application with MidiCore

## Related Documentation

- **[Development](../development/)** - Technical implementation details
- **[Configuration](../configuration/)** - UART and module configuration
- **[Hardware](../hardware/)** - MBHP hardware compatibility
- **[USB](../usb/)** - USB MIDI implementation

## External Resources

- [MIOS32 Project](http://www.midibox.org/mios32/)
- [MIDIbox Wiki](http://wiki.midibox.org/)
- [MBHP Hardware](http://www.ucapps.de/mbhp.html)
