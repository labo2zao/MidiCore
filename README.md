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
4. **Test** individual modules - see [Testing Guide](Docs/testing/TESTING_QUICKSTART.md)

## Documentation

### Quick Start Guides
- **[Module Configuration](Docs/configuration/README_MODULE_CONFIG.md)** - Enable/disable modules
- **[Testing Quick Start](Docs/testing/TESTING_QUICKSTART.md)** - Quick test examples
- **[Integration Guide](Docs/guides/README_INTEGRATION.md)** - Project integration

### Configuration
- **[MIOS32 UART Configuration](Docs/configuration/README_MIOS32_UART_CONFIG.md)** - UART setup and dbg_print API
- **[SPI Configuration Reference](Docs/configuration/SPI_CONFIGURATION_REFERENCE.md)** - SPI setup
- **[CubeMX Regeneration Guide](Docs/configuration/CUBEMX_REGENERATION_GUIDE.md)** - How to regenerate with CubeMX
- **[FreeRTOS Protection Guide](Docs/configuration/FREERTOS_PROTECTION_GUIDE.md)** - Protect custom code from CubeMX

### Testing
- **[Module Testing Guide](Docs/testing/README_MODULE_TESTING.md)** - Test modules individually
- **[Testing Protocol](Docs/testing/TESTING_PROTOCOL.md)** - Complete testing protocol
- **[Test Execution](Docs/testing/TEST_EXECUTION.md)** - Test execution details
- **[Test Validation Report](Docs/testing/TEST_VALIDATION_REPORT.md)** - Validation results

### USB Documentation
- **[USB Host MIDI](Docs/guides/README_USBH_MIDI.md)** - USB Host setup
- **[USB Configuration Guide](Docs/usb/USB_CONFIGURATION_GUIDE.md)** - USB configuration
- **[USB Device and Host Guide](Docs/usb/USB_DEVICE_AND_HOST_GUIDE.md)** - Complete USB setup
- **[USB Debug Guide](Docs/usb/USB_DEBUG_GUIDE.md)** - USB debugging

### MIOS32 Compatibility
- **[MIOS32 Compatibility](Docs/mios32/MIOS32_COMPATIBILITY.md)** - MIOS32 compatibility overview
- **[MIOS32 Deep Comparison](Docs/mios32/MIOS32_DEEP_COMPARISON.md)** - Detailed comparison with MIOS32
- **[MIOS32 USB Implementation Guide](Docs/mios32/MIOS32_USB_IMPLEMENTATION_GUIDE.md)** - USB implementation details

### Compatibility & Portability
- **[Portability Guide](Docs/compatibility/README_PORTABILITY.md)** - STM32F4/H7 compatibility
- **[Compatibility Summary](Docs/compatibility/COMPATIBILITY_SUMMARY.md)** - Overall compatibility status
- **[Driver Compatibility Report](Docs/compatibility/DRIVER_COMPATIBILITY_REPORT.md)** - Driver compatibility details

### Other Resources
- **[Module Details](Modules_MidiCore_Detail_par_Module.txt)** - Detailed architecture (French)
- **[Full Documentation Index](Docs/)** - Browse all documentation

## Module Testing

MidiCore includes a comprehensive testing framework for validating modules in isolation:

```c
// Select test via preprocessor define in build settings
#define MODULE_TEST_AINSER64
```

See [TESTING_QUICKSTART.md](Docs/testing/TESTING_QUICKSTART.md) for step-by-step examples.

**Note:** Preprocessor defines (e.g., `MODULE_TEST_AINSER64`) are used in build configuration. Internal enum values have `_ID` suffix to avoid conflicts.

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

See [README_MODULE_CONFIG.md](Docs/configuration/README_MODULE_CONFIG.md) for details.

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

See [README_PORTABILITY.md](Docs/compatibility/README_PORTABILITY.md) for migration guide.  
See [README_MIOS32_UART_CONFIG.md](Docs/configuration/README_MIOS32_UART_CONFIG.md) for MIOS32 compatibility details.

