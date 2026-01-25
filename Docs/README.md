# MidiCore Documentation

> **✨ Documentation has been recently reorganized!** All documentation files have been consolidated and organized into this folder structure. See [DOCUMENTATION_REORGANIZATION.md](DOCUMENTATION_REORGANIZATION.md) for details about what was moved and where.

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
- [OLED Wiring Guide](hardware/OLED_WIRING_GUIDE.md) - OLED display wiring (SSD1322/SSD1306 LoopA compatible)
- [OLED Improvements Summary](hardware/OLED_IMPROVEMENTS_SUMMARY.md) - OLED display improvements
- [OLED SSD1322 Fix History](hardware/OLED_SSD1322_FIX_HISTORY.md) - Fix history for SSD1322
- [OLED SSD1322 Technical Reference](hardware/OLED_SSD1322_TECHNICAL_REFERENCE.md) - Technical reference
- [OLED Quick Test](hardware/OLED_QUICK_TEST.md) - Quick OLED test procedure
- [OLED Test Page Guide](hardware/OLED_TEST_PAGE_GUIDE.md) - Test page guide
- [OLED Troubleshooting](hardware/OLED_TROUBLESHOOTING.md) - Troubleshooting guide
- [J1 OLED Connector Pinout](hardware/J1_OLED_CONNECTOR_PINOUT.md) - Connector pinout
- [UI Improvements Summary](hardware/UI_IMPROVEMENTS_SUMMARY.md) - UI improvements
- [UI Rendering Improvements](hardware/UI_RENDERING_IMPROVEMENTS.md) - Rendering improvements
- [UI Layout Ergonomic](hardware/UI_LAYOUT_ERGONOMIC.md) - Ergonomic layout guide
- [Combined Key Navigation](hardware/COMBINED_KEY_NAVIGATION.md) - Key navigation system
- [CubeMX Regeneration Guide](hardware/CUBEMX_REGENERATION_GUIDE.md) - Protect custom code
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
- [Testing Quick Start](testing/TESTING_QUICKSTART.md) - Quick test examples
- [Module Testing Guide](testing/README_MODULE_TESTING.md) - Complete testing guide
- [Testing Protocol](testing/TESTING_PROTOCOL.md) - Comprehensive procedures (300+ tests)
- [OLED Test Protocol](testing/OLED_TEST_PROTOCOL.md) - OLED display validation (15 test modes)
- [Test Execution](testing/TEST_EXECUTION.md) - Execution details
- [Test Validation Report](testing/TEST_VALIDATION_REPORT.md) - Validation results
- [Debug Checklist](testing/FINAL_DEBUG_CHECKLIST.md) - Systematic verification
- [Phase 2 Complete Module Test All](testing/PHASE_2_COMPLETE_MODULE_TEST_ALL.md) - Phase 2 testing
- [Phase 3 Advanced Features](testing/PHASE_3_ADVANCED_FEATURES.md) - Phase 3 testing (from Docs/testing/)
- [Phase 3 Complete Advanced Features](testing/PHASE_3_COMPLETE_ADVANCED_FEATURES.md) - Phase 3 testing (moved from root)
- [README Tests](testing/README_TESTS.md) - Test documentation
- [README USB MIDI Test](testing/README_USB_MIDI_TEST.md) - USB MIDI testing
- [Breath Controller Test Guide](testing/BREATH_CONTROLLER_TEST_GUIDE.md) - Breath controller testing
- [MIDI DIN Examples](testing/MIDI_DIN_EXAMPLES.md) - MIDI DIN test examples
- [MIDI DIN LiveFX Test](testing/MIDI_DIN_LIVEFX_TEST.md) - LiveFX testing
- [MIDI DIN New Features](testing/MIDI_DIN_NEW_FEATURES.md) - New features testing
- [Module Test All](testing/MODULE_TEST_ALL.md) - All module testing
- [Module Test MIDI DIN All Phases Roadmap](testing/MODULE_TEST_MIDI_DIN_ALL_PHASES_ROADMAP.md) - Testing roadmap
- [Module Test MIDI DIN Summary](testing/MODULE_TEST_MIDI_DIN_SUMMARY.md) - Testing summary
- [Module Test Patch SD](testing/MODULE_TEST_PATCH_SD.md) - Patch SD testing
- [Module Test Router Output](testing/MODULE_TEST_ROUTER_OUTPUT.md) - Router testing
- [Phase B Implementation Guide](testing/PHASE_B_IMPLEMENTATION_GUIDE.md) - Phase B guide
- [Quickstart Patch SD](testing/QUICKSTART_PATCH_SD.md) - Patch SD quickstart

