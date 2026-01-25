# Looper v1

## Features
- 4 tracks
- record / play / overdub
- quantize on record: OFF / straight notes / triplets / quintuplets / sextuplets / septuplets / dotted notes
  - Straight: 1/4, 1/8, 1/16
  - Triplets: 1/8T, 1/16T, 1/32T, 1/2T (3 notes per beat division) - jazz ballads
  - Quintuplets: 1/8Q, 1/16Q, 1/32Q (5 notes per beat division)
  - Sextuplets: 1/8S, 1/16S (6 notes per beat division) - jazz phrases
  - Septuplets: 1/8x7, 1/16x7 (7 notes per beat division) - jazz runs
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
  - Triplets: `LOOPER_QUANT_1_8T`, `LOOPER_QUANT_1_16T`, `LOOPER_QUANT_1_32T`, `LOOPER_QUANT_1_2T` (jazz)
  - Quintuplets: `LOOPER_QUANT_1_8Q`, `LOOPER_QUANT_1_16Q`, `LOOPER_QUANT_1_32Q`
  - Sextuplets: `LOOPER_QUANT_1_8S`, `LOOPER_QUANT_1_16S` (jazz)
  - Septuplets: `LOOPER_QUANT_1_8SEPT`, `LOOPER_QUANT_1_16SEPT` (jazz)
  - Dotted: `LOOPER_QUANT_1_4_DOT`, `LOOPER_QUANT_1_8_DOT`, `LOOPER_QUANT_1_16_DOT`
- `looper_save_track(track, "0:/loops/t0.loop")`
- `looper_load_track(track, "0:/loops/t0.loop")`
