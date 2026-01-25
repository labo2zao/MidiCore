# SD Card Mapping & Parse Modules Documentation

## Overview

This document provides a comprehensive reference for the MidiCore mapping modules that parse text-based configuration files from SD card (`.ngc` format). These modules provide flexible hardware abstraction layers for analog inputs, digital inputs, and digital outputs.

## Modules

| Module | Purpose | Config File | Documentation |
|--------|---------|-------------|---------------|
| **AINSER Mapping** | Analog input (sensors, potentiometers) → MIDI CC | `0:/cfg/ainser_map.ngc` | [English](../Services/ainser/README.md) • [Français](../Services/ainser/README_FR.md) |
| **DIN Mapping** | Digital input (buttons, switches) → MIDI Notes/CC | `0:/cfg/din_map.ngc` | [English](../Services/din/README.md) • [Français](../Services/din/README_FR.md) |
| **DOUT Mapping** | Logical bits → Physical hardware outputs (LEDs, relays) | Programmatic only | [English](../Services/dout/README.md) • [Français](../Services/dout/README_FR.md) |

## Quick Reference

### AINSER Mapping Functions

| Function | Purpose | Parameters | Return |
|----------|---------|------------|--------|
| `ainser_map_init_defaults()` | Initialize with default CC mappings | None | void |
| `ainser_map_get_table()` | Get mapping table for runtime config | None | `AINSER_MapEntry*` |
| `ainser_map_set_output_cb()` | Set MIDI output callback | `AINSER_MapOutputFn cb` | void |
| `ainser_map_process_channel()` | Process ADC reading | `uint8_t index, uint16_t raw12` | void |
| `ainser_map_load_sd()` | Load config from SD card | `const char* path` | int (0=success) |

**Configuration Keys:** `CC`, `CHAN`, `CURVE`, `INVERT`, `MIN`, `MAX`, `THRESHOLD`, `ENABLED`

### DIN Mapping Functions

| Function | Purpose | Parameters | Return |
|----------|---------|------------|--------|
| `din_map_init_defaults()` | Initialize with note mappings | `uint8_t base_note` | void |
| `din_map_get_table()` | Get mapping table for runtime config | None | `DIN_MapEntry*` |
| `din_map_set_output_cb()` | Set MIDI output callback | `DIN_MapOutputFn cb` | void |
| `din_map_process_event()` | Process button/switch event | `uint8_t index, uint8_t pressed` | void |
| `din_map_load_sd()` | Load config from SD card | `const char* path` | int (0=success) |

**Configuration Keys:** `TYPE`, `CHAN`, `NUMBER`, `VEL_ON`, `VEL_OFF`, `INVERT`, `ENABLED`

### DOUT Mapping Functions

| Function | Purpose | Parameters | Return |
|----------|---------|------------|--------|
| `dout_map_init()` | Initialize with config | `const config_t* cfg` | void |
| `dout_map_apply()` | Convert logical → physical bits | `const uint8_t* logical, uint8_t* physical, uint16_t bytes` | void |
| `dout_set_rgb()` | Set RGB LED state | `uint8_t* logical, uint8_t led, uint8_t r, uint8_t g, uint8_t b` | void |

**Configuration:** Uses global `config_t` structure with `dout_invert_default`, `bit_inv[]`, `rgb_map_*[]` fields.

## File Format Specification

### General NGC Format

All mapping configuration files use the `.ngc` format:

```ini
# Comments start with # or ;
# Case-insensitive keys
# Numeric values: decimal (123), hex (0x7B), octal (0173)

[SECTIONn]      # Section header with index n
KEY=value       # Key-value pair
KEY2=value2     # Multiple keys per section
```

### AINSER Map Format (`ainser_map.ngc`)

**Important:** Don't confuse `[CHn]` (hardware input channel index) with `CHAN=` (MIDI channel number).

```ini
[CHn]                  # n = 0..63 (hardware input channel index)
CC=0..127              # MIDI CC number
CHAN=0..15             # MIDI channel (0 = MIDI Channel 1)
CURVE=0|1|2            # 0=LINEAR, 1=EXPO, 2=LOG (or LIN/EXPO/LOG)
INVERT=0|1             # 0=normal, 1=inverted
MIN=0..4095            # 12-bit ADC minimum threshold
MAX=0..4095            # 12-bit ADC maximum threshold
THRESHOLD=0..4095      # Minimum change to trigger update
ENABLED=0|1            # 0=disabled, 1=enabled
```

**Example:**
```ini
[CH16]
CC=36
CHAN=0
CURVE=LOG
INVERT=1
MIN=200
MAX=3800
THRESHOLD=12
ENABLED=1
```

### DIN Map Format (`din_map.ngc`)

**Important:** Don't confuse `[CHn]` (hardware input channel index) with `CHAN=` (MIDI channel number).

```ini
[CHn]                  # n = 0..63 (hardware input channel index)
TYPE=NOTE|CC|0|1|2     # Event type: NOTE, CC, or numeric
CHAN=0..15             # MIDI channel (0 = MIDI Channel 1)
NUMBER=0..127          # Note number or CC number
VEL_ON=0..127          # Note On velocity or CC value when pressed
VEL_OFF=0..127         # Note Off velocity
INVERT=0|1             # 0=active-low, 1=active-high
ENABLED=0|1            # 0=disabled, 1=enabled
```

