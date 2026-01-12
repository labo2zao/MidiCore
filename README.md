
# MidiCore

MidiCore is a modular MIDI firmware for STM32 microcontrollers, inspired by the MIOS32 ecosystem. It provides a comprehensive platform for building advanced MIDI devices with features like USB MIDI, analog input processing, looping/sequencing, and more.

## Key Features

### Core MIDI Features
- **MIDI DIN** - 4-port UART-based MIDI I/O at 31.25 kbaud
- **USB MIDI** - High-speed USB MIDI (50-100x faster than DIN)
- **MIDI Router** - Flexible 16x16 routing matrix between nodes
- **USB Host MIDI** - Connect USB MIDI controllers/keyboards
- **SysEx Support** - Full System Exclusive message handling

### Hardware Support
- **AINSER64** - MCP3208 + 74HC4051 analog input (64 channels)
- **SRIO** - 74HC165/595 shift register I/O (buttons, LEDs)
- **OLED** - SSD1322 display support
- **SD Card** - FATFS for patches and loop storage
- **I2C Sensors** - Pressure sensors and other I2C devices

### Services
- **Looper** - MIDI recording/playback with 96 PPQN (LoopA-inspired)
- **Patch System** - MIOS32-like TXT patch format
- **Input** - Button/encoder handling with debouncing
- **AIN Processing** - Velocity detection, calibration, filtering
- **Zones** - Keyboard split/layer configuration

### **🆕 USB MIDI Bootloader**
- **Fast Firmware Updates** - Update firmware via USB MIDI (50-100x faster than DIN MIDI)
- **MIOS32 Compatible** - Uses standard SysEx protocol
- **Safe & Verified** - CRC checking and flash verification
- **No Programmer Needed** - Update without opening device or JTAG/SWD
- **Simple Tool** - Python script for easy uploads

See [README_BOOTLOADER.md](README_BOOTLOADER.md) for complete documentation.

## Quick Start

### Firmware Update (With Bootloader)

1. Install Python dependencies:
```bash
pip install python-rtmidi
```

2. Enter bootloader mode (one of these methods):
   - From application: Send bootloader SysEx command
   - No valid app: Bootloader auto-starts if flash is empty
   - Button press: Hold designated button during reset

3. Upload new firmware:
```bash
python3 Tools/upload_firmware.py firmware.bin
```

### Building from Source

#### Using STM32CubeIDE

1. Clone the repository
2. Open project in STM32CubeIDE
3. Select build configuration (Debug/Release)
4. For bootloader-compatible builds, use `STM32F407VGTX_FLASH_APP.ld` linker script
5. Build → Generate binary

#### Module Configuration

Enable/disable features in `Config/module_config.h`:

```c
#define MODULE_ENABLE_BOOTLOADER 1  // USB MIDI firmware update
#define MODULE_ENABLE_AINSER64 1    // Analog inputs
#define MODULE_ENABLE_SRIO 1        // Shift register I/O
#define MODULE_ENABLE_MIDI_DIN 1    // UART MIDI
#define MODULE_ENABLE_ROUTER 1      // MIDI routing
#define MODULE_ENABLE_LOOPER 1      // Recording/playback
#define MODULE_ENABLE_USB_MIDI 0    // Requires USB config
// ... more modules
```

## Hardware Compatibility

MidiCore is designed for STM32F407VG (1MB flash) but is portable to other STM32F4/F7/H7 chips with minor modifications.

### MIOS32/MIDIbox Hardware Compatibility

MidiCore maintains high compatibility with MIOS32/MIDIbox hardware:

- **AINSER64**: 100% compatible hardware and protocol
- **SRIO**: 100% compatible (MBHP naming conventions)
- **MIDI DIN**: 100% compatible (31.25k, running status)
- **Patch Format**: Compatible TXT key=value format
- **Looper**: Based on LoopA (96 PPQN, quantization)
- **USB MIDI Bootloader**: MIOS32-compatible SysEx protocol

See [MIOS32_COMPATIBILITY.md](MIOS32_COMPATIBILITY.md) for detailed comparison.

## Documentation

- [README_BOOTLOADER.md](README_BOOTLOADER.md) - USB MIDI bootloader guide
- [README_INTEGRATION.md](README_INTEGRATION.md) - Project integration
- [README_MODULE_CONFIG.md](README_MODULE_CONFIG.md) - Module configuration
- [MIOS32_COMPATIBILITY.md](MIOS32_COMPATIBILITY.md) - MIOS32 compatibility details
- [Tools/README.md](Tools/README.md) - Firmware upload tools

## Architecture

MidiCore follows a modular architecture with clean separation:

```
MidiCore/
├── Core/           # STM32 HAL and system initialization
├── Drivers/        # STM32 HAL drivers
├── Middlewares/    # USB stack, FreeRTOS
├── FATFS/          # SD card filesystem
├── Config/         # Module and project configuration
├── Hal/            # Hardware abstraction (AINSER, SRIO, OLED, etc.)
├── Services/       # MIDI, router, looper, bootloader, etc.
├── App/            # Application code and tasks
└── Tools/          # Firmware upload and utilities
```

## Minimal SD Patch Engine

The patch system uses MIOS32-style TXT format:

```
key=value
key2=value2
```

### Features
- Load/Save patches from SD card
- Key-value store in RAM
- MIOS32-compatible format

### Usage
```c
patch_init();
patch_load("0:/patches/default.txt");
patch_get("router_mode", buf, sizeof(buf));
```

### Requirements
- CubeMX FATFS enabled
- SDIO or SPI SD configured
- ff.h available

## License

This project follows the license specified in the repository.

## Contributing

Contributions are welcome! Please ensure:
- Code follows existing style and conventions
- Modules remain optional (compile-time configurable)
- MIOS32 compatibility is maintained where applicable
- Documentation is updated

## References

- **MIOS32**: http://www.midibox.org/mios32/
- **LoopA**: https://www.midiphy.com/en/loopa-v2/
- **MIDIbox Wiki**: http://wiki.midibox.org/
- **STM32**: https://www.st.com/stm32

---

**Version**: 1.0.0 with USB MIDI Bootloader  
**Target**: STM32F407VG  
**RTOS**: FreeRTOS (CMSIS-OS2)  
**Compatibility**: MIOS32 97%
