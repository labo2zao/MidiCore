# MidiCore Studio - Build Instructions

## Quick Build (if JUCE is installed)

```bash
cd MidiCoreStudio
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Detailed Setup

### 1. Install JUCE

**Option A: Download JUCE SDK**
```bash
# Download from https://juce.com/get-juce/download
# Extract to ~/JUCE (or your preferred location)
```

**Option B: Use Package Manager** (if available)
```bash
# macOS with Homebrew
brew install juce

# Ubuntu/Debian
sudo apt install juce-tools juce-modules-source
```

### 2. Set JUCE_PATH (if not system-installed)

```bash
export JUCE_PATH=~/JUCE
```

Or set in CMakeLists.txt:
```cmake
set(JUCE_PATH "/path/to/JUCE")
```

### 3. Install Dependencies

**Ubuntu/Debian:**
```bash
sudo apt install build-essential cmake libfreetype6-dev \
    libx11-dev libxinerama-dev libxrandr-dev libxcursor-dev \
    mesa-common-dev libasound2-dev libcurl4-openssl-dev libwebkit2gtk-4.0-dev \
    libgtk-3-dev
```

**macOS:**
```bash
# Xcode Command Line Tools required
xcode-select --install
```

**Windows:**
- Visual Studio 2019+ with C++ Desktop Development
- CMake 3.15+

### 4. Build

```bash
cd MidiCoreStudio
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### 5. Run

**Linux:**
```bash
./build/MidiCoreStudio_artefacts/Release/MidiCoreStudio
```

**macOS:**
```bash
open build/MidiCoreStudio_artefacts/Release/MidiCoreStudio.app
```

**Windows:**
```
build\MidiCoreStudio_artefacts\Release\MidiCoreStudio.exe
```

## Development Build

For debugging:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
```

## IDE Integration

### Visual Studio Code
```bash
code .
# Install CMake Tools extension
# Select kit and build
```

### CLion
```
Open project from CMakeLists.txt
```

### Xcode
```bash
cmake -B build -G Xcode
open build/MidiCoreStudio.xcodeproj
```

### Visual Studio
```bash
cmake -B build -G "Visual Studio 17 2022"
start build/MidiCoreStudio.sln
```

## Troubleshooting

### JUCE not found
```
CMake Error: Could not find a package configuration file provided by "JUCE"
```
**Solution**: Set JUCE_PATH environment variable or install JUCE system-wide

### Missing dependencies (Linux)
```
fatal error: 'alsa/asoundlib.h' file not found
```
**Solution**: Install ALSA development libraries:
```bash
sudo apt install libasound2-dev
```

### C++17 not supported
```
error: unknown type name 'std::filesystem'
```
**Solution**: Update compiler to GCC 8+, Clang 10+, or MSVC 2019+

## CI/CD Build

GitHub Actions example:
```yaml
- name: Install JUCE
  run: |
    wget https://github.com/juce-framework/JUCE/releases/download/7.0.5/juce-7.0.5-linux.zip
    unzip juce-7.0.5-linux.zip
    echo "JUCE_PATH=$(pwd)/JUCE" >> $GITHUB_ENV
    
- name: Build
  run: |
    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build
```

## Distribution

### Create Installer (Windows)
Use Inno Setup or WiX Toolset

### Create DMG (macOS)
```bash
hdiutil create -volname "MidiCore Studio" -srcfolder build/MidiCoreStudio_artefacts/Release/MidiCoreStudio.app -ov -format UDZO MidiCoreStudio.dmg
```

### Create AppImage (Linux)
Use linuxdeploy with Qt plugin
