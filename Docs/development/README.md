# Development Documentation

Technical documentation, implementation details, and compatibility information for MidiCore developers.

## Implementation Details

### USB Implementation
- **[USB Host MIDI](README_USBH_MIDI.md)** - USB Host implementation for STM32F407
- **[USB Device Integration](README_USB_DEVICE_INTEGRATION.md)** - USB MIDI Device integration guide
- **[Final Implementation](README_IMPLEMENTATION_FINAL.md)** - USB MIDI final configuration guide

### Testing Infrastructure
- **[Implementation Summary](IMPLEMENTATION_SUMMARY.md)** - Module testing infrastructure implementation
- **[Production Mode Fix](PRODUCTION_MODE_FIX.md)** - Critical production issues resolved
- **[Solution Summary](SOLUTION_SUMMARY.md)** - Windows Error 0xC00000E5 fix

## Compatibility & Portability

### MIOS32 Compatibility
- **[Compatibility Index](COMPATIBILITY_INDEX.md)** - MIOS32 compatibility documentation index
- **[Compatibility Summary](COMPATIBILITY_SUMMARY.md)** - Quick reference (98.95% compatible)
- **[Driver Compatibility Report](DRIVER_COMPATIBILITY_REPORT.md)** - Detailed driver analysis
- **[LoopA Compatibility Report](LOOPA_COMPATIBILITY_REPORT.md)** - LoopA features compatibility

### Platform Portability
- **[Portability Guide](README_PORTABILITY.md)** - STM32F4 → STM32H7 migration guide
  - Memory considerations
  - Peripheral differences
  - Performance optimizations
  - Migration checklist

## Architecture

MidiCore is built on a modular architecture featuring:
- **HAL Abstraction Layer** - Hardware abstraction for portability
- **FreeRTOS Tasks** - Real-time multitasking
- **Service Modules** - Looper, UI, Router, Patch Manager
- **Driver Layer** - AINSER64, SRIO, OLED, SD Card

## Development Workflow

1. **Understand** the architecture through implementation docs
2. **Check compatibility** for your target platform
3. **Follow** portability guide for platform changes
4. **Reference** USB implementation for USB features
5. **Test** using the testing infrastructure

## Related Documentation

- **[USB Documentation](../usb/)** - Detailed USB guides and bug fixes
- **[MIOS32 Compatibility](../mios32/)** - MIOS32-specific guides
- **[Configuration](../configuration/)** - System configuration
- **[Testing](../testing/)** - Testing procedures

## For Contributors

When adding new features or fixing bugs:
1. Maintain HAL abstraction for portability
2. Follow existing coding standards
3. Add tests for new functionality
4. Update compatibility reports if needed
5. Document USB changes thoroughly
