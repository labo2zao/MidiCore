# DIN Mapping Module

## Overview

The DIN Mapping module provides a flexible mapping layer for digital inputs (buttons, switches, footswitches), decoupling hardware pin states from MIDI event generation. It supports per-channel configuration with NOTE or CC mapping, velocity settings, inversion, and enable/disable control. Configuration can be loaded from SD card text files (`.ngc` format).

## Features

- **64 Digital Input Channels**: Support for 0-63 logical DIN indices
- **Per-Channel Mapping**: Individual configuration for each channel
- **Dual Output Types**: Send MIDI Notes or Control Changes
- **Configurable Velocities**: Independent Note On and Note Off velocities
- **MIDI Channel Routing**: Send to any MIDI channel (0-15)
- **Polarity Inversion**: Support for active-high or active-low inputs
- **Enable/Disable Control**: Turn channels on/off individually
- **SD Card Configuration**: Load mappings from text config files
- **Output Callback**: Integrate with MIDI routers or direct output

## API Reference

### Data Types

#### DIN_MapType

Event type for digital input mapping:

| Value | Name | Description |
|-------|------|-------------|
| `0` | `DIN_MAP_TYPE_NONE` | Channel disabled |
| `1` | `DIN_MAP_TYPE_NOTE` | Send MIDI Note On/Off messages |
| `2` | `DIN_MAP_TYPE_CC` | Send MIDI Control Change messages |

#### DIN_MapEntry

Per-channel configuration structure:

```c
typedef struct {
    uint8_t enabled;    // 0 = ignore, 1 = active
    uint8_t invert;     // 0 = active-low, 1 = active-high
    uint8_t type;       // DIN_MAP_TYPE_*
    uint8_t channel;    // MIDI channel 0..15 (0 = MIDI ch1)
    uint8_t number;     // Note number or CC number (0-127)
    uint8_t vel_on;     // Velocity for Note On (0-127)
    uint8_t vel_off;    // Velocity for Note Off (0-127, 0 = note-off with vel 0)
    uint8_t reserved;   // Padding for future use
} DIN_MapEntry;
```

#### DIN_MapOutputFn

Output callback function type:

```c
typedef void (*DIN_MapOutputFn)(DIN_MapType type,
                                uint8_t channel,
                                uint8_t number,
                                uint8_t value);
```

**Parameters:**
- `type`: Event type (`DIN_MAP_TYPE_NOTE` or `DIN_MAP_TYPE_CC`)
- `channel`: MIDI channel (0-15)
- `number`: Note number or CC number (0-127)
- `value`: Velocity (for NOTE) or CC value (for CC: 127=pressed, 0=released)

### Functions

| Function | Description |
|----------|-------------|
| `din_map_init_defaults()` | Initialize mapping table with default note mapping |
| `din_map_get_table()` | Returns pointer to internal mapping table for runtime modifications |
| `din_map_set_output_cb()` | Set callback function for MIDI output |
| `din_map_process_event()` | Process a digital input state change |
| `din_map_load_sd()` | Load mapping configuration from SD card file |

#### din_map_init_defaults

```c
void din_map_init_defaults(uint8_t base_note);
```

Initialize the mapping table with default settings:
- All 64 channels enabled
- Type: NOTE
- MIDI channel 0 (Channel 1)
- Note numbers: `base_note` + channel index (0-63)
- Velocity on: 100
- Velocity off: 0
- Active-low (invert=0)

**Parameters:**
- `base_note`: Starting MIDI note number (typically 36 for C1 or 48 for C2)

**Example:**
```c
// Map to notes starting at C2 (48)
din_map_init_defaults(48);
```

#### din_map_get_table

```c
DIN_MapEntry *din_map_get_table(void);
```

Returns a pointer to the internal mapping table allowing direct modification of channel configurations at runtime.

**Returns:** Pointer to array of 64 `DIN_MapEntry` structures

**Example:**
```c
DIN_MapEntry *table = din_map_get_table();
table[0].type = DIN_MAP_TYPE_CC;    // Change to CC mode
table[0].number = 64;                 // CC64 = Sustain pedal
```

#### din_map_set_output_cb

```c
void din_map_set_output_cb(DIN_MapOutputFn cb);
```

Set the output callback function that will be called when a button/switch changes state.

**Parameters:**
- `cb`: Callback function or `NULL` to disable output

**Example:**
```c
void my_din_output(DIN_MapType type, uint8_t channel, 
                   uint8_t number, uint8_t value) {
    if (type == DIN_MAP_TYPE_NOTE) {
        if (value > 0) {
            midi_send_note_on(channel, number, value);
        } else {
            midi_send_note_off(channel, number, value);
        }
    } else if (type == DIN_MAP_TYPE_CC) {
        midi_send_cc(channel, number, value);
    }
}

din_map_set_output_cb(my_din_output);
```

#### din_map_process_event

```c
void din_map_process_event(uint8_t index, uint8_t pressed);
```

