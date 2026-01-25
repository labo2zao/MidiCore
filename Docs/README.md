# MidiCore Documentation

> **✨ Documentation Consolidated (January 2026)!** All documentation files have been significantly consolidated from 132 files to 70 files (-47%) while preserving all information. Each folder now contains maximum 12 well-organized documents for easier navigation.

> **🌐 Bilingual Support:** Most folders now include README_FR.md for French translations. Look for 🇫🇷/🇬🇧 flags in folder README files.

Complete documentation for the MidiCore MIDI controller system.

## 📚 Documentation Structure

### 🚀 [Getting Started](getting-started/) • [🇫🇷 Français](getting-started/README_FR.md)
Quick start guides and project integration
- [Main Guide](getting-started/README.md) - Consolidated integration guide
- [What To Do Now](getting-started/WHAT_TO_DO_NOW.md) - Clear action plan

### 👤 [User Guides](user-guides/) • [🇫🇷 Français](user-guides/README_FR.md)
End-user features and functionality guides
- [Overview](user-guides/README.md) - User guides overview
- [Footswitch Guide](user-guides/FOOTSWITCH_GUIDE.md) - Pedal integration
- [LoopA Features Plan](user-guides/LOOPA_FEATURES_PLAN.md) - Looper functionality
- [UI Implementation](user-guides/UI_LOOPA_IMPLEMENTATION.md) - User interface guide
- [UI Page Testing](user-guides/UI_PAGE_TESTING_GUIDE.md) - UI testing guide
- [Automation System](user-guides/AUTOMATION_SYSTEM.md) - Automation documentation
- [SCS Buttons Analysis](user-guides/SCS_BUTTONS_ANALYSIS.md) - Button system analysis

### 🔌 [Hardware](hardware/) • [🇫🇷 Français](hardware/README_FR.md)
Hardware setup, wiring guides, and pinout information
- [Overview](hardware/README.md) - Hardware documentation overview
- **[OLED Guide](hardware/OLED_GUIDE.md)** - Complete OLED display guide (wiring, testing, troubleshooting, CubeMX config)
- **[UI Guide](hardware/UI_GUIDE.md)** - Complete UI system guide (fonts, design, layout, rendering)
- [NHD-OLEDSSD1322DISP.pdf](hardware/NHD-OLEDSSD1322DISP.pdf) - OLED datasheet
- [SSD1322.pdf](hardware/SSD1322%20(4).pdf) - SSD1322 controller datasheet

### ⚙️ [Configuration](configuration/) • [🇫🇷 Français](configuration/README_FR.md)
System configuration and setup
- [Overview](configuration/README.md) - Configuration overview
- [Module Configuration](configuration/README_MODULE_CONFIG.md) - Enable/disable modules
- [MIOS32 UART Config](configuration/README_MIOS32_UART_CONFIG.md) - UART and debug setup
- [SPI Configuration](configuration/SPI_CONFIGURATION_REFERENCE.md) - SPI parameters
- [CubeMX Regeneration](configuration/CUBEMX_REGENERATION_GUIDE.md) - Protect custom code
- [FreeRTOS Protection](configuration/FREERTOS_PROTECTION_GUIDE.md) - Protect tasks from CubeMX

### 🧪 [Testing](testing/) • [🇫🇷 Français](testing/README_FR.md)
Testing procedures and validation
- [Overview](testing/README.md) - Testing documentation overview
- [Testing Protocol](testing/TESTING_PROTOCOL.md) - Comprehensive procedures (300+ tests)
- [OLED Test Protocol](testing/OLED_TEST_PROTOCOL.md) - OLED display validation (15 test modes)
- **[Module Testing](testing/MODULE_TESTING.md)** - Complete module testing guide with all procedures
- **[Phase Testing](testing/PHASE_TESTING.md)** - Implementation status for Phases 2, 3, and B
- **[MIDI DIN Testing](testing/MIDI_DIN_TESTING.md)** - MIDI DIN quick start, LiveFX, and advanced features
- **[Specialized Testing](testing/SPECIALIZED_TESTING.md)** - Breath controller, USB MIDI, execution, debug checklists

