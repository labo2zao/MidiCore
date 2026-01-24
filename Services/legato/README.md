# Legato/Mono/Priority Module

## Overview

The Legato/Mono/Priority module provides sophisticated monophonic note handling with configurable priority modes, legato transitions, and retrigger control. It's perfect for emulating classic mono synthesizers, creating expressive solo instruments, and implementing smooth note transitions without envelope retriggering.

## Features

- **Per-Track Configuration**: Independent settings for up to 4 MIDI tracks
- **Note Priority Modes**:
  - **Last**: Most recently pressed note has priority (classic mono synth behavior)
  - **Highest**: Highest note number has priority (lead playing)
  - **Lowest**: Lowest note number has priority (bass playing)
  - **First**: First pressed note retains priority until released
- **Legato Mode**: Smooth note transitions without note-off events
- **Retrigger Control**: Toggle between envelope retrigger and smooth glide
- **Note Stealing**: Intelligent note buffer management with configurable priority
- **Full Note Tracking**: Tracks up to 16 simultaneously held notes per track
- **Event Callbacks**: Rich event system for integration with synthesizers
- **Portamento/Glide**: Configurable glide time (0-2000ms)
- **MidiCore Compatible**: Uses stdint.h types and follows project conventions

## Usage

### Initialization

```c
#include "legato.h"

// Initialize the legato module (call once at startup)
legato_init();
```

### Basic Configuration

```c
// Enable legato/mono mode on track 0
legato_set_enabled(0, 1);

// Set priority mode to "last note" (most recent)
legato_set_priority(0, LEGATO_PRIORITY_LAST);

// Disable envelope retriggering (true legato)
legato_set_retrigger(0, LEGATO_RETRIGGER_OFF);

// Set glide time to 50ms
legato_set_glide_time(0, 50);
```

### Processing MIDI Events

```c
// Process note on
if (legato_process_note_on(0, note, velocity, channel)) {
    // Send MIDI note on (returned 1 = not suppressed)
    midi_send_note_on(note, velocity, channel);
}

// Process note off
if (legato_process_note_off(0, note, channel)) {
    // Send MIDI note off (returned 1 = not suppressed)
    midi_send_note_off(note, 0, channel);
}
```

### Event Callback Integration

```c
void my_legato_callback(uint8_t track, const legato_event_t* event) {
    switch (event->type) {
        case LEGATO_EVENT_NOTE_ON:
            // First note pressed - trigger full envelope
            synth_note_on(event->note, event->velocity);
            break;
            
        case LEGATO_EVENT_NOTE_OFF:
            // All notes released - trigger release
            synth_note_off(event->note);
            break;
            
        case LEGATO_EVENT_NOTE_CHANGE:
            if (event->is_legato) {
                // Legato transition - update pitch without retriggering
                synth_glide_to_note(event->note, event->velocity);
            } else {
                // Retrigger enabled - full note restart
                synth_note_off(event->prev_note);
                synth_note_on(event->note, event->velocity);
            }
            break;
            
        case LEGATO_EVENT_RETRIGGER:
            // Same note pressed again with retrigger enabled
            synth_note_on(event->note, event->velocity);
            break;
    }
}

// Register callback
legato_set_event_callback(my_legato_callback);
```

## API Reference

### Initialization

**`void legato_init(void)`**
- Initializes all tracks with default settings
- Default: disabled, last note priority, retrigger off, 0ms glide

### Enable/Disable

**`void legato_set_enabled(uint8_t track, uint8_t enabled)`**
- Enable (1) or disable (0) legato/mono mode for a track
- Track: 0-3
- When disabled, all MIDI events pass through unchanged

**`uint8_t legato_is_enabled(uint8_t track)`**
- Returns 1 if enabled, 0 if disabled

### Priority Mode

