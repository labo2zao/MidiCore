# CLI Quick Start Guide

**Get started with MidiCore CLI in 5 minutes**

---

## What is the CLI?

The Command-Line Interface (CLI) lets you control all MidiCore modules via text commands over:
- USB CDC (Virtual COM port)
- Hardware UART
- MIOS Studio Terminal

---

## Quick Start

### 1. Connect to Terminal

**Option A: USB CDC (Recommended)**
```bash
# Linux/Mac
screen /dev/ttyACM0 115200

# Windows
# Use PuTTY, TeraTerm, or Arduino Serial Monitor
# Port: COMx, Baud: 115200
```

**Option B: MIOS Studio**
- Open MIOS Studio
- Select MIDI IN/OUT ports
- Open Terminal tab

### 2. Test Connection

```bash
# Type and press Enter:
help

# You should see:
# MidiCore CLI - Available Commands:
# help, version, uptime, status, reboot, clear, module, config, ...
```

### 3. Try Basic Commands

```bash
# Show firmware version
version

# Show system status
status

# List all modules
module list

# Get info about a module
module info looper
```

---

## Common Tasks

### Control the Looper

```bash
# Set tempo
module set looper bpm 120

# Start recording track 0
module set looper state REC 0

# Switch to playback
module set looper state PLAY 0

# Mute track 1
module set looper mute true 1

# Check status
module get looper state 0
```

### Setup MIDI Effects

```bash
# Enable quantizer on track 0
module enable quantizer 0

# Set 1/16 note quantization
module set quantizer resolution 1_16 0

# Set 75% quantization strength
module set quantizer strength 75 0

# Enable harmonizer
module enable harmonizer 0

# Add third above
module set harmonizer voice1_interval THIRD_UP 0
module set harmonizer voice1_enabled true 0
```

### Configure Accordion

```bash
# Set bellows curve
module set bellows_expression curve EXPONENTIAL

# Set pressure range (in Pascals)
module set bellows_expression min_pa 100
module set bellows_expression max_pa 2000

# Enable push/pull detection
module set bellows_expression bidirectional true

# Configure bass system
module set bass_chord_system layout STRADELLA_120 0
module set bass_chord_system octave_doubling true 0

# Set register
module set register_coupling register MUSETTE 0
```

### Save Your Configuration

```bash
# Save all settings to SD card
config save 0:/my_session.ini

# Load settings
config load 0:/my_session.ini

# Save specific module
config save 0:/looper_config.ini looper
```

---

## Command Structure

### Module Control

```bash
module <action> <module_name> [track]

# Actions:
enable      # Enable module/track
disable     # Disable module/track
status      # Check if enabled
info        # Show module information
params      # List all parameters
list        # List all modules
```

### Parameter Access

```bash
module get <module> <param> [track]
module set <module> <param> <value> [track]

# Examples:
module get looper bpm                    # Get global parameter
module set looper bpm 140                # Set global parameter
module get midi_filter enabled 0         # Get track 0 parameter
module set midi_filter enabled true 0    # Set track 0 parameter
```

### System Commands

```bash
help [command]      # Show help
version             # Show version
uptime              # Show uptime
status              # System status
reboot              # Reboot system
clear               # Clear screen
```

---

## Parameter Types

### Boolean

Accepts: `true`, `false`, `1`, `0`, `on`, `off`, `yes`, `no`

```bash
module set midi_filter enabled true 0
module set looper mute false 1
```

### Integer

Numeric value within range:

```bash
module set looper bpm 120           # Range: 20-300
module set midi_filter min_note 36 0  # Range: 0-127
```

### Enum

Named values:

```bash
module set looper state PLAY 0
module set arpeggiator pattern UP
module set chord type MINOR 0
```

---

## Keyboard Shortcuts

- **↑ (Up Arrow)**: Previous command in history
- **↓ (Down Arrow)**: Next command in history
- **Tab**: Auto-complete (future feature)
- **Ctrl+C**: Cancel current command
- **Ctrl+L**: Clear screen (same as `clear`)

---

## Tips & Tricks

### 1. Use Tab Completion (planned)

```bash
module set arp<TAB>        # Completes to "arpeggiator"
module set looper st<TAB>  # Shows: "state", "status"
```

### 2. Check Ranges

```bash
# Always check valid ranges with:
module params <module>

# Example:
module params looper
# Shows: bpm (int, 20-300): Tempo
```

### 3. Use Enum Names

```bash
# Don't use numbers for enums:
module set looper state 2 0      # ✗ Hard to remember

# Use names instead:
module set looper state PLAY 0   # ✓ Clear and readable
```