Process a digital input state change for a specific channel. This function:
1. Applies polarity inversion if configured
2. Checks if channel is enabled
3. Generates appropriate MIDI message based on type (NOTE or CC)
4. Calls the output callback if set

**Parameters:**
- `index`: Channel index (0-63)
- `pressed`: Raw input state (1 = pressed/active, 0 = released/inactive, before inversion)

**Example:**
```c
// In your button scan loop
for (uint8_t i = 0; i < 64; i++) {
    uint8_t state = read_button_state(i);
    if (state != previous_state[i]) {
        din_map_process_event(i, state);
        previous_state[i] = state;
    }
}
```

#### din_map_load_sd

```c
int din_map_load_sd(const char* path);
```

Load mapping configuration from an SD card text file. Only channels explicitly listed in the file are modified; others retain their current settings.

**Parameters:**
- `path`: Path to configuration file (e.g., `"0:/cfg/din_map.ngc"`)

**Returns:**
- `0`: Success
- Negative value: Error (e.g., `-2` for file not found, `-10` if FATFS not available)

**Example:**
```c
int result = din_map_load_sd("0:/cfg/din_map.ngc");
if (result != 0) {
    // Handle error - defaults remain in place
}
```

## SD Card Configuration File Format

Configuration files use the `.ngc` format (text-based INI-style).

### File Path

Default location: `0:/cfg/din_map.ngc`

### Syntax

```ini
# Comments start with # or ;
# Only listed channels override compiled defaults

[CHn]           # n = 0..63 logical DIN index
TYPE=NOTE|CC|0|1|2
CHAN=0..15      # 0 = MIDI ch1
NUMBER=0..127   # Note or CC number
VEL_ON=0..127
VEL_OFF=0..127
INVERT=0|1
ENABLED=0|1
```

### Configuration Keys

| Key | Type | Range/Values | Description |
|-----|------|--------------|-------------|
| `TYPE` | String/Integer | NOTE, CC, 0-2 | Event type (0=NONE, 1=NOTE, 2=CC) |
| `CHAN` or `CHANNEL` | Integer | 0-15 | MIDI channel (0 = Channel 1) |
| `NUMBER`, `NOTE`, or `CC` | Integer | 0-127 | MIDI note number or CC number |
| `VEL_ON` or `VELON` | Integer | 0-127 | Velocity for Note On or CC value when pressed |
| `VEL_OFF` or `VELOFF` | Integer | 0-127 | Velocity for Note Off (0 = note-off with vel 0) |
| `INVERT` | Boolean | 0-1 | Invert polarity (1 = active-high) |
| `ENABLED` or `ENABLE` | Boolean | 0-1 | Enable this channel (1 = enabled) |

### Example Configuration

```ini
# DIN mapping config for button matrix and footswitches

[CH0]
# First button sends Note C3 on channel 1
TYPE=NOTE
CHAN=0
NUMBER=48
VEL_ON=100
VEL_OFF=0
ENABLED=1

[CH1]
# Second button acts as CC64 (sustain) toggle
TYPE=CC
CHAN=0
NUMBER=64
INVERT=0
ENABLED=1

[CH2]
# Footswitch 1: sustain pedal (active-high)
TYPE=CC
CHAN=0
NUMBER=64
INVERT=1
ENABLED=1

[CH3]
# Footswitch 2: soft pedal
TYPE=CC
CHAN=0
NUMBER=67
INVERT=1
ENABLED=1

[CH16]
# Drum pad: snare with velocity layers
TYPE=NOTE
CHAN=9         # MIDI Channel 10 (drums)
NUMBER=38      # Snare drum
VEL_ON=110
VEL_OFF=0
ENABLED=1
```

## Usage Examples

### Basic Setup

```c
#include "Services/din/din_map.h"

void din_init(void) {
    // Initialize with C2 as base note (48)
    din_map_init_defaults(48);
    
    // Set output callback
    din_map_set_output_cb(my_midi_output);
    
    // Load custom mappings from SD card
    din_map_load_sd("0:/cfg/din_map.ngc");
}

void my_midi_output(DIN_MapType type, uint8_t channel, 
                    uint8_t number, uint8_t value) {
    // Route to MIDI output
    midi_router_send(MIDI_ROUTER_SRC_DIN, type, channel, number, value);
}
```

### Runtime Configuration

```c
// Configure channel 0 as sustain pedal at runtime
DIN_MapEntry *table = din_map_get_table();

table[0].type = DIN_MAP_TYPE_CC;
table[0].channel = 0;       // MIDI Channel 1
table[0].number = 64;       // CC64 = Sustain
table[0].invert = 1;        // Active-high footswitch
table[0].enabled = 1;
```

### Button Scan Loop Integration

```c
static uint8_t prev_state[64] = {0};

void din_scan_task(void *params) {
    while (1) {
        // Scan all 64 DIN channels
        for (uint8_t ch = 0; ch < 64; ch++) {
            uint8_t current = hardware_read_button(ch);
            
            // Only process on state change
            if (current != prev_state[ch]) {
                din_map_process_event(ch, current);
                prev_state[ch] = current;
            }
        }
        
        // Delay between scans (debouncing)
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
```