**`void legato_set_priority(uint8_t track, legato_priority_t priority)`**
- Set note priority mode
- Values:
  - `LEGATO_PRIORITY_LAST`: Most recent note (default)
  - `LEGATO_PRIORITY_HIGHEST`: Highest note number
  - `LEGATO_PRIORITY_LOWEST`: Lowest note number
  - `LEGATO_PRIORITY_FIRST`: First pressed note
- Automatically recalculates active note when changed

**`legato_priority_t legato_get_priority(uint8_t track)`**
- Returns current priority mode

**`const char* legato_get_priority_name(legato_priority_t priority)`**
- Returns human-readable priority name
- Examples: "Last", "Highest", "Lowest", "First"

### Retrigger Mode

**`void legato_set_retrigger(uint8_t track, legato_retrigger_t retrigger)`**
- Set envelope retrigger behavior
- Values:
  - `LEGATO_RETRIGGER_OFF`: True legato (no retrigger)
  - `LEGATO_RETRIGGER_ON`: Always retrigger envelope
- Track: 0-3

**`legato_retrigger_t legato_get_retrigger(uint8_t track)`**
- Returns current retrigger mode

**`const char* legato_get_retrigger_name(legato_retrigger_t retrigger)`**
- Returns human-readable retrigger name

### Glide/Portamento

**`void legato_set_glide_time(uint8_t track, uint16_t time_ms)`**
- Set portamento/glide time in milliseconds
- Range: 0-2000ms (values > 2000 clamped)
- 0 = instant pitch change
- Used by synthesizer for smooth pitch transitions

**`uint16_t legato_get_glide_time(uint8_t track)`**
- Returns current glide time in milliseconds

### Note Processing

**`uint8_t legato_process_note_on(uint8_t track, uint8_t note, uint8_t velocity, uint8_t channel)`**
- Process incoming note on event
- **Returns**:
  - `1`: Pass note through (should be sent to MIDI output)
  - `0`: Suppress note (handled internally by legato logic)
- Also triggers appropriate events via callback

**`uint8_t legato_process_note_off(uint8_t track, uint8_t note, uint8_t channel)`**
- Process incoming note off event
- **Returns**:
  - `1`: Pass note off through
  - `0`: Suppress note off (still holding another note)
- Automatically switches to next priority note if available

### State Query

**`uint8_t legato_get_active_note(uint8_t track)`**
- Returns currently sounding note number
- Returns `0xFF` if no note active

**`uint8_t legato_get_active_velocity(uint8_t track)`**
- Returns velocity of currently sounding note
- Returns `0` if no note active

**`uint8_t legato_get_held_note_count(uint8_t track)`**
- Returns number of notes currently held
- Range: 0-16

### Reset/Panic

**`void legato_clear_all_notes(uint8_t track)`**
- Clear all held notes on a track (panic)
- Sends note off event if note was active
- Track: 0-3

**`void legato_clear_all_tracks(void)`**
- Clear all held notes on all tracks (global panic)
- Sends note off events for all active notes

### Mono Mode

**`void legato_set_mono_mode(uint8_t track, uint8_t enabled)`**
- Enable/disable strict mono mode enforcement
- When disabled, module tracks priorities without enforcing mono output
- Useful for priority tracking in polyphonic contexts

**`uint8_t legato_is_mono_mode(uint8_t track)`**
- Returns 1 if mono mode enabled, 0 if poly tracking

### Event Callback

**`void legato_set_event_callback(legato_event_cb_t callback)`**
- Set callback for legato events
- Callback signature:
```c
typedef void (*legato_event_cb_t)(uint8_t track, const legato_event_t* event);
```

**Event Structure**:
```c
typedef struct {
    legato_event_type_t type;    // Event type
    uint8_t note;                // MIDI note number
    uint8_t velocity;            // Note velocity
    uint8_t channel;             // MIDI channel
    uint8_t prev_note;           // Previous active note (for transitions)
    uint8_t is_legato;           // 1 if legato transition (no retrigger)
} legato_event_t;
```

