# MidiCore Documentation

Complete documentation for the MidiCore MIDI controller system.

## 📚 Documentation Structure

### 🚀 [Getting Started](getting-started/)
Quick start guides and project integration
- [Integration Guide](getting-started/README_INTEGRATION.md) - Add modules to your project
- [Project Integration](getting-started/README_PROJECT_INTEGRATION.md) - Merged project overview
- [Bundle Guide](getting-started/README_BUNDLE.md) - AccordeonInstrument bundle
- [What To Do Now](getting-started/WHAT_TO_DO_NOW.md) - Clear action plan

### 👤 [User Guides](user-guides/)
End-user features and functionality guides
- [Footswitch Guide](user-guides/FOOTSWITCH_GUIDE.md) - Pedal integration
- [LoopA Features Plan](user-guides/LOOPA_FEATURES_PLAN.md) - Looper functionality
- [UI Implementation](user-guides/UI_LOOPA_IMPLEMENTATION.md) - User interface guide
- [UI Page Testing](user-guides/UI_PAGE_TESTING_GUIDE.md) - UI testing guide
- [Automation System](user-guides/AUTOMATION_SYSTEM.md) - Automation documentation
- [SCS Buttons Analysis](user-guides/SCS_BUTTONS_ANALYSIS.md) - Button system analysis

### ⚙️ [Configuration](configuration/)
System configuration and setup
- [Module Configuration](configuration/README_MODULE_CONFIG.md) - Enable/disable modules
- [MIOS32 UART Config](configuration/README_MIOS32_UART_CONFIG.md) - UART and debug setup
- [SPI Configuration](configuration/SPI_CONFIGURATION_REFERENCE.md) - SPI parameters
- [CubeMX Regeneration](configuration/CUBEMX_REGENERATION_GUIDE.md) - Protect custom code
- [FreeRTOS Protection](configuration/FREERTOS_PROTECTION_GUIDE.md) - Protect tasks from CubeMX

### 🧪 [Testing](testing/)
Testing procedures and validation
- [Testing Quick Start](testing/TESTING_QUICKSTART.md) - Quick test examples
- [Module Testing Guide](testing/README_MODULE_TESTING.md) - Complete testing guide
- [Testing Protocol](testing/TESTING_PROTOCOL.md) - Comprehensive procedures (300+ tests)
- [Test Execution](testing/TEST_EXECUTION.md) - Execution details
- [Test Validation Report](testing/TEST_VALIDATION_REPORT.md) - Validation results
- [Debug Checklist](testing/FINAL_DEBUG_CHECKLIST.md) - Systematic verification

### 🔧 [Development](development/)
Technical documentation and implementation details
- [Implementation Summary](development/IMPLEMENTATION_SUMMARY.md) - Testing infrastructure
- [USB Host MIDI](development/README_USBH_MIDI.md) - USB Host implementation
- [USB Device Integration](development/README_USB_DEVICE_INTEGRATION.md) - USB Device setup
- [Portability Guide](development/README_PORTABILITY.md) - STM32F4/H7 migration
- [Compatibility Index](development/COMPATIBILITY_INDEX.md) - MIOS32 compatibility index
- [Compatibility Summary](development/COMPATIBILITY_SUMMARY.md) - Quick reference
- [Driver Compatibility](development/DRIVER_COMPATIBILITY_REPORT.md) - Driver analysis
- [LoopA Compatibility](development/LOOPA_COMPATIBILITY_REPORT.md) - LoopA features
- [Production Mode Fix](development/PRODUCTION_MODE_FIX.md) - Critical fixes
- [Solution Summary](development/SOLUTION_SUMMARY.md) - Windows error fix
- [Final Implementation](development/README_IMPLEMENTATION_FINAL.md) - USB MIDI final config