## License

See individual source files for license information.

## Support

For detailed module documentation (in French), see:
- `Modules_MidiCore_Detail_par_Module.txt`

For testing and development:
- [README_MODULE_TESTING.md](Docs/testing/README_MODULE_TESTING.md)
- [TESTING_QUICKSTART.md](Docs/testing/TESTING_QUICKSTART.md)
# MidiCore - Advanced MIDI Looper & Performance System

A professional MIDI looper and performance system built on STM32F407 with comprehensive features for live performance, music production, and rhythm training.

## Overview

MidiCore implements a complete MIDI looper system with 4 tracks, 8 scenes, real-time effects processing, advanced workflow tools, and professional-grade features for accordion, wind controllers, and general MIDI performance.

**Key Statistics**:
- ~14,350 lines of code across 15 modules
- 43 configuration parameters with SD card persistence
- 6 custom UI pages on 256×64 grayscale OLED (SSD1322)
- 14 rhythm subdivisions including polyrhythms
- 13 advanced enhancement features
- Production-ready with comprehensive API documentation

## Core Features

### Phase 1: UI Pages (6 Custom Pages)

1. **Song Mode** (`ui_page_song.c/h`)
   - 4 tracks × 8 scenes (A-H) grid view with visual clip indicators
   - Scene selection and playback control
   - Real-time scene chaining visualization

2. **MIDI Monitor** (`ui_page_midi_monitor.c/h`)
   - Real-time MIDI message display with timestamps
   - Message decoding (NoteOn, CC, PitchBend, etc.)
   - Circular buffer with pause/clear functionality

3. **SysEx Viewer** (`ui_page_sysex.c/h`)
   - Hex viewer (16 bytes/row, scrollable)
   - Manufacturer ID decoding (1-byte/3-byte support)
   - Truncation warnings for large messages

4. **Config Editor** (`ui_page_config.c/h`)
   - SCS-style parameter editor for all hardware modules
   - VIEW/EDIT modes with encoder navigation
   - SD card save/load with .NGC format support

5. **LiveFX Control** (`ui_page_livefx.c/h`)
   - Per-track transpose (±12 semitones)
   - Velocity scaling (0-200%)
   - Force-to-scale with 15 musical scales
   - Enable/disable per track

6. **Rhythm Trainer** (`ui_page_rhythm.c/h`)
   - **14 rhythm subdivisions**: straight notes, triplets (1/8T, 1/16T), dotted notes (1/4., 1/8., 1/16.), polyrhythms (5, 7, 8, 11, 13-tuplets)
   - LoopA-style measure bars with Reaper-style threshold zones
   - Audio feedback: MUTE or WARNING mode
   - 4 difficulty levels (EASY, MEDIUM, HARD, EXPERT)
   - Real-time timing evaluation and statistics

### Phase 2: LiveFX System & Router Integration

- **LiveFX Module**: Real-time MIDI transformation engine
- **Scale Module**: 15 musical scales (Major, Minor, Dorian, Phrygian, etc.)
- **Router Integration**: Transform and tap hooks for MIDI processing
- **Scene Management**: 8 scenes × 4 tracks with song arrangement
- **Step Playback**: Manual cursor navigation through timeline
- **Metronome**: BPM-synchronized with count-in support

### Phase 3: Advanced Features

#### Comprehensive SD Card Configuration System

**43 Configuration Parameters** across hardware modules:
- **DIN Module** (7 params): Enable, byte count, invert logic, debounce time
- **AINSER Module** (3 params): Enable, scan interval, deadband (SPI-based)
- **AIN Module** (5 params): Enable, velocity sensing, auto-calibration
- **MIDI Settings** (2 params): Default channel, velocity curve
- **Professional Breath Controller Support**:
  - **Pressure Module** (9 params): XGZP6847D I2C sensor configuration
  - **Expression Module** (15 params): Pressure-to-CC mapping with curves
  - **Calibration Module** (5 params): Automatic sensor calibration
  - Low latency (<5ms), high resolution (24-bit ADC)
  - Bidirectional bellows support for accordion/wind instruments

