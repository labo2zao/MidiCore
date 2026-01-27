# What is CLI? - Simple Explanation for MidiCore

## CLI = Command Line Interface

**CLI** means **Command Line Interface** - it's a way to control your device by typing text commands instead of clicking buttons.

## Think of it like this:

### With Buttons/Knobs (Normal UI):
```
You physically press buttons and turn knobs on your accordion
   ↓
The screen shows menus
   ↓
You navigate with rotary encoders
```

### With CLI (Command Line):
```
You connect a computer to the UART port
   ↓
You type text commands in a terminal program
   ↓
The device responds with text
```

---

## Real Example for MidiCore:

### Scenario: You want to enable the arpeggiator and set it to "UP" pattern

#### Option 1: Using Physical Buttons (UI)
1. Turn rotary encoder to navigate menu
2. Press button to enter "Effects" category
3. Turn encoder to find "Arpeggiator"
4. Press button to select
5. Turn encoder to change pattern
6. Press button to save

#### Option 2: Using CLI (Text Commands via UART)
Just type:
```
module enable arpeggiator
module set arpeggiator pattern UP
```

**That's it! Much faster!**

---

## What is UART?

**UART** = Universal Asynchronous Receiver/Transmitter

It's a simple serial communication port on your STM32 microcontroller that lets you connect a computer.

### Hardware Setup:
```
STM32 (MidiCore)          Computer
    TX pin   ─────────→   RX (receive)
    RX pin   ←─────────   TX (transmit)
    GND      ─────────────  GND
```

You need a **USB-to-Serial adapter** (like FTDI cable or CH340) that plugs into your computer's USB port.

---

## What is a Terminal Program?

A **terminal program** is software on your computer that lets you type text commands and see responses.

### Popular Terminal Programs:

**Windows:**
- PuTTY (most popular, free)
- TeraTerm (free)
- Arduino Serial Monitor (if you have Arduino IDE)

**Mac/Linux:**
- screen (built-in)
- minicom
- Arduino Serial Monitor

### Connection Settings:
- **Baud Rate:** 115200 (speed of communication)
- **Data bits:** 8
- **Parity:** None
- **Stop bits:** 1
- **Flow control:** None

---

## Why Use CLI for MidiCore?

### ✅ Advantages:

1. **Fast Configuration**
   - Type one command instead of navigating menus
   - Change multiple settings at once

2. **Scripting**
   - Save a file with all your settings
   - Load configurations instantly

3. **Testing**
   - Quickly test features during development
   - No need to rebuild/reflash firmware

4. **Debugging**
   - See exactly what's happening
   - Get detailed error messages

5. **Documentation**
   - Commands are written down - easy to remember
   - Share settings with other users

### ❌ Disadvantages:

1. Requires a computer connected
2. Need to learn/remember commands
3. Not practical during live performance

---

## CLI vs UI - When to Use Each

### Use CLI When:
- **Developing/Testing** - Quick changes during development
- **Initial Setup** - Configure everything before first use
- **Troubleshooting** - Debug problems
- **Batch Changes** - Change many settings at once
- **Documentation** - Share exact settings with others

### Use UI (Buttons/Screen) When:
- **Performing Live** - Playing the accordion
- **Quick Adjustments** - Change tempo, transpose, etc.
- **No Computer Available** - Standalone use
- **Easier/More Intuitive** - Visual feedback

### Best Practice:
**Use BOTH!**
- CLI for setup and testing
- UI for performance and quick changes

---

## Example CLI Session for MidiCore

### Connect Terminal:
```
1. Plug USB-to-Serial adapter into STM32 UART2
2. Open PuTTY (or other terminal)
3. Select COM port (e.g., COM3)
4. Set baud rate: 115200
5. Click "Open"
```

### You'll See:
```
=====================================
   MidiCore CLI v1.0
   Firmware Configuration Interface
=====================================

Type 'help' for available commands.

midicore> _
```

