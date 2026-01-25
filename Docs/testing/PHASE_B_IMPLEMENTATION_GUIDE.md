# Phase B Implementation Guide - Advanced MIDI Processing

## Status: Framework Created, Implementation In Progress

This document guides the implementation of Phase B features for MODULE_TEST_MIDI_DIN.

## Overview

Phase B adds advanced MIDI processing capabilities:
1. **Arpeggiator** - Pattern-based note arpeggiation
2. **Chord Generator** - Transform single notes into chords
3. **Note Delay/Echo** - Time-based note repetition with feedback
4. **MIDI Clock Sync** - Synchronize to external MIDI clock

## 1. Arpeggiator (CC 90-94)

### Architecture
- **Files Created**: `Services/arpeggiator/arpeggiator.h`, `arpeggiator.c`
- **State**: Stub implementation complete, full implementation pending

### Features
- **CC 90**: Enable/Disable (val > 64 = ON)
- **CC 91**: Pattern selection (0-5)
  * 0 = Up (low to high)
  * 1 = Down (high to low)
  * 2 = Up-Down (ping-pong)
  * 3 = Random
  * 4 = As Played
- **CC 92**: Rate divider (0-127 → 1/32 to 1/1 note)
- **CC 93**: Number of octaves (1-4)
- **CC 94**: Gate length (10%-200%)

### Implementation Steps

#### Step 1: Core Data Structures ✅ DONE
```c
typedef struct {
  uint8_t note;
  uint8_t velocity;
  uint32_t timestamp;
} arp_note_t;

static arp_note_t arp_notes[ARP_MAX_NOTES];
static uint8_t arp_note_count = 0;
```

#### Step 2: Note Buffer Management (TODO)
```c
int arp_note_on(uint8_t note, uint8_t velocity) {
  // Add note to buffer if not full
  // Sort by pitch for Up/Down patterns
  // Store timestamp for AsPlayed pattern
}

int arp_note_off(uint8_t note) {
  // Remove note from buffer
  // Shift remaining notes
}
```

#### Step 3: Pattern Generation (TODO)
```c
uint8_t arp_get_next_note(void) {
  // Generate next note based on pattern
  // Handle octave spreading
  // Return note number
}
```

#### Step 4: Timing and Gate (TODO)
```c
void arp_tick_1ms(void (*callback)(uint8_t, uint8_t, uint8_t)) {
  // Increment tick counter
  // Check if time for next note based on rate
  // Call callback for note on/off
  // Handle gate length
}
```

#### Step 5: Integration with MODULE_TEST_MIDI_DIN (TODO)
```c
// In module_tests.c CC switch:
case 90:  // Arpeggiator Enable
  arp_set_enabled(val > 64);
  break;

// In note processing:
if (arp_get_enabled()) {
  arp_note_on(note, velocity);
  // Don't pass note through - arp will generate
} else {
  // Normal processing
}
```

### Testing
- Send CC 90 (127) to enable
- Play chord (C-E-G)
- Should hear notes arpeggi ated in pattern
- Change CC 91 to test different patterns

## 2. Chord Generator (CC 95-98)

### Architecture
- **Files**: `Services/chord/chord.h`, `chord.c` (to be created)
- **State**: Not yet implemented

### Features
- **CC 95**: Enable/Disable chord mode
- **CC 96**: Chord type (0-8)
  * 0 = Major (0, 4, 7)
  * 1 = Minor (0, 3, 7)
  * 2 = Dom7 (0, 4, 7, 10)
  * 3 = Maj7 (0, 4, 7, 11)
  * 4 = Min7 (0, 3, 7, 10)
  * 5 = Dim (0, 3, 6)
  * 6 = Aug (0, 4, 8)
  * 7 = Sus2 (0, 2, 7)
  * 8 = Sus4 (0, 5, 7)
- **CC 97**: Inversion (0-3)
- **CC 98**: Spread (0=tight, 1-3=octave spread)

### Implementation Steps

#### Step 1: Chord Interval Tables (TODO)
```c
static const uint8_t chord_intervals[][4] = {
  {0, 4, 7, 0},    // Major
  {0, 3, 7, 0},    // Minor
  {0, 4, 7, 10},   // Dom7
  // ... etc
};
```

#### Step 2: Chord Generation (TODO)
```c
void chord_generate(uint8_t root, uint8_t type, 
                    uint8_t inversion, uint8_t spread,
                    uint8_t *notes, uint8_t *count) {
  // Generate chord notes from root
  // Apply inversion
  // Apply spread across octaves
}
```

#### Step 3: Integration (TODO)
- Input: single note from MIDI IN
- Output: multiple notes forming chord
- Works with arpeggiator (can arp a chord)

### Testing
- Send CC 95 (127) to enable
- Play single note C4
- Should hear C-E-G (Major chord)
- Change CC 96 to hear different chord types

## 3. Note Delay/Echo (CC 100-103)

### Architecture
- **Files**: `Services/delay/delay.h`, `delay.c` (to be created)
- **State**: Not yet implemented

### Features
- **CC 100**: Enable/Disable delay
- **CC 101**: Delay time (0-127 → 0ms to 2000ms)
- **CC 102**: Feedback (0-127 → 0% to 95%)
- **CC 103**: Mix (0-127 → 0% dry to 100% wet)

### Implementation Steps

#### Step 1: Delay Buffer (TODO)
```c
#define DELAY_BUFFER_SIZE 128

typedef struct {
  uint8_t note;
  uint8_t velocity;
  uint32_t time_ms;
} delay_event_t;

static delay_event_t delay_buffer[DELAY_BUFFER_SIZE];
static uint8_t delay_write_idx = 0;
```

