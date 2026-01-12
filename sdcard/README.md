# MidiCore SD Card Configuration Templates

## Overview

This folder contains standard configuration file templates for the MidiCore looper system. All settings are stored in a single `config.ngc` file using the MIDIbox NG format.

## Files

- **config.ngc** - Standard configuration for typical hardware setup
- **config_minimal.ngc** - Minimal configuration for breadboard testing
- **config_full.ngc** - Full configuration with all I/O and breath controller enabled

## Installation

1. Format SD card as FAT32
2. Copy your chosen template to SD card root directory as `0:/config.ngc`
3. Insert SD card into MidiCore device
4. Power on - configuration loads automatically

## Configuration Parameters

### DIN Module (Digital Inputs via SRIO)

- `SRIO_DIN_ENABLE` (0/1) - Enable DIN module
- `SRIO_DIN_BYTES` (1-16) - Number of SRIO shift register bytes (1 byte = 8 inputs)
- `DIN_INVERT_DEFAULT` (0/1) - Invert button logic (1 = active low)
- `DIN_DEBOUNCE_MS` (1-255) - Debounce time in milliseconds (default: 20)

### AINSER Module (Analog Inputs via SPI)

- `AINSER_ENABLE` (0/1) - Enable AINSER64 module
- `AINSER_SCAN_MS` (1-255) - Scan interval in milliseconds (default: 5)
- `AINSER_DEADBAND` (0-127) - Minimum value change to trigger update (default: 2)

**Note**: AINSER64 uses SPI, not I2C. Communicates via SPI3 with chip select pins.

### AIN Module (Built-in ADC Analog Inputs)

- `AIN_ENABLE` (0/1) - Enable AIN module
- `AIN_VELOCITY_ENABLE` (0/1) - Enable velocity sensing for analog inputs
- `AIN_CALIBRATE_AUTO` (0/1) - Enable automatic calibration
- `AIN_SCAN_MS` (1-255) - Scan interval in milliseconds (default: 10)
- `AIN_DEADBAND` (0-127) - Minimum value change to trigger update (default: 2)

### MIDI Settings

- `MIDI_DEFAULT_CHANNEL` (0-15) - Default MIDI channel (0 = channel 1)
- `MIDI_VELOCITY_CURVE` (0-3) - Velocity curve type:
  - 0 = Linear
  - 1 = Logarithmic
  - 2 = Exponential
  - 3 = S-curve

### Pressure Module (Breath Controller - XGZP6847D Sensor)

- `PRESSURE_ENABLE` (0/1) - Enable pressure sensor module
- `PRESSURE_I2C_BUS` (1/2) - I2C bus number
- `PRESSURE_ADDR` (0x00-0xFF) - I2C address (default: 0x58, try 0x6D if NACK)
- `PRESSURE_TYPE` (0-3) - Sensor type (2 = XGZP6847D)
- `PRESSURE_MAP_MODE` (0/1) - Mapping mode
- `PRESSURE_INTERVAL_MS` (1-255) - Sampling interval in milliseconds
- `PMIN_PA` (-100000 to 0) - Minimum pressure in Pascal (pull direction)
- `PMAX_PA` (0 to 100000) - Maximum pressure in Pascal (push direction)
- `ATM0_PA` (-100000 to 100000) - Atmospheric baseline in Pascal (set by calibration)

### Expression Module (Pressure to MIDI CC Mapping)

- `EXPRESSION_ENABLE` (0/1) - Enable expression module
- `EXPRESSION_MIDI_CH` (0-15) - MIDI channel for expression
- `BIDIR` (0/1) - Bidirectional mode:
  - 0 = Single CC for both directions
  - 1 = Separate CCs for push/pull
