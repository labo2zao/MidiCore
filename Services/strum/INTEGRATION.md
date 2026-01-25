# Strum Module Integration Guide

## Quick Start

### 1. Include the Header
```c
#include "Services/strum/strum.h"
```

### 2. Initialize at Startup
```c
void system_init(void) {
    // ... other initializations ...
    strum_init();
}
```

### 3. Configure in Your Track Setup
```c
void setup_guitar_track(void) {
    uint8_t track = 0;
    
    // Enable strum with 30ms timing
    strum_set_enabled(track, 1);
    strum_set_time(track, 30);
    strum_set_direction(track, STRUM_DIR_DOWN);
    
    // Optional: add velocity ramping
    strum_set_velocity_ramp(track, STRUM_RAMP_INCREASE);
    strum_set_ramp_amount(track, 20);
}
```

### 4. Process MIDI Notes
```c
void process_chord(uint8_t track, uint8_t* chord_notes, uint8_t chord_size, uint8_t velocity) {
    for (uint8_t i = 0; i < chord_size; i++) {
        uint8_t delay_ms, new_velocity;
        
        strum_process_note(
            track,
            chord_notes[i],
            velocity,
            chord_notes,
            chord_size,
            &delay_ms,
            &new_velocity
        );
        
        // Schedule the note with calculated delay
        if (delay_ms > 0) {
            schedule_delayed_note(chord_notes[i], new_velocity, delay_ms);
        } else {
            send_note_immediately(chord_notes[i], new_velocity);
        }
    }
}
```

## Integration with Existing MidiCore Modules

### With Chord Module
```c
#include "Services/chord/chord.h"
#include "Services/strum/strum.h"

void process_note_with_chord_and_strum(uint8_t track, uint8_t note, uint8_t velocity) {
    // First, expand the chord
    uint8_t chord_notes[8];
    uint8_t chord_size = 0;
    
    if (chord_is_enabled(track)) {
        chord_size = chord_process(track, note, chord_notes);
    } else {
        chord_notes[0] = note;
        chord_size = 1;
    }
    
    // Then apply strum effect
    for (uint8_t i = 0; i < chord_size; i++) {
        uint8_t delay, vel;
        strum_process_note(track, chord_notes[i], velocity, 
                          chord_notes, chord_size, &delay, &vel);
        schedule_note(chord_notes[i], vel, delay);
    }
}
```

### With Arpeggiator
```c
// Note: Strum and arpeggiator are mutually exclusive - use one or the other
void configure_track_effect(uint8_t track, effect_type_t effect) {
    if (effect == EFFECT_STRUM) {
        arp_set_enabled(track, 0);
        strum_set_enabled(track, 1);
        strum_set_time(track, 40);
    } else if (effect == EFFECT_ARP) {
        strum_set_enabled(track, 0);
        arp_set_enabled(track, 1);
    }
}
```

### With MIDI Delay
```c
// Strum + Delay can create interesting rhythmic effects
void setup_strum_with_delay(uint8_t track) {
    // Quick strum
    strum_set_enabled(track, 1);
    strum_set_time(track, 20);
    strum_set_direction(track, STRUM_DIR_DOWN);
    
    // With echo/delay
    midi_delay_set_enabled(track, 1);
    midi_delay_set_time(track, 500);  // 500ms echo
    midi_delay_set_feedback(track, 30);
}
```

## UI Integration Examples

### OLED Menu Configuration
```c
void render_strum_menu(uint8_t track) {
    oled_clear();
    oled_printf(0, 0, "STRUM TRACK %d", track + 1);
    oled_printf(0, 1, "Enable: %s", strum_is_enabled(track) ? "ON" : "OFF");
    oled_printf(0, 2, "Time:   %3dms", strum_get_time(track));
    oled_printf(0, 3, "Dir:    %s", strum_get_direction_name(strum_get_direction(track)));
    oled_printf(0, 4, "Ramp:   %s", strum_get_ramp_name(strum_get_velocity_ramp(track)));
    oled_printf(0, 5, "Amount: %3d%%", strum_get_ramp_amount(track));
    oled_update();
}

void handle_strum_encoder(uint8_t track, int8_t delta, menu_item_t item) {
    switch (item) {
        case MENU_STRUM_ENABLE:
            strum_set_enabled(track, !strum_is_enabled(track));
            break;
            
        case MENU_STRUM_TIME: {
            int16_t time = strum_get_time(track) + delta;
            if (time < 0) time = 0;
            if (time > 200) time = 200;
            strum_set_time(track, time);
            break;
        }
            
        case MENU_STRUM_DIRECTION: {
            uint8_t dir = strum_get_direction(track);
            if (delta > 0) {
                dir = (dir + 1) % STRUM_DIR_COUNT;
            } else {
                dir = (dir + STRUM_DIR_COUNT - 1) % STRUM_DIR_COUNT;
            }
            strum_set_direction(track, dir);
            break;
        }
        
        // ... similar for other parameters
    }
    render_strum_menu(track);
}
```

