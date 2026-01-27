# MidiCore Studio

**A JUCE-based desktop application for MidiCore accordion MIDI controllers**

Compatible with MIOS Studio protocol while providing MidiCore-specific features.

## Features

- 🎹 **MIDI Monitor**: Real-time MIDI message display (4 ports)
- 📁 **SD Card File Manager**: Edit configuration files directly via USB CDC
- 🔧 **Device Configuration**: Visual editors for .ngc/.ngp files
- 📊 **Performance Monitoring**: CPU, RAM, MIDI throughput
- 🎵 **Patch Management**: Browse and organize patches
- 🔌 **Multi-Device Support**: Connect multiple MidiCore devices
- 🌍 **Cross-Platform**: Windows, macOS, Linux

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

## Architecture

```
MidiCoreStudio/
├── Source/              # C++ source files
├── Resources/           # Assets
├── CMakeLists.txt       # Build config
└── README.md
```

## License

Same as MidiCore firmware.