- `CC` (0-127) - MIDI CC number (unidirectional mode)
- `CC_PUSH` (0-127) - MIDI CC for push (bidirectional mode)
- `CC_PULL` (0-127) - MIDI CC for pull (bidirectional mode)
- `OUT_MIN` (0-127) - Minimum MIDI CC output value
- `OUT_MAX` (0-127) - Maximum MIDI CC output value
- `RATE_MS` (1-255) - Update rate in milliseconds
- `SMOOTH` (0-1000) - Smoothing factor (higher = smoother)
- `DEADBAND_CC` (0-127) - Anti-jitter deadband for CC changes
- `HYST_CC` (0-127) - Extra deadband when direction flips
- `CURVE` (0-2) - Response curve:
  - 0 = Linear
  - 1 = Exponential (use CURVE_PARAM for gamma)
  - 2 = S-curve (smoothstep)
- `CURVE_PARAM` (0-1000) - Curve parameter (gamma * 100 for exponential, e.g., 180 = 1.80)
- `ZERO_DEADBAND_PA` (0-10000) - Neutral zone around 0 Pa to avoid direction flip

### Calibration (One-Shot Auto-Calibration at Boot)

- `CALIBRATION_ENABLE` (0/1) - Enable auto-calibration at startup
- `ATM_MS` (100-10000) - Time to measure atmospheric baseline (hold bellows at rest)
- `EXT_MS` (1000-30000) - Time to measure min/max range (do full push/pull)
- `MARGIN_RAW` (0-255) - Safety margin for raw ADC values
- `CAL_KEEP_FILES` (0/1) - Keep original config files after calibration

## Using the Config Editor UI

Navigate to the **CONFIG** page using BTN5:

- **BTN1 (SAVE)** - Save current settings to `0:/config.ngc`
- **BTN2 (LOAD)** - Reload settings from SD card
- **BTN3 (EDIT)** - Toggle VIEW/EDIT mode
- **BTN4 (CAT)** - Cycle through parameter categories
- **Encoder** - Navigate parameters (VIEW) or adjust values (EDIT)

## SD Card Status

The header shows SD card status:
- **SD:OK** - Card available and writable
- **SD:N/A** - Card not detected or read-only

## Troubleshooting

### SD Card Not Detected
- Verify card is FAT32 formatted
- Check card is properly inserted
- Try reformatting card

### Configuration Not Loading
- Verify filename is exactly `0:/config.ngc`
- Check file for syntax errors (KEY = VALUE format)
- View Config Editor for error messages

### AINSER Not Working
- Verify AINSER_ENABLE = 1
- Check SPI3 connections (not I2C)
- Ensure chip select pins configured correctly

### Breath Controller Not Responding
1. Enable both modules: PRESSURE_ENABLE = 1, EXPRESSION_ENABLE = 1
2. Run calibration: CALIBRATION_ENABLE = 1, reboot, follow on-screen instructions
3. Check I2C address (try 0x6D if 0x58 fails)
4. Verify sensor power and I2C bus wiring

## File Format

The .NGC format uses simple KEY = VALUE pairs:

```
# Comment lines start with #
SRIO_DIN_ENABLE = 1
AINSER_SCAN_MS = 5
PMIN_PA = -40000
```

- Keys are case-sensitive
- Values are integers (hex supported with 0x prefix)
- Whitespace around `=` is ignored
- Comments and blank lines are ignored
- Invalid lines are skipped silently

## Advanced Configuration

For advanced users, the config.ngc file can be edited on a computer:
1. Remove SD card from device
2. Mount on computer
3. Edit 0:/config.ngc with text editor
4. Save changes
5. Unmount safely and reinsert into device

All changes take effect on next power cycle or after pressing BTN2 (LOAD) on Config page.

## Breath Controller (Soufflet) Professional Setup

For professional accordion/breath controller applications:

1. Use **config_full.ngc** as starting point
2. Enable pressure and expression modules
3. Run calibration at first boot (CALIBRATION_ENABLE = 1)
4. Choose appropriate curve (CURVE = 1 for natural feel)
5. Adjust SMOOTH for desired response speed
6. Set BIDIR = 1 for separate push/pull CCs if needed
7. Fine-tune ZERO_DEADBAND_PA to eliminate flutter at rest

The system supports professional-grade breath control with low latency (<5ms), high resolution (24-bit ADC), and flexible CC mapping with curves and bidirectional control.
