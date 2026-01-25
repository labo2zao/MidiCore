# NGC Multi-Channel Selection Specification

## Overview

The NGC format supports multi-channel selection syntax, allowing a single configuration block to apply to multiple channels simultaneously. This reduces configuration file size and makes bulk configuration more maintainable.

## Syntax

### Single Channel (Basic)
```ini
[CH5]
CC=36
```

### Multiple Channels (Comma-Separated)
```ini
[CH0,1,2,3]
CC=16
ENABLED=1
```
Applies the same configuration to channels 0, 1, 2, and 3.

### Range Selection (Dash)
```ini
[CH0-7]
CC=16
ENABLED=1
```
Applies the same configuration to channels 0 through 7 (inclusive).

### Mixed Notation
```ini
[CH0,2,4-7,10]
CC=16
ENABLED=1
```
Applies to channels: 0, 2, 4, 5, 6, 7, and 10.

## Parsing Algorithm

### Channel List Parsing

```c
// Parse channel list: "0,1,2" or "0-5" or "0,2-4,7"
typedef struct {
    uint8_t channels[64];  // Array of channel indices
    uint8_t count;         // Number of channels
} ChannelList;

int parse_channel_list(const char* str, ChannelList* list) {
    list->count = 0;
    char buffer[128];
    strncpy(buffer, str, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = 0;
    
    // Tokenize by comma
    char* token = strtok(buffer, ",");
    while (token && list->count < 64) {
        // Trim whitespace
        while (*token == ' ') token++;
        
        // Check for range (dash)
        char* dash = strchr(token, '-');
        if (dash) {
            // Range: "0-5"
            *dash = 0;
            int start = atoi(token);
            int end = atoi(dash + 1);
            
            if (start >= 0 && end < 64 && start <= end) {
                for (int i = start; i <= end && list->count < 64; i++) {
                    list->channels[list->count++] = (uint8_t)i;
                }
            }
        } else {
            // Single channel
            int ch = atoi(token);
            if (ch >= 0 && ch < 64) {
                list->channels[list->count++] = (uint8_t)ch;
            }
        }
        
        token = strtok(NULL, ",");
    }
    
    return list->count > 0 ? 0 : -1;
}
```

### Section Header Parser Enhancement

```c
// Enhanced section parser for [CH0,1,2] or [CH0-5] syntax
if (line[0] == '[') {
    char* end = strchr(line, ']');
    if (!end) continue;
    *end = 0;
    char* tag = line + 1;
    trim(tag);
    
    current_channels.count = 0;
    
    if ((tag[0] == 'C' || tag[0] == 'c') && 
        (tag[1] == 'H' || tag[1] == 'h')) {
        // Parse channel list after "CH"
        if (parse_channel_list(tag + 2, &current_channels) == 0) {
            // Successfully parsed channel list
            // Subsequent key-value pairs apply to all channels in list
        }
    }
    continue;
}

// Apply configuration to all channels in current_channels
if (current_channels.count > 0) {
    for (uint8_t i = 0; i < current_channels.count; i++) {
        uint8_t ch = current_channels.channels[i];
        // Apply key-value to channel 'ch'
        apply_config(ch, key, value);
    }
}
```

## Examples

### AINSER Mapping with Multi-Channel

```ini
# Configure all bellows sensors (channels 16-19) identically
[CH16-19]
CC=36
CURVE=LOG
INVERT=1
MIN=200
MAX=3800
THRESHOLD=12
ENABLED=1

# Configure specific expression pedals
[CH0,2,4]
CC=11
CURVE=LINEAR
MIN=50
MAX=4000
ENABLED=1

# Configure a mix of ranges and individual channels
[CH8-11,14,15]
CC=20
CURVE=EXPO
ENABLED=1
```

### DIN Mapping with Multi-Channel

```ini
# Configure 12 piano keys (C to B)
[CH0-11]
TYPE=NOTE
CHAN=0
NUMBER=48     # Starting note C3, auto-incremented per channel
VEL_ON=100
ENABLED=1

# Configure footswitch array
[CH48,49,50,51]
TYPE=CC
CHAN=0
INVERT=1
ENABLED=1
# Individual CC numbers would be set per channel
```

### Auto-Increment Feature

Some implementations support auto-incrementing values:

```ini
# Auto-increment CC numbers
[CH0-7]
CC=16          # CH0=16, CH1=17, CH2=18, ..., CH7=23
ENABLED=1

# Auto-increment note numbers
[CH0-11]
TYPE=NOTE
NUMBER=48      # CH0=48, CH1=49, CH2=50, ..., CH11=59
VEL_ON=100
```

