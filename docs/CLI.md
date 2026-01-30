# MidiCore CLI Command Reference

## Overview

Complete reference for all Command-Line Interface (CLI) commands available in MidiCore firmware. Commands can be executed via UART terminal (115200 baud, 8N1) or USB CDC virtual COM port.

## Quick Start

### Connecting to CLI

**Via UART:**
```bash
# Default: UART2, 115200 baud, 8N1
# Use any serial terminal (PuTTY, minicom, screen, etc.)
screen /dev/ttyUSB0 115200
```

**Via USB CDC:**
```bash
# Virtual COM port appears as /dev/ttyACMx (Linux) or COMx (Windows)
screen /dev/ttyACM0 115200
```

### Basic Interaction

```
> help                    # Show all commands
> help <command>          # Show help for specific command
> list                    # List all registered commands
> <TAB>                   # Auto-complete (if supported)
> <Up>/<Down>             # Command history (if enabled)
```

---

## System Commands

Built-in commands for system control and information.

### help

Show help for all commands or a specific command.

**Syntax:**
```
help [command]
```

**Examples:**
```
> help                    # List all commands with descriptions
> help module             # Show detailed help for 'module' command
> help stack              # Show help for stack monitoring
```

**Output:**
```
=== MidiCore CLI Commands ===

System Commands:
  help     - Show help for commands
  list     - List all registered commands
  version  - Show firmware version
  uptime   - Show system uptime
  status   - Show system status
  reboot   - Reboot system
  clear    - Clear terminal screen
  ...
```

### list

List all registered CLI commands grouped by category.

**Syntax:**
```
list
```

**Output:**
```
=== Registered Commands by Category ===

[system] (7 commands)
  help, list, version, uptime, status, reboot, clear

[modules] (1 command)
  module

[stack] (3 commands)
  stack, stack_all, stack_monitor

[router] (1 command)
  router

...
```

### version

Display firmware version information.

**Syntax:**
```
version
```

**Output:**
```
MidiCore Firmware
Version: 1.0.0
Build: Jan 30 2025 11:00:00
Target: STM32F407VGT6
FreeRTOS: v10.3.1
```

### uptime

Show system uptime since last boot.

**Syntax:**
```
uptime
```

**Output:**
```
System uptime: 00:15:43 (943256 ms)
```

### status

Display overall system status.

**Syntax:**
```
status
```

**Output:**
```
=== System Status ===
Status: Running
Tasks: 11 active
CPU: 45%
Heap free: 8192 bytes
Stack OK: Yes
SD Card: Mounted
USB: Connected
```

### reboot

Reboot the system (soft reset).

**Syntax:**
```
reboot
```

**Output:**
```
Rebooting system...
[system reboots]
```

**Warning:** Unsaved configuration will be lost.

### clear

Clear the terminal screen.

**Syntax:**
```
clear
```

**Output:** Terminal screen is cleared (sends ANSI escape codes).

---

## Module Commands

Commands for discovering, controlling, and configuring firmware modules via the module registry system.

### module list

List all registered modules.

**Syntax:**
```
module list [category]
```

**Examples:**
```
> module list                    # List all modules
> module list effect             # List only effect modules
```

**Output:**
```
=== Registered Modules (60 total) ===

[input] (5 modules)
  ain, ainser, din, srio, ...

[midi] (12 modules)
  router, midi_filter, midi_converter, midi_delay, ...

[effect] (15 modules)
  arpeggiator, chord, harmonizer, lfo, humanize, ...

[sequencer] (3 modules)
  looper, metronome, rhythm_trainer

...
```

### module info

Show detailed information about a specific module.

**Syntax:**
```
module info <module_name>
```

**Examples:**
```
> module info arpeggiator
> module info looper
> module info midi_filter
```

**Output:**
```
=== Module: arpeggiator ===
Name: arpeggiator
Description: MIDI arpeggiator with multiple patterns
Category: effect
Status: Enabled (global)
Per-track: Yes

Parameters (4):
  enabled    [bool]   Enable/disable arpeggiator
  pattern    [enum]   Arpeggio pattern (UP, DOWN, UPDOWN, RANDOM)
  octaves    [int]    Number of octaves (1-4)
  gate       [int]    Note gate time in % (1-100)
```

### module enable

Enable a module globally or for a specific track.

**Syntax:**
```
module enable <module_name> [track]
```

**Parameters:**
- `module_name`: Name of the module to enable
- `track`: Optional track number (0-15), omit for global

**Examples:**
```
> module enable arpeggiator           # Enable globally
> module enable midi_filter 0         # Enable for track 0
> module enable harmonizer 2          # Enable for track 2
```

**Output:**
```
✓ Enabled module: arpeggiator
```

### module disable

Disable a module globally or for a specific track.

