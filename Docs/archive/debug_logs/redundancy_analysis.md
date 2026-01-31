# Code Redundancy Analysis - MidiCore

## Summary

- Total functions analyzed: 1014
- Files analyzed: 147
- Exact duplicates found: 7
- Similar function groups: 11

## Exact Duplicates

### am_keyeq (9 lines, 3 copies)

- Services/ainser/ainser_map.c
- Services/midi/midi_router.c
- Services/din/din_map.c

### ieq (7 lines, 3 copies)

- Services/patch/patch_router.c
- Services/zones/zones_cfg.c
- Services/instrument/instrument_cfg.c

### keyeq (7 lines, 2 copies)

- Services/pressure/pressure_i2c.c
- Services/expression/expression_cfg.c

### parse_u32 (7 lines, 2 copies)

- Services/ui/ui_encoders.c
- Services/ui/ui_bindings.c

### wrap_tick_i32 (7 lines, 2 copies)

- Services/ui/ui_page_looper_pianoroll.c
- Services/ui/ui_page_looper_timeline.c

### trim (6 lines, 2 copies)

- Services/zones/zones_cfg.c
- Services/instrument/instrument_cfg.c

### gpio_pin_index (7 lines, 2 copies)

- App/tests/module_tests.c
- App/tests/test_debug.c

## Similar Functions

### Pattern with 3 occurrences (30 total lines)

- cli_error in Services/cli/cli.c (10 lines)
- cli_success in Services/cli/cli.c (10 lines)
- cli_warning in Services/cli/cli.c (10 lines)

### Pattern with 4 occurrences (28 total lines)

- test_channel_filter in App/tests/test_midi_din_livefx_automated.c (7 lines)
- test_velocity_curves in App/tests/test_midi_din_livefx_automated.c (7 lines)
- test_note_range_limiting in App/tests/test_midi_din_livefx_automated.c (7 lines)
- test_statistics_tracking in App/tests/test_midi_din_livefx_automated.c (7 lines)

### Pattern with 3 occurrences (27 total lines)

- am_keyeq in Services/ainser/ainser_map.c (9 lines)
- mr_keyeq in Services/midi/midi_router.c (9 lines)
- dm_keyeq in Services/din/din_map.c (9 lines)

### Pattern with 3 occurrences (21 total lines)

- ieq in Services/patch/patch_router.c (7 lines)
- keyeq in Services/zones/zones_cfg.c (7 lines)
- keyeq in Services/instrument/instrument_cfg.c (7 lines)

### Pattern with 2 occurrences (14 total lines)

- keyeq in Services/pressure/pressure_i2c.c (7 lines)
- keyeq in Services/expression/expression_cfg.c (7 lines)

### Pattern with 2 occurrences (14 total lines)

- parse_u32 in Services/ui/ui_encoders.c (7 lines)
- parse_u32 in Services/ui/ui_bindings.c (7 lines)

### Pattern with 2 occurrences (14 total lines)

- wrap_tick_i32 in Services/ui/ui_page_looper_pianoroll.c (7 lines)
- wrap_tick_i32 in Services/ui/ui_page_looper_timeline.c (7 lines)

### Pattern with 2 occurrences (14 total lines)

- note_to_y in Services/ui/ui_page_looper_pianoroll.c (7 lines)
- note_to_y in Services/ui/ui_page_looper_timeline.c (7 lines)

### Pattern with 2 occurrences (14 total lines)

- gpio_pin_index in App/tests/module_tests.c (7 lines)
- gpio_pin_index in App/tests/test_debug.c (7 lines)

### Pattern with 2 occurrences (12 total lines)

- trim in Services/zones/zones_cfg.c (6 lines)
- trim in Services/instrument/instrument_cfg.c (6 lines)

### Pattern with 2 occurrences (12 total lines)

- looper_automation_start_record in Services/looper/looper.c (6 lines)
- looper_automation_stop_record in Services/looper/looper.c (6 lines)

## Common Patterns

| Pattern | Count | Files |
|---------|-------|-------|
| memset_zero | 71 | 57 |
| snprintf | 57 | 27 |
| strncpy | 46 | 25 |
| memcpy | 36 | 25 |
| strlen | 24 | 18 |
| strcpy | 2 | 2 |