#### Beatloop Visual Enhancements

- **Loop Region Markers**: 2px vertical lines with triangle indicators
- **Real-Time Playhead**: Dynamic position tracking with bright line
- **Enhanced Header**: Loop length in bars, playback state indicator
- DAW-inspired visual feedback (Reaper/LoopA style)

#### Scene Chaining & Automation

- Auto-trigger next scene when loop ends on track 0
- API: `looper_set_scene_chain()`, `looper_get_scene_chain()`, `looper_is_scene_chain_enabled()`
- Thread-safe with mutex protection
- Use cases: song arrangement, live performance, setlist mode

#### MIDI File Export

- **SMF Format 1** (multi-track) support
- VLQ delta-time encoding, tempo/time signature embedding
- API: `looper_export_midi()`, `looper_export_track_midi()`, `looper_export_scene_midi()`
- Compatible with all major DAWs (Reaper, Ableton, Logic, FL Studio, etc.)

## Enhancement Features (13 Advanced Tools)

### Performance & Optimization

1. **Tempo Tap Function**
   - Dynamic BPM detection from user taps
   - Intuitive tempo setting during live performance

2. **Undo/Redo System**
   - Multi-level recording history (configurable 3-10 levels)
   - ~72-240KB memory usage (adjustable via `LOOPER_UNDO_STACK_DEPTH`)
   - Full track state restoration

3. **Loop Quantization**
   - Auto-align MIDI events to grid
   - 5 resolutions: 1/4, 1/8, 1/16, 1/32, 1/64 notes
   - Smart rounding with event auto-sorting

4. **MIDI Clock Sync**
   - External tempo source synchronization
   - Auto-detect with ±0.1 BPM accuracy
   - Jitter filtering for stable playback

### Workflow Enhancements

5. **Track Mute/Solo Controls**
   - Individual track playback management
   - Exclusive solo mode with zero latency
   - Integrated with `looper_is_track_audible()` API

6. **Copy/Paste Functionality**
   - Duplicate scenes and tracks with clipboard system
   - Supports up to 512 events per track
   - Full scene state preservation

7. **Global Transpose**
   - Apply transpose to all tracks simultaneously
   - ±24 semitones range with note clamping (0-127)
   - ~5-15ms operation time

8. **Quick-Save Slots**
   - 8 session slots for instant save/recall
   - Full state capture: tracks, scenes, BPM, mute/solo states
   - Optional compression support (~40-60% reduction with `LOOPER_QUICKSAVE_COMPRESS`)
   - SD card persistence across power cycles

### Creative Tools

9. **Randomizer**
   - Velocity randomization (±64)
   - Timing randomization (±12 ticks for shuffle/swing)
   - Note skip probability (0-100% for sparse patterns)
   - Thread-safe random number generation

10. **Humanizer**
    - Musical humanization with groove-aware algorithms
    - Velocity humanization (0-32) with smooth curves
    - Timing humanization (0-6 ticks) with on-beat preservation
    - Intensity control (0-100%)

11. **Arpeggiator**
    - Per-track arpeggiation with 5 patterns (Up, Down, UpDown, Random, Chord)
    - Adjustable gate length (10-95%)
    - Octave range (1-4 octaves)
    - Note order: FIFO or sorted by pitch

### Hardware Integration

12. **Footswitch Mapping**
    - 8 footswitch inputs for hands-free control
    - 13 mappable actions: Play/Stop, Record, Overdub, Undo, Redo, Tap Tempo, etc.
    - 20ms debounce protection
    - SD card persistence

13. **MIDI Learn**
    - External controller mapping (up to 32 mappings)
    - Auto-detection of CC and Note messages
    - Channel filtering (omni or specific channel)
    - 10-second timeout with <1ms latency
    - Comprehensive workflow examples in documentation

## Production-Ready Code Quality

### API Consistency & Documentation

