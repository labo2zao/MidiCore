# NGC Text Format Parsing Reference

## Overview

The NGC (Next Generation Config) format is a text-based configuration file format used throughout MidiCore for loading settings from SD card. It uses an INI-style syntax with sections and key-value pairs, supporting comments and multiple numeric formats.

## File Format Specification

### File Extension
- **Extension:** `.ngc`
- **Encoding:** ASCII text
- **Line Endings:** CR+LF (Windows) or LF (Unix) both supported
- **Maximum Line Length:** 128-160 characters (module dependent)

### Basic Syntax

```ini
# This is a comment
; This is also a comment

[SECTION]           # Section header
KEY=value           # Key-value pair
ANOTHER_KEY=123     # Numeric value
```

## Text Parsing Commands

### Comment Commands

| Command | Description | Example |
|---------|-------------|---------|
| `#` | Line comment (start of line or inline) | `# Full line comment` |
| `;` | Line comment (alternative syntax) | `; Alternative comment` |

**Behavior:**
- Comments extend from the marker to end of line
- Empty lines are ignored
- Whitespace-only lines are ignored

**Examples:**
```ini
# This is a full-line comment
KEY=value  # Inline comment after value
; Alternative comment style
   # Comment with leading whitespace (also valid)
```

### Section Headers

**Syntax:** `[SECTION_NAME]` or `[SECTIONn]` where `n` is a numeric index

**Rules:**
- Section name enclosed in square brackets `[ ]`
- Case-insensitive matching
- Alphanumeric characters and underscores allowed
- Numeric suffix supported (e.g., `[CH0]`, `[CH63]`)

**Examples:**
```ini
[CH0]          # Channel section with index 0
[CH16]         # Channel section with index 16
[GLOBAL]       # Named section without index
[SETTINGS]     # Another named section
```

**Section Patterns by Module:**

| Module | Section Format | Index Range | Example |
|--------|---------------|-------------|---------|
| AINSER | `[CHn]` | n = 0..63 | `[CH16]` |
| DIN | `[CHn]` | n = 0..63 | `[CH0]` |
| Router | `[SRCn]` | n = 0..6 | `[SRC1]` |
| Chord Bank | `[CHORDn]` | n = 0..127 | `[CHORD0]` |
| UI Actions | `[ENCn_action]` | n = 0..1 | `[ENC0_CW]` |

### Key-Value Assignment

**Syntax:** `KEY=value`

**Rules:**
- Keys are case-insensitive
- No spaces allowed within key names
- `=` (equals sign) is the delimiter
- Leading/trailing whitespace is automatically trimmed
- Value parsing depends on expected data type

**Examples:**
```ini
CC=36                    # Numeric value
CHANNEL=0                # Numeric value
CURVE=LINEAR             # String value
ENABLED=1                # Boolean value (0 or 1)
MIN=200                  # Numeric value
```

### Numeric Value Formats

The parser supports multiple numeric formats using `strtoul()`:

| Format | Prefix | Example | Decimal Value |
|--------|--------|---------|---------------|
| Decimal | None | `123` | 123 |
| Hexadecimal | `0x` or `0X` | `0x7B` | 123 |
| Octal | `0` | `0173` | 123 |

**Examples:**
```ini
# All these set the same value (123)
NUMBER=123              # Decimal
NUMBER=0x7B             # Hexadecimal
NUMBER=0173             # Octal

# Common hex examples
CC=0x24                 # CC 36
CHANNEL=0x0             # Channel 0
THRESHOLD=0x0C          # Threshold 12
```

### String Value Formats

String values are used for enumerated types and names:

**Case-Insensitive Matching:**
```ini
CURVE=LINEAR            # Same as CURVE=linear or CURVE=Linear
TYPE=NOTE               # Same as TYPE=note or TYPE=Note
```

**Common String Values:**

| Module | Key | Valid Values |
|--------|-----|--------------|
| AINSER | `CURVE` | `LINEAR`, `LIN`, `EXPO`, `LOG` |
| DIN | `TYPE` | `NONE`, `NOTE`, `CC` |
| Router | `UART` | `0`, `1`, or numeric flags |

### Boolean Value Formats

Boolean values accept multiple representations:

| Value | Interpreted As |
|-------|----------------|
| `0` | False/Disabled |
| `1` | True/Enabled |
| `false` | False (string, case-insensitive) |
| `true` | True (string, case-insensitive) |
| `no` | False (some modules) |
| `yes` | True (some modules) |

**Examples:**
```ini
ENABLED=1               # Enabled
ENABLED=0               # Disabled
INVERT=1                # Inverted
INVERT=0                # Not inverted
```

## Parser Behavior

### Whitespace Handling

**Automatic Trimming:**
- Leading whitespace before keys is removed
- Trailing whitespace after values is removed
- Whitespace around `=` delimiter is ignored

