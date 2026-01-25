# DOUT Mapping Module

## Overview

The DOUT Mapping module provides a flexible hardware abstraction layer for digital outputs (LEDs, relays, indicators), allowing logical-to-physical bit mapping with per-bit and global inversion control. It also supports RGB LED mapping with configurable channel assignments and polarity inversion.

## Features

- **64-bit Digital Output Mapping**: Support for up to 64 logical output bits
- **Flexible Bit Mapping**: Remap logical bits to physical hardware pins
- **Per-Bit Inversion**: Individual inversion control for each output bit
- **Global Inversion**: Apply default inversion to all outputs
- **RGB LED Support**: Map up to 16 RGB LEDs with configurable pin assignments
- **RGB Channel Inversion**: Independent inversion for R, G, B channels
- **Configuration Integration**: Works with the global config system

## API Reference

### Functions

| Function | Description |
|----------|-------------|
| `dout_map_init()` | Initialize DOUT mapping with configuration |
| `dout_map_apply()` | Apply mapping and inversion to convert logical to physical bits |
| `dout_set_rgb()` | Set RGB LED state with color values |

#### dout_map_init

```c
void dout_map_init(const config_t* cfg);
```

Initialize the DOUT mapping module with configuration settings. If no configuration is provided, defaults are used.

**Parameters:**
- `cfg`: Pointer to configuration structure or `NULL` for defaults

**Example:**
```c
#include "Services/dout/dout_map.h"
#include "Services/config/config.h"

config_t my_config;
config_set_defaults(&my_config);

// Customize configuration
my_config.dout_invert_default = 0;  // No global inversion
my_config.bit_inv[0] = 1;           // Invert bit 0 only

dout_map_init(&my_config);
```

#### dout_map_apply

```c
void dout_map_apply(const uint8_t* logical, uint8_t* physical, uint16_t bytes);
```

Convert logical output states to physical hardware states by applying:
1. Copy logical buffer to physical buffer
2. Apply global default inversion if configured
3. Apply per-bit inversion for each configured bit

**Parameters:**
- `logical`: Source buffer with logical bit states
- `physical`: Destination buffer for physical bit states (hardware output)
- `bytes`: Number of bytes to process (typically 8 for 64 bits)

**Example:**
```c
uint8_t logical[8] = {0};   // Logical state (application level)
uint8_t physical[8];         // Physical state (hardware level)

// Set some logical bits
logical[0] |= (1 << 0);  // Set bit 0
logical[0] |= (1 << 5);  // Set bit 5

// Apply mapping and inversion
dout_map_apply(logical, physical, 8);

// Write physical buffer to hardware
hardware_write_shift_registers(physical, 8);
```

#### dout_set_rgb

```c
void dout_set_rgb(uint8_t* logical, uint8_t led, uint8_t r, uint8_t g, uint8_t b);
```

Set the state of an RGB LED by setting the appropriate bits in the logical output buffer. Applies configured RGB channel inversions and pin mappings.

**Parameters:**
- `logical`: Logical output buffer to modify (8 bytes for 64 bits)
- `led`: LED index (0-15)
- `r`: Red state (0=off, 1=on)
- `g`: Green state (0=off, 1=on)
- `b`: Blue state (0=off, 1=on)

**Example:**
```c
uint8_t logical[8] = {0};

// Set LED 0 to red
dout_set_rgb(logical, 0, 1, 0, 0);

// Set LED 1 to yellow (red + green)
dout_set_rgb(logical, 1, 1, 1, 0);

// Set LED 2 to white
dout_set_rgb(logical, 2, 1, 1, 1);

// Apply mapping and write to hardware
uint8_t physical[8];
dout_map_apply(logical, physical, 8);
hardware_write_shift_registers(physical, 8);
```

## Configuration Structure

The DOUT mapping module uses the `config_t` structure from `Services/config/config.h`:

```c
typedef struct {
    // Global inversion
    uint8_t dout_invert_default;  // 0=normal, 1=invert all outputs by default
    
    // Per-bit inversion (64 bits)
    uint8_t bit_inv[64];          // 0=normal, 1=invert this specific bit
    
    // RGB LED mapping (16 LEDs, 3 channels each)
    uint8_t rgb_map_r[16];        // Physical bit index for Red channel (0xFF=unused)
    uint8_t rgb_map_g[16];        // Physical bit index for Green channel (0xFF=unused)
    uint8_t rgb_map_b[16];        // Physical bit index for Blue channel (0xFF=unused)
    
    // RGB channel inversion
    uint8_t rgb_r_invert;         // 0=normal, 1=invert red channel for all LEDs
    uint8_t rgb_g_invert;         // 0=normal, 1=invert green channel for all LEDs
    uint8_t rgb_b_invert;         // 0=normal, 1=invert blue channel for all LEDs
    
    // ... other config fields
} config_t;
```

