# MidiCore Studio

**A complete JUCE-based desktop application for MidiCore accordion MIDI controllers**

Full MIOS Studio compatibility with MidiCore-specific enhancements.

## Features

### 🎛️ Device Manager
- Auto-detect MidiCore devices
- Device information query
- Connection status monitoring
- Multi-device support

### 🎹 Virtual Piano
- Full MIDI keyboard interface
- Adjustable velocity and octave
- Channel selection (1-16)
- Mouse and computer keyboard input

### 🎚️ CC Controller
- 16 configurable CC sliders
- Common controllers (Modulation, Volume, Pan, Expression, etc.)
- Accordion-specific CCs (Bellows, Registers)
- Send all / Reset all functions
- Real-time MIDI output

### 📊 MIDI Monitor
- Real-time MIDI message display (4 ports)
- Timestamped messages
- Message filtering
- Auto-scroll option
- Export to log file

### 📁 File Manager
- SD card file browser via USB CDC
- Edit .ngc/.ngp configuration files
- Direct file upload/download
- File deletion
- Connection status monitoring

### 💻 Terminal
- Command-line interface
- Command history (up/down arrows)
- Color-coded output
- Custom command support
- MIOS32-compatible commands

### 🔧 Firmware Updater
- MIOS32 bootloader compatible
- Intel HEX file support
- Automatic bootloader detection
- Progress monitoring
- Flash erase and verify
- Safe update process
- Detailed logging

## Quick Start

### Prerequisites

- **JUCE Framework** 7.0+: Download from https://juce.com/
- **CMake** 3.15+
- **C++17 compiler**

### Build

```bash
cd MidiCoreStudio
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

See `BUILD.md` for detailed platform-specific instructions.

## Usage Guide

### 1. Device Manager Tab

**Connect Your Device:**
1. Connect MidiCore device via USB
2. Click "Refresh" to scan for devices
3. Select your device from the list
4. Click "Connect"
5. Click "Query Info" to view device details

### 2. MIDI Monitor Tab

**Monitor MIDI Activity:**
- All MIDI messages displayed in real-time
- Use "Auto Scroll" to follow new messages
- Click "Clear" to reset the display
- Copy/paste messages for debugging

### 3. Virtual Piano Tab

**Play Notes:**
- Click piano keys with mouse
- Use computer keyboard (ASDF... keys)
- Adjust "MIDI Channel" for routing
- Set "Velocity" for note dynamics
- Change "Octave" to shift key range

### 4. CC Controller Tab

**Send Control Changes:**
- Move sliders to send CC messages
- Common CCs pre-configured
- Accordion-specific controls available
- "Send All" sends current values
- "Reset All" sets everything to 0

### 5. File Manager Tab

**Edit SD Card Files:**
1. Ensure device connected (see Device Manager)
2. File list auto-refreshes
3. Double-click file to open editor
4. Edit content in text editor
5. Click "Save" to upload changes

**CDC Protocol:**
```
LIST              - Get file list
GET <file>        - Download file content
PUT <file> <size> - Upload file content
DELETE <file>     - Delete file
```

### 6. Terminal Tab

**Run Commands:**
- Type commands in input field
- Press Enter to execute
- Use Up/Down arrows for history
- Type `help` for command list

**Common Commands:**
```
help              - Show available commands
version           - Show firmware version
reset             - Reset device
save              - Save current configuration
load <file>       - Load configuration
debug on/off      - Enable/disable debug output
```

### 7. Firmware Update Tab

**Update Firmware:**

⚠️ **WARNING**: Do not disconnect device during update!

1. Click "Select Firmware (.hex)"
2. Choose your `.hex` file
3. Click "Enter Bootloader" (device will restart)
4. Wait for "Bootloader: MIOS32 Bootloader v1.0"
5. Click "Query Bootloader" to verify connection
6. Click "Start Update"
7. Wait for "Update Complete"
8. Device will auto-restart with new firmware

**Bootloader Protocol:**
- MIOS32-compatible MIDI SysEx
- Automatic flash erase
- Block-by-block programming
- Verification after each block
- Safe recovery mode

## MIOS Studio Compatibility

MidiCore Studio implements MIOS Studio features:

✅ **Compatible Features:**
- Device detection and selection
- MIDI monitoring
- Virtual piano keyboard
- CC controller sending
- Terminal/console interface
- Firmware update tool (MIOS32 bootloader)
- File manager (via CDC instead of MSC)

🆕 **MidiCore Enhancements:**
- Modern JUCE-based UI
- Cross-platform (Windows/macOS/Linux)
- Accordion-specific CC mappings
- Enhanced device information
- Improved file manager
- Better progress monitoring

## Architecture

```
MidiCoreStudio/
├── Source/
│   ├── Main.cpp                      # Application entry
│   ├── MainWindow.h/cpp              # Main window with tabs
│   ├── MidiIO/
│   │   └── MidiManager.h/cpp         # MIDI I/O handling
│   ├── CDCComm/
│   │   └── CDCManager.h/cpp          # CDC VCP communication
│   └── Components/
│       ├── DeviceManager.h/cpp       # Device selection
│       ├── MidiMonitor.h/cpp         # MIDI display
│       ├── VirtualPiano.h/cpp        # Virtual keyboard
│       ├── CCController.h/cpp        # CC sliders
│       ├── FileManagerComponent.h/cpp # File browser
│       ├── Terminal.h/cpp            # Command line
│       └── FirmwareUpdater.h/cpp     # Bootloader interface
├── CMakeLists.txt                    # Build configuration
├── README.md                         # This file
└── BUILD.md                          # Build instructions
```

## Development

### Adding New Features

1. Create component in `Source/Components/`
2. Add to `MainWindow.cpp` as new tab
3. Update `CMakeLists.txt` with new files
4. Build and test

### MIDI Protocol

Uses standard MIDI + MIOS32 SysEx extensions:
```
F0 00 00 7E 48 [device_id] [command] [data...] F7
```

### CDC File Protocol

Custom text-based protocol over CDC VCP:
```
Command: <CMD> [args]\r\n
Response: <DATA>\r\nOK\r\n or ERROR: <msg>\r\n
```

## Troubleshooting

### Device Not Detected
- Check USB connection
- Verify driver installation (Windows)
- Check permissions (Linux: udev rules)
- Try different USB port
- Restart application

### MIDI Not Working
- Select correct MIDI device
- Check MIDI channel settings
- Verify device not in bootloader mode
- Check MIDI routing in DAW

### CDC Connection Failed
- Device may not have CDC enabled
- Check `MODULE_ENABLE_USB_CDC=1` in firmware
- Try reconnecting device
- Check serial port permissions

### Firmware Update Failed
- Ensure `.hex` file is correct
- Verify bootloader mode entered
- Check USB connection stable
- Try update again
- Contact support if persists

## License

Same as MidiCore firmware - see main repository LICENSE file.

## Contributing

1. Fork the repository
2. Create feature branch
3. Commit changes
4. Push to branch
5. Create Pull Request

## Support

- **Issues**: https://github.com/labodezao/MidiCore/issues
- **Discussions**: https://github.com/labodezao/MidiCore/discussions

## Credits

- **JUCE Framework**: https://juce.com/
- **MIOS Studio**: https://github.com/midibox/ (protocol reference)
- **MidiCore Team**: Firmware and application development
