# AINSER Mapping Module

## Overview

The AINSER Mapping module provides a flexible mapping layer for analog inputs, decoupling raw 12-bit ADC readings from MIDI event generation. It supports per-channel configuration with CC mapping, curves, inversion, and threshold-based filtering. Configuration can be loaded from SD card text files (`.ngc` format).

## Features

- **64 Analog Input Channels**: Support for 0-63 logical AINSER indices
- **Per-Channel Mapping**: Individual configuration for each channel
- **CC Number Assignment**: Map each channel to any MIDI CC (0-127)
- **MIDI Channel Routing**: Send to any MIDI channel (0-15)
- **Curve Types**: Linear, Exponential, and Logarithmic response curves
- **Range Configuration**: Adjustable min/max ADC thresholds (0-4095)
- **Inversion Support**: Reverse the polarity of any channel
- **Threshold Filtering**: Configurable hysteresis to avoid jitter
- **Smoothing**: Built-in low-pass filtering for stable readings
- **SD Card Configuration**: Load mappings from text config files
- **Output Callback**: Integrate with MIDI routers or direct output

## API Reference

### Data Types

#### AINSER_Curve

Response curve types for value mapping:

| Value | Name | Description |
|-------|------|-------------|
| `0` | `AINSER_CURVE_LINEAR` | Linear mapping (default) |
| `1` | `AINSER_CURVE_EXPO` | Exponential curve (more resolution near 0) |
| `2` | `AINSER_CURVE_LOG` | Logarithmic curve (more resolution near 127) |

#### AINSER_MapEntry

Per-channel configuration structure:

```c
typedef struct {
    uint8_t  cc;        // MIDI CC number (0..127)
    uint8_t  channel;   // MIDI channel (0..15)
    uint8_t  curve;     // AINSER_Curve
    uint8_t  invert;    // 0=normal, 1=inverted
    uint8_t  enabled;   // 0=ignore, 1=active
    uint8_t  reserved;  // padding / future use
    uint16_t min;       // 12-bit ADC min  (0..4095)
    uint16_t max;       // 12-bit ADC max  (0..4095), must be > min
    uint16_t threshold; // minimal delta (12-bit) to trigger an update
} AINSER_MapEntry;
```

#### AINSER_MapOutputFn

Output callback function type:

```c
typedef void (*AINSER_MapOutputFn)(uint8_t channel, uint8_t cc, uint8_t value);
```

**Parameters:**
- `channel`: MIDI channel (0-15)
- `cc`: MIDI CC number (0-127)
- `value`: MIDI value (0-127)

### Functions

| Function | Description |
|----------|-------------|
| `ainser_map_get_table()` | Returns pointer to internal mapping table for runtime modifications |
| `ainser_map_init_defaults()` | Initialize mapping table with reasonable defaults |
| `ainser_map_set_output_cb()` | Set callback function for MIDI CC output |
| `ainser_map_process_channel()` | Process single channel with 12-bit raw ADC value |
| `ainser_map_load_sd()` | Load mapping configuration from SD card file |

#### ainser_map_get_table

```c
AINSER_MapEntry *ainser_map_get_table(void);
```

Returns a pointer to the internal mapping table allowing direct modification of channel configurations at runtime.

**Returns:** Pointer to array of 64 `AINSER_MapEntry` structures

**Example:**
```c
AINSER_MapEntry *table = ainser_map_get_table();
table[0].cc = 1;        // Set channel 0 to CC1 (Modulation)
table[0].curve = AINSER_CURVE_EXPO;
```

#### ainser_map_init_defaults

```c
void ainser_map_init_defaults(void);
```

Initialize the mapping table with default settings:
- All 64 channels enabled
- CC numbers 16-79 (16+channel index)
- MIDI channel 0 (Channel 1)
- Full ADC range (0-4095)
- Linear curve
- Default threshold (8 raw ADC units)
- No inversion

**Example:**
```c
ainser_map_init_defaults();
```

#### ainser_map_set_output_cb

```c
void ainser_map_set_output_cb(AINSER_MapOutputFn cb);
```

Set the output callback function that will be called when a channel value changes.

**Parameters:**
- `cb`: Callback function or `NULL` to disable output