#### Step 2: Delay Processing (TODO)
```c
void delay_add_note(uint8_t note, uint8_t velocity, uint32_t delay_ms) {
  // Add note to delay buffer
  // Calculate playback time
}

void delay_tick_1ms(uint32_t now_ms, 
                    void (*callback)(uint8_t, uint8_t)) {
  // Check buffer for notes to play
  // Apply feedback (re-add with reduced velocity)
  // Call callback for output
}
```

#### Step 3: Integration (TODO)
- Dry signal passes through immediately
- Delayed copies added to buffer
- Feedback creates multiple echoes

### Testing
- Enable delay with CC 100 (127)
- Set delay time CC 101 (64) = ~1000ms
- Set feedback CC 102 (64) = ~50%
- Play note, hear echo 1 second later

## 4. MIDI Clock Sync (CC 110)

### Architecture
- **Files**: Integrate into existing `module_tests.c`
- **State**: Not yet implemented

### Features
- **CC 110**: Enable/Disable clock sync
- Auto-detect external MIDI clock (0xF8)
- Calculate BPM from clock timing
- Sync arpeggiator rate to clock
- Sync delay time to musical divisions

### Implementation Steps

#### Step 1: Clock Detection (TODO)
```c
static uint32_t last_clock_ms = 0;
static uint16_t clock_interval_ms = 0;
static uint16_t calculated_bpm = 120;

void process_midi_clock(uint32_t now_ms) {
  if (last_clock_ms > 0) {
    clock_interval_ms = now_ms - last_clock_ms;
    // 24 clocks per quarter note
    // BPM = 60000 / (interval_ms * 24)
    calculated_bpm = 2500 / clock_interval_ms;
  }
  last_clock_ms = now_ms;
}
```

#### Step 2: Tempo Sync (TODO)
```c
uint32_t get_note_duration_ms(uint8_t division) {
  // division: 1=whole, 4=quarter, 8=eighth, etc.
  // Return duration in ms based on calculated_bpm
  return (60000 * 4) / (calculated_bpm * division);
}
```

#### Step 3: Integration (TODO)
- Listen for 0xF8 (clock) messages
- Update arp rate based on synced tempo
- Update delay time to musical divisions

### Testing
- Start external sequencer with MIDI clock
- Enable sync with CC 110 (127)
- Arpeggiator should sync to tempo
- Change sequencer tempo, arp follows

## Integration Architecture

### Call Sequence
```
MIDI IN
  ↓
Channel Filter
  ↓
Chord Generator (optional)
  ↓
Arpeggiator (optional)
  ↓
LiveFX Transform
  ↓
Note Delay/Echo (optional)
  ↓
MIDI OUT
```

### Module Dependencies
```c
#if MODULE_ENABLE_ARPEGGIATOR
#include "Services/arpeggiator/arpeggiator.h"
#endif

#if MODULE_ENABLE_CHORD
#include "Services/chord/chord.h"
#endif

#if MODULE_ENABLE_DELAY
#include "Services/delay/delay.h"
#endif
```

### Configuration
```c
// In module_config.h:
#ifndef MODULE_ENABLE_ARPEGGIATOR
#define MODULE_ENABLE_ARPEGGIATOR 1
#endif

#ifndef MODULE_ENABLE_CHORD
#define MODULE_ENABLE_CHORD 1
#endif

#ifndef MODULE_ENABLE_DELAY
#define MODULE_ENABLE_DELAY 1
#endif
```

## Memory Budget

| Feature | Memory Usage | Notes |
|---------|--------------|-------|
| Arpeggiator | ~256 bytes | 16 notes × 16 bytes |
| Chord Generator | ~64 bytes | Minimal state |
| Note Delay | ~1024 bytes | 128 events × 8 bytes |
| MIDI Clock | ~16 bytes | Timing state |
| **Total** | **~1360 bytes** | Acceptable for STM32F4 |

## Performance Impact

| Feature | CPU Impact | Latency |
|---------|-----------|---------|
| Arpeggiator | ~2% | <1ms |
| Chord Generator | <1% | <100µs |
| Note Delay | ~1% | <500µs |
| MIDI Clock | <0.5% | <50µs |
| **Total** | **~4-5%** | **<2ms** |

## Testing Strategy

### Unit Tests
- Test each feature in isolation
- Verify pattern generation
- Check memory bounds
- Validate timing accuracy

### Integration Tests
- Enable all features together
- Test interaction between features
- Stress test with many notes
- Verify no buffer overflows

### User Acceptance Tests
- Musical functionality test
- Tempo sync accuracy
- Delay time accuracy
- Pattern variety

## Current Status Summary

### ✅ Completed
- Arpeggiator header and stub
- Architecture documentation
- Integration plan
- CC command allocation

### 🔄 In Progress
- Arpeggiator full implementation

### 📋 Planned
- Chord generator
- Note delay/echo
- MIDI clock sync
- Full integration
- Testing

## Next Steps

1. Complete arpeggiator note buffer management
2. Implement pattern generation algorithms
3. Add timing/gate control
4. Integrate with MODULE_TEST_MIDI_DIN
5. Test arpeggiator thoroughly
6. Move to chord generator implementation
7. Continue with delay and clock sync

## Estimated Timeline

- **Arpeggiator**: 2-3 hours
- **Chord Generator**: 1-2 hours
- **Note Delay**: 2-3 hours
- **MIDI Clock**: 1 hour
- **Integration & Testing**: 2 hours
- **Total**: ~10 hours for complete Phase B

---

*Document Version*: 1.0  
*Last Updated*: 2026-01-23  
*Status*: Phase B Framework Created, Implementation Started