- **Comprehensive Doxygen Documentation**: All public functions with @brief, @param, @return, @note tags
- **Return Value Standards**:
  - `int` functions: 0 = success, -1 = error
  - `uint8_t` functions: Boolean (0/1) or status values
  - Type-specific functions: Valid value or default on error
- **Boundary Validation**: All APIs validate track (0-3) and scene (0-7) indices
- **API Naming Consistency**: `looper_set_*`, `looper_get_*`, `looper_is_*` patterns

### Memory Optimization

- **Configurable Undo Stack**: Adjustable via `LOOPER_UNDO_STACK_DEPTH`
  - 3 levels: ~72KB memory
  - 5 levels: ~120KB memory
  - 10 levels: ~240KB memory (default)

### Thread Safety & Error Handling

- Mutex-protected operations for all critical sections
- Volatile variables for concurrent access (e.g., randomizer seed)
- Enhanced read-modify-write patterns
- Comprehensive NULL pointer checks
- Graceful error handling with clear return codes

### Optional Performance Features

- **Quick-Save Compression**: Enable via `LOOPER_QUICKSAVE_COMPRESS` (requires ZLIB)
- **BPM Validation**: Explicit 20-300 BPM range checking
- Zero runtime overhead from documentation
- <1μs validation overhead per API call

## Hardware Requirements

- **MCU**: STM32F407VGT6 (or compatible)
- **Display**: SSD1322 256×64 grayscale OLED
- **Storage**: SD card (SDIO or SPI)
- **Optional**: 
  - XGZP6847D I2C pressure sensor (breath controller)
  - Up to 8 footswitch inputs
  - AINSER64 module (SPI)
  - DIN/AIN modules

## Getting Started

### Building the Project

```bash
# CubeMX FATFS must be enabled
# Configure SDIO or SPI for SD card
# Set up OLED display (SSD1322)

# Build configuration options in looper.h:
#define LOOPER_UNDO_STACK_DEPTH 10    // Adjust memory usage
#define LOOPER_QUICKSAVE_COMPRESS     // Enable compression (requires ZLIB)
```

### Basic Usage

```c
// Initialize looper system
looper_init();

// Start recording on track 0
looper_arm_track(0);
looper_start();

// Apply LiveFX
livefx_set_transpose(0, 5);  // +5 semitones on track 0
livefx_set_scale_enabled(0, 1);
livefx_set_scale_type(0, SCALE_MINOR);

// Scene management
looper_trigger_scene(1);  // Switch to scene B
looper_set_scene_chain(0, 1, 1);  // Auto-chain scene A → B

// Export to MIDI file
looper_export_midi("0:/loops/song.mid");

// Save session
looper_quicksave_session(0, "My Song");
```

### Testing

See [TESTING_PROTOCOL.md](Docs/testing/TESTING_PROTOCOL.md) for comprehensive testing procedures with 300+ test cases covering all features.

## Configuration Files

- **config.ngc**: Standard professional configuration
- **config_minimal.ngc**: Minimal testing setup
- **config_full.ngc**: Maximum I/O with breath controller
- All configs support 43 parameters unified in MIDIbox NG .NGC format

## API Reference

See header files for complete API documentation:
- `Services/looper/looper.h` - Main looper API (102 functions)
- `Services/livefx/livefx.h` - Real-time MIDI effects
- `Services/rhythm_trainer/rhythm_trainer.h` - Rhythm training system
- `Services/config_io/config_io.h` - Configuration parser

## License

[Add your license information here]

## Contributing

This is a comprehensive implementation with production-ready code quality. For contributions, please:
1. Follow existing API consistency standards
2. Add Doxygen documentation for all public functions
3. Include test cases in [TESTING_PROTOCOL.md](Docs/testing/TESTING_PROTOCOL.md)
4. Validate boundary conditions and error handling

## Acknowledgments

- Inspired by LoopA (UI design, rhythm subdivisions)
- DAW visual feedback inspired by Reaper
- MIDIbox NG .NGC configuration format compatibility
- Professional breath controller support for accordion/wind instruments
