# MidiCore CLI System

Command Line Interface for MidiCore firmware configuration and control over UART.

## Overview

The MidiCore CLI provides a comprehensive command-line interface for:
- Discovering and listing all firmware modules
- Enabling/disabling modules per-track or globally
- Getting and setting module parameters
- Saving/loading configurations to/from SD card
- System status and diagnostics

## Architecture

```
┌─────────────────────────────────────────────┐
│              Application Layer               │
│  (App tasks, UI, module initialization)     │
└─────────────────────────────────────────────┘
                    ▲
                    │
┌─────────────────────────────────────────────┐
│              CLI Layer                       │
│  - Command parsing                           │
│  - Command registration                      │
│  - Help system                               │
│  - Module commands (cli_module_commands)     │
└─────────────────────────────────────────────┘
                    ▲
                    │
┌─────────────────────────────────────────────┐
│         Module Registry Layer                │
│  - Module discovery                          │
│  - Parameter metadata                        │
│  - Enable/disable control                    │
│  - Parameter get/set                         │
└─────────────────────────────────────────────┘
                    ▲
                    │
┌─────────────────────────────────────────────┐
│           Service Modules                    │
│  (looper, arpeggiator, harmonizer, etc.)    │
└─────────────────────────────────────────────┘
                    ▲
                    │
┌─────────────────────────────────────────────┐
│              HAL Layer                       │
│  (UART, SPI, I2C, GPIO, etc.)               │
└─────────────────────────────────────────────┘
```

## Usage

### Connecting via UART

1. Connect UART to your terminal (default: UART2, 115200 baud, 8N1)
2. Reset the MidiCore device
3. You should see the CLI banner and prompt

### Basic Commands

```bash
# Show all available commands
help

# List all registered modules
module list

# Get information about a specific module
module info looper
module info arpeggiator

# Enable/disable a module
module enable arpeggiator
module disable arpeggiator

# Get module status
module status looper
module status arpeggiator 0    # Status for track 0

# List module parameters
module params arpeggiator
module params midi_filter

# Get parameter value
module get arpeggiator pattern
module get looper bpm

# Set parameter value
module set arpeggiator pattern UP
module set looper bpm 120
module set midi_filter enabled true 0

# Configuration management
config save 0:/midicore.ini    # Save to SD card
config load 0:/midicore.ini    # Load from SD card
config list                    # List all config entries
config get arpeggiator.pattern
config set arpeggiator.pattern DOWN

# System information
version                        # Firmware version
uptime                         # System uptime
status                         # System status
reboot                         # Reboot device
clear                          # Clear screen
```

## Module Registration

Each module registers itself with the module registry at initialization:

```c
#include "Services/module_registry/module_registry.h"
#include "Services/cli/cli.h"

// Define module descriptor
static module_descriptor_t s_arpeggiator_module = {
  .name = "arpeggiator",
  .description = "MIDI arpeggiator with multiple patterns",
  .category = MODULE_CATEGORY_EFFECT,
  
  // Control functions
  .init = arp_init,
  .enable = arp_enable_wrapper,
  .disable = arp_disable_wrapper,
  .get_status = arp_get_status_wrapper,
  
  // Parameters
  .params = {
    {
      .name = "enabled",
      .description = "Enable arpeggiator",
      .type = PARAM_TYPE_BOOL,
      .get_value = arp_get_enabled_param,
      .set_value = arp_set_enabled_param
    },
    {
      .name = "pattern",
      .description = "Arpeggio pattern",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = ARP_PATTERN_COUNT - 1,
      .enum_values = arp_pattern_names,
      .enum_count = ARP_PATTERN_COUNT,
      .get_value = arp_get_pattern_param,
      .set_value = arp_set_pattern_param
    }
  },
  .param_count = 2,
  
  .has_per_track_state = 1,
  .is_global = 0
};

// Register at initialization
void arp_init(void) {
  // ... module initialization ...
  
  // Register with module registry
  module_registry_register(&s_arpeggiator_module);
}
```

## Integration with Existing Modules

To add CLI support to an existing module:

1. **Include headers:**
   ```c
   #include "Services/module_registry/module_registry.h"
   ```