## Usage Examples

### Basic Digital Output

```c
#include "Services/dout/dout_map.h"

// Initialize with defaults
dout_map_init(NULL);

// Create logical state buffer
uint8_t logical[8] = {0};
uint8_t physical[8];

// Set some outputs
logical[0] = 0xFF;  // All bits in byte 0 on
logical[1] = 0x0F;  // Lower 4 bits in byte 1 on

// Apply mapping
dout_map_apply(logical, physical, 8);

// Write to hardware (shift registers, GPIO, etc.)
srio_write_outputs(physical, 8);
```

### Using Per-Bit Inversion

```c
config_t cfg;
config_set_defaults(&cfg);

// Invert specific bits (for active-low LEDs or inverted logic)
cfg.bit_inv[0] = 1;   // Invert bit 0
cfg.bit_inv[1] = 1;   // Invert bit 1
cfg.bit_inv[15] = 1;  // Invert bit 15

dout_map_init(&cfg);

uint8_t logical[8] = {0};
uint8_t physical[8];

// Setting bit 0 in logical will result in it being cleared in physical
logical[0] = 0x01;  // Bit 0 = 1 (logically on)

dout_map_apply(logical, physical, 8);
// physical[0] will have bit 0 = 0 (physically inverted)
```

### RGB LED Control

```c
config_t cfg;
config_set_defaults(&cfg);

// Map RGB LED 0 to physical pins
cfg.rgb_map_r[0] = 0;   // Red channel on bit 0
cfg.rgb_map_g[0] = 1;   // Green channel on bit 1
cfg.rgb_map_b[0] = 2;   // Blue channel on bit 2

// Map RGB LED 1 to different pins
cfg.rgb_map_r[1] = 3;   // Red channel on bit 3
cfg.rgb_map_g[1] = 4;   // Green channel on bit 4
cfg.rgb_map_b[1] = 5;   // Blue channel on bit 5

// Common cathode RGB LEDs (active high)
cfg.rgb_r_invert = 0;
cfg.rgb_g_invert = 0;
cfg.rgb_b_invert = 0;

dout_map_init(&cfg);

uint8_t logical[8] = {0};
uint8_t physical[8];

// Set LED 0 to magenta (red + blue)
dout_set_rgb(logical, 0, 1, 0, 1);

// Set LED 1 to cyan (green + blue)
dout_set_rgb(logical, 1, 0, 1, 1);

// Apply and output
dout_map_apply(logical, physical, 8);
hardware_write(physical, 8);
```

### Common Anode RGB LEDs

```c
// For common anode RGB LEDs (active low), invert all RGB channels
config_t cfg;
config_set_defaults(&cfg);

cfg.rgb_r_invert = 1;  // Invert red channel
cfg.rgb_g_invert = 1;  // Invert green channel
cfg.rgb_b_invert = 1;  // Invert blue channel

// Map RGB LED pins
cfg.rgb_map_r[0] = 0;
cfg.rgb_map_g[0] = 1;
cfg.rgb_map_b[0] = 2;

dout_map_init(&cfg);

uint8_t logical[8] = {0};
uint8_t physical[8];

// Set LED to red (will be inverted to active-low)
dout_set_rgb(logical, 0, 1, 0, 0);

dout_map_apply(logical, physical, 8);
// Bit 0 will be 0 (active-low for red on)
// Bits 1,2 will be 1 (active-low for green/blue off)
```

### LED Status Indicators

```c
void update_status_leds(uint8_t midi_active, uint8_t sd_ok, uint8_t error) {
    uint8_t logical[8] = {0};
    uint8_t physical[8];
    
    // Bit assignments
    const uint8_t LED_MIDI = 0;
    const uint8_t LED_SD = 1;
    const uint8_t LED_ERROR = 2;
    
    // Set logical states
    if (midi_active) logical[0] |= (1 << LED_MIDI);
    if (sd_ok)       logical[0] |= (1 << LED_SD);
    if (error)       logical[0] |= (1 << LED_ERROR);
    
    // Apply mapping and write to hardware
    dout_map_apply(logical, physical, 8);
    hardware_update_leds(physical);
}
```

### Multiplexed RGB Display