### 🔌 [USB Documentation](usb/)
USB MIDI implementation and debugging
- [USB Configuration Guide](usb/USB_CONFIGURATION_GUIDE.md) - USB setup
- [USB Device and Host Guide](usb/USB_DEVICE_AND_HOST_GUIDE.md) - Complete guide
- [USB Host Explained](usb/USB_HOST_AND_DEVICE_EXPLAINED.md) - Architecture explanation
- [USB Debug Guide](usb/USB_DEBUG_GUIDE.md) - Debugging USB issues
- [USB Debug UART Quickstart](usb/USB_DEBUG_UART_QUICKSTART.md) - Quick debug setup
- [CubeMX OTG Issue](usb/CUBEMX_OTG_ISSUE_EXPLAINED.md) - OTG problem resolution
- [Protect USB Host](usb/PROTECT_USB_HOST_FROM_CUBEMX.md) - Protect from regeneration
- [STM32 USB Library Bugs](usb/STM32_USB_LIBRARY_BUGS.md) - Known library issues
- [USB MIDI CubeMX Protection](usb/USB_MIDI_CUBEMX_PROTECTION.md) - Protection guide
- [USB MIDI Jacks Explained](usb/USB_MIDI_JACKS_EXPLAINED.md) - Jack configuration (EN)
- [USB MIDI Jacks Explications](usb/USB_MIDI_JACKS_EXPLICATIONS_FR.md) - Jack configuration (FR)

**Bug Fixes:**
- [Bulk Endpoint Bug Fix](usb/USB_BULK_ENDPOINT_BUG_FIX.md)
- [Descriptor Size Bug Fix](usb/USB_DESCRIPTOR_SIZE_BUG_FIX.md)
- [Device Descriptor Failure](usb/USB_DEVICE_DESCRIPTOR_FAILURE_ANALYSIS.md)
- [IAD Fix](usb/USB_IAD_FIX.md)
- [IAD Removal Fix](usb/USB_IAD_REMOVAL_FIX.md)
- [MS Header Bug Fix](usb/USB_MIDI_MS_HEADER_BUG_FIX.md)

### 🔄 [MIOS32 Compatibility](mios32/)
MIOS32 compatibility and migration guides
- [MIOS32 Compatibility](mios32/MIOS32_COMPATIBILITY.md) - Overall compatibility
- [MIOS32 Deep Comparison](mios32/MIOS32_DEEP_COMPARISON.md) - Detailed analysis
- [MIOS32 Dual Mode Guide](mios32/MIOS32_DUAL_MODE_GUIDE.md) - Dual-mode USB
- [MIOS32 USB Implementation](mios32/MIOS32_USB_IMPLEMENTATION_GUIDE.md) - USB guide
- [MIOS32 Style Auto Switching](mios32/MIOS32_STYLE_AUTO_SWITCHING.md) - Mode switching
- [MIOS32 Descriptor Analysis](mios32/MIOS32_DESCRIPTOR_ANALYSIS.md) - Descriptor details

### 💼 [Commercial](commercial/)
Commercial documentation and presentations
- [Commercial README](commercial/README_COMMERCIAL.md) - Professional system overview (EN)
- [Commercial README FR](commercial/README_COMMERCIAL_FR.md) - Vue d'ensemble professionnelle (FR)
- [Product Presentation](commercial/PRESENTATION_PRODUIT.md) - Présentation produit (FR)

---

## Quick Links

- **New to MidiCore?** → Start with [Getting Started](getting-started/)
- **Need to configure?** → See [Configuration](configuration/)
- **Want to test?** → Check [Testing Quick Start](testing/TESTING_QUICKSTART.md)
- **USB issues?** → Browse [USB Documentation](usb/)
- **From MIOS32?** → Read [MIOS32 Compatibility](mios32/)

## Additional Resources

- **Main README**: [../README.md](../README.md)
- **Module Details**: [../Modules_MidiCore_Detail_par_Module.txt](../Modules_MidiCore_Detail_par_Module.txt)
- **Source Code**: [../App/](../App/), [../Services/](../Services/), [../Hal/](../Hal/)
