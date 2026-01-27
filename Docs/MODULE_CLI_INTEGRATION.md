# Module CLI Integration Guide

This guide explains how to add CLI support to existing MidiCore modules.

## Quick Start

For most modules, integration is simple using the helper macros:

1. Create a `<module>_cli.c` file (or add to existing module)
2. Use the helper macros to define wrappers
3. Define the module descriptor
4. Call registration function from module init

## Step-by-Step Integration

### Step 1: Include Headers

```c
#include "Services/<module>/<module>.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>
```

### Step 2: Create Parameter Wrappers

#### For Simple Boolean Parameters

```c
// If you have: uint8_t my_module_get_enabled(void)
//              void my_module_set_enabled(uint8_t val)

DEFINE_PARAM_BOOL(my_module, enabled, my_module_get_enabled, my_module_set_enabled)
```

#### For Per-Track Boolean Parameters

```c
// If you have: uint8_t my_module_get_enabled(uint8_t track)
//              void my_module_set_enabled(uint8_t track, uint8_t val)

DEFINE_PARAM_BOOL_TRACK(my_module, enabled, my_module_get_enabled, my_module_set_enabled)
```

#### For Integer Parameters

```c
// Global integer
DEFINE_PARAM_INT(my_module, rate, my_module_get_rate, my_module_set_rate)

// Per-track integer
DEFINE_PARAM_INT_TRACK(my_module, gain, my_module_get_gain, my_module_set_gain)
```

#### For Enum Parameters

```c
// Global enum
DEFINE_PARAM_ENUM(my_module, mode, my_module_get_mode, my_module_set_mode, my_mode_t)

// Per-track enum
DEFINE_PARAM_ENUM_TRACK(my_module, pattern, my_module_get_pattern, my_module_set_pattern, pattern_t)
```

#### For Complex Parameters (Manual Wrapper)

If your module stores config in a struct, create manual wrappers:

```c
static int my_module_param_get_channel(uint8_t track, param_value_t* out) {
  (void)track;
  my_module_config_t config;
  my_module_get_config(&config);
  out->int_val = config.channel;
  return 0;
}

static int my_module_param_set_channel(uint8_t track, const param_value_t* val) {
  (void)track;
  if (val->int_val < 0 || val->int_val > 15) return -1;  // Validate
  
  my_module_config_t config;
  my_module_get_config(&config);
  config.channel = (uint8_t)val->int_val;
  my_module_set_config(&config);
  return 0;
}
```

### Step 3: Create Module Control Wrappers

#### For Global Modules

```c
// If you have: void my_module_set_enabled(uint8_t val)
//              uint8_t my_module_get_enabled(void)

DEFINE_MODULE_CONTROL_GLOBAL(my_module, my_module_set_enabled, my_module_get_enabled)
```

#### For Per-Track Modules

```c
// If you have: void my_module_set_enabled(uint8_t track, uint8_t val)
//              uint8_t my_module_get_enabled(uint8_t track)

DEFINE_MODULE_CONTROL_TRACK(my_module, my_module_set_enabled, my_module_get_enabled)
```

### Step 4: Define Enum Strings (if applicable)

```c
static const char* s_my_mode_names[] = {
  "OFF",
  "ON",
  "AUTO"
};
```

### Step 5: Create Module Descriptor

```c
static module_descriptor_t s_my_module_descriptor = {
  .name = "my_module",
  .description = "My awesome module description",
  .category = MODULE_CATEGORY_EFFECT,  // Choose appropriate category
  .init = my_module_init,
  .enable = my_module_cli_enable,
  .disable = my_module_cli_disable,
  .get_status = my_module_cli_get_status,
  .has_per_track_state = 0,  // 1 if per-track, 0 if global
  .is_global = 1,             // 1 if global, 0 if per-track
  .registered = 0
};
```

### Step 6: Setup Parameters

Create a function to populate the parameters array:

```c
static void setup_my_module_parameters(void) {
  module_param_t params[] = {
    {
      .name = "enabled",
      .description = "Enable module",
      .type = PARAM_TYPE_BOOL,
      .min = 0,
      .max = 1,
      .read_only = 0,
      .get_value = my_module_param_get_enabled,
      .set_value = my_module_param_set_enabled
    },
    {
      .name = "rate",
      .description = "Processing rate (1-100)",
      .type = PARAM_TYPE_INT,
      .min = 1,
      .max = 100,
      .read_only = 0,
      .get_value = my_module_param_get_rate,
      .set_value = my_module_param_set_rate
    },
    {
      .name = "mode",
      .description = "Operating mode",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 2,
      .enum_values = s_my_mode_names,
      .enum_count = 3,
      .read_only = 0,
      .get_value = my_module_param_get_mode,
      .set_value = my_module_param_set_mode
    }
  };
  
  s_my_module_descriptor.param_count = 3;
  memcpy(s_my_module_descriptor.params, params, sizeof(params));
}
```

### Step 7: Create Registration Function

```c
/**
 * @brief Register module with CLI
 * Call from module init or app initialization
 */
int my_module_register_cli(void) {
  setup_my_module_parameters();
  return module_registry_register(&s_my_module_descriptor);
}
```

### Step 8: Add to Module Init

In your module's init function:

```c
void my_module_init(void) {
  // ... existing initialization ...
  
  #ifdef MODULE_ENABLE_CLI
  my_module_register_cli();
  #endif
}
```

Or call from application initialization:

```c
void app_init(void) {
  // Initialize registry and CLI
  module_registry_init();
  cli_init();
  cli_module_commands_init();
  
  // Initialize modules
  my_module_init();
  
  // Register with CLI
  #ifdef MODULE_ENABLE_CLI
  my_module_register_cli();
  #endif
}
```