```c
void display_rgb_pattern(void) {
    config_t cfg;
    config_set_defaults(&cfg);
    
    // Map 4 RGB LEDs
    for (uint8_t i = 0; i < 4; i++) {
        cfg.rgb_map_r[i] = i * 3 + 0;
        cfg.rgb_map_g[i] = i * 3 + 1;
        cfg.rgb_map_b[i] = i * 3 + 2;
    }
    
    dout_map_init(&cfg);
    
    uint8_t logical[8] = {0};
    uint8_t physical[8];
    
    // Create color pattern
    dout_set_rgb(logical, 0, 1, 0, 0);  // Red
    dout_set_rgb(logical, 1, 0, 1, 0);  // Green
    dout_set_rgb(logical, 2, 0, 0, 1);  // Blue
    dout_set_rgb(logical, 3, 1, 1, 1);  // White
    
    dout_map_apply(logical, physical, 8);
    hardware_write(physical, 8);
}
```

## Implementation Details

### Bit Indexing

Bits are indexed from 0-63:
- Bit 0 = byte 0, bit 0 (LSB)
- Bit 7 = byte 0, bit 7 (MSB)
- Bit 8 = byte 1, bit 0
- ...
- Bit 63 = byte 7, bit 7

### Inversion Order

The `dout_map_apply()` function applies inversions in this order:
1. Copy logical buffer to physical buffer
2. Apply global default inversion (`dout_invert_default`)
3. Apply per-bit inversions (`bit_inv[]`)

This allows per-bit settings to override the global default.

### RGB LED Mapping

RGB LEDs use separate bit indices for each color channel:
- Each LED (0-15) has three configurable bit indices
- Set bit index to 0xFF to disable a channel
- Inversion is applied after mapping but before writing to logical buffer

### Maximum Capacity

- **Total bits**: 64 (8 bytes)
- **RGB LEDs**: Up to 16 (requires 48 bits if all RGB channels used)
- **Remaining bits**: Can be used for non-RGB outputs

## Tips and Best Practices

1. **Consistent Inversion**: Use global `dout_invert_default` for consistent hardware polarity
2. **Per-Bit Overrides**: Use `bit_inv[]` for individual exceptions (mixed active-high/low)
3. **RGB Channel Order**: Verify your RGB LED wiring matches your configuration (R-G-B vs B-G-R)
4. **Common Anode/Cathode**: Set RGB channel inversions based on LED type
5. **Unused Pins**: Set unused RGB channels to 0xFF to avoid conflicts
6. **Buffer Size**: Always use 8-byte buffers for full 64-bit support
7. **Update Frequency**: Call `dout_map_apply()` only when logical state changes to reduce CPU load

## Common Hardware Configurations

### Standard Active-High LEDs

```c
config_t cfg;
config_set_defaults(&cfg);
cfg.dout_invert_default = 0;  // Active high
dout_map_init(&cfg);
```

### Standard Active-Low LEDs

```c
config_t cfg;
config_set_defaults(&cfg);
cfg.dout_invert_default = 1;  // Active low
dout_map_init(&cfg);
```

### Mixed Active-High and Active-Low

```c
config_t cfg;
config_set_defaults(&cfg);
cfg.dout_invert_default = 0;  // Default to active high

// Invert specific bits that are active-low
cfg.bit_inv[5] = 1;
cfg.bit_inv[6] = 1;

dout_map_init(&cfg);
```

### Common Cathode RGB LEDs (Active High)

```c
config_t cfg;
config_set_defaults(&cfg);
cfg.rgb_r_invert = 0;
cfg.rgb_g_invert = 0;
cfg.rgb_b_invert = 0;
dout_map_init(&cfg);
```

### Common Anode RGB LEDs (Active Low)

```c
config_t cfg;
config_set_defaults(&cfg);
cfg.rgb_r_invert = 1;
cfg.rgb_g_invert = 1;
cfg.rgb_b_invert = 1;
dout_map_init(&cfg);
```

## Related Modules

- **AINSER Mapping**: Analog input mapping (potentiometers, sensors)
- **DIN Mapping**: Digital input mapping (buttons, switches)
- **SRIO Service**: Shift register I/O hardware driver
- **Config Service**: Central configuration management

## See Also

- [AINSER Mapping Documentation](../ainser/README.md)
- [DIN Mapping Documentation](../din/README.md)
- [Config Service Documentation](../config/README.md)
- [SRIO Service Documentation](../srio/README.md)

---

**Module Path:** `Services/dout/`  
**Header File:** `dout_map.h`  
**Implementation:** `dout_map.c`  
**Dependencies:** `Services/config/config.h`