### 🔧 [Development](development/) • [🇫🇷 Français](development/README_FR.md)
Technical documentation and implementation details
- [Overview](development/README.md) - Development documentation overview
- **[Bootloader](development/BOOTLOADER.md)** - Complete bootloader guide (USB MIDI, memory layout, build modes)
- **[Implementation](development/IMPLEMENTATION.md)** - Module testing infrastructure, production utilities, USB MIDI
- **[Compatibility](development/COMPATIBILITY.md)** - MIOS32 compatibility analysis (98.95%), drivers, LoopA features
- **[USB Integration](development/USB_INTEGRATION.md)** - USB Device MIDI (4 ports) and USB Host MIDI
- [Build Modes](development/BUILD_MODES.md) - Build configuration modes
- [Production Utilities](development/PRODUCTION_UTILITIES.md) - Production tools and critical fixes
- [RAM Overflow Fix](development/RAM_OVERFLOW_FIX.md) - RAM overflow resolution
- [Portability Guide](development/README_PORTABILITY.md) - STM32F4/H7 migration
- [Module Details](development/Modules_MidiCore_Detail_par_Module.txt) - Detailed architecture (French)

### 🔌 [USB Documentation](usb/) • [🇫🇷 Français](usb/README_FR.md)
USB MIDI implementation and debugging
- [Overview](usb/README.md) - USB documentation overview
- [USB Configuration Guide](usb/USB_CONFIGURATION_GUIDE.md) - USB setup
- [USB Debug Guide](usb/USB_DEBUG_GUIDE.md) - Debugging USB issues
- [USB Debug UART Quickstart](usb/USB_DEBUG_UART_QUICKSTART.md) - Quick debug setup
- [Protect USB Host](usb/PROTECT_USB_HOST_FROM_CUBEMX.md) - Protect from regeneration
- **[USB Host & Device](usb/USB_HOST_DEVICE.md)** - Complete architecture guide (Device + Host modes)
- **[USB Bug Fixes](usb/USB_BUG_FIXES.md)** - All critical USB MIDI descriptor bugs and fixes
- **[USB Technical](usb/USB_TECHNICAL.md)** - Complete technical reference (descriptors, protocol, SysEx)
- **[USB Jacks](usb/USB_JACKS.md)** - Bilingual explanation of USB MIDI "Jacks" terminology
- **[USB CubeMX](usb/USB_CUBEMX.md)** - STM32CubeMX configuration and protection guide

### 🔄 [MIOS32 Compatibility](mios32/) • [🇫🇷 Français](mios32/README_FR.md)
MIOS32 compatibility and migration guides
- [Overview](mios32/README.md) - MIOS32 compatibility overview
- **[MIOS32 Compatibility](mios32/MIOS32_COMPATIBILITY.md)** - Complete compatibility analysis (98.95%) and deep comparison
- **[MIOS32 USB](mios32/MIOS32_USB.md)** - Dual-mode USB implementation and automatic switching
- [MIOS32 Descriptor Analysis](mios32/MIOS32_DESCRIPTOR_ANALYSIS.md) - Descriptor details

### 💼 [Commercial](commercial/)
Commercial documentation and presentations
- **[Commercial Guide](commercial/COMMERCIAL_GUIDE.md)** - Bilingual (🇬🇧/🇫🇷) complete commercial documentation and product presentation

---

## Quick Links

- **New to MidiCore?** → Start with [Getting Started](getting-started/)
- **Need to configure?** → See [Configuration](configuration/)
- **Want to test?** → Check [Testing Quick Start](testing/TESTING_QUICKSTART.md)
- **USB issues?** → Browse [USB Documentation](usb/)
- **From MIOS32?** → Read [MIOS32 Compatibility](mios32/)

## Additional Resources

- **Main README**: [../README.md](../README.md)
- **Module Details**: [development/Modules_MidiCore_Detail_par_Module.txt](development/Modules_MidiCore_Detail_par_Module.txt)
- **Source Code**: [../App/](../App/), [../Services/](../Services/), [../Hal/](../Hal/)