## Module Categories

Choose the appropriate category for your module:

```c
MODULE_CATEGORY_SYSTEM      // Bootloader, config, watchdog
MODULE_CATEGORY_MIDI        // Router, filter, converter
MODULE_CATEGORY_INPUT       // AIN, AINSER, SRIO, footswitch
MODULE_CATEGORY_OUTPUT      // DOUT, display
MODULE_CATEGORY_EFFECT      // Arpeggiator, harmonizer, delay
MODULE_CATEGORY_GENERATOR   // Metronome, LFO
MODULE_CATEGORY_LOOPER      // Looper sequencer
MODULE_CATEGORY_UI          // UI pages, input handling
MODULE_CATEGORY_ACCORDION   // Bass system, bellows, registers
MODULE_CATEGORY_OTHER       // Everything else
```

## Parameter Types

### PARAM_TYPE_BOOL
- Range: 0-1
- Values displayed as "true"/"false"
- Use for on/off switches

### PARAM_TYPE_INT
- Range: min to max (defined in descriptor)
- Use for numeric values with integer precision
- Examples: MIDI channel (0-15), velocity (1-127)

### PARAM_TYPE_FLOAT
- Use for decimal values
- Examples: frequency, gain multiplier

### PARAM_TYPE_ENUM
- Range: 0 to enum_count-1
- Provide array of string names
- Use for selecting from a list of options

### PARAM_TYPE_STRING
- For text values
- Less common, use sparingly

## Testing Your Integration

### 1. Build and Flash

Add your `<module>_cli.c` to the build system.

### 2. Connect Terminal

Connect to UART (default: 115200 baud, 8N1).

### 3. Test Commands

```bash
# List all modules
module list

# Your module should appear in the list
module info my_module

# List parameters
module params my_module

# Get parameter
module get my_module enabled

# Set parameter
module set my_module enabled true

# Enable module
module enable my_module

# Check status
module status my_module
```

### 4. Test Configuration Persistence

```bash
# Set some values
module set my_module enabled true
module set my_module rate 50

# Save to SD card
config save 0:/my_module.ini

# Reboot
reboot

# After reboot, load config
config load 0:/my_module.ini

# Verify values persisted
module get my_module enabled
module get my_module rate
```

## Common Patterns

### Pattern 1: Simple Global Module

Module with global state and simple enable/disable:

```c
DEFINE_PARAM_BOOL(my_module, enabled, my_module_get_enabled, my_module_set_enabled)
DEFINE_MODULE_CONTROL_GLOBAL(my_module, my_module_set_enabled, my_module_get_enabled)
```

### Pattern 2: Per-Track Module

Module with per-track state:

```c
DEFINE_PARAM_BOOL_TRACK(my_module, enabled, my_module_get_enabled, my_module_set_enabled)
DEFINE_MODULE_CONTROL_TRACK(my_module, my_module_set_enabled, my_module_get_enabled)
```

### Pattern 3: Module with Config Struct

Module that stores parameters in a config structure:

```c
static int my_module_param_get_<param>(uint8_t track, param_value_t* out) {
  my_module_config_t config;
  my_module_get_config(&config);
  out->int_val = config.<param>;
  return 0;
}

static int my_module_param_set_<param>(uint8_t track, const param_value_t* val) {
  my_module_config_t config;
  my_module_get_config(&config);
  config.<param> = val->int_val;
  my_module_set_config(&config);
  return 0;
}
```

### Pattern 4: Read-Only Parameter

For status/diagnostic values:

```c
{
  .name = "active_notes",
  .description = "Number of active notes (read-only)",
  .type = PARAM_TYPE_INT,
  .read_only = 1,
  .get_value = my_module_param_get_active_notes,
  .set_value = NULL  // No setter for read-only
}
```

## Complete Examples

See these files for complete integration examples:

1. **Services/cli/arpeggiator_cli_integration.c** - Simple global module
2. **Services/cli/metronome_cli.c** - Module with config struct
3. **Docs/MODULE_INVENTORY.md** - List of all modules to integrate

## Troubleshooting

### Module Not Appearing in List

- Check that `module_registry_register()` is being called
- Verify module init is being called
- Check return value of registration function

### Parameter Not Working

- Verify getter/setter function signatures match
- Check parameter type matches data type
- Ensure min/max ranges are correct
- Add validation in setter function

### Crash When Setting Parameter

- Check bounds validation in setter
- Ensure you're not dereferencing NULL pointers
- Verify struct sizes in memcpy operations
- Check for buffer overflows

### Values Not Persisting

- Verify config save/load is being called
- Check SD card is mounted
- Ensure runtime_config is initialized
- Check file permissions on SD card

## Build System Integration

### Add to Makefile

```makefile
# Module CLI integrations
SOURCES += Services/cli/arpeggiator_cli_integration.c
SOURCES += Services/cli/metronome_cli.c
# ... add more as you create them
```

### Add Build Flag (Optional)

In `Config/module_config.h`:

```c
// Enable CLI integration
#define MODULE_ENABLE_CLI 1
```

Then use in code:

```c
#ifdef MODULE_ENABLE_CLI
  my_module_register_cli();
#endif
```

## Next Steps

1. Start with simpler modules (arpeggiator, metronome)
2. Test thoroughly before moving to next module
3. Document any module-specific CLI commands
4. Consider creating UI integration after CLI is working
5. Add to `Docs/MODULE_INVENTORY.md` as you complete each module

## See Also

- `Services/cli/README.md` - CLI system overview
- `Services/cli/module_cli_helpers.h` - Helper macros reference
- `Services/module_registry/module_registry.h` - Registry API
- `Docs/MODULE_INVENTORY.md` - Complete module list
