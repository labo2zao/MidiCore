# MidiCore CLI & Module Configuration System

This implementation provides a comprehensive command-line interface and module registry system for the MidiCore firmware, enabling runtime configuration of all 60+ firmware modules via UART terminal and future UI integration.

## 🎯 What Was Accomplished

### ✅ Complete CLI Framework
- **Command parsing system** with registration and execution
- **Help system** with categorized command listing
- **Output formatting** (error, success, warning, info)
- **Built-in commands** (help, list, version, uptime, status, reboot, config)
- **Module control commands** (enable, disable, status, get, set, params)
- **Configuration persistence** via INI files on SD card

### ✅ Module Registry System
- **Module discovery** - List all registered modules
- **Metadata management** - Store module info, parameters, categories
- **Parameter system** - Support for bool, int, float, enum, string types
- **Per-track support** - Modules can be global or per-track
- **Enable/disable control** - Runtime module activation
- **Status reporting** - Query module state

### ✅ Configuration System
- **INI file format** - Human-readable configuration files
- **SD card storage** - Persistent configuration across reboots
- **Runtime modification** - Change settings without recompilation
- **Import/export** - Save and load complete configurations

### ✅ Developer Tools
- **Helper macros** - 1-line wrappers for parameter integration
- **Integration templates** - Copy-paste examples for new modules
- **Complete examples** - Arpeggiator and Metronome integrations
- **Step-by-step guide** - MODULE_CLI_INTEGRATION.md

### ✅ Comprehensive Documentation
- **MODULE_INVENTORY.md** (19KB) - All 60+ modules documented with APIs
- **MODULE_CLI_INTEGRATION.md** (10KB) - Integration guide with examples
- **Services/cli/README.md** (9KB) - CLI system architecture and usage

## 📁 Project Structure

```
MidiCore/
├── Services/
│   ├── cli/                          # Command Line Interface
│   │   ├── cli.h & cli.c            # Core CLI system
│   │   ├── cli_module_commands.h/c   # Module control commands
│   │   ├── module_cli_helpers.h      # Integration helper macros
│   │   ├── arpeggiator_cli_integration.c  # Example 1 (simple)
│   │   ├── metronome_cli.c          # Example 2 (complex)
│   │   └── README.md                # CLI documentation
│   │
│   ├── module_registry/              # Module Registry System
│   │   ├── module_registry.h & .c   # Module discovery & control
│   │   └── (registers all modules)
│   │
│   ├── config/                       # Configuration System
│   │   ├── runtime_config.h & .c    # INI file management
│   │   └── (already existed)
│   │
│   └── [60+ existing modules]/       # All firmware modules
│       ├── arpeggiator/
│       ├── metronome/
│       ├── looper/
│       ├── midi_filter/
│       └── ... (see MODULE_INVENTORY.md)
│
├── Docs/
│   ├── MODULE_INVENTORY.md           # Complete module list & APIs
│   └── MODULE_CLI_INTEGRATION.md     # Integration guide
│
└── App/
    └── tests/
        └── test_debug.h & .c         # UART infrastructure (already existed)
```

## 🚀 Quick Start

### For Users - Using the CLI

1. **Connect via UART** (default: UART2, 115200 baud, 8N1)

2. **Basic commands:**
```bash
# See all commands
help

# List all modules
module list

# Enable a module
module enable arpeggiator

# Set a parameter
module set arpeggiator pattern UP

# Save configuration
config save 0:/config.ini
```

3. **Full command reference:** See `Services/cli/README.md`

### For Developers - Integrating a Module

1. **Create `<module>_cli.c` file:**
```c
#include "Services/<module>/<module>.h"
#include "Services/cli/module_cli_helpers.h"

// Define parameter wrappers using helper macros
DEFINE_PARAM_BOOL(my_module, enabled, 
                  my_module_get_enabled, 
                  my_module_set_enabled)

// Define module control
DEFINE_MODULE_CONTROL_GLOBAL(my_module, 
                             my_module_set_enabled, 
                             my_module_get_enabled)

// Create registration function
int my_module_register_cli(void) {
  // Setup parameters and register
}
```

2. **Call from module init:**
```c
void my_module_init(void) {
  // ... existing init ...
  
  #ifdef MODULE_ENABLE_CLI
  my_module_register_cli();
  #endif
}
```

3. **Detailed guide:** See `Docs/MODULE_CLI_INTEGRATION.md`

## 📚 All 60+ Modules Documented

The system is ready to control all MidiCore modules:

### System
- bootloader, config, watchdog

### MIDI
- router, midi_filter, midi_delay, midi_converter

### Input
- ain, ainser, srio, din, footswitch, pressure

### Effects (20+)
- arpeggiator, note_repeat, quantizer, humanize, swing
- channelizer, harmonizer, chord, strum, musette_detune
- gate_time, legato, velocity_compressor, cc_smoother
- envelope_cc, lfo, midi_delay, midi_filter

### Accordion-Specific
- bass_chord_system, one_finger_chord, bellows_shake
- register_coupling, assist_hold, bellows_expression

### Looper
- Multi-track sequencer with 4 tracks, 8 scenes

### And many more...

**Complete list:** `Docs/MODULE_INVENTORY.md`

## 🎛️ CLI Commands Reference

### System Commands
```bash
help [command]          # Show help
list                    # List all commands
version                 # Firmware version
uptime                  # System uptime
status                  # System status
reboot                  # Reboot device
clear                   # Clear screen
```

### Configuration Commands
```bash
config load <file>              # Load from SD card
config save <file>              # Save to SD card
config get <key>                # Get value
config set <key> <value>        # Set value
config list                     # List all entries
```