### Try Some Commands:
```
midicore> help
=== MidiCore CLI Help ===

[system]
  help                 - Show help
  version              - Show version
  list                 - List commands

[modules]
  module list          - List all modules
  module info          - Module information
  module enable        - Enable module
  ...

midicore> module list
=== Registered Modules ===

[Effect]
  arpeggiator
  harmonizer
  quantizer
  ...

midicore> module enable arpeggiator
OK: Enabled module: arpeggiator

midicore> module set arpeggiator pattern UP
OK: Set arpeggiator.pattern = UP

midicore> config save 0:/myconfig.ini
OK: Configuration saved to 0:/myconfig.ini
```

---

## How the Two Systems Work Together

```
┌─────────────────────────────────────┐
│   You (the user)                     │
└─────────────────────────────────────┘
         │                    │
         │                    │
    Physical             Computer
    Buttons              Terminal
    & Encoders           (CLI)
         │                    │
         ▼                    ▼
┌─────────────────────────────────────┐
│      MidiCore Firmware               │
│  ┌──────────────────────────────┐   │
│  │   Module Registry            │   │
│  │   (All modules & settings)   │   │
│  └──────────────────────────────┘   │
│         ▲              ▲             │
│         │              │             │
│    ┌────┴────┐    ┌───┴────┐        │
│    │   UI    │    │  CLI   │        │
│    │ System  │    │ System │        │
│    └─────────┘    └────────┘        │
└─────────────────────────────────────┘

Both access the SAME settings!
Changes in CLI appear in UI, and vice versa.
```

---

## Configuration File Example

When you type `config save 0:/myconfig.ini`, it creates a text file on the SD card:

**File: myconfig.ini**
```ini
[arpeggiator]
enabled = true
pattern = UP
rate = 8

[looper]
bpm = 120
time_signature_num = 4
time_signature_den = 4

[metronome]
enabled = true
midi_channel = 9
accent_note = 76
regular_note = 77
```

You can:
- Edit this file on your computer
- Copy it to another accordion
- Save different files for different performances

---

## Quick Reference

### Most Useful Commands:

```bash
# See all modules
module list

# Get information about a module
module info arpeggiator

# Enable/disable a module
module enable arpeggiator
module disable arpeggiator

# View module parameters
module params arpeggiator

# Get a parameter value
module get arpeggiator pattern

# Change a parameter
module set arpeggiator pattern UP
module set looper bpm 120

# Save your settings
config save 0:/mysettings.ini

# Load settings
config load 0:/mysettings.ini

# See firmware version
version

# Restart the device
reboot
```

---

## Do You Need CLI?

### You MUST have CLI working for:
- Development and testing
- Initial configuration
- Troubleshooting

### You CAN skip CLI if:
- You only use the physical buttons/screen
- Someone else configured it for you
- You don't need to change settings often

### Recommendation:
**Set up CLI first!** It makes everything else easier.

---

## Summary

| Feature | CLI | UI (Buttons/Screen) |
|---------|-----|---------------------|
| **What it is** | Type text commands | Press buttons, turn knobs |
| **Connection** | Computer via UART | Built-in hardware |
| **Speed** | Very fast | Slower (menu navigation) |
| **When to use** | Setup, testing, debugging | Live performance |
| **Learning curve** | Need to learn commands | More intuitive |
| **Power** | Can change many things quickly | One setting at a time |

**Both systems control the same thing - your MidiCore modules!**

The CLI is like using a keyboard shortcut, while the UI is like clicking through menus. Both get you to the same place.

---

## Next Steps

1. **Connect UART** - Get a USB-to-Serial adapter
2. **Open Terminal** - Install PuTTY or similar
3. **Type `help`** - See what commands are available
4. **Experiment** - Try enabling modules and changing settings
5. **Use UI** - See how the changes appear on the screen

## Questions?

Common questions:

**Q: Do I need to use CLI?**
A: For development/testing, yes. For playing, no.

**Q: Can I break something with CLI?**
A: No! You can always reboot or reload a good configuration.

**Q: Is CLI better than UI?**
A: Neither is better - they're for different purposes. Use both!

**Q: Can I use CLI without a screen?**
A: Yes! CLI works even if the OLED screen isn't connected.

**Q: How do I know what commands exist?**
A: Type `help` to see all commands. Type `help <command>` for details.

---

**Remember:** CLI is just another way to talk to your MidiCore firmware. It's text-based instead of button-based, but it controls the same things!
