# MIDI Filter Integration Guide

## Quick Start

### 1. Add to Your Project

The MIDI Filter module is located in `/Services/midi_filter/` and consists of:
- `midi_filter.h` - Header file
- `midi_filter.c` - Implementation file

Include in your STM32CubeMX project or Makefile.

### 2. Initialize

Add to your initialization code (e.g., in `main.c`):

```c
#include "Services/midi_filter/midi_filter.h"

int main(void) {
    // ... other initialization ...
    
    midi_filter_init();
    
    // ... rest of main ...
}
```

### 3. Configure Filters

Configure filters based on your needs. Example for Track 0:

```c
// Enable filtering for track 0
midi_filter_set_enabled(0, 1);

// Only allow note messages
midi_filter_set_allowed_messages(0, 
    MIDI_FILTER_MSG_NOTE_ON | MIDI_FILTER_MSG_NOTE_OFF);

// Only channels 1-4
midi_filter_set_channel_mode(0, MIDI_FILTER_CHANNEL_MODE_ALLOW);
midi_filter_set_channel_mask(0, 0x000F);  // Channels 0-3
```

### 4. Apply Filters

In your MIDI processing callback:

```c
void on_midi_receive(uint8_t status, uint8_t data1, uint8_t data2) {
    uint8_t track = get_current_track();  // Your track selection logic
    
    // Test if message should pass
    if (midi_filter_test_message(track, status, data1, data2) == MIDI_FILTER_RESULT_PASS) {
        // Forward the message
        midi_router_send3(MIDI_ROUTER_SRC_INTERNAL, status, data1, data2);
    } else {
        // Message blocked - optionally log or count
    }
}
```

## Common Use Cases

### Use Case 1: Block MIDI Clock

Stop MIDI clock from flooding your system:

```c
midi_filter_set_enabled(0, 1);
midi_filter_set_allowed_messages(0, MIDI_FILTER_MSG_ALL);
midi_filter_set_message_enabled(0, MIDI_FILTER_MSG_CLOCK, 0);
```

### Use Case 2: Piano Split (Note Range)

Create split points for different sounds:

```c
// Track 0: Bass (notes below C3)
midi_filter_set_enabled(0, 1);
midi_filter_set_note_range_enabled(0, 1);
midi_filter_set_note_range(0, 0, 47);  // Up to B2

// Track 1: Piano (notes C3 and above)
midi_filter_set_enabled(1, 1);
midi_filter_set_note_range_enabled(1, 1);
midi_filter_set_note_range(1, 48, 127);  // C3 and up
```

### Use Case 3: Velocity Layers

Route based on velocity:

```c
// Track 0: Soft layer
midi_filter_set_enabled(0, 1);
midi_filter_set_velocity_range_enabled(0, 1);
midi_filter_set_velocity_range(0, 1, 60);

// Track 1: Loud layer
midi_filter_set_enabled(1, 1);
midi_filter_set_velocity_range_enabled(1, 1);
midi_filter_set_velocity_range(1, 61, 127);
```

### Use Case 4: Drum Channel Isolation

Route drum channel separately:

```c
// Track 0: Melodic instruments (all except channel 10)
midi_filter_set_enabled(0, 1);
midi_filter_set_channel_mode(0, MIDI_FILTER_CHANNEL_MODE_BLOCK);
midi_filter_set_channel_mask(0, 0x0000);
midi_filter_set_channel_enabled(0, 9, 1);  // Block channel 10

// Track 1: Drums only (channel 10)
midi_filter_set_enabled(1, 1);
midi_filter_set_channel_mode(1, MIDI_FILTER_CHANNEL_MODE_ALLOW);
midi_filter_set_channel_mask(1, 0x0000);
midi_filter_set_channel_enabled(1, 9, 1);  // Allow only channel 10
```

### Use Case 5: Block Sustain Pedal

Remove sustain pedal CC#64:

```c
midi_filter_set_enabled(0, 1);
midi_filter_set_cc_filter_enabled(0, 1);
midi_filter_set_cc_enabled(0, 64, 0);  // Block CC#64 (sustain)
```

## Advanced Integration

### Dynamic Filter Updates

Filters can be changed in real-time:

```c
void on_button_press(uint8_t button) {
    switch (button) {
        case BTN_FILTER_TOGGLE:
            // Toggle filter on/off
            uint8_t enabled = midi_filter_is_enabled(0);
            midi_filter_set_enabled(0, !enabled);
            break;
            
        case BTN_SPLIT_POINT_UP:
            // Move split point up
            uint8_t min, max;
            midi_filter_get_note_range(0, &min, &max);
            midi_filter_set_note_range(0, min, max + 1);
            break;
    }
}
```