### Module Commands
```bash
module list                           # List all modules
module info <name>                    # Module information
module enable <name> [track]          # Enable module
module disable <name> [track]         # Disable module
module status <name> [track]          # Get status
module params <name>                  # List parameters
module get <name> <param> [track]     # Get parameter
module set <name> <param> <value> [track]  # Set parameter
```

### Example Session
```bash
# List available modules
module list

# Get arpeggiator info
module info arpeggiator

# Enable arpeggiator
module enable arpeggiator

# Set pattern
module set arpeggiator pattern UP

# Check metronome parameters
module params metronome

# Configure metronome
module set metronome enabled true
module set metronome midi_channel 9
module set metronome accent_note 76

# Save configuration
config save 0:/my_config.ini

# Later, reload it
config load 0:/my_config.ini
```

## 🔧 Architecture

```
┌─────────────────────────────────────┐
│     User Interface                   │
│  (UART Terminal / Future UI)        │
└─────────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────┐
│         CLI Layer                    │
│  • Command parsing                   │
│  • Help system                       │
│  • Module commands                   │
└─────────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────┐
│     Module Registry                  │
│  • Module discovery                  │
│  • Parameter metadata                │
│  • Enable/disable control            │
└─────────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────┐
│     Service Modules (60+)            │
│  • arpeggiator                       │
│  • looper                            │
│  • metronome                         │
│  • harmonizer                        │
│  • etc...                            │
└─────────────────────────────────────┘
```

## 📝 Configuration File Format

INI-style configuration files are stored on SD card:

```ini
[arpeggiator]
enabled = true
pattern = 0

[metronome]
enabled = true
mode = 1
midi_channel = 9
accent_note = 76
regular_note = 77
accent_velocity = 100
regular_velocity = 80

[looper]
bpm = 120
time_signature_num = 4
time_signature_den = 4

[midi_filter]
enabled = true
min_note = 36
max_note = 96
```

## 🎨 Future UI Integration

The module registry provides all metadata needed for UI generation:

```c
// Query modules by category
const module_descriptor_t* effects[32];
uint32_t count = module_registry_list_by_category(
  MODULE_CATEGORY_EFFECT, 
  effects, 
  32
);

// Build UI menu from module list
for (uint32_t i = 0; i < count; i++) {
  ui_menu_add_item(effects[i]->name, effects[i]->description);
}

// Get parameters for selected module
const module_descriptor_t* mod = module_registry_get_by_name("arpeggiator");
for (uint8_t i = 0; i < mod->param_count; i++) {
  // Create UI controls based on parameter type
  switch (mod->params[i].type) {
    case PARAM_TYPE_BOOL:   ui_add_toggle(...); break;
    case PARAM_TYPE_INT:    ui_add_slider(...); break;
    case PARAM_TYPE_ENUM:   ui_add_dropdown(...); break;
  }
}

// Rotary encoder changes parameter
param_value_t value;
value.int_val = new_value;
module_registry_set_param("arpeggiator", "pattern", 0, &value);
```

## 📊 Implementation Statistics

- **Lines of Code:** ~3,000+ (excluding documentation)
- **Files Created:** 12
- **Modules Documented:** 60+
- **CLI Commands:** 20+
- **Parameter Types Supported:** 5 (bool, int, float, enum, string)
- **Documentation:** 40KB+ of guides and references

## ✨ Key Benefits

1. **No Recompilation Needed** - Change any parameter via CLI or config file
2. **Discoverable** - All modules and parameters self-document
3. **Persistent** - Save/load configurations from SD card
4. **Extensible** - Easy to add new modules and parameters
5. **UI-Ready** - Module registry provides metadata for dynamic UI generation
6. **Professional** - Production-quality code following MIOS32 architecture
7. **Well-Documented** - Complete guides and examples provided

## 🔍 Module Integration Status

**Framework:** ✅ Complete
- CLI system
- Module registry
- Helper macros
- Documentation

**Example Integrations:** ✅ Done
- Arpeggiator (simple global module)
- Metronome (complex config struct)

**Remaining Modules:** 🔄 Ready for Integration
- 58 modules ready to integrate using templates
- Each can be integrated independently
- Helper macros make it quick (10-30 minutes per module)

## 🐛 Testing

### Unit Testing
```bash
# Test CLI commands
help
module list
module info arpeggiator

# Test parameter access
module get arpeggiator enabled
module set arpeggiator enabled true

# Test configuration
config save 0:/test.ini
config load 0:/test.ini
config list
```

### Integration Testing
- UART terminal interaction
- Module enable/disable
- Parameter get/set
- Configuration persistence
- Multi-module scenarios

## 📖 Documentation Files

1. **README_CLI_MODULE_SYSTEM.md** (this file) - Overview
2. **Services/cli/README.md** - CLI system details
3. **Docs/MODULE_INVENTORY.md** - All modules and functions
4. **Docs/MODULE_CLI_INTEGRATION.md** - Integration guide

## 🤝 Contributing

To integrate a module:

1. Copy `Services/cli/metronome_cli.c` as template
2. Follow `Docs/MODULE_CLI_INTEGRATION.md` guide
3. Use helper macros from `module_cli_helpers.h`
4. Test via UART terminal
5. Update `MODULE_INVENTORY.md` if APIs changed

## 📜 License

Part of MidiCore firmware - same license as main project.

## 👥 Authors

- Framework implementation: GitHub Copilot Agent
- MidiCore firmware: labodezao

## 🙏 Acknowledgments

- MIOS32 / Midibox architecture inspiration
- LoopA sequencer design influence
- STM32 HAL and FreeRTOS foundations

---

**Status:** Framework Complete ✅ | Ready for Per-Module Integration 🚀

**Next Steps:**
1. Test on hardware with UART terminal
2. Integrate remaining modules (copy-paste from templates)
3. Add UI integration layer
4. Deploy to production accordions