### 🔧 [Development](development/) • [🇫🇷 Français](development/README_FR.md)
Technical documentation and implementation details
- [Overview](development/README.md) - Development documentation overview
- [Bootloader Implementation](development/BOOTLOADER_IMPLEMENTATION.md) - USB MIDI bootloader implementation
- [Bootloader Verification](development/BOOTLOADER_VERIFICATION.md) - Bootloader verification guide
- [README Bootloader](development/README_BOOTLOADER.md) - Main bootloader documentation
- [README Bootloader RAM](development/README_BOOTLOADER_RAM.md) - RAM-based bootloader
- [Build Modes](development/BUILD_MODES.md) - Build configuration modes
- [Complete Implementation Summary](development/COMPLETE_IMPLEMENTATION_SUMMARY.md) - Complete implementation
- [Implementation Complete](development/IMPLEMENTATION_COMPLETE.md) - Implementation status
- [Implementation Summary](development/IMPLEMENTATION_SUMMARY.md) - Testing infrastructure
- [Implementation Summary Patch SD](development/IMPLEMENTATION_SUMMARY_PATCH_SD.md) - Patch SD implementation
- [Production Utilities](development/PRODUCTION_UTILITIES.md) - Production tools
- [RAM Overflow Fix](development/RAM_OVERFLOW_FIX.md) - RAM overflow resolution
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
- [Module Details](development/Modules_MidiCore_Detail_par_Module.txt) - Detailed architecture (French)

### 🔌 [USB Documentation](usb/) • [🇫🇷 Français](usb/README_FR.md)
USB MIDI implementation and debugging
- [Overview](usb/README.md) - USB documentation overview
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

**Technical Documents:**
- [USB Descriptor Structure](usb/USB_DESCRIPTOR_STRUCTURE.txt) - Descriptor structure details
- [USB MIDI Descriptor Analysis](usb/USB_MIDI_DESCRIPTOR_ANALYSIS.md) - Descriptor analysis
- [USB MIDI Protocol Audit](usb/USB_MIDI_PROTOCOL_AUDIT.md) - Protocol audit
- [USB MIDI SysEx Engine](usb/USB_MIDI_SYSEX_ENGINE.md) - SysEx implementation

### 🔄 [MIOS32 Compatibility](mios32/) • [🇫🇷 Français](mios32/README_FR.md)
MIOS32 compatibility and migration guides
- [Overview](mios32/README.md) - MIOS32 compatibility overview
- [MIOS32 Compatibility](mios32/MIOS32_COMPATIBILITY.md) - Overall compatibility
- [MIOS32 Deep Comparison](mios32/MIOS32_DEEP_COMPARISON.md) - Detailed analysis
- [MIOS32 Dual Mode Guide](mios32/MIOS32_DUAL_MODE_GUIDE.md) - Dual-mode USB
- [MIOS32 USB Implementation](mios32/MIOS32_USB_IMPLEMENTATION_GUIDE.md) - USB guide
- [MIOS32 Style Auto Switching](mios32/MIOS32_STYLE_AUTO_SWITCHING.md) - Mode switching
- [MIOS32 Descriptor Analysis](mios32/MIOS32_DESCRIPTOR_ANALYSIS.md) - Descriptor details

### 💼 [Commercial](commercial/) • [🇫🇷 Français](commercial/README_COMMERCIAL_FR.md)
Commercial documentation and presentations
- [Commercial README](commercial/README_COMMERCIAL.md) - Professional system overview (EN)
- [Commercial README FR](commercial/README_COMMERCIAL_FR.md) - Vue d'ensemble professionnelle (FR)
- [Product Presentation](commercial/PRESENTATION_PRODUIT_EN.md) - Product presentation (EN)
- [Présentation produit](commercial/PRESENTATION_PRODUIT.md) - Présentation produit (FR)

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
