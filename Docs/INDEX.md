# MidiCore Documentation Index

**Master index for all MidiCore firmware documentation**

Last Updated: 2025-01-28

---

## 📚 Core Documentation

### Getting Started
- **[README.md](README.md)** - Project overview, quick start, and introduction
- **[README_FR.md](README_FR.md)** - Documentation en français

### Module System
- **[MODULE_INVENTORY.md](MODULE_INVENTORY.md)** - Complete inventory of all firmware modules with parameters
- **[CLI_COMMAND_REFERENCE.md](CLI_COMMAND_REFERENCE.md)** - Comprehensive CLI command reference for all modules (NEW!)
- **[MODULE_CLI_INTEGRATION.md](MODULE_CLI_INTEGRATION.md)** - Guide for integrating CLI support into modules

### Command-Line Interface (CLI)
- **[CLI_COMMAND_REFERENCE.md](CLI_COMMAND_REFERENCE.md)** - Complete CLI command reference with examples
- **[WHAT_IS_CLI.md](WHAT_IS_CLI.md)** - Introduction to CLI concepts
- **[CLI_VS_UI_VISUAL.md](CLI_VS_UI_VISUAL.md)** - Comparison of CLI vs OLED UI interaction

### Configuration System
- **[NGC_TEXT_PARSING_REFERENCE.md](NGC_TEXT_PARSING_REFERENCE.md)** - NGC text-based configuration format
- **[NGC_TEXT_PARSING_REFERENCE_FR.md](NGC_TEXT_PARSING_REFERENCE_FR.md)** - Format de configuration NGC (Français)
- **[NGC_MULTI_CHANNEL_SPEC.md](NGC_MULTI_CHANNEL_SPEC.md)** - Multi-channel NGC specification
- **[NGC_MULTI_CHANNEL_SPEC_FR.md](NGC_MULTI_CHANNEL_SPEC_FR.md)** - Spécification NGC multi-canaux (Français)

### Mapping & Routing
- **[MAPPING_DOCUMENTATION_INDEX.md](MAPPING_DOCUMENTATION_INDEX.md)** - Index for mapping documentation
- **[MAPPING_DOCUMENTATION_INDEX_FR.md](MAPPING_DOCUMENTATION_INDEX_FR.md)** - Index de la documentation de mapping (Français)
- **[MAPPING_MODULES_REFERENCE.md](MAPPING_MODULES_REFERENCE.md)** - Reference for AINSER/DIN mapping modules

---

## 🗂️ Specialized Documentation

### Development
- **[development/](development/)** - Development guides, coding standards, architecture decisions
  - Architecture patterns
  - Build system
  - Testing strategies
  - Debugging guides

### Hardware Integration
- **[hardware/](hardware/)** - Hardware-specific documentation
  - STM32 configuration
  - Pin mappings
  - Peripheral setup
  - SRIO/AINSER/DIN hardware

### USB Subsystem
- **[usb/](usb/)** - USB MIDI and CDC documentation
  - USB descriptors
  - MIDI device configuration
  - CDC virtual COM port
  - Troubleshooting

### Memory Management
- **[memory/](memory/)** - Memory optimization and analysis
  - RAM usage reports
  - CCMRAM allocation
  - Optimization strategies

### Testing
- **[testing/](testing/)** - Test documentation and procedures
  - Unit test guides
  - Integration testing
  - Hardware validation

### MIOS32 Compatibility
- **[mios32/](mios32/)** - MIOS32 protocol and compatibility
  - Protocol specifications
  - MIOS Studio integration
  - SysEx commands

### Configuration Examples
- **[configuration/](configuration/)** - Sample configurations and presets
  - Accordion configurations
  - Effect chains
  - Looper sessions
  - INI file examples

### User Guides
- **[user-guides/](user-guides/)** - End-user documentation
  - Quick start guides
  - Feature tutorials
  - FAQ

### Commercial/Production
- **[commercial/](commercial/)** - Commercial deployment information
  - Production configuration
  - Firmware update procedures
  - Support documentation

---

## 🚀 Quick Links

### For New Users
1. Start with [README.md](README.md) - Project overview
2. Read [WHAT_IS_CLI.md](WHAT_IS_CLI.md) - Understand the CLI
3. Explore [CLI_COMMAND_REFERENCE.md](CLI_COMMAND_REFERENCE.md) - Learn all commands
4. Check [MODULE_INVENTORY.md](MODULE_INVENTORY.md) - See all available modules