**Example:**
```c
void my_cc_output(uint8_t channel, uint8_t cc, uint8_t value) {
    midi_send_cc(channel, cc, value);
}

ainser_map_set_output_cb(my_cc_output);
```

#### ainser_map_process_channel

```c
void ainser_map_process_channel(uint8_t index, uint16_t raw12);
```

Process a single AINSER channel with a raw 12-bit ADC reading. This function:
1. Applies per-channel threshold filtering
2. Applies smoothing (low-pass filter)
3. Clamps to min/max range
4. Applies inversion if enabled
5. Applies curve transformation
6. Quantizes to 7-bit MIDI value (0-127)
7. Only emits output when value actually changes

**Parameters:**
- `index`: Channel index (0-63)
- `raw12`: Raw 12-bit ADC value (0-4095)

**Example:**
```c
// In your ADC scan loop
for (uint8_t ch = 0; ch < 64; ch++) {
    uint16_t raw = adc_read_channel(ch);
    ainser_map_process_channel(ch, raw);
}
```

#### ainser_map_load_sd

```c
int ainser_map_load_sd(const char* path);
```

Load mapping configuration from an SD card text file. Only channels explicitly listed in the file are modified; others retain their current settings.

**Parameters:**
- `path`: Path to configuration file (e.g., `"0:/cfg/ainser_map.ngc"`)

**Returns:**
- `0`: Success
- Negative value: Error (e.g., `-2` for file not found, `-10` if FATFS not available)

**Example:**
```c
int result = ainser_map_load_sd("0:/cfg/ainser_map.ngc");
if (result != 0) {
    // Handle error - defaults remain in place
}
```

## SD Card Configuration File Format

Configuration files use the `.ngc` format (text-based INI-style).

### File Path

Default location: `0:/cfg/ainser_map.ngc`

### Syntax

```ini
# Comments start with # or ;
# Only channels listed here override compiled defaults

[CHn]          # n = 0..63 logical AINSER index
CC=number      # 0..127
CHAN=number    # MIDI channel 0..15 (0 = ch1)
CURVE=0|1|2    # 0=linear, 1=expo, 2=log (or LIN/EXPO/LOG)
INVERT=0|1
MIN=0..4095
MAX=0..4095
THRESHOLD=delta_raw_12bit
ENABLED=0|1
```

### Configuration Keys

| Key | Type | Range | Description |
|-----|------|-------|-------------|
| `CC` | Integer | 0-127 | MIDI CC number to send |
| `CHAN` or `CHANNEL` | Integer | 0-15 | MIDI channel (0 = Channel 1) |
| `CURVE` | Integer/String | 0-2 or LIN/EXPO/LOG | Response curve type |
| `INVERT` | Boolean | 0-1 | Invert the value (1 = inverted) |
| `MIN` | Integer | 0-4095 | Minimum ADC threshold |
| `MAX` | Integer | 0-4095 | Maximum ADC threshold |
| `THRESHOLD` or `THR` | Integer | 0-4095 | Minimum change required to emit update |
| `ENABLED` or `ENABLE` | Boolean | 0-1 | Enable this channel (1 = enabled) |

### Example Configuration

```ini
# AINSER mapping config for bellows sensors

[CH16]
# Primary bellows sensor
CC=36
CHAN=0
CURVE=LOG       # or CURVE=2
INVERT=1
MIN=200
MAX=3800
THRESHOLD=12
ENABLED=1

[CH17]
# Secondary bellows sensor
CC=37
CHAN=0
CURVE=LOG
INVERT=0
MIN=200
MAX=3800
THRESHOLD=12
ENABLED=1

[CH20]
# Expression pedal with linear response
CC=11          # MIDI Expression
CHAN=0
CURVE=LINEAR   # or CURVE=0
MIN=50
MAX=4000
THRESHOLD=8
ENABLED=1
```

## Usage Examples

### Basic Setup