### Button Control
```c
void handle_strum_button(uint8_t track, button_event_t event) {
    if (event == BUTTON_SHORT_PRESS) {
        // Toggle strum on/off
        strum_set_enabled(track, !strum_is_enabled(track));
        
    } else if (event == BUTTON_LONG_PRESS) {
        // Cycle through directions
        strum_direction_t dir = strum_get_direction(track);
        dir = (dir + 1) % STRUM_DIR_COUNT;
        strum_set_direction(track, dir);
        
        // Visual feedback
        led_blink(track, 3);
    }
}
```

## Performance Considerations

### Memory Usage
- **24 bytes** static RAM for 4 track configurations
- **No heap allocation** - all static memory
- **Minimal stack** usage (<32 bytes per call)

### CPU Usage
- **O(N)** complexity where N = chord size (typically 3-6 notes)
- **~1µs** processing time per note on STM32F4 @ 168MHz
- **No blocking operations** - immediate return

### Real-Time Safety
```c
// Safe for real-time audio callback
void audio_callback(void) {
    uint8_t delay, velocity;
    
    // This is fast enough for audio thread
    strum_process_note(track, note, vel, chord, size, &delay, &velocity);
    
    // Schedule using your real-time scheduler
    rt_schedule_note(note, velocity, delay);
}
```

## Configuration Persistence

### Save to Flash/SD Card
```c
typedef struct {
    uint8_t enabled;
    uint8_t time_ms;
    uint8_t direction;
    uint8_t velocity_ramp;
    uint8_t ramp_amount;
} strum_preset_t;

void save_strum_preset(uint8_t track, strum_preset_t* preset) {
    preset->enabled = strum_is_enabled(track);
    preset->time_ms = strum_get_time(track);
    preset->direction = strum_get_direction(track);
    preset->velocity_ramp = strum_get_velocity_ramp(track);
    preset->ramp_amount = strum_get_ramp_amount(track);
}

void load_strum_preset(uint8_t track, const strum_preset_t* preset) {
    strum_set_enabled(track, preset->enabled);
    strum_set_time(track, preset->time_ms);
    strum_set_direction(track, preset->direction);
    strum_set_velocity_ramp(track, preset->velocity_ramp);
    strum_set_ramp_amount(track, preset->ramp_amount);
}
```

## Troubleshooting

### Issue: Strum not working
```c
// Check if enabled
if (!strum_is_enabled(track)) {
    strum_set_enabled(track, 1);
}

// Check if time is non-zero
if (strum_get_time(track) == 0) {
    strum_set_time(track, 30);  // 30ms default
}
```

### Issue: Unexpected velocity values
```c
// Disable velocity ramping for debugging
strum_set_velocity_ramp(track, STRUM_RAMP_NONE);

// Or reduce ramp amount
strum_set_ramp_amount(track, 10);  // Subtle effect
```

### Issue: Up-Down not alternating
```c
// Reset the state when changing patches
strum_reset(track);

// Or when user manually switches directions
strum_set_direction(track, new_direction);
strum_reset(track);
```

## Testing

### Unit Test Example
```c
void test_strum_basic(void) {
    strum_init();
    strum_set_enabled(0, 1);
    strum_set_time(0, 60);
    strum_set_direction(0, STRUM_DIR_UP);
    
    uint8_t chord[] = {60, 64, 67};
    uint8_t delay, velocity;
    
    // First note should have 0 delay
    strum_process_note(0, 60, 100, chord, 3, &delay, &velocity);
    assert(delay == 0);
    
    // Last note should have full delay
    strum_process_note(0, 67, 100, chord, 3, &delay, &velocity);
    assert(delay == 60);
}
```

See `strum_example.c` for comprehensive test cases.

## Support

For questions or issues:
1. Check the main README.md in this directory
2. Review the example code in strum_example.c
3. Ensure chord_notes array is sorted low-to-high
4. Verify track numbers are 0-3
5. Check that chord_size matches actual array length
