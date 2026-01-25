# Swing/Groove Quick Reference

## Quick Start

```c
#include "Services/swing/swing.h"

// Initialize
swing_init(120);  // 120 BPM

// Configure track 0
swing_set_enabled(0, 1);
swing_set_groove(0, SWING_GROOVE_SWING);
swing_set_amount(0, 66);  // 66% swing
swing_set_resolution(0, SWING_RESOLUTION_16TH);

// Use in your MIDI processing
int16_t offset = swing_calculate_offset(0, tick_position, ppqn);
// Apply offset: play note at (scheduled_time + offset)
```

## Groove Types

| Type | Feel | Amount |
|------|------|--------|
| `SWING_GROOVE_STRAIGHT` | No swing | 50% |
| `SWING_GROOVE_SWING` | Classic swing | 66% |
| `SWING_GROOVE_SHUFFLE` | Heavy shuffle | 75% |
| `SWING_GROOVE_TRIPLET` | Triplet feel | 67% |
| `SWING_GROOVE_DOTTED` | Dotted 8th | 62% |
| `SWING_GROOVE_HALF_TIME` | Half-time shuffle | varies |
| `SWING_GROOVE_CUSTOM` | User-defined | custom |

## Resolution Types

| Type | Description | Notes Per Beat |
|------|-------------|----------------|
| `SWING_RESOLUTION_8TH` | 8th notes | 2 |
| `SWING_RESOLUTION_16TH` | 16th notes | 4 |
| `SWING_RESOLUTION_32ND` | 32nd notes | 8 |

## Swing Amount Scale

```
0%  ────────────── Very early swing (reverse)
25% ────────────── Early swing
50% ────────────── No swing (straight)
60% ────────────── Light swing
66% ────────────── Classic swing ⭐
75% ────────────── Heavy shuffle
100% ───────────── Maximum late swing
```

## Common Patterns

### Jazz Swing
```c
swing_set_groove(track, SWING_GROOVE_SWING);
swing_set_amount(track, 66);
swing_set_resolution(track, SWING_RESOLUTION_8TH);
```

### Hip-Hop
```c
swing_set_groove(track, SWING_GROOVE_SHUFFLE);
swing_set_amount(track, 70);
swing_set_resolution(track, SWING_RESOLUTION_16TH);
```

### Subtle Humanization
```c
swing_set_groove(track, SWING_GROOVE_SWING);
swing_set_amount(track, 52);  // Very subtle
swing_set_depth(track, 70);   // 70% of beats
swing_set_resolution(track, SWING_RESOLUTION_16TH);
```

### Triplet Feel
```c
swing_set_groove(track, SWING_GROOVE_TRIPLET);
swing_set_amount(track, 67);
swing_set_resolution(track, SWING_RESOLUTION_8TH);
```

## Key Functions

| Function | Purpose |
|----------|---------|
| `swing_init(tempo)` | Initialize module |
| `swing_set_enabled(track, enabled)` | Enable/disable |
| `swing_set_amount(track, amount)` | Set swing 0-100% |
| `swing_set_groove(track, groove)` | Select groove type |
| `swing_set_resolution(track, res)` | Set note resolution |
| `swing_calculate_offset(track, tick, ppqn)` | Get timing offset (ticks) |
| `swing_calculate_offset_ms(track, time)` | Get timing offset (ms) |
| `swing_set_custom_pattern(track, pattern, len)` | Define custom groove |

## Offset Interpretation

```c
int16_t offset = swing_calculate_offset(track, position, ppqn);

if (offset > 0) {
    // Positive: delay note by offset milliseconds
    schedule_note(time + offset);
}
else if (offset < 0) {
    // Negative: play note earlier
    schedule_note(time + offset);  // offset is negative
}
else {
    // Zero: no timing change
    schedule_note(time);
}
```

## Memory Usage

- Per track: ~100 bytes
- Total (4 tracks): ~400 bytes
- No dynamic allocation
- Static only

## Typical Offset Ranges

At 120 BPM with 66% swing:

| Resolution | Max Offset |
|------------|------------|
| 8th notes | ±26 ms |
| 16th notes | ±13 ms |
| 32nd notes | ±6 ms |

Offsets scale with tempo (faster = smaller offsets).

## Tips

1. **Start subtle** - Try 52-55% for light feel
2. **Match genre** - Jazz = 8ths, Hip-Hop = 16ths
3. **Use depth** - Reduce depth for partial swing
4. **Tempo matters** - Update with `swing_set_tempo()`
5. **Per-track** - Different grooves per instrument

## Troubleshooting

| Issue | Solution |
|-------|----------|
| No effect | Check `swing_is_enabled()` returns 1 |
| Too strong | Reduce amount closer to 50% |
| Wrong feel | Try different resolution (8th vs 16th) |
| Weird timing | Verify tempo is set correctly |
| Inconsistent | Check PPQN matches your sequencer |

## Architecture

```
MIDI Input
    ↓
[Your Note Processing]
    ↓
swing_calculate_offset()  ← Returns timing offset
    ↓
Apply offset to note timing
    ↓
MIDI Output (with groove!)
```

## Files

- `swing.h` - API header
- `swing.c` - Implementation
- `README.md` - Full documentation
- `swing_example.c` - Usage examples
- `IMPLEMENTATION_SUMMARY.md` - Implementation details
- `QUICK_REFERENCE.md` - This file