**Example:**
```ini
[CH0]
TYPE=NOTE
CHAN=0
NUMBER=48
VEL_ON=100
VEL_OFF=0
ENABLED=1

[CH1]
TYPE=CC
CHAN=0
NUMBER=64
INVERT=0
ENABLED=1
```

## Parsing Implementation

All mapping modules follow a consistent parsing pattern:

1. **File Opening:** Use FATFS `f_open()` with `FA_READ` mode
2. **Line Reading:** Use `f_gets()` to read line-by-line (max 128-160 bytes per line)
3. **Comment Handling:** Skip lines starting with `#` or `;`
4. **Section Detection:** Parse `[CHn]` headers to set current channel index
5. **Key-Value Parsing:** Split on `=` character, trim whitespace
6. **Case-Insensitive Matching:** Use `tolower()` for key comparison
7. **Value Parsing:** Use `strtoul()` for numeric conversion (supports decimal, hex, octal)
8. **Validation:** Clamp values to valid ranges after parsing
9. **Partial Loading:** Only listed channels are modified; others keep defaults

### Common Parser Functions

Each module implements these helper functions:

```c
// Trim leading/trailing whitespace
static void trim(char* s);

// Case-insensitive key comparison
static int keyeq(const char* a, const char* b);

// Parse unsigned 8-bit integer
static uint8_t parse_u8(const char* s);

// Parse unsigned 16-bit integer  
static uint16_t parse_u16(const char* s);
```

### Error Handling

Return codes for `*_load_sd()` functions:
- `0`: Success
- `-1`: Invalid path parameter
- `-2`: File not found or cannot open
- `-10`: FATFS not available (compiled without SD support)

## Common Configuration Patterns

### Multi-Sensor Setup (AINSER)

```ini
# Expression pedal with linear response
[CH0]
CC=11
CURVE=LINEAR
MIN=50
MAX=4000
THRESHOLD=8

# Breath controller with exponential response
[CH1]
CC=2
CURVE=EXPO
MIN=100
MAX=3900
THRESHOLD=12

# Volume slider with logarithmic response
[CH2]
CC=7
CURVE=LOG
MIN=0
MAX=4095
THRESHOLD=10
```

### Button Matrix (DIN)

```ini
# Piano keys (C2-B2)
[CH0]
TYPE=NOTE
NUMBER=48
VEL_ON=100

[CH1]
TYPE=NOTE
NUMBER=49
VEL_ON=100

# ... continue for all keys

# Sustain pedal
[CH48]
TYPE=CC
NUMBER=64
INVERT=1
```

### Footswitch Array (DIN)

```ini
# FS1: Sustain
[CH0]
TYPE=CC
NUMBER=64
INVERT=1

# FS2: Soft Pedal
[CH1]
TYPE=CC
NUMBER=67
INVERT=1

# FS3: Sostenuto
[CH2]
TYPE=CC
NUMBER=66
INVERT=1

# FS4: Expression Toggle
[CH3]
TYPE=CC
NUMBER=11
INVERT=1
```

## Integration Examples

### Complete AINSER Integration

```c
#include "Services/ainser/ainser_map.h"
#include "Services/midi/midi_router.h"

void my_cc_output(uint8_t channel, uint8_t cc, uint8_t value) {
    midi_router_send_cc(MIDI_ROUTER_SRC_AINSER, channel, cc, value);
}

void ainser_system_init(void) {
    // 1. Initialize with defaults
    ainser_map_init_defaults();
    
    // 2. Set output callback
    ainser_map_set_output_cb(my_cc_output);
    
    // 3. Load custom configuration from SD
    int result = ainser_map_load_sd("0:/cfg/ainser_map.ngc");
    if (result != 0) {
        log_error("Failed to load AINSER config: %d", result);
        // Defaults remain in place
    }
}

void ainser_scan_task(void *params) {
    while (1) {
        for (uint8_t ch = 0; ch < 64; ch++) {
            uint16_t raw = hardware_adc_read(ch);
            ainser_map_process_channel(ch, raw);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
```

### Complete DIN Integration

```c
#include "Services/din/din_map.h"
#include "Services/midi/midi_router.h"

static uint8_t prev_state[64] = {0};

void my_din_output(DIN_MapType type, uint8_t channel, 
                   uint8_t number, uint8_t value) {
    if (type == DIN_MAP_TYPE_NOTE) {
        if (value > 0) {
            midi_router_send_note_on(MIDI_ROUTER_SRC_DIN, channel, number, value);
        } else {
            midi_router_send_note_off(MIDI_ROUTER_SRC_DIN, channel, number, value);
        }
    } else if (type == DIN_MAP_TYPE_CC) {
        midi_router_send_cc(MIDI_ROUTER_SRC_DIN, channel, number, value);
    }
}

void din_system_init(void) {
    // 1. Initialize with C2 base note
    din_map_init_defaults(48);
    
    // 2. Set output callback
    din_map_set_output_cb(my_din_output);
    
    // 3. Load custom configuration from SD
    int result = din_map_load_sd("0:/cfg/din_map.ngc");
    if (result != 0) {
        log_error("Failed to load DIN config: %d", result);
    }
}

void din_scan_task(void *params) {
    while (1) {
        for (uint8_t ch = 0; ch < 64; ch++) {
            uint8_t current = hardware_read_button(ch);
            if (current != prev_state[ch]) {
                din_map_process_event(ch, current);
                prev_state[ch] = current;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5));  // Debouncing
    }
}
```