2. **Create parameter wrapper functions:**
   ```c
   static int arp_get_enabled_param(uint8_t track, param_value_t* out) {
     out->bool_val = arp_get_enabled();
     return 0;
   }
   
   static int arp_set_enabled_param(uint8_t track, const param_value_t* val) {
     arp_set_enabled(val->bool_val);
     return 0;
   }
   ```

3. **Define module descriptor** (see example above)

4. **Register at init time:**
   ```c
   module_registry_register(&s_your_module);
   ```

## Files

### Services/cli/
- `cli.h` / `cli.c` - Core CLI system
- `cli_module_commands.h` / `cli_module_commands.c` - Module control commands

### Services/module_registry/
- `module_registry.h` / `module_registry.c` - Module registry system

### Docs/
- `MODULE_INVENTORY.md` - Comprehensive list of all modules and functions

## Configuration Persistence

The CLI integrates with the `runtime_config` system to persist configuration:

```bash
# Set a value via CLI
module set arpeggiator pattern UP

# This is automatically stored in runtime_config as:
# "arpeggiator.pattern" = "UP"

# Save to SD card
config save 0:/midicore.ini

# On next boot, load config
config load 0:/midicore.ini
```

Configuration files use INI format:

```ini
[arpeggiator]
enabled = true
pattern = UP

[looper]
bpm = 120
time_signature_num = 4
time_signature_den = 4

[midi_filter]
enabled = true
min_note = 36
max_note = 96
```

## UI Integration

The module registry provides all the metadata needed for dynamic UI generation:

```c
// Get all modules in a category
const module_descriptor_t* modules[64];
uint32_t count = module_registry_list_by_category(
  MODULE_CATEGORY_EFFECT, 
  modules, 
  64
);

// Build UI menu from modules
for (uint32_t i = 0; i < count; i++) {
  ui_menu_add_item(modules[i]->name, modules[i]->description);
}

// Get parameters for selected module
const module_descriptor_t* module = module_registry_get_by_name("arpeggiator");
for (uint8_t i = 0; i < module->param_count; i++) {
  const module_param_t* param = &module->params[i];
  
  // Create UI control based on parameter type
  switch (param->type) {
    case PARAM_TYPE_BOOL:
      ui_add_toggle(param->name, param->description);
      break;
    case PARAM_TYPE_INT:
      ui_add_slider(param->name, param->min, param->max);
      break;
    case PARAM_TYPE_ENUM:
      ui_add_dropdown(param->name, param->enum_values, param->enum_count);
      break;
  }
}

// When user changes a value via rotary encoder:
param_value_t value;
value.int_val = new_value;
module_registry_set_param("arpeggiator", "pattern", 0, &value);
```

## Building and Testing

The CLI system has no external dependencies beyond the existing MidiCore infrastructure.

### Build Configuration

Add to your build system:

```makefile
# CLI sources
SOURCES += Services/cli/cli.c
SOURCES += Services/cli/cli_module_commands.c
SOURCES += Services/module_registry/module_registry.c

# Include paths
INCLUDES += -IServices/cli
INCLUDES += -IServices/module_registry
```

### Initialization

In your application initialization:

```c
#include "Services/cli/cli.h"
#include "Services/cli/cli_module_commands.h"
#include "Services/module_registry/module_registry.h"

void app_init(void) {
  // Initialize module registry
  module_registry_init();
  
  // Initialize CLI
  cli_init();
  
  // Register module commands
  cli_module_commands_init();
  
  // ... initialize other modules ...
  // Each module will register itself with the registry
}
```

### Runtime

Call `cli_task()` periodically from a FreeRTOS task or main loop to process UART input:

```c
void cli_task_main(void *argument) {
  for (;;) {
    cli_task();
    osDelay(10); // 10ms
  }
}
```

## Future Enhancements

- [ ] Command history (up/down arrow keys)
- [ ] Tab auto-completion
- [ ] Command aliases
- [ ] Macro/script support
- [ ] Telnet access (if TCP/IP is added)
- [ ] Bluetooth Serial (via ESP32)
- [ ] Command logging to SD card
- [ ] Interactive help with examples
- [ ] Batch command execution from file

## See Also

- `Docs/MODULE_INVENTORY.md` - Complete list of all modules
- `Services/config/runtime_config.h` - Configuration system
- `App/tests/test_debug.h` - UART debug infrastructure