### 4. Save Often

```bash
# Save after configuring:
config save 0:/current.ini

# Name your presets:
config save 0:/jazz_ballad.ini
config save 0:/rock_loop.ini
config save 0:/accordion_musette.ini
```

### 5. Command Chaining

```bash
# Execute multiple commands (semicolon separated):
module enable quantizer 0; module set quantizer resolution 1_16 0; module status quantizer 0
```

---

## Example Workflows

### Setup a Looper Session

```bash
# 1. Configure transport
module set looper bpm 120
module set looper time_sig_num 4
module set looper time_sig_den 4

# 2. Configure track 0 (bass)
module set looper midi_channel 0 0
module set looper quantize 1_8 0

# 3. Configure track 1 (melody)
module set looper midi_channel 5 1
module set looper quantize 1_16 1

# 4. Start recording
module set looper state REC 0

# 5. Save session
config save 0:/my_session.ini
```

### Create an Effect Chain

```bash
# Track 0: Stabilizer → Quantizer → Harmonizer

# 1. Note stabilizer
module enable note_stabilizer 0
module set note_stabilizer min_duration_ms 50 0

# 2. Quantizer
module enable quantizer 0
module set quantizer resolution 1_16 0
module set quantizer strength 80 0

# 3. Harmonizer
module enable harmonizer 0
module set harmonizer voice1_interval THIRD_UP 0
module set harmonizer voice1_enabled true 0

# 4. Save chain
config save 0:/effect_chain.ini
```

### Configure Accordion

```bash
# 1. Bellows
module set bellows_expression curve EXPONENTIAL
module set bellows_expression min_pa 100
module set bellows_expression max_pa 2000
module set bellows_expression bidirectional true

# 2. Shake detection
module enable bellows_shake
module set bellows_shake sensitivity 60
module set bellows_shake depth 50

# 3. Bass system
module set bass_chord_system layout STRADELLA_120 0
module set bass_chord_system octave_doubling true 0

# 4. Register
module set register_coupling register MUSETTE 0

# 5. Save
config save 0:/accordion.ini
```

---

## Troubleshooting

### Command Not Found

```bash
# Error: Command 'xyz' not found
# Solution: List all commands
help
```

### Module Not Found

```bash
# Error: Module 'xyz' not found
# Solution: List all modules
module list
```

### Parameter Not Found

```bash
# Error: Parameter 'xyz' not found
# Solution: List module parameters
module params <module_name>
```

### Value Out of Range

```bash
# Error: Value out of range (20-300)
# Solution: Check valid range
module params <module_name>
```

### Invalid Track Index

```bash
# Error: Invalid track index (0-3)
# Solution: Use 0-3 for 4-track modules
```

---

## Getting Help

### Built-in Help

```bash
help                     # All commands
help module              # Module command help
module info <name>       # Module information
module params <name>     # Parameter list with ranges
```

### Documentation

- **Full Reference**: `Docs/CLI_COMMAND_REFERENCE.md`
- **Module Inventory**: `Docs/MODULE_INVENTORY.md`
- **Status Tracker**: `Docs/MODULE_CLI_IMPLEMENTATION_STATUS.md`

### Examples

Every module in `CLI_COMMAND_REFERENCE.md` includes:
- Command syntax
- Parameter descriptions
- Usage examples
- Valid ranges/values

---

## Next Steps

1. **Explore Modules**: `module list` and try different modules
2. **Read Full Reference**: See `CLI_COMMAND_REFERENCE.md` for all commands
3. **Create Presets**: Save your favorite configurations
4. **Script Workflows**: Create batch files for common tasks
5. **Share Configs**: Exchange INI files with other users

---

## Quick Command Reference

```bash
# System
help                    # Show all commands
version                 # Show version
status                  # System status

# Modules
module list             # List all modules
module info <name>      # Module information
module params <name>    # List parameters

# Control
module enable <name> [track]
module disable <name> [track]
module status <name> [track]

# Parameters
module get <name> <param> [track]
module set <name> <param> <value> [track]

# Configuration
config save <file> [module]
config load <file> [module]
config list
config get <key>
config set <key> <value>

# Looper
module set looper bpm <20-300>
module set looper state <STOP|REC|PLAY|OVERDUB> <track>
module set looper mute <true|false> <track>

# Effects
module enable <name> <track>
module set <name> <param> <value> <track>
```

---

**Ready to start! Type `help` in your terminal and explore!**

For complete documentation, see `Docs/CLI_COMMAND_REFERENCE.md`