### Complete DOUT Integration

```c
#include "Services/dout/dout_map.h"
#include "Services/config/config.h"

void dout_system_init(const config_t* cfg) {
    // Initialize DOUT mapping with configuration
    dout_map_init(cfg);
}

void update_all_leds(void) {
    uint8_t logical[8] = {0};
    uint8_t physical[8];
    
    // Set status LEDs
    if (midi_is_active()) logical[0] |= (1 << 0);
    if (sd_is_ready())    logical[0] |= (1 << 1);
    if (has_error())      logical[0] |= (1 << 2);
    
    // Set RGB LED 0 based on mode
    switch (get_current_mode()) {
        case MODE_NORMAL:
            dout_set_rgb(logical, 0, 0, 1, 0);  // Green
            break;
        case MODE_RECORD:
            dout_set_rgb(logical, 0, 1, 0, 0);  // Red
            break;
        case MODE_PLAY:
            dout_set_rgb(logical, 0, 0, 0, 1);  // Blue
            break;
    }
    
    // Apply mapping and write to hardware
    dout_map_apply(logical, physical, 8);
    srio_write_outputs(physical, 8);
}
```

## Performance Considerations

### AINSER Module
- **Smoothing:** Built-in low-pass filter (ALPHA=6) adds minimal overhead
- **Threshold:** Reduces unnecessary MIDI traffic by 80-90% for stable sensors
- **Optimization:** Only processes enabled channels, skips unchanged values

### DIN Module
- **Debouncing:** Implement in scan loop (5-10ms delay), not in mapping layer
- **State Tracking:** Only call `process_event()` on actual state changes
- **Optimization:** Disabled channels are skipped immediately

### DOUT Module
- **Update Frequency:** Call `dout_map_apply()` only when logical state changes
- **Buffer Size:** Always 8 bytes (64 bits) regardless of actual pin count
- **Optimization:** XOR operations for inversion are very fast

## Troubleshooting

### AINSER Issues

| Problem | Cause | Solution |
|---------|-------|----------|
| Jittery output | Threshold too low | Increase `THRESHOLD` to 12-20 |
| Unresponsive channel | Wrong min/max range | Measure actual ADC range, adjust `MIN`/`MAX` |
| Wrong direction | Polarity inverted | Toggle `INVERT` setting |
| No output | Channel disabled | Set `ENABLED=1` |

### DIN Issues

| Problem | Cause | Solution |
|---------|-------|----------|
| Multiple triggers | No debouncing | Add 5-10ms delay in scan loop |
| Wrong polarity | Inversion mismatch | Toggle `INVERT` (0=active-low, 1=active-high) |
| No response | Channel disabled | Set `ENABLED=1` |
| Wrong note/CC | Incorrect mapping | Check `NUMBER` and `TYPE` settings |

### DOUT Issues

| Problem | Cause | Solution |
|---------|-------|----------|
| Inverted output | Wrong polarity | Check `dout_invert_default` or `bit_inv[]` |
| RGB wrong colors | Incorrect pin mapping | Verify `rgb_map_r/g/b[]` against hardware |
| Some LEDs don't work | Unused channels | Ensure all used channels mapped (not 0xFF) |

## File Locations

```
MidiCore/
├── Services/
│   ├── ainser/
│   │   ├── ainser_map.h          # AINSER API header
│   │   ├── ainser_map.c          # AINSER implementation with parser
│   │   ├── README.md             # English documentation
│   │   └── README_FR.md          # French documentation
│   ├── din/
│   │   ├── din_map.h             # DIN API header
│   │   ├── din_map.c             # DIN implementation with parser
│   │   ├── README.md             # English documentation
│   │   └── README_FR.md          # French documentation
│   └── dout/
│       ├── dout_map.h            # DOUT API header
│       ├── dout_map.c            # DOUT implementation
│       ├── README.md             # English documentation
│       └── README_FR.md          # French documentation
└── Assets/
    └── sd_cfg/
        ├── cfg/
        │   ├── ainser_map.ngc    # AINSER example config
        │   ├── din_map.ngc       # DIN example config
        │   └── ...
        └── README_SD_TREE.txt    # SD card structure documentation
```

## See Also

- [SD Card File Structure](../Assets/sd_cfg/README_SD_TREE.txt)
- [Config Service Documentation](../Services/config/README.md)
- [MIDI Router Documentation](../Services/router/README.md)
- [SRIO Service Documentation](../Services/srio/README.md)

---

**Document Version:** 1.0  
**Last Updated:** 2026-01-25  
**Maintained By:** MidiCore Development Team