### Button Matrix with Velocity Layers

```c
// Configure a velocity-sensitive button matrix
DIN_MapEntry *table = din_map_get_table();

// Soft velocity zone (channels 0-15)
for (uint8_t i = 0; i < 16; i++) {
    table[i].type = DIN_MAP_TYPE_NOTE;
    table[i].channel = 0;
    table[i].number = 36 + i;    // C1 and up
    table[i].vel_on = 64;        // Soft velocity
    table[i].enabled = 1;
}

// Hard velocity zone (channels 16-31)
for (uint8_t i = 16; i < 32; i++) {
    table[i].type = DIN_MAP_TYPE_NOTE;
    table[i].channel = 0;
    table[i].number = 36 + (i - 16);  // Same notes
    table[i].vel_on = 127;            // Hard velocity
    table[i].enabled = 1;
}
```

### Footswitch Controllers

```c
// Configure 4 footswitches for common functions
DIN_MapEntry *table = din_map_get_table();

// FS1: Sustain pedal
table[0].type = DIN_MAP_TYPE_CC;
table[0].number = 64;
table[0].invert = 1;  // Active-high

// FS2: Soft pedal
table[1].type = DIN_MAP_TYPE_CC;
table[1].number = 67;
table[1].invert = 1;

// FS3: Sostenuto
table[2].type = DIN_MAP_TYPE_CC;
table[2].number = 66;
table[2].invert = 1;

// FS4: Expression on/off
table[3].type = DIN_MAP_TYPE_CC;
table[3].number = 11;
table[3].invert = 1;
```

## Implementation Details

### Polarity Handling

The `invert` flag controls how the physical button state maps to the logical state:

**Active-Low (invert=0, default):**
- Physical HIGH (1) → Logical RELEASED (0)
- Physical LOW (0) → Logical PRESSED (1)

**Active-High (invert=1):**
- Physical HIGH (1) → Logical PRESSED (1)
- Physical LOW (0) → Logical RELEASED (0)

### NOTE Type Behavior

When `type = DIN_MAP_TYPE_NOTE`:
- **Pressed:** Sends Note On with `vel_on` velocity
- **Released:** Sends Note Off with `vel_off` velocity
- If `vel_off = 0`, sends Note Off with velocity 0 (standard)

### CC Type Behavior

When `type = DIN_MAP_TYPE_CC`:
- **Pressed:** Sends CC with value 127
- **Released:** Sends CC with value 0

This creates a toggle behavior suitable for sustain pedals, switches, and similar controls.

## Tips and Best Practices

1. **Debouncing**: Implement debouncing in your scan loop (5-10ms delay) to avoid multiple triggers
2. **State Tracking**: Only call `din_map_process_event()` when state actually changes to reduce CPU load
3. **Active-Low vs Active-High**: 
   - Use `invert=0` for buttons with pull-up resistors (most common)
   - Use `invert=1` for footswitches and momentary switches with pull-down resistors
4. **Velocity Layers**: Use multiple DIN channels mapped to the same note with different velocities for velocity-sensitive pads
5. **Channel Assignment**: Use MIDI channel 10 (set `channel=9`) for drum/percussion mappings
6. **CC vs NOTE**: Use CC for pedals, switches, and toggles; use NOTE for keys and pads

## Common Use Cases

### Piano Keyboard

```ini
# 88-key piano keyboard starting at A0 (note 21)
[CH0]
TYPE=NOTE
NUMBER=21
VEL_ON=100
# ... continue for all 88 keys
```

### Drum Pad Controller

```ini
# General MIDI drum mapping
[CH0]
TYPE=NOTE
CHAN=9        # Channel 10 for drums
NUMBER=36     # Bass Drum 1
VEL_ON=127

[CH1]
TYPE=NOTE
CHAN=9
NUMBER=38     # Snare
VEL_ON=127
```

### Control Surface

```ini
# Transport controls as CCs
[CH0]
TYPE=CC
NUMBER=110    # Play/Stop
[CH1]
TYPE=CC
NUMBER=111    # Record
[CH2]
TYPE=CC
NUMBER=112    # Rewind
```

## Related Modules

- **AINSER Mapping**: Analog input mapping (potentiometers, sensors)
- **DOUT Mapping**: Digital output mapping (LEDs, relays)
- **MIDI Router**: Routes MIDI messages between sources and destinations
- **Button Debounce**: Hardware-level debouncing utilities

## See Also

- [AINSER Mapping Documentation](../ainser/README.md)
- [DOUT Mapping Documentation](../dout/README.md)
- [SD Card File Structure](../../Assets/sd_cfg/README_SD_TREE.txt)

---

**Module Path:** `Services/din/`  
**Header File:** `din_map.h`  
**Implementation:** `din_map.c`  
**Dependencies:** FATFS (optional, for SD card support)
