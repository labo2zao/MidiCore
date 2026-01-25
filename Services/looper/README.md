# Looper v1

## Features
- 4 tracks
- record / play / overdub
- quantize on record: OFF / straight notes / triplets / quintuplets / dotted notes
  - Straight: 1/4, 1/8, 1/16
  - Triplets: 1/8T, 1/16T, 1/32T (3 notes per beat division)
  - Quintuplets: 1/8Q, 1/16Q, 1/32Q (5 notes per beat division)
  - Dotted: 1/4., 1/8., 1/16. (uneven rhythms, 1.5x base note value)
- auto-loop when loop length is predefined (beats) and transport.auto_loop=1
- safety: note-off flush on loop wrap
- per-track mute
- save/load track to SD via FATFS (binary)

## Defaults
- loop_beats = 4 (one bar in 4/4)
- auto_loop = 1
- quantize = OFF

## API
- `looper_set_state(track, LOOPER_STATE_REC/STOP/PLAY/OVERDUB)`
- `looper_set_loop_beats(track, beats)`
- `looper_set_quant(track, LOOPER_QUANT_...)`
  - Basic: `LOOPER_QUANT_OFF`, `LOOPER_QUANT_1_4`, `LOOPER_QUANT_1_8`, `LOOPER_QUANT_1_16`
  - Triplets: `LOOPER_QUANT_1_8T`, `LOOPER_QUANT_1_16T`, `LOOPER_QUANT_1_32T`
  - Quintuplets: `LOOPER_QUANT_1_8Q`, `LOOPER_QUANT_1_16Q`, `LOOPER_QUANT_1_32Q`
  - Dotted: `LOOPER_QUANT_1_4_DOT`, `LOOPER_QUANT_1_8_DOT`, `LOOPER_QUANT_1_16_DOT`
- `looper_save_track(track, "0:/loops/t0.loop")`
- `looper_load_track(track, "0:/loops/t0.loop")`