**Syntax:**
```
module disable <module_name> [track]
```

**Examples:**
```
> module disable arpeggiator          # Disable globally
> module disable midi_filter 0        # Disable for track 0
```

**Output:**
```
✓ Disabled module: arpeggiator
```

### module status

Show enable/disable status of a module.

**Syntax:**
```
module status <module_name> [track]
```

**Examples:**
```
> module status arpeggiator           # Global status
> module status midi_filter 0         # Status for track 0
```

**Output:**
```
Module: arpeggiator
Status: Enabled
Scope: Global
```

### module get

Get the value of a module parameter.

**Syntax:**
```
module get <module_name> <parameter> [track]
```

**Examples:**
```
> module get arpeggiator pattern
> module get looper bpm
> module get midi_filter transpose 0
```

**Output:**
```
arpeggiator.pattern = UP
```

### module set

Set the value of a module parameter.

**Syntax:**
```
module set <module_name> <parameter> <value> [track]
```

**Parameters:**
- `module_name`: Module to configure
- `parameter`: Parameter name
- `value`: New value (type depends on parameter)
- `track`: Optional track number

**Examples:**
```
> module set arpeggiator pattern UP
> module set arpeggiator octaves 2
> module set looper bpm 120
> module set midi_filter transpose 12 0
> module set humanize enabled true
```

**Output:**
```
✓ Set arpeggiator.pattern = UP
```

### module params

List all parameters for a module.

**Syntax:**
```
module params <module_name>
```

**Examples:**
```
> module params arpeggiator
> module params looper
```

**Output:**
```
=== Parameters for arpeggiator ===

enabled [bool]
  Description: Enable/disable arpeggiator
  Current: true

pattern [enum]
  Description: Arpeggio pattern
  Values: UP, DOWN, UPDOWN, RANDOM
  Current: UP

octaves [int]
  Description: Number of octaves
  Range: 1-4
  Current: 2

gate [int]
  Description: Note gate time percentage
  Range: 1-100
  Current: 90
```

---

## Stack Monitoring Commands

Commands for monitoring FreeRTOS task stack usage and detecting overflows.

### stack

Show stack usage for the current task or a specific task.

**Syntax:**
```
stack [task_name]
```

**Examples:**
```
> stack                       # Show current task
> stack CliTask               # Show specific task
> stack defaultTask
```

**Output:**
```
Task: CliTask
  Stack size:    5120 bytes (1280 words)
  Used:          3584 bytes (70%)
  Free:          1536 bytes (30%)
  High-water:    1536 bytes
  Status:        OK
```

**Status Levels:**
- `OK`: Stack usage is healthy
- `WARNING`: Approaching limit (< 20% free)
- `CRITICAL`: Critically low (< 5% free)
- `OVERFLOW`: Stack overflow detected

### stack_all

Show stack usage for all tasks.

**Syntax:**
```
stack_all [-v]
```

**Options:**
- `-v`, `--verbose`: Include detailed information

**Examples:**
```
> stack_all                   # Basic view
> stack_all -v                # Verbose view
```

**Output:**
```
=== Stack Usage Report (11 tasks) ===
Task            Used         Total    Used%   Free%  Status
--------------- ------------ -------- ------- ------ ------
defaultTask         8192 B  12288 B      67%     33% OK
CliTask             3584 B   5120 B      70%     30% OK
AinTask              450 B   1024 B      44%     56% OK
OledDemo             512 B   1024 B      50%     50% OK
MidiIOTask           768 B   1024 B      75%     25% OK
CalibrationTask      890 B   1400 B      64%     36% OK
AinMidiTask          612 B   1024 B      60%     40% OK
PressureTask         384 B    768 B      50%     50% OK
StackMon             256 B    512 B      50%     50% OK
Tmr Svc              512 B   1024 B      50%     50% OK
IDLE                 128 B    128 B     100%      0% OK
```

**Verbose Output:**
Includes high-water mark details:
```
defaultTask
  High-water mark: 4096 bytes (1024 words)
```

### stack_monitor

Control and configure the stack monitoring system.

**Syntax:**
```
stack_monitor <subcommand> [args]
```

**Subcommands:**
- `start`: Start monitoring
- `stop`: Stop monitoring
- `stats`: Show monitor statistics
- `config`: View/set configuration
- `check`: Force immediate check
- `export`: Export data as CSV

#### stack_monitor start

Start stack monitoring (if stopped).

**Syntax:**
```
stack_monitor start
```

**Output:**
```
✓ Stack monitoring started
```

#### stack_monitor stop

Stop stack monitoring.

**Syntax:**
```
stack_monitor stop
```

**Output:**
```
✓ Stack monitoring stopped
```

#### stack_monitor stats

Show monitoring statistics.

**Syntax:**
```
stack_monitor stats
```

