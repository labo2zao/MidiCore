# Program Change Manager Module

Program Change/Bank Select Manager for MidiCore MIDI FX suite.

## Overview

The program_change_mgr module manages program change and bank select messages with preset storage. It stores complete program + bank configurations in 128 slots and recalls them by slot number or name, sending proper CC 0 (Bank MSB), CC 32 (Bank LSB), and Program Change message sequences.

## Features

- **128 preset slots** for storing program configurations
- **Store program + bank MSB/LSB + channel** in each preset
- **Preset names** (32 char max) for easy identification
- **Recall by slot number or name**
- **Send full bank select + program change sequences**
- **Proper CC 0, CC 32, PC ordering** for device compatibility
- **Callbacks for CC and PC output**
- **Copy, rename, clear functions** for preset management

## Usage Example

```c
#include "Services/program_change_mgr/program_change_mgr.h"

// Callbacks for output
void my_cc_callback(uint8_t cc_number, uint8_t cc_value, uint8_t channel) {
    MIDI_SendCC(channel, cc_number, cc_value);
}

void my_pc_callback(uint8_t program, uint8_t channel) {
    MIDI_SendProgramChange(channel, program);
}

// Initialize
program_change_mgr_init();
program_change_mgr_set_cc_callback(my_cc_callback);
program_change_mgr_set_pc_callback(my_pc_callback);

// Store presets
program_change_mgr_store(0, 5, 0, 0, 0, "Piano 1");
program_change_mgr_store(1, 24, 0, 1, 0, "Nylon Guitar");
program_change_mgr_store(2, 48, 1, 0, 0, "String Ensemble");

// Recall by slot number
program_change_mgr_recall(0);  // Sends: CC 0=0, CC 32=0, PC=5

// Recall by name
program_change_mgr_recall_by_name("Nylon Guitar");

// Send manually (without storing)
program_change_mgr_send(10, 2, 5, 0);  // Program 10, Bank 2:5, Ch 1

// Send program only
program_change_mgr_send_program(20, 0);

// Send bank only
program_change_mgr_send_bank(1, 0, 0);

// Get preset info
program_preset_t preset;
if (program_change_mgr_get_preset(0, &preset)) {
    printf("Preset 0: %s - Prog=%d, Bank=%d:%d, Ch=%d\n",
           preset.name, preset.program, preset.bank_msb, 
           preset.bank_lsb, preset.channel);
}

// Find by name
int16_t slot = program_change_mgr_find_by_name("Piano 1");
if (slot >= 0) {
    printf("Found at slot %d\n", slot);
}

// Copy preset
program_change_mgr_copy_preset(0, 10);  // Copy slot 0 to slot 10

// Rename preset
program_change_mgr_rename_preset(0, "Grand Piano");

// Get list of valid presets
uint8_t slots[128];
uint8_t count = program_change_mgr_get_valid_slots(slots);
printf("%d presets stored\n", count);

// Clear slot
program_change_mgr_clear_slot(5);
```

## API Reference

### Initialization

- `program_change_mgr_init()` - Initialize module
- `program_change_mgr_set_cc_callback()` - Set CC output callback
- `program_change_mgr_set_pc_callback()` - Set PC output callback

### Preset Management

- `program_change_mgr_store()` - Store preset in slot
- `program_change_mgr_recall()` - Recall preset by slot number
- `program_change_mgr_recall_by_name()` - Recall preset by name
- `program_change_mgr_get_preset()` - Get preset data from slot
- `program_change_mgr_clear_slot()` - Clear single slot
- `program_change_mgr_clear_all()` - Clear all slots

### Sending

- `program_change_mgr_send()` - Send bank select + program change
- `program_change_mgr_send_program()` - Send program change only
- `program_change_mgr_send_bank()` - Send bank select only

### Utilities

- `program_change_mgr_is_slot_valid()` - Check if slot has data
- `program_change_mgr_find_by_name()` - Find slot by preset name
- `program_change_mgr_get_preset_count()` - Get number of valid presets
- `program_change_mgr_get_valid_slots()` - Get list of valid slot numbers
- `program_change_mgr_copy_preset()` - Copy preset to another slot
- `program_change_mgr_rename_preset()` - Rename a preset

## Bank Select Sequence

When recalling or sending, the module sends messages in this order:

1. **CC 0** (Bank Select MSB) - High byte of bank number
2. **CC 32** (Bank Select LSB) - Low byte of bank number
3. **Program Change** - Program number

This order ensures compatibility with most MIDI devices.

## Bank Number Format

Bank numbers are split into MSB and LSB:
- **Bank MSB** (CC 0): High 7 bits (0-127)
- **Bank LSB** (CC 32): Low 7 bits (0-127)
- Total banks: Up to 16,384 (128 × 128)

Most devices use:
- Bank MSB only (128 banks)
- Bank MSB + LSB (16,384 banks)

## Preset Structure

Each preset contains:
- **Program number** (0-127)
- **Bank MSB** (0-127)
- **Bank LSB** (0-127)
- **MIDI channel** (0-15, where 0 = channel 1)
- **Name** (up to 31 characters + null terminator)
- **Valid flag** (internal)

## Integration

1. Include header in your project
2. Call `program_change_mgr_init()` at startup
3. Set callbacks for CC and PC output
4. Store presets with program and bank information
5. Recall presets by slot or name
6. Optionally manage presets (copy, rename, clear)

## Common Applications

- **Patch librarian**: Store and recall synth patches
- **Live performance**: Quick patch changes by name
- **Multi-timbral setup**: Different channels per slot
- **Sound browser**: Navigate large sound banks
- **Patch organization**: Name and categorize sounds
- **Backup/restore**: Copy presets between slots

## General MIDI Bank Numbers

Some common GM bank arrangements:
- **Bank 0:0** - GM1 sound set
- **Bank 0:1** - Variation bank
- **Bank 127:0** - Drum kits
- **Bank 120:0** - Sound effects

## Notes

- Slot 0-127 can each store one preset
- Empty slots return 0 when recalled
- Name search is case-sensitive
- Proper message ordering ensures device compatibility
- Channel numbers are 0-indexed (0 = MIDI channel 1)
- Some devices ignore bank LSB (use 0)

## License

Part of MidiCore project
