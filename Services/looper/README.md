# Looper v1

## Features
- 4 tracks
- record / play / overdub
- quantize on record: OFF / 1-16 / 1-8 / 1-4
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
- `looper_set_quant(track, LOOPER_QUANT_1_16 ...)`
- `looper_save_track(track, "0:/loops/t0.loop")`
- `looper_load_track(track, "0:/loops/t0.loop")`