```c
#include "Services/ainser/ainser_map.h"

void ainser_init(void) {
    // Initialize with defaults
    ainser_map_init_defaults();
    
    // Set output callback
    ainser_map_set_output_cb(my_midi_output);
    
    // Load custom mappings from SD card
    ainser_map_load_sd("0:/cfg/ainser_map.ngc");
}

void my_midi_output(uint8_t channel, uint8_t cc, uint8_t value) {
    // Route to MIDI output
    midi_router_send_cc(MIDI_ROUTER_SRC_AINSER, channel, cc, value);
}
```

### Runtime Configuration

```c
// Modify channel 0 settings at runtime
AINSER_MapEntry *table = ainser_map_get_table();

// Configure as modulation wheel
table[0].cc = 1;              // CC1 = Modulation
table[0].channel = 0;         // MIDI Channel 1
table[0].curve = AINSER_CURVE_LINEAR;
table[0].min = 100;           // Ignore bottom 100 ADC units
table[0].max = 4000;          // Ignore top 95 ADC units
table[0].threshold = 10;      // Reduce jitter
table[0].enabled = 1;
```

### Integration with ADC Scan Loop

```c
void ainser_scan_task(void *params) {
    while (1) {
        // Scan all 64 AINSER channels
        for (uint8_t ch = 0; ch < 64; ch++) {
            uint16_t raw = hardware_adc_read(ch);
            ainser_map_process_channel(ch, raw);
        }
        
        // Delay between scans
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
```

### Curve Comparison

```c
// Test different curves on the same channel
AINSER_MapEntry *table = ainser_map_get_table();

// Linear: direct proportional response
table[0].curve = AINSER_CURVE_LINEAR;

// Exponential: more sensitivity at low values
table[1].curve = AINSER_CURVE_EXPO;

// Logarithmic: more sensitivity at high values
table[2].curve = AINSER_CURVE_LOG;
```

## Implementation Details

### Smoothing Algorithm

The module uses a simple one-pole low-pass filter for smoothing:

```
filtered = (filtered * ALPHA + raw) / (ALPHA + 1)
```

Default ALPHA = 6, providing good balance between responsiveness and stability.

### Threshold Behavior

The threshold prevents emitting MIDI CC messages for small ADC fluctuations:
- Only values changing by more than `threshold` ADC units trigger processing
- Helps eliminate jitter from noisy analog inputs
- Default threshold is 8 ADC units (~0.2% of full scale)

### Value Quantization

The module converts 12-bit ADC values (0-4095) to 7-bit MIDI values (0-127):
1. Clamp raw value to configured min/max range
2. Apply inversion if enabled
3. Scale to 0-127 range with rounding
4. Apply curve transformation
5. Only emit if different from last sent value

### Curve Implementations

**Linear:** Direct mapping
```
output = input
```

**Exponential:** Square operation for more resolution at low end
```
output = (input * input) / 127
```

**Logarithmic:** Square root operation for more resolution at high end
```
output = sqrt(input * 127)
```

## Tips and Best Practices

1. **Threshold Settings**: Increase threshold (12-20) for noisy sensors, decrease (4-8) for clean signals
2. **Min/Max Calibration**: Measure actual sensor range and set min/max accordingly to use full MIDI range
3. **Curve Selection**: 
   - Use LINEAR for most applications
   - Use EXPO for breath controllers and expression pedals (more control at quiet levels)
   - Use LOG for volume/fader controls (more precision at loud levels)
4. **Smoothing**: The built-in smoothing (ALPHA=6) works well for most sensors; modify `AINSER_MAP_DEFAULT_SMOOTHING` if needed
5. **Channel Count**: Only enable channels with connected sensors to reduce CPU load

## Related Modules

- **DIN Mapping**: Digital input mapping (buttons, switches)
- **DOUT Mapping**: Digital output mapping (LEDs, relays)
- **MIDI Router**: Routes MIDI messages between sources and destinations
- **Expression Service**: Higher-level expression pedal handling

## See Also

- [DIN Mapping Documentation](../din/README.md)
- [DOUT Mapping Documentation](../dout/README.md)
- [SD Card File Structure](../../Assets/sd_cfg/README_SD_TREE.txt)

---

**Module Path:** `Services/ainser/`  
**Header File:** `ainser_map.h`  
**Implementation:** `ainser_map.c`  
**Dependencies:** FATFS (optional, for SD card support)