**Output:**
```
=== Stack Monitor Statistics ===
Total checks:    120
Warnings:        2
Critical alerts: 0
Overflows:       0
Last check:      600523 ms
Interval:        5000 ms
Warn threshold:  20%
Crit threshold:  5%
```

#### stack_monitor config

View or set monitor configuration.

**Syntax:**
```
stack_monitor config [parameter] [value]
```

**Parameters:**
- `interval`: Check interval in milliseconds
- `warning`: Warning threshold (% free)
- `critical`: Critical threshold (% free)

**Examples:**
```
> stack_monitor config                    # Show current config
> stack_monitor config interval 10000     # Check every 10s
> stack_monitor config warning 25         # Warn at 25% free
> stack_monitor config critical 10        # Critical at 10% free
```

**Output:**
```
Stack Monitor Configuration:
  Status:            Running
  Interval:          5000 ms
  Warning threshold: 20%
  Critical threshold: 5%
```

#### stack_monitor check

Force an immediate stack check (bypass interval).

**Syntax:**
```
stack_monitor check
```

**Output:**
```
Forcing immediate stack check...
✓ Stack check completed
```

#### stack_monitor export

Export stack data as CSV for logging/analysis.

**Syntax:**
```
stack_monitor export
```

**Output:**
```
Exporting stack data as CSV...

task_name,used_bytes,total_bytes,used_pct,free_pct,hwm_bytes,status
defaultTask,8192,12288,67,33,4096,0
CliTask,3584,5120,70,30,1536,0
AinTask,450,1024,44,56,574,0
...
```

**Format:** CSV with columns:
- `task_name`: Task name
- `used_bytes`: Stack used (bytes)
- `total_bytes`: Total stack size (bytes)
- `used_pct`: Usage percentage
- `free_pct`: Free percentage
- `hwm_bytes`: High-water mark (bytes)
- `status`: Status code (0=OK, 1=WARN, 2=CRIT, 3=OVFL)

---

## Router Commands

Commands for controlling the MIDI routing matrix.

### router

Control MIDI routing between nodes.

**Syntax:**
```
router <subcommand> [args]
```

**Subcommands:**
- `matrix`: Show routing matrix
- `enable`: Enable a route
- `disable`: Disable a route
- `channel`: Set channel mapping
- `label`: Set node label
- `info`: Show router info
- `test`: Test routing

#### router matrix

Display the current routing matrix.

**Syntax:**
```
router matrix
```

**Output:**
```
=== MIDI Routing Matrix (16x16) ===

Source→Target    DIN1 DIN2 DIN3 DIN4 USB1 USB2 LOOP APP
DIN IN 1           ●    -    -    -    ●    -    -    ●
DIN IN 2           -    ●    -    -    ●    -    -    -
DIN IN 3           -    -    ●    -    ●    -    -    -
DIN IN 4           -    -    -    ●    ●    -    -    -
USB IN 1           ●    -    -    -    -    ●    -    ●
Looper             ●    -    -    -    ●    -    -    -

● = Enabled    - = Disabled
```

#### router enable

Enable a route between source and destination nodes.

**Syntax:**
```
router enable <source> <dest>
```

**Node Names:**
- `DIN_IN1` through `DIN_IN4`: MIDI DIN inputs
- `DIN_OUT1` through `DIN_OUT4`: MIDI DIN outputs
- `USB_IN1`, `USB_IN2`: USB MIDI inputs
- `USB_OUT1`, `USB_OUT2`: USB MIDI outputs
- `LOOPER`: Looper sequencer
- `APP`: Application layer

**Examples:**
```
> router enable DIN_IN1 DIN_OUT1
> router enable USB_IN1 DIN_OUT1
> router enable LOOPER DIN_OUT1
```

**Output:**
```
✓ Enabled route: DIN_IN1 → DIN_OUT1
```

#### router disable

Disable a route between nodes.

**Syntax:**
```
router disable <source> <dest>
```

**Examples:**
```
> router disable DIN_IN1 DIN_OUT1
```

**Output:**
```
✓ Disabled route: DIN_IN1 → DIN_OUT1
```

---

## Configuration Commands

Commands for managing system configuration (via `config` command prefix).

### config load

Load configuration from SD card.

**Syntax:**
```
config load <path>
```

**Examples:**
```
> config load 0:/config.ini
> config load 0:/patches/bank1.cfg
```

**Output:**
```
Loading configuration from 0:/config.ini...
✓ Configuration loaded successfully
120 parameters restored
```

### config save

Save current configuration to SD card.

**Syntax:**
```
config save <path>
```

**Examples:**
```
> config save 0:/config.ini
> config save 0:/backup/config_$(date).ini
```

**Output:**
```
Saving configuration to 0:/config.ini...
✓ Configuration saved successfully
120 parameters written
```

