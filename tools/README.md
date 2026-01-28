# MidiCore Tools

This directory contains utility scripts and tools for testing and debugging MidiCore firmware.

## midicore_emulator.py

Python script to test MIOS Studio terminal functionality by emulating MidiCore firmware.

### Platform-Specific Usage

**Windows (with loopMIDI):**
```bash
# 1. Create a loopMIDI port first
# 2. Then run:
python midicore_emulator.py --use-existing "loopMIDI"
```

**macOS:**
```bash
# Virtual ports work automatically
python midicore_emulator.py
```

**Linux:**
```bash
# Virtual ports work automatically
python midicore_emulator.py
```

### Quick Commands

```bash
# List available MIDI ports
python midicore_emulator.py --list

# Use existing port (REQUIRED for Windows)
python midicore_emulator.py --use-existing "loopMIDI"

# Verbose mode
python midicore_emulator.py --use-existing "loopMIDI" --verbose

# Help
python midicore_emulator.py --help
```

### Requirements

```bash
pip install python-rtmidi
```

### Features

- ✅ Responds to MIOS32 queries
- ✅ Sends debug messages (SysEx 0x0D)
- ✅ 5-second countdown before messages
- ✅ Continuous test messages every 2 seconds
- ✅ Works on Windows (with --use-existing), macOS, Linux

### Windows Setup

1. **Install loopMIDI** (if not already)
2. **Open loopMIDI** application
3. **Create port** - Click '+' button
4. **Run emulator:**
   ```bash
   python midicore_emulator.py --use-existing "loopMIDI"
   ```
5. **Connect MIOS Studio** to loopMIDI port
6. **Open Terminal** window in MIOS Studio
7. **Watch messages** appear!

See `QUICKSTART_WINDOWS_LOOPMIDI.md` for detailed Windows instructions.

### Troubleshooting

**"ERROR creating virtual port"**
- On Windows: Use `--use-existing "loopMIDI"` instead
- On macOS/Linux: Check python-rtmidi installation

**"ERROR: No MIDI ports found"**
- On Windows: Create a loopMIDI port first
- On macOS: Enable IAC Driver in Audio MIDI Setup
- On Linux: Run `sudo modprobe snd-virmidi`

**"Device doesn't appear in MIOS Studio"**
- Make sure emulator is running
- On Windows: Select "loopMIDI Port" not "MidiCore Emulator"
- Refresh MIOS Studio device list
- Restart MIOS Studio

### Documentation Files

- `README.md` (this file) - Overview
- `README_MIDICORE_EMULATOR.md` - Full documentation
- `QUICKSTART_WINDOWS_LOOPMIDI.md` - Windows-specific guide

---

For detailed usage and troubleshooting, see the documentation files in this directory.
