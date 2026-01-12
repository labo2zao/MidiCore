# SD Card Configuration Files

This directory contains standard configuration files for the MidiCore looper firmware. These files should be copied to the root directory of your SD card (formatted as FAT32).

## Files Overview

### `config.ngc`
Main configuration file for hardware modules (DIN, AINSER, AIN). This file uses the MIDIbox NG .NGC format for compatibility.

**Location on SD card**: `0:/config.ngc`

### `livefx_defaults.ngc`
Default LiveFX settings for each track (transpose, velocity scaling, force-to-scale).

**Location on SD card**: `0:/livefx_defaults.ngc` (future enhancement)

### `rhythm_trainer.ngc`
Rhythm trainer configuration (difficulty presets, thresholds, feedback mode).

**Location on SD card**: `0:/rhythm_trainer.ngc` (future enhancement)

## File Format

All configuration files use the MIDIbox NG .NGC format:
- Lines starting with `#` are comments
- Configuration format: `KEY = VALUE`
- Whitespace around `=` is ignored
- Values can be decimal (123) or hexadecimal (0x7B)

## Usage

1. Format an SD card as FAT32
2. Copy `config.ngc` (and any other desired files) to the SD card root
3. Insert SD card into MidiCore hardware
4. Configuration is loaded automatically at startup
5. Use the Config Editor UI page to modify settings
6. Press BTN1 (SAVE) to save changes back to SD card
7. Press BTN2 (LOAD) to reload from SD card

## Configuration Editor UI

Navigate to the Config page (CONF) using BTN5:
- **BTN1 (SAVE)**: Save current configuration to SD card
- **BTN2 (LOAD)**: Reload configuration from SD card
- **BTN3 (EDIT)**: Toggle VIEW/EDIT mode
- **BTN4 (CAT)**: Cycle through configuration categories
- **Encoder**: Navigate parameters (VIEW) or edit values (EDIT)

## Default Behavior

If `config.ngc` is not found on the SD card, MidiCore will use built-in defaults and display "Using defaults (SD load failed)" on startup.

## Troubleshooting

**"SD:N/A" displayed on Config page**:
- Check that SD card is properly inserted
- Verify SD card is formatted as FAT32
- Try a different SD card

**"Save failed" error**:
- SD card may be write-protected
- SD card may be full
- SD card may be failing (after 3 consecutive write errors, card is set to read-only mode)

**Configuration not loading**:
- Verify file is named exactly `config.ngc` (lowercase)
- Check file is in root directory, not a subdirectory
- Verify file format matches examples below

## Examples

See individual `.ngc` files in this directory for configuration examples.
