# MidiCore CLI - Quick Reference Card

## Connection
- **Baud Rate**: 115200
- **Ports**: UART5 or USB CDC
- **Settings**: 8N1, No flow control

---

## Essential Commands

### Getting Started
```bash
help                          # Show all commands
module list                   # List all modules
test list                     # List all tests
system status                 # System info
```

### Module Control
```bash
module info <name>            # Module details
module enable <name>          # Enable module
module disable <name>         # Disable module
module status <name>          # Module status
```

### Get/Set Parameters
```bash
module get <mod> <param> [track]        # Get parameter value
module set <mod> <param> <val> [track]  # Set parameter value
```

### Running Tests
```bash
test list                     # Show available tests
test run <name>               # Run a test
test stop                     # Stop current test
test status                   # Test status
```

---

## Common Test Examples

### Test Analog Inputs (AINSER64)
```bash
test run ainser64
# Shows real-time values from all 64 ADC channels
# Press buttons/move sliders to see changes
# Reset device to stop
```

### Test Digital I/O (SRIO)
```bash
test run srio
# Tests shift register chains (DIN/DOUT)
# Verifies all digital inputs and outputs
```

### Test MIDI Router
```bash
test run router
# Tests MIDI routing matrix
# Verifies all 16 nodes
```

### Test USB MIDI
```bash
test run usb_midi
# Tests USB MIDI transmission/reception
# Send MIDI to see messages echoed
```

---

## Module Configuration Examples

### Configure Looper
```bash
module info looper                    # See parameters
module set looper bpm 140            # Set tempo
module set looper quantize true      # Enable quantize
module get looper bpm                # Verify BPM
```

### Configure Chord Module
```bash
module enable chord                   # Enable chord
module set chord enabled true 0      # Enable for track 0
module set chord inversion 1 0       # Set inversion
module get chord enabled 0           # Check status
```

### Configure AINSER Threshold
```bash
module set ainser threshold 10 0     # Channel 0, threshold 10
module set ainser threshold 15 1     # Channel 1, threshold 15
module get ainser threshold 0        # Verify channel 0
```

---

## System Commands

### Configuration
```bash
config save                   # Save to SD card
config load                   # Load from SD card
config reset                  # Reset to defaults
```

### Logging
```bash
log show                      # View system log
log clear                     # Clear log buffer
log level 3                   # Set log level (0-4)
```

### System Info
```bash
system status                 # RAM, CPU, uptime
system reset                  # Software reset
```

---

## Tips

- **Command History**: Use Up/Down arrows
- **Stop Test**: Reset device (most tests loop infinitely)
- **Tab Completion**: Planned feature
- **Case Sensitive**: Command names are case-sensitive
- **Spaces**: Use spaces between arguments

---

## Module Categories

### Hardware I/O
`ain`, `ainser_map`, `din_map`, `dout_map`, `srio`, `footswitch`

### MIDI Processing  
`router`, `midi`, `midi_filter`, `midi_delay`, `midi_converter`

### Effects
`arpeggiator`, `chord`, `harmonizer`, `quantizer`, `humanize`, `legato`, `lfo`, `strum`

### Accordion
`bellows_pressure`, `bellows_expression`, `bellows_shake`, `register_coupling`, `bass_chord`

### Sequencer
`looper`, `metronome`, `rhythm_trainer`

### System
`system`, `config`, `usb_midi`, `usb_cdc`, `log`, `test`

---

## Parameter Types

- **bool**: `true` or `false`
- **int**: Integer number
- **enum**: Named value (see module info)
- **string**: Text string

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| No response | Check baud rate (115200) |
| Test won't stop | Reset device |
| Module not found | Use `module list` for exact names |
| Invalid parameter | Use `module info <name>` to see parameters |
| Command not found | Run `help` to see available commands |

---

## Quick Workflow

### 1. Initial Setup
```bash
help                    # Learn available commands
module list            # See all modules
system status          # Check system health
```

### 2. Test Hardware
```bash
test list              # See available tests
test run ainser64      # Test analog inputs
test run srio          # Test digital I/O
```

### 3. Configure Modules
```bash
module info looper     # Check parameters
module set looper bpm 140
module enable chord
config save            # Save changes
```

### 4. Monitor System
```bash
system status          # Check resources
log show               # View log
module status looper   # Check module state
```

---

**For detailed documentation, see `CLI_USER_GUIDE.md`**