**Event Types**:
- `LEGATO_EVENT_NOTE_ON`: First note pressed
- `LEGATO_EVENT_NOTE_OFF`: All notes released
- `LEGATO_EVENT_NOTE_CHANGE`: Active note changed (during overlap)
- `LEGATO_EVENT_RETRIGGER`: Same note retriggered

## Configuration Examples

### Classic Mono Synth (Last Note Priority)
```c
legato_set_enabled(0, 1);
legato_set_priority(0, LEGATO_PRIORITY_LAST);
legato_set_retrigger(0, LEGATO_RETRIGGER_ON);  // Retrigger envelope
legato_set_glide_time(0, 0);  // No glide
```

### Smooth Legato Lead (No Retrigger)
```c
legato_set_enabled(0, 1);
legato_set_priority(0, LEGATO_PRIORITY_LAST);
legato_set_retrigger(0, LEGATO_RETRIGGER_OFF);  // True legato
legato_set_glide_time(0, 30);  // 30ms glide
```

### High Note Priority (Lead Playing)
```c
legato_set_enabled(0, 1);
legato_set_priority(0, LEGATO_PRIORITY_HIGHEST);
legato_set_retrigger(0, LEGATO_RETRIGGER_OFF);
legato_set_glide_time(0, 50);
```

### Low Note Priority (Bass)
```c
legato_set_enabled(0, 1);
legato_set_priority(0, LEGATO_PRIORITY_LOWEST);
legato_set_retrigger(0, LEGATO_RETRIGGER_ON);
legato_set_glide_time(0, 0);
```

### Drone Note (First Note Priority)
```c
legato_set_enabled(0, 1);
legato_set_priority(0, LEGATO_PRIORITY_FIRST);
legato_set_retrigger(0, LEGATO_RETRIGGER_OFF);
legato_set_glide_time(0, 100);  // Slow glide
```

## Implementation Details

### Priority Algorithm

**Last Note Priority (LEGATO_PRIORITY_LAST)**:
- Tracks timestamp of each note press
- Active note = most recent timestamp
- Most common for expressive mono playing

**Highest Note Priority (LEGATO_PRIORITY_HIGHEST)**:
- Active note = highest MIDI note number currently held
- Useful for lead playing over chords

**Lowest Note Priority (LEGATO_PRIORITY_LOWEST)**:
- Active note = lowest MIDI note number currently held
- Useful for bass lines with chord voicings

**First Note Priority (LEGATO_PRIORITY_FIRST)**:
- Active note = oldest timestamp (first pressed)
- Maintains initial note until released
- Creates "drone" effect

### Note Buffer Management

- Each track maintains buffer of up to 16 notes (`LEGATO_MAX_NOTES`)
- When buffer full, oldest note is stolen
- Timestamp tracking ensures correct priority calculation
- Per-note velocity tracking for accurate retriggering

### Legato vs. Retrigger

**Retrigger Off (True Legato)**:
- Note transitions don't send note-off events
- Envelope continues without restart
- Smooth glide between pitches
- `is_legato` flag set in `LEGATO_EVENT_NOTE_CHANGE`

**Retrigger On**:
- Each note change triggers full envelope restart
- Better for percussive/plucky sounds
- More "mono synth" behavior
- `is_legato` flag clear in `LEGATO_EVENT_NOTE_CHANGE`

### Thread Safety

The module is **not thread-safe**. Ensure all functions are called from the same thread or add appropriate locking mechanisms.

## Memory Usage

- **Static Memory**: ~544 bytes total
  - Per track: 136 bytes (16 notes × 8 bytes + config)
  - 4 tracks × 136 bytes = 544 bytes
- **Stack Usage**: Minimal (<64 bytes per function call)
- **No Dynamic Allocation**: All memory is static

## Performance

- **Processing Time**: O(N) where N is number of held notes (max 16)
- **CPU Usage**: Negligible (simple comparisons and updates)
- **Typical Latency**: < 5µs per note event on STM32F4

## Integration Examples