### config list

List all configuration entries.

**Syntax:**
```
config list
```

**Output:**
```
=== Configuration Entries ===

[arpeggiator]
enabled = true
pattern = UP
octaves = 2
gate = 90

[looper]
bpm = 120
quantize = true
...
```

### config get

Get a configuration value.

**Syntax:**
```
config get <key>
```

**Examples:**
```
> config get arpeggiator.pattern
> config get looper.bpm
```

**Output:**
```
arpeggiator.pattern = UP
```

### config set

Set a configuration value.

**Syntax:**
```
config set <key> <value>
```

**Examples:**
```
> config set arpeggiator.pattern DOWN
> config set looper.bpm 140
```

**Output:**
```
✓ Set arpeggiator.pattern = DOWN
```

---

## Command Categories

### By Category

| Category | Commands |
|----------|----------|
| **System** | help, list, version, uptime, status, reboot, clear |
| **Modules** | module (list, info, enable, disable, status, get, set, params) |
| **Stack** | stack, stack_all, stack_monitor |
| **Router** | router (matrix, enable, disable, channel, label, info, test) |
| **Config** | config (load, save, list, get, set) |

### Alphabetical Index

- `clear` - Clear terminal screen
- `config get` - Get configuration value
- `config list` - List all config entries
- `config load` - Load config from SD card
- `config save` - Save config to SD card
- `config set` - Set configuration value
- `help` - Show command help
- `list` - List all commands
- `module disable` - Disable a module
- `module enable` - Enable a module
- `module get` - Get module parameter
- `module info` - Show module info
- `module list` - List all modules
- `module params` - List module parameters
- `module set` - Set module parameter
- `module status` - Show module status
- `reboot` - Reboot system
- `router disable` - Disable MIDI route
- `router enable` - Enable MIDI route
- `router matrix` - Show routing matrix
- `stack` - Show task stack usage
- `stack_all` - Show all task stacks
- `stack_monitor` - Control stack monitor
- `status` - Show system status
- `uptime` - Show system uptime
- `version` - Show firmware version

---

## Tips & Best Practices

### Command History

If history is enabled (`CLI_HISTORY_SIZE > 0`):
- Press `Up` arrow to recall previous commands
- Press `Down` arrow to move forward in history
- History persists for the current session only

### Tab Completion

If auto-completion is implemented:
- Press `Tab` to auto-complete command names
- Press `Tab` twice to show all matching commands

### Output Redirection

CLI output goes to the connected terminal. To capture:
```bash
# On Linux/Mac
screen -L /dev/ttyUSB0 115200    # Logs to screenlog.0

# Or use logging software like CoolTerm, PuTTY (Session Logging)
```

### Scripting

Send commands programmatically:
```bash
# Using echo and redirection
echo "stack_all" > /dev/ttyUSB0

# Using Python
import serial
ser = serial.Serial('/dev/ttyUSB0', 115200)
ser.write(b'stack_all\r\n')
print(ser.readline())
```

### Safety

- **`reboot` command**: Unsaved configuration will be lost
- **Parameter changes**: Take effect immediately (not persistent until saved)
- **Stack monitoring**: Should always be enabled in development
- **Routing changes**: Affect MIDI flow immediately

---

## Troubleshooting

### CLI Not Responding

**Check:**
1. Correct UART port and baud rate (115200)
2. 8 data bits, No parity, 1 stop bit (8N1)
3. No flow control
4. CR+LF line endings (or just LF)

**Test:**
```
> help
[If no response, check physical connection]
```

### Command Not Found

**Possible causes:**
1. Typo in command name (commands are case-sensitive)
2. Module not enabled (`MODULE_ENABLE_xxx` in config)
3. Feature not compiled in

**Check:**
```
> list                    # See all available commands
> help <command>          # Check if command exists
```

### Module Not Listed

If a module doesn't appear in `module list`:
1. Check `MODULE_ENABLE_xxx` in `Config/module_config.h`
2. Verify module is registered in `module_registry_init()`
3. Rebuild firmware

### Stack Overflow Detected

If `stack_all` shows CRITICAL or OVERFLOW:
1. Immediately investigate the task
2. Check for large local variables
3. Review call stack depth
4. Increase task stack size if needed

**See:** `docs/STACK_ANALYSIS.md` for detailed debugging procedure

---

## References

- **CLI System Documentation**: `Services/cli/README.md`
- **Module Registry**: `Services/module_registry/README.md`
- **Module Inventory**: `MODULE_INVENTORY.md`
- **Integration Guide**: `MODULE_CLI_INTEGRATION.md`
- **Stack Analysis**: `docs/STACK_ANALYSIS.md`

---

**Document Version**: 1.0  
**Last Updated**: 2025-01-30  
**Firmware Version**: MidiCore v1.0.0
