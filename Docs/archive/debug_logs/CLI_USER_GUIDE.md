# MidiCore CLI User Guide

## Overview

The MidiCore CLI (Command Line Interface) provides a powerful terminal-based interface for controlling, configuring, and testing all firmware modules at runtime. You can access it via UART or USB CDC (virtual COM port).

---

## Getting Started

### 1. Connect to the CLI

**Hardware Connection:**
- **UART5** pins (see schematic)
- **USB CDC** (appears as virtual COM port on your computer)

**Terminal Settings:**
- Baud Rate: **115200**
- Data: 8 bits
- Parity: None
- Stop: 1 bit
- Flow Control: None

**Recommended Terminals:**
- **Windows**: PuTTY, TeraTerm, CoolTerm
- **Linux/Mac**: screen, minicom, picocom
- **Cross-platform**: Serial Monitor in VS Code

**Example connection (Linux/Mac):**
```bash
# Find device
ls /dev/tty*

# Connect with screen
screen /dev/ttyUSB0 115200

# Or with minicom
minicom -D /dev/ttyUSB0 -b 115200
```

**Example connection (Windows):**
```
Open PuTTY:
1. Select "Serial"
2. Serial line: COM3 (check Device Manager)
3. Speed: 115200
4. Click "Open"
```

### 2. First Commands

Once connected, try these commands:

```
help                  # Show all available commands
module list          # List all modules
test list            # List all available tests
system status        # Show system information
```

---

## Basic CLI Usage

### Command Syntax

```
command [subcommand] [arguments]
```

**Examples:**
```
help                          # No arguments
module list                   # One subcommand
module info looper           # Subcommand with argument
module set looper bpm 120    # Multiple arguments
```

### Special Keys

- **Backspace**: Delete character
- **Enter**: Execute command
- **Ctrl+C**: Cancel current line (not stop test)
- **Up/Down Arrow**: Command history
- **Tab**: Auto-completion (planned)

---

## Module Commands

The `module` command provides comprehensive control over all firmware modules.

### List All Modules

```
module list
```

**Example Output:**
```
=== Registered Modules ===

Total Modules: 57

System Modules (12):
  1. system         - System management
  2. config         - Configuration manager
  3. midi           - MIDI router
  ...

Hardware I/O (8):
  13. ain           - Analog inputs
  14. ainser_map    - AINSER64 mapping
  ...

Effects (18):
  21. arpeggiator   - MIDI arpeggiator
  22. chord         - Chord trigger
  ...
```

### Get Module Information

```
module info <module_name>
```

**Example:**
```
module info looper

Output:
=== Module: looper ===
Description: LoopA-style MIDI looper/sequencer
Category: Sequencer
Status: ENABLED
Has per-track state: Yes
Tracks: 8

Parameters:
  bpm (int): 120 [60-240]
    Beats per minute
  quantize (bool): true
    Enable quantization
  ...
```

### Enable/Disable Module

```
module enable <module_name>
module disable <module_name>
```

**Examples:**
```
module enable looper       # Enable looper
module disable arpeggiator # Disable arpeggiator
```

### Get Parameter Value

```
module get <module_name> <parameter_name> [track]
```

**Examples:**
```
module get looper bpm          # Get global BPM
module get chord enabled 0     # Get chord enabled for track 0
module get ainser threshold 5  # Get threshold for AINSER channel 5
```

### Set Parameter Value

```
module set <module_name> <parameter_name> <value> [track]
```

**Examples:**
```
module set looper bpm 140           # Set BPM to 140
module set chord enabled true 0     # Enable chord on track 0
module set ainser threshold 10 5    # Set threshold to 10 for channel 5
```

### Get Module Status

```
module status <module_name>
```

**Example:**
```
module status looper

Output:
Status: ENABLED
Running: Yes
Current track: 3
Position: 1.2.0 (bar.beat.tick)
```

---

## Test System

The test system allows you to run hardware and software tests interactively.

**Note**: Tests are only available when `MODULE_ENABLE_TEST=1` is set in `Config/module_config.h`.

### List Available Tests

```
test list
```

**Example Output:**
```
=== Available Tests ===

Count: 15 tests

  1. ainser64
     Test AINSER64 64-channel ADC scanning

  2. srio
     Test SRIO (DIN/DOUT) shift register I/O

  3. router
     Test MIDI router matrix

  4. looper
     Test looper recording and playback

  5. usb_midi
     Test USB MIDI transmission and reception

  ...

Usage: test run <name>
Example: test run ainser64
```

### Run a Test

```
test run <test_name>
```

**Examples:**

#### Run AINSER64 Test
```
test run ainser64

Output:
=== Starting Test ===
Test: ainser64
Duration: Infinite (until reset)

Note: Most tests run in infinite loops.
Reset the device to stop the test.
======================

[TEST] AINSER64 Test Started
[TEST] Scanning 64 channels...
[TEST] Channel 0: 2047 (0.50V)
[TEST] Channel 1: 4095 (1.00V)
...
```

This test continuously scans all 64 analog inputs and displays their values. Useful for:
- Verifying Hall sensor connections
- Calibrating input ranges
- Checking for noise/drift

#### Run Router Test
```
test run router

Output:
[TEST] MIDI Router Test Started
[TEST] Testing all 16 nodes...
[TEST] Node 0 -> 1: PASS
[TEST] Node 1 -> 2: PASS
...
```

Tests the MIDI routing matrix to verify all connections work correctly.

#### Run SRIO Test
```
test run srio

Output:
[TEST] SRIO Test Started
[TEST] Testing DIN inputs (74HC165)...
[TEST] Testing DOUT outputs (74HC595)...
[TEST] DIN[0]: 1 1 0 0 1 1 0 0
[TEST] DOUT[0]: Writing pattern...
```