**Examples:**
```ini
KEY=value               # Standard
  KEY=value             # Leading spaces ignored
KEY=value               # Trailing spaces ignored
  KEY  =  value         # All extra spaces ignored
```

### Key Alias Support

Many modules support multiple key names for the same parameter:

| Primary Key | Aliases | Module |
|-------------|---------|--------|
| `CHANNEL` | `CHAN` | AINSER, DIN, Router |
| `ENABLED` | `ENABLE` | AINSER, DIN |
| `THRESHOLD` | `THR` | AINSER |
| `VEL_ON` | `VELON` | DIN |
| `VEL_OFF` | `VELOFF` | DIN |
| `NUMBER` | `NOTE`, `CC` | DIN |

**Example:**
```ini
# These are equivalent
CHANNEL=0
CHAN=0

# These are equivalent
ENABLED=1
ENABLE=1
```

### Value Validation and Clamping

The parser automatically validates and clamps values to valid ranges:

**AINSER Module:**
```ini
CC=200          # Clamped to 127 (max MIDI CC)
MIN=5000        # Accepted as-is (12-bit ADC: 0-4095)
CURVE=5         # Clamped to 2 (max curve type)
```

**DIN Module:**
```ini
NUMBER=200      # Clamped to 127 (max MIDI note/CC)
CHANNEL=20      # Masked to 0x0F (0-15 range)
VEL_ON=150      # Accepted as-is, depends on module
```

### Partial Configuration Loading

**Important:** Only explicitly listed items are modified; all others retain their default or current values.

**Example:**
```ini
# Only CH0 and CH16 are modified
# Channels 1-15 and 17-63 keep their defaults

[CH0]
CC=1
ENABLED=1

[CH16]
CC=36
CURVE=LOG
```

### Error Handling

Parser behavior on errors:

| Error Condition | Parser Behavior |
|-----------------|-----------------|
| File not found | Returns error code (-2), keeps defaults |
| Invalid section | Ignores section and its keys |
| Invalid key | Ignores key, continues parsing |
| Invalid value | Uses 0 or default value |
| Line too long | Truncates line at buffer size |
| Malformed line | Ignores line, continues parsing |

**Robust Parsing:**
The parser is designed to be forgiving - it continues parsing even when encountering errors, allowing partial configuration to succeed.

## Common Parsing Patterns

### Range Specification

```ini
# Analog sensor with custom range
[CH0]
MIN=100         # Ignore readings below 100
MAX=3900        # Ignore readings above 3900
```

### Multi-Channel Configuration

```ini
# Configure multiple channels in sequence
[CH0]
CC=16
ENABLED=1

[CH1]
CC=17
ENABLED=1

[CH2]
CC=18
ENABLED=1
```

### Conditional Configuration

```ini
# Enable only specific channels
[CH0]
ENABLED=1       # This channel active

[CH1]
ENABLED=0       # This channel disabled

[CH2]
# Not listed = keeps default enabled state
```

### Mixed Format Values

```ini
# Use different number formats as needed
[CH0]
CC=0x24         # Hex for MIDI values
MIN=200         # Decimal for ADC
THRESHOLD=0x0A  # Hex for readability
```

## Module-Specific Commands

### Important Terminology

**Don't confuse `[CHn]` with `CHAN=`:**
- `[CHn]` = Section header for **hardware event index** (e.g., `[CH0]`, `[CH16]`) - identifies which input
- `CHAN=` = Configuration key for **MIDI channel** number (0-15) - sets MIDI output channel

### AINSER Mapping Commands

```ini
[CHn]                   # n = 0..63 (AINSER event index - sensor identifier)
CC=0..127               # MIDI CC number
CHAN=0..15              # MIDI channel (0 = MIDI channel 1)
CURVE=LINEAR|EXPO|LOG   # Response curve (or 0|1|2)
INVERT=0|1              # Invert polarity
MIN=0..4095             # ADC minimum (12-bit)
MAX=0..4095             # ADC maximum (12-bit)
THRESHOLD=0..4095       # Change threshold
ENABLED=0|1             # Enable/disable channel
```

**Full Example:**
```ini
# Primary bellows sensor with exponential response
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

### DIN Mapping Commands

```ini
[CHn]                   # n = 0..63 (DIN event index - button identifier)
TYPE=NOTE|CC|0|1|2      # Event type
CHAN=0..15              # MIDI channel (0 = MIDI channel 1)
NUMBER=0..127           # Note or CC number
VEL_ON=0..127           # Note On velocity
VEL_OFF=0..127          # Note Off velocity
INVERT=0|1              # Active-high/low
ENABLED=0|1             # Enable/disable channel
```

**Full Example:**
```ini
# Piano key sending C3
[CH0]
TYPE=NOTE
CHAN=0
NUMBER=48
VEL_ON=100
VEL_OFF=0
ENABLED=1