### With MIDI Router
```c
void midi_note_handler(uint8_t note, uint8_t velocity, uint8_t channel, uint8_t is_on) {
    uint8_t track = get_track_for_channel(channel);
    
    if (is_on) {
        if (legato_process_note_on(track, note, velocity, channel)) {
            // Module says to send this note
            hardware_send_note_on(note, velocity, channel);
        }
    } else {
        if (legato_process_note_off(track, note, channel)) {
            // Module says to send note off
            hardware_send_note_off(note, channel);
        }
    }
}
```

### With Software Synthesizer
```c
void synth_legato_callback(uint8_t track, const legato_event_t* event) {
    synth_voice_t* voice = &synth_voices[track];
    
    switch (event->type) {
        case LEGATO_EVENT_NOTE_ON:
            // Start new voice with full envelope
            synth_voice_start(voice, event->note, event->velocity);
            break;
            
        case LEGATO_EVENT_NOTE_OFF:
            // Release envelope
            synth_voice_release(voice);
            break;
            
        case LEGATO_EVENT_NOTE_CHANGE:
            if (event->is_legato) {
                // Smooth glide without retriggering
                uint16_t glide_time = legato_get_glide_time(track);
                synth_voice_glide(voice, event->note, glide_time);
            } else {
                // Retrigger envelope
                synth_voice_release(voice);
                synth_voice_start(voice, event->note, event->velocity);
            }
            break;
            
        case LEGATO_EVENT_RETRIGGER:
            // Retrigger same note
            synth_voice_retrigger(voice, event->velocity);
            break;
    }
}

// In initialization
legato_set_event_callback(synth_legato_callback);
```

### With Arpeggiator
```c
// Disable legato for arpeggiator tracks to allow full polyphony
legato_set_enabled(ARP_TRACK, 0);

// Enable legato for solo tracks
legato_set_enabled(SOLO_TRACK, 1);
```

## Advanced Use Cases

### Dynamic Priority Switching
```c
// Switch priority based on playing style
if (held_note_count > 3) {
    // Playing chords - use highest note priority
    legato_set_priority(track, LEGATO_PRIORITY_HIGHEST);
} else {
    // Single notes - use last note priority
    legato_set_priority(track, LEGATO_PRIORITY_LAST);
}
```

### Conditional Legato
```c
// Enable legato only when sustain pedal is held
void sustain_pedal_changed(uint8_t value) {
    if (value >= 64) {
        legato_set_enabled(track, 1);
        legato_set_retrigger(track, LEGATO_RETRIGGER_OFF);
    } else {
        legato_set_enabled(track, 0);
    }
}
```

### Velocity-Sensitive Retrigger
```c
void my_legato_callback(uint8_t track, const legato_event_t* event) {
    if (event->type == LEGATO_EVENT_NOTE_CHANGE) {
        // High velocity = retrigger, low velocity = legato
        if (event->velocity > 100) {
            synth_voice_retrigger(&voice, event->note, event->velocity);
        } else {
            synth_voice_glide(&voice, event->note);
        }
    }
}
```

## Limitations

- Maximum 16 notes held simultaneously per track (defined by `LEGATO_MAX_NOTES`)
- Maximum 4 tracks (defined by `LEGATO_MAX_TRACKS`)
- Maximum 2000ms glide time
- No MIDI channel filtering (caller must handle channel routing)
- Note stealing uses oldest timestamp (FIFO) when buffer full

## Troubleshooting

**Notes getting stuck**:
- Call `legato_clear_all_notes()` on panic/reset
- Ensure note-offs are processed for all note-ons

**Wrong note has priority**:
- Check priority mode setting
- Verify notes are being processed in order received

**Legato not working**:
- Confirm `legato_set_enabled()` is called
- Check that event callback is registered
- Verify `is_legato` flag in callback

**No glide/portamento**:
- Glide time is informational - synthesizer must implement pitch ramping
- Check `legato_get_glide_time()` in callback

## License

Part of MidiCore project. See project LICENSE for details.