## Implementation Variations

### Basic Multi-Channel (Identical Config)

All selected channels receive **identical** configuration:

```ini
[CH0-3]
CC=16          # All channels use CC16
```

### Advanced Multi-Channel (Auto-Increment)

Selected channels receive **incremented** values:

```ini
[CH0-3]
CC=16          # CH0=16, CH1=17, CH2=18, CH3=19
```

### Per-Channel Override

Individual channel configs override multi-channel configs:

```ini
# Bulk configuration
[CH0-7]
CC=16
ENABLED=1

# Override specific channel
[CH3]
CC=20          # CH3 now uses CC20 instead of 19
ENABLED=0      # CH3 is disabled
```

## Format Validation

### Valid Formats

```ini
[CH0]           # Single channel
[CH0,1]         # Two channels
[CH0-5]         # Range (6 channels)
[CH0,2-4,7]     # Mixed (channels 0,2,3,4,7)
[CH0-3,8-11]    # Multiple ranges
```

### Invalid Formats (Should be Ignored)

```ini
[CH]            # No channel number
[CH-5]          # Missing start of range
[CH0-]          # Missing end of range
[CH5-2]         # Invalid range (start > end)
[CH0,,2]        # Double comma
[CH 0-5]        # Space in channel spec
```

## Best Practices

### 1. Use Ranges for Sequential Channels

```ini
# Good: Uses range
[CH0-15]
ENABLED=1

# Less efficient: Individual channels
[CH0]
ENABLED=1
[CH1]
ENABLED=1
# ... repeat 14 more times
```

### 2. Group Related Channels

```ini
# Bellows sensors
[CH16-19]
CC=36
CURVE=LOG

# Expression pedals
[CH0,2,4,6]
CC=11
CURVE=LINEAR
```

### 3. Document Multi-Channel Sections

```ini
# Piano keyboard octave 1 (C1-B1)
[CH0-11]
TYPE=NOTE
NUMBER=36
VEL_ON=100

# Piano keyboard octave 2 (C2-B2)
[CH12-23]
TYPE=NOTE
NUMBER=48
VEL_ON=100
```

### 4. Use Override Pattern for Exceptions

```ini
# Configure all channels
[CH0-63]
ENABLED=1
THRESHOLD=8

# Disable specific channels
[CH10,20,30]
ENABLED=0

# Custom threshold for noisy sensors
[CH5-7]
THRESHOLD=20
```

## Compatibility Notes

### Legacy Format Support

Multi-channel parsers should maintain backward compatibility with single-channel format:

```ini
# Legacy format - still valid
[CH0]
CC=16

[CH1]
CC=17
```

### Parser Detection

Applications can detect multi-channel support by checking for:
- Comma (`,`) in section name
- Dash (`-`) in section name (not at start/end)

```c
bool is_multi_channel(const char* section) {
    // Check for comma or internal dash
    const char* ch_start = section + 2; // After "CH"
    return (strchr(ch_start, ',') != NULL) || 
           (strchr(ch_start, '-') != NULL && 
            ch_start[0] != '-' && 
            ch_start[strlen(ch_start)-1] != '-');
}
```

## Error Handling

### Duplicate Channel Definitions

Later definitions override earlier ones:

```ini
[CH0-5]
CC=16

[CH3]      # Overrides CH3 from previous block
CC=20
```

### Out of Range Channels

Channels outside valid range (0-63 for most modules) are silently ignored:

```ini
[CH0-100]   # Only CH0-63 are configured
CC=16
```

### Empty Channel Lists

Empty or invalid channel lists are treated as errors and the section is skipped:

```ini
[CH]        # Invalid - no channels specified
CC=16       # This configuration is ignored
```

## Implementation Checklist

- [ ] Parse comma-separated channel lists
- [ ] Parse dash-based channel ranges  
- [ ] Support mixed notation (commas and dashes)
- [ ] Validate channel indices (0-63 range)
- [ ] Handle whitespace in channel specifications
- [ ] Apply identical config to all selected channels
- [ ] (Optional) Support auto-increment for certain keys
- [ ] (Optional) Support per-channel override
- [ ] Maintain backward compatibility with single-channel format
- [ ] Document supported syntax in module README

## See Also

- [NGC Text Parsing Reference](NGC_TEXT_PARSING_REFERENCE.md)
- [NGC Text Parsing Reference (Français)](NGC_TEXT_PARSING_REFERENCE_FR.md)
- [Mapping Modules Reference](MAPPING_MODULES_REFERENCE.md)

---

**Specification Version:** 1.0  
**Last Updated:** 2026-01-25  
**Status:** Feature Specification