### For Developers
1. Review [development/](development/) - Architecture and patterns
2. Read [MODULE_CLI_INTEGRATION.md](MODULE_CLI_INTEGRATION.md) - How to add CLI to modules
3. Check [testing/](testing/) - Testing procedures
4. See [memory/](memory/) - Memory optimization

### For System Integrators
1. Study [NGC_TEXT_PARSING_REFERENCE.md](NGC_TEXT_PARSING_REFERENCE.md) - Configuration format
2. Review [MAPPING_MODULES_REFERENCE.md](MAPPING_MODULES_REFERENCE.md) - Input mapping
3. Check [configuration/](configuration/) - Example configs
4. See [hardware/](hardware/) - Hardware setup

---

## 📖 Documentation Organization

### By Category

#### User Documentation
- CLI Command Reference
- Module Inventory
- What is CLI?
- CLI vs UI Comparison
- User Guides

#### Developer Documentation
- Module CLI Integration
- NGC Text Parsing Reference
- Architecture guides (in development/)
- Testing guides (in testing/)

#### System Documentation
- Mapping Documentation
- Hardware Integration
- USB Configuration
- Memory Management

#### Language-Specific
- French (FR) versions available for major documents
- English is primary language

---

## 🔄 Recent Updates

### January 28, 2025
- ✅ **NEW**: Complete CLI command reference for ALL modules
- ✅ **NEW**: 32 CLI integration files created
- ✅ Looper CLI with full transport and track control
- ✅ All MIDI effect modules (filter, delay, converter)
- ✅ All effect modules (quantizer, harmonizer, compressor, etc.)
- ✅ All accordion modules (bellows, bass, registers, musette)
- ✅ All input modules (AIN, AINSER via existing mapping)
- ✅ System modules (config, watchdog)

### Previously
- Module Inventory with comprehensive parameter listings
- NGC configuration format documentation
- MIOS32 protocol compatibility
- USB subsystem documentation

---

## 🛠️ Tools & Utilities

### CLI Tools
- **Services/cli/** - CLI implementation
  - `cli.c/cli.h` - Main CLI system
  - `cli_module_commands.c/h` - Generic module commands
  - `module_cli_helpers.h` - Helper macros for integration
  - `*_cli.c` - Per-module CLI implementations (32 modules)

### Code Generation
- **Tools/generate_cli_files.py** - Auto-generate CLI integration files

### Configuration Utilities
- NGC parser implementation
- Config save/load system
- INI file format support

---

## 📝 Contributing to Documentation

### Documentation Standards
- Use Markdown format (.md)
- Include code examples with proper syntax highlighting
- Keep examples up-to-date with current API
- Cross-reference related documents
- Provide both conceptual explanation and practical examples

### File Naming
- Use SCREAMING_SNAKE_CASE for major docs (README.md, MODULE_INVENTORY.md)
- Use lowercase-with-dashes for subdirectory docs
- Use language suffix for translations (_FR.md, _ES.md, etc.)

### Structure
- Start with clear title and description
- Include table of contents for long documents
- Use consistent heading hierarchy (H1 for title, H2 for sections, etc.)
- End with references to related documents

---

## 🔍 Finding Information

### By Topic

**CLI Commands**
→ [CLI_COMMAND_REFERENCE.md](CLI_COMMAND_REFERENCE.md)

**Module Parameters**
→ [MODULE_INVENTORY.md](MODULE_INVENTORY.md)

**Configuration Format**
→ [NGC_TEXT_PARSING_REFERENCE.md](NGC_TEXT_PARSING_REFERENCE.md)

**Hardware Setup**
→ [hardware/](hardware/)

**Troubleshooting**
→ [CLI_COMMAND_REFERENCE.md](CLI_COMMAND_REFERENCE.md#troubleshooting) (CLI issues)
→ [usb/](usb/) (USB issues)
→ [testing/](testing/) (General issues)

**Development Guides**
→ [development/](development/)

---

## 📮 Support & Feedback

For questions, issues, or contributions:
- Check existing documentation first
- Review README.md for contact information
- Open GitHub issues for bugs or feature requests
- Contribute documentation improvements via pull requests

---

**Last Updated**: January 28, 2025  
**Documentation Version**: 2.0  
**Firmware Version**: Compatible with MidiCore v1.x
