# MidiCore Tools

This directory contains utility scripts and tools for testing and debugging MidiCore firmware.

## midicore_emulator.py

Python script to test MIOS Studio terminal functionality by emulating MidiCore firmware.

### Platform-Specific Usage

**Windows (auto-detects loopMIDI):**
```bash
# Just run it - finds loopMIDI automatically!
python midicore_emulator.py
```

**macOS:**
```bash
# Creates virtual port automatically
python midicore_emulator.py
```

**Linux:**
```bash
# Creates virtual port automatically
python midicore_emulator.py
```

### Quick Commands

```bash
# Simple (auto-detects on all platforms)
python midicore_emulator.py

# List available MIDI ports
python midicore_emulator.py --list

# Use specific port (override auto-detection)
python midicore_emulator.py --use-existing "My Port"

# Verbose mode
python midicore_emulator.py --verbose

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
- ✅ Auto-detects Windows and finds loopMIDI
- ✅ Works on all platforms (Windows, macOS, Linux)

### Windows Setup

1. **Install loopMIDI** (if not already)
2. **Open loopMIDI** application
3. **Create port** - Click '+' button
4. **Keep loopMIDI running**
5. **Run emulator:**
   ```bash
   python midicore_emulator.py
   ```
   (Auto-finds loopMIDI - no flags needed!)
6. **Connect MIOS Studio** to loopMIDI port
7. **Open Terminal** window in MIOS Studio
8. **Watch messages** appear!

See `QUICKSTART_WINDOWS_LOOPMIDI.md` for detailed Windows instructions.

### Troubleshooting

**"ERROR: loopMIDI port not found"** (Windows)
- Open loopMIDI application
- Create a port (click '+' button)
- Keep loopMIDI running
- Run emulator again

**"ERROR creating virtual port"** (macOS/Linux)
- On macOS: Enable IAC Driver in Audio MIDI Setup
- On Linux: Run `sudo modprobe snd-virmidi`
- Or use: `python midicore_emulator.py --use-existing "Port Name"`

**"Device doesn't appear in MIOS Studio"**
- Make sure emulator is running
- On Windows: Select "loopMIDI Port" in MIOS Studio
- Refresh MIOS Studio device list
- Restart MIOS Studio

### Documentation Files

- `README.md` (this file) - Overview
- `README_MIDICORE_EMULATOR.md` - Full documentation
- `QUICKSTART_WINDOWS_LOOPMIDI.md` - Windows-specific guide

---

For detailed usage and troubleshooting, see the documentation files in this directory.