Tests the shift register I/O chains for digital inputs and outputs.

### Stop a Test

Most tests run in infinite loops. To stop a test:

```
test stop
```

**Or simply reset the device** (most common method).

### Check Test Status

```
test status
```

**Example Output:**
```
=== Test Status ===

Test: ainser64
Status: RUNNING
Started: 00:01:23.456
Duration: 45.2 seconds
Iterations: 452
Assertions: 28800 total, 28800 passed, 0 failed
```

---

## Real-World Usage Examples

### Example 1: Configure Looper BPM

```
# Check current BPM
module get looper bpm
> BPM: 120

# Change to 140 BPM
module set looper bpm 140
> OK

# Verify change
module get looper bpm
> BPM: 140
```

### Example 2: Test AINSER64 Connections

```
# List available tests
test list

# Run AINSER64 test
test run ainser64

# Observe real-time values
# Press buttons, move sliders to see input changes
# Values should be stable and responsive

# Reset device to stop test (or use test stop if implemented)
```

### Example 3: Configure Chord Module

```
# Check if chord module is enabled
module status chord
> Status: DISABLED

# Enable chord module
module enable chord
> OK

# Configure chord for track 0
module set chord enabled true 0
module set chord inversion 1 0

# Verify settings
module get chord enabled 0
> Enabled: true
module get chord inversion 0
> Inversion: 1
```

### Example 4: Monitor System Health

```
# Check system status
system status

Output:
=== System Status ===
Uptime: 01:23:45
Free RAM: 42KB / 128KB
CPU Load: 45%
Tasks: 8 running

# Check specific module
module status midi
module status looper
```

### Example 5: Configure MIDI Routing

```
# Show current routing
module info router

# Configure routing
# (Specific commands depend on router implementation)
router show              # Show routing matrix
router connect 0 1       # Connect node 0 to node 1
router disconnect 0 1    # Disconnect route
```

---

## Advanced Features

### Command History

The CLI maintains a command history. Use **Up/Down arrows** to recall previous commands:

```
(Press Up Arrow)
> module get looper bpm    # Previous command recalled

(Press Up Arrow again)
> test run ainser64        # Earlier command recalled
```

### Multiple Commands

You can enter commands in quick succession:

```
module enable looper
module set looper bpm 140
module get looper bpm
test run looper
```

### Scripting (Future)

In future versions, you may be able to run command scripts:

```
# Save commands to file on SD card
script run startup.txt
```

---

## Tips and Best Practices

### 1. Use Help Liberally

```
help                  # General help
help module           # Help for module command
help test             # Help for test command
```

### 2. Test Hardware Systematically

When debugging hardware issues:

```
1. test run srio      # Test digital I/O first
2. test run ainser64  # Test analog inputs
3. test run router    # Test MIDI routing
4. test run usb_midi  # Test USB communication
```

### 3. Save Configurations

After configuring modules, save to SD card:

```
config save           # Save current configuration
config load           # Load saved configuration
```

### 4. Monitor Resource Usage

```
system status         # Check RAM and CPU
log show             # View system log
```

---

## Troubleshooting

### CLI Not Responding

**Check:**
1. Correct baud rate (115200)
2. Correct COM port selected
3. USB cable properly connected
4. Terminal program running

**Try:**
```
# Send a few newlines
(Press Enter several times)

# Try simple command
help
```

### Test Won't Stop

**Solution:**
- Reset the device (most tests run infinite loops)
- Or use `test stop` command if graceful stop is implemented

### Module Not Found

```
ERROR: Module 'looperr' not found
```

**Solution:**
- Check spelling: `looper` not `looperr`
- Use `module list` to see exact module names

### Invalid Parameter

```
ERROR: Parameter 'temp' not found for module 'looper'
```

**Solution:**
- Use `module info looper` to see available parameters
- Check parameter name spelling

### Permission Denied

Some commands may require specific modes:

```
ERROR: Command requires test mode
```

**Solution:**
- Ensure `MODULE_ENABLE_TEST=1` in `module_config.h`
- Rebuild and reflash firmware

---

## Common CLI Commands Reference

### General
```
help                     # Show all commands
help <command>          # Show command help
system status           # System information
```

### Modules
```
module list             # List all modules
module info <name>      # Module details
module enable <name>    # Enable module
module disable <name>   # Disable module
module status <name>    # Module status
module get <name> <param> [track]   # Get parameter
module set <name> <param> <value> [track]  # Set parameter
```

### Tests
```
test list               # List tests
test run <name>         # Run test
test stop               # Stop test
test status             # Test status
```

### Configuration
```
config save             # Save config to SD
config load             # Load config from SD
config reset            # Reset to defaults
```

### Logging
```
log show                # Show system log
log clear               # Clear log
log level <level>       # Set log level (0-4)
```

---

## Next Steps

Now that you know how to use the CLI:

1. **Connect** to your MidiCore device
2. **Run** `module list` to see available modules
3. **Run** `test list` to see available tests
4. **Try** running a test: `test run ainser64`
5. **Configure** modules to your needs
6. **Save** your configuration

For module-specific details, see:
- `Docs/MODULE_CLI_IMPLEMENTATION_STATUS.md`
- Individual module README files in `Services/<module>/`

---

## Getting Help

If you encounter issues:

1. Check this guide
2. Run `help <command>` for specific command help
3. Review module documentation
4. Check the MidiCore GitHub issues
5. Ask in the community forum

---

**Happy testing and configuring!** 🎹🎵
