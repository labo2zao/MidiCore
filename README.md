# MidiCore

Professional MIDI instrument controller based on STM32F407VGT6.

## Overview

MidiCore is a modular MIDI system featuring:
- 64-channel analog input (AINSER64/MCP3208)
- Shift register I/O (SRIO/MBHP compatible)
- MIDI DIN, USB Device, and USB Host MIDI
- MIDI looper with recording/playback
- Configurable routing and processing
- OLED UI (SSD1322)
- SD card patch management

## Quick Start

1. **Build the project** in STM32CubeIDE
2. **Flash** to STM32F407 target
3. **Configure** modules in `Config/module_config.h`
4. **Test** individual modules - see [Testing Guide](TESTING_QUICKSTART.md)

## Documentation

- **[MIOS32 UART Configuration](README_MIOS32_UART_CONFIG.md)** - UART setup and dbg_print API
- **[Module Testing Guide](README_MODULE_TESTING.md)** - Test modules individually
- **[Testing Quick Start](TESTING_QUICKSTART.md)** - Quick test examples
- **[Module Configuration](README_MODULE_CONFIG.md)** - Enable/disable modules
- **[Module Details](Modules_MidiCore_Detail_par_Module.txt)** - Detailed architecture (French)
- **[Portability Guide](README_PORTABILITY.md)** - STM32F4/H7 compatibility
- **[Integration Guide](README_INTEGRATION.md)** - Project integration
- **[USB Host MIDI](README_USBH_MIDI.md)** - USB Host setup

## Module Testing

MidiCore includes a comprehensive testing framework for validating modules in isolation:

```c
// Example: Test AINSER64 analog inputs only
#define MODULE_TEST_AINSER64
```

See [TESTING_QUICKSTART.md](TESTING_QUICKSTART.md) for step-by-step examples.

## Architecture

### Core Modules
- **SYSTEM**: Panic handling, safe mode, watchdog
- **TEMPO/LOOPER**: Clock, MIDI recording/playback
- **ROUTER**: Message routing with DSL configuration
- **UI**: OLED display, encoders, pages

### Hardware Modules
- **AINSER64**: 64-ch analog input (MCP3208 + mux)
- **SRIO**: Shift register DIN/DOUT (74HC165/595)
- **OLED**: SSD1322 grayscale display
- **SD**: Patch loading via FatFS

### MIDI Modules
- **MIDI DIN**: Standard 5-pin DIN I/O
- **USB Device**: USB MIDI to PC
- **USB Host**: Connect external MIDI controllers
- **Router**: Configurable message routing

## Features

- **Patch Management**: SD-based TXT patch format (MIOS32 compatible)
- **MIDI Looper**: Multi-track recording with quantization
- **Modular Design**: Enable only needed modules
- **Testing Framework**: Test modules individually
- **Safe Mode**: Graceful degradation on errors
- **Configurable I/O**: Define zones, instruments, routing rules

## Hardware Requirements

- STM32F407VGT6 microcontroller
- Optional: MCP3208 for analog inputs
- Optional: 74HC165/595 for button/LED I/O
- Optional: SSD1322 OLED display
- Optional: SD card slot (SDIO or SPI)
- Optional: MIDI DIN connectors
- Optional: USB Host port

## Build Configuration

### Enable/Disable Modules

Edit `Config/module_config.h`:

```c
#define MODULE_ENABLE_AINSER64  1  // Enable analog inputs
#define MODULE_ENABLE_SRIO      1  // Enable digital I/O
#define MODULE_ENABLE_OLED      1  // Enable display
#define MODULE_ENABLE_LOOPER    1  // Enable looper
// ... etc
```

### Test Specific Module

Add preprocessor define in build settings:

```
MODULE_TEST_AINSER64  // Test analog inputs only
```

See [README_MODULE_CONFIG.md](README_MODULE_CONFIG.md) for details.

## Development Workflow

1. **Isolate Module**: Define `MODULE_TEST_xxx` to test specific module
2. **Develop**: Make changes to module code
3. **Test**: Build and flash to verify functionality
4. **Integrate**: Remove test define, enable in production
5. **Validate**: Run full system test

## Contributing

This project uses:
- STM32CubeIDE for development
- FreeRTOS for task management
- FatFS for file system
- Custom HAL wrappers for hardware abstraction

### Adding a New Module

1. Create module in `Services/your_module/`
2. Add enable flag in `Config/module_config.h`
3. Add test in `App/tests/module_tests.c`
4. Document in appropriate README

## Compatibility

- **Primary Target**: STM32F407VGT6
- **Future Support**: STM32H7 (more RAM/Flash)
- **MIOS32 Compatible**: 100% compatible UART mapping and hardware interface
- **MBHP Compatible**: Works with MIOS32 MBHP shields and modules

See [README_PORTABILITY.md](README_PORTABILITY.md) for migration guide.  
See [README_MIOS32_UART_CONFIG.md](README_MIOS32_UART_CONFIG.md) for MIOS32 compatibility details.

## License

See individual source files for license information.

## Support

For detailed module documentation (in French), see:
- `Modules_MidiCore_Detail_par_Module.txt`

For testing and development:
- `README_MODULE_TESTING.md`
- `TESTING_QUICKSTART.md`