# Sustain pedal (footswitch)
[CH48]
TYPE=CC
CHAN=0
NUMBER=64
INVERT=1
ENABLED=1
```

### Router Mapping Commands

```ini
[SRCn]                  # n = 0..6 (source index)
DST=bitmask             # Destination bitmask
UART=0|1                # Route to UART
USBH=0|1                # Route to USB Host
USBD=0|1                # Route to USB Device
DREAM=0|1               # Route to DREAM synth
```

**Full Example:**
```ini
# DIN input routes to UART and USB Host
[SRC1]
UART=1
USBH=1
USBD=0
DREAM=0
```

## Advanced Parsing Features

### Implicit Value Defaults

When a key is present without a value:

```ini
KEY=            # Empty value = 0 or ""
```

### Section Index Extraction

The parser extracts numeric indices from section names:

```ini
[CH0]           # Index = 0
[CH16]          # Index = 16
[CHORD127]      # Index = 127
[SRC1]          # Index = 1
```

### Case Normalization

All key and string value comparisons use case-insensitive matching:

```ini
# These are all equivalent
CURVE=LINEAR
curve=linear
CuRvE=LiNeAr
```

## Best Practices

### File Organization

```ini
# 1. File header with description
# AINSER Mapping Configuration
# Project: MyInstrument v1.0
# Date: 2026-01-25

# 2. Group related channels
# Bellows sensors
[CH16]
CC=36
CURVE=LOG

[CH17]
CC=37
CURVE=LOG

# 3. Separate sections with blank lines
[CH20]
CC=11
CURVE=LINEAR
```

### Readability

```ini
# Use comments to document purpose
[CH0]
CC=1            # Modulation wheel
CURVE=LINEAR    # Direct response
MIN=50          # Calibrated minimum
MAX=4000        # Calibrated maximum
ENABLED=1       # Active

# Use hex for MIDI values, decimal for measurements
CC=0x40         # MIDI CC 64 (hex more readable)
MIN=200         # ADC value (decimal more intuitive)
```

### Testing and Validation

```ini
# Temporarily disable channels for testing
[CH5]
ENABLED=0       # TODO: Test this sensor tomorrow

# Document calibration values
[CH10]
MIN=180         # Calibrated 2026-01-20
MAX=3850        # Calibrated 2026-01-20
```

## Troubleshooting

### Common Parsing Issues

| Problem | Cause | Solution |
|---------|-------|----------|
| Configuration not loading | File path wrong | Check SD card path (0:/cfg/...) |
| Section ignored | Wrong section name | Verify section format ([CHn]) |
| Key ignored | Typo in key name | Check spelling, case doesn't matter |
| Wrong value loaded | Number format error | Use decimal (123) or hex (0x7B) |
| Value clamped | Out of range | Check min/max for parameter |

### Debugging Tips

**Enable Logging:**
```c
int result = ainser_map_load_sd("0:/cfg/ainser_map.ngc");
if (result != 0) {
    log_error("Failed to load AINSER config: %d", result);
}
```

**Verify File Contents:**
- Check file exists on SD card
- Verify file is not empty
- Ensure proper line endings (LF or CRLF)
- Check for hidden characters

**Test Incrementally:**
- Start with minimal config (one section)
- Add sections one at a time
- Test after each addition

## Implementation Reference

### Parser State Machine

```
START → READ_LINE → TRIM
  ↓
CHECK_COMMENT → (Yes) → SKIP_LINE → READ_LINE
  ↓
(No) → CHECK_SECTION → (Yes) → PARSE_SECTION → STORE_INDEX → READ_LINE
  ↓
(No) → CHECK_KEY_VALUE → (Yes) → PARSE_KEY → PARSE_VALUE → STORE → READ_LINE
  ↓
(No) → SKIP_LINE → READ_LINE
  ↓
END_OF_FILE → VALIDATE → RETURN
```

### Memory Usage

Typical parser memory footprint:
- Line buffer: 128-160 bytes (stack)
- Mapping table: 64 × entry size (static/global)
  - AINSER: 64 × 12 bytes = 768 bytes
  - DIN: 64 × 8 bytes = 512 bytes
- State variables: ~10 bytes

**Total:** Less than 1.5 KB per module

## See Also

- [AINSER Mapping Documentation](../Services/ainser/README.md)
- [DIN Mapping Documentation](../Services/din/README.md)
- [Mapping Modules Reference](MAPPING_MODULES_REFERENCE.md)
- [SD Card File Structure](../Assets/sd_cfg/README_SD_TREE.txt)

---

**Document Version:** 1.0  
**Last Updated:** 2026-01-25  
**Format Specification:** NGC v1.0