### Multiple Filters Per Track

Combine multiple filter types:

```c
// Track 0: Complex filtering
midi_filter_set_enabled(0, 1);

// Only notes and CCs
midi_filter_set_allowed_messages(0, 
    MIDI_FILTER_MSG_NOTE_ON | 
    MIDI_FILTER_MSG_NOTE_OFF | 
    MIDI_FILTER_MSG_CONTROL_CHANGE);

// Only channels 1-4
midi_filter_set_channel_mode(0, MIDI_FILTER_CHANNEL_MODE_ALLOW);
midi_filter_set_channel_mask(0, 0x000F);

// Note range: middle 3 octaves
midi_filter_set_note_range_enabled(0, 1);
midi_filter_set_note_range(0, 48, 84);  // C3 to C6

// Velocity range: medium to loud
midi_filter_set_velocity_range_enabled(0, 1);
midi_filter_set_velocity_range(0, 40, 127);

// Block volume and pan CCs
midi_filter_set_cc_filter_enabled(0, 1);
midi_filter_set_cc_enabled(0, 7, 0);   // Block volume
midi_filter_set_cc_enabled(0, 10, 0);  // Block pan
```

### Performance Monitoring

Track filter statistics:

```c
typedef struct {
    uint32_t passed;
    uint32_t blocked;
} filter_stats_t;

filter_stats_t stats[4] = {0};

void on_midi_receive(uint8_t status, uint8_t data1, uint8_t data2) {
    uint8_t track = get_current_track();
    
    if (midi_filter_test_message(track, status, data1, data2) == MIDI_FILTER_RESULT_PASS) {
        stats[track].passed++;
        midi_router_send3(MIDI_ROUTER_SRC_INTERNAL, status, data1, data2);
    } else {
        stats[track].blocked++;
    }
}
```

## Thread Safety

The MIDI Filter module is **not inherently thread-safe**. If you need to:

1. **Configure from multiple contexts**: Add mutex protection
2. **Test from multiple contexts**: Safe if no configuration changes
3. **Change config during testing**: Add mutex protection

Example with mutex:

```c
osMutexId_t filter_mutex;

void init_filters(void) {
    midi_filter_init();
    filter_mutex = osMutexNew(NULL);
}

void safe_configure(uint8_t track, uint16_t msg_types) {
    osMutexAcquire(filter_mutex, osWaitForever);
    midi_filter_set_allowed_messages(track, msg_types);
    osMutexRelease(filter_mutex);
}
```

## Memory Considerations

- **Static allocation**: ~160 bytes total (4 tracks × 40 bytes)
- **Stack usage**: Minimal (<20 bytes per function call)
- **No heap**: No malloc/free
- **No recursion**: All functions are non-recursive

Safe for embedded systems with limited RAM.

## Performance Tips

1. **Disable unused filters**: If not using note range filter, keep it disabled
2. **Use message type filter first**: Most efficient to block entire message types
3. **Batch configuration**: Set channel mask once instead of 16 individual calls
4. **Cache track selection**: Avoid recalculating track on every message

Example of efficient configuration:

```c
// Good: Set mask once
midi_filter_set_channel_mask(0, 0x000F);

// Less efficient: 16 individual calls
for (int i = 0; i < 16; i++) {
    midi_filter_set_channel_enabled(0, i, i < 4);
}
```

## Debugging

Enable debug output:

```c
#define MIDI_FILTER_DEBUG 1

void on_midi_receive(uint8_t status, uint8_t data1, uint8_t data2) {
    uint8_t track = get_current_track();
    midi_filter_result_t result = midi_filter_test_message(track, status, data1, data2);
    
    #ifdef MIDI_FILTER_DEBUG
    if (result == MIDI_FILTER_RESULT_BLOCK) {
        printf("BLOCKED: Track=%d Status=0x%02X D1=%d D2=%d\n", 
               track, status, data1, data2);
    }
    #endif
    
    if (result == MIDI_FILTER_RESULT_PASS) {
        midi_router_send3(MIDI_ROUTER_SRC_INTERNAL, status, data1, data2);
    }
}
```

## Testing

Use the provided example file to verify your configuration:

```bash
# Compile test program
cd Services/midi_filter
gcc -Wall -Wextra -std=c99 -I../.. midi_filter.c midi_filter_example.c -o test

# Run tests
./test
```

All 9 examples should pass.

## Support

For questions or issues:
1. Check README.md for complete API documentation
2. Review midi_filter_example.c for usage patterns
3. Verify initialization and configuration order
4. Check return values for boundary conditions

## Version History

- **1.0.0** (2024-01-24): Initial release
  - All features implemented
  - Full test coverage
  - Complete documentation
