# MidiCore Build Modes - Bootloader Configuration

## Overview

MidiCore supports three different build modes to accommodate different deployment scenarios:

1. **Full Project Build (Mode 0)** - Single ELF with no bootloader separation
2. **Bootloader Only Build (Mode 2)** - Only the bootloader (32KB)
3. **Application Only Build (Mode 3)** - Only the application (992KB)

## Build Modes

### Mode 0: Full Project (Default)

**Use Case**: Development, testing, or when bootloader functionality is not needed.

**Configuration**:
- **Linker Script**: `STM32F407VGTX_FLASH.ld`
- **BOOTLOADER_MODE**: `0` (or `BOOTLOADER_MODE_FULL`)
- **Flash Layout**: 
  - Origin: 0x08000000
  - Length: 1024KB (full flash)
- **Result**: Single `.elf` file containing all code

**Setup**:
```c
// In Config/module_config.h or build settings
#define BOOTLOADER_MODE 0
```

**Build Command** (STM32CubeIDE):
1. Select "Debug" or "Release" configuration
2. Ensure linker script is set to `STM32F407VGTX_FLASH.ld`
3. Build Project

---

### Mode 2: Bootloader Only

**Use Case**: Building the bootloader separately for initial programming via JTAG/SWD.

**Configuration**:
- **Linker Script**: `STM32F407VGTX_FLASH_BOOT.ld`
- **BOOTLOADER_MODE**: `2` (or `BOOTLOADER_MODE_BOOTLOADER_ONLY`)
- **Flash Layout**: 
  - Origin: 0x08000000
  - Length: 32KB (bootloader area only)
- **Result**: Bootloader `.elf` file (small, ~32KB max)

**Setup**:
```c
// In Config/module_config.h or build settings
#define BOOTLOADER_MODE 2
```

**Build Command** (STM32CubeIDE):
1. Create new build configuration: "Bootloader"
2. Set linker script to `STM32F407VGTX_FLASH_BOOT.ld`
3. Add preprocessor define: `BOOTLOADER_MODE=2`
4. Build Project

**Flash Programming**:
```bash
# Via STM32CubeProgrammer or OpenOCD
st-flash write MidiCore_Bootloader.bin 0x08000000
```

---

### Mode 3: Application Only

**Use Case**: Building application for upload via USB MIDI bootloader.

**Configuration**:
- **Linker Script**: `STM32F407VGTX_FLASH_APP.ld`
- **BOOTLOADER_MODE**: `3` (or `BOOTLOADER_MODE_APP_ONLY`)
- **Flash Layout**: 
  - Origin: 0x08008000 (32KB offset)
  - Length: 992KB (application area)
- **Result**: Application `.elf` file that expects bootloader at 0x08000000

**Setup**:
```c
// In Config/module_config.h or build settings
#define BOOTLOADER_MODE 3
```

**Build Command** (STM32CubeIDE):
1. Create new build configuration: "Application"
2. Set linker script to `STM32F407VGTX_FLASH_APP.ld`
3. Add preprocessor define: `BOOTLOADER_MODE=3`
4. Build Project

**Firmware Upload**:
```bash
# Convert ELF to binary
arm-none-eabi-objcopy -O binary MidiCore_App.elf MidiCore_App.bin

# Upload via USB MIDI bootloader
python3 Tools/upload_firmware.py MidiCore_App.bin
```

---

## Memory Layout

The STM32F407VG has 1MB (1024KB) of flash memory, partitioned as follows:

```
┌─────────────────────────────────────────────┐
│ 0x08000000 - 0x08007FFF (32KB)              │
│ Bootloader Area (Sectors 0-1)               │
│ - Bootloader code                           │
│ - USB MIDI firmware update protocol         │
│ - Application validation & jump             │
├─────────────────────────────────────────────┤
│ 0x08008000 - 0x080FFFFF (992KB)             │
│ Application Area (Sectors 2-11)             │
│ - Main application code                     │
│ - RTOS, middleware, services                │
│ - User interface, MIDI processing           │
└─────────────────────────────────────────────┘
```

---

## Linker Scripts Reference

### `STM32F407VGTX_FLASH.ld` (Mode 0)
- **Purpose**: Full project build
- **FLASH Origin**: 0x08000000
- **FLASH Length**: 1024K
- **Use**: Development, testing, non-bootloader builds

### `STM32F407VGTX_FLASH_BOOT.ld` (Mode 2)
- **Purpose**: Bootloader-only build
- **FLASH Origin**: 0x08000000
- **FLASH Length**: 32K
- **Use**: Building bootloader for initial programming

### `STM32F407VGTX_FLASH_APP.ld` (Mode 3)
- **Purpose**: Application-only build
- **FLASH Origin**: 0x08008000 (offset by 32KB)
- **FLASH Length**: 992K
- **Use**: Building application for bootloader upload

---

## Setting Up Build Configurations in STM32CubeIDE

### Step 1: Create New Build Configurations

1. Right-click project → **Properties**
2. Go to **C/C++ Build** → **Manage Configurations**
3. Click **New...** to create new configurations:
   - Name: "Bootloader" (based on Debug)
   - Name: "Application" (based on Debug)

### Step 2: Configure "Bootloader" Build

1. Select **Bootloader** configuration
2. Go to **C/C++ Build** → **Settings** → **MCU GCC Linker** → **General**
3. Set **Linker Script** to: `${workspace_loc:/${ProjName}/STM32F407VGTX_FLASH_BOOT.ld}`
4. Go to **MCU GCC Compiler** → **Preprocessor**
5. Add define: `BOOTLOADER_MODE=2`

### Step 3: Configure "Application" Build

1. Select **Application** configuration
2. Go to **C/C++ Build** → **Settings** → **MCU GCC Linker** → **General**
3. Set **Linker Script** to: `${workspace_loc:/${ProjName}/STM32F407VGTX_FLASH_APP.ld}`
4. Go to **MCU GCC Compiler** → **Preprocessor**
5. Add define: `BOOTLOADER_MODE=3`

### Step 4: Build

1. Select desired configuration from dropdown (Debug/Release/Bootloader/Application)
2. Build project
3. Output `.elf` and `.bin` files will be in respective build directories

---

## Verification & Testing

### Verify Linker Scripts Have No Conflicts

All three linker scripts are designed to work without conflicts:

- **Mode 0 (Full)**: Uses entire 1024KB flash from 0x08000000
- **Mode 2 (Bootloader)**: Uses only first 32KB from 0x08000000
- **Mode 3 (Application)**: Uses 992KB from 0x08008000 (offset)

**No memory overlap** between Mode 2 and Mode 3 builds.

### Test Build Modes

```bash
# Clean all builds
make clean

# Test Mode 0 (Full)
# - Set BOOTLOADER_MODE=0
# - Use STM32F407VGTX_FLASH.ld
# - Build and verify size < 1024KB

# Test Mode 2 (Bootloader)
# - Set BOOTLOADER_MODE=2
# - Use STM32F407VGTX_FLASH_BOOT.ld
# - Build and verify size < 32KB

# Test Mode 3 (Application)
# - Set BOOTLOADER_MODE=3
# - Use STM32F407VGTX_FLASH_APP.ld
# - Build and verify size < 992KB
```

### Verify Binary Addresses

```bash
# Check bootloader starts at 0x08000000
arm-none-eabi-objdump -h MidiCore_Bootloader.elf | grep -E "\.text|\.isr_vector"

# Check application starts at 0x08008000
arm-none-eabi-objdump -h MidiCore_App.elf | grep -E "\.text|\.isr_vector"
```

---

## Deployment Workflow

### Initial Device Programming (One-Time Setup)

1. **Build Bootloader** (Mode 2):
   ```bash
   # Select "Bootloader" configuration
   # Build project
   ```

2. **Program Bootloader via JTAG/SWD**:
   ```bash
   st-flash write Debug_Bootloader/MidiCore.bin 0x08000000
   ```

3. **Build Application** (Mode 3):
   ```bash
   # Select "Application" configuration
   # Build project
   ```

4. **Program Application via JTAG/SWD** (first time):
   ```bash
   st-flash write Debug_Application/MidiCore.bin 0x08008000
   ```

### Subsequent Updates (USB MIDI)

After initial programming, all future application updates can be done via USB MIDI:

1. **Build Application** (Mode 3)
2. **Convert to Binary**:
   ```bash
   arm-none-eabi-objcopy -O binary MidiCore.elf MidiCore.bin
   ```
3. **Upload via USB MIDI**:
   ```bash
   python3 Tools/upload_firmware.py MidiCore.bin
   ```

---

## Troubleshooting

### Build Fails: "Flash Overflow"

**Cause**: Binary size exceeds flash allocation for selected mode.

**Solution**:
- Mode 2 (Bootloader): Reduce bootloader code, ensure < 32KB
- Mode 3 (Application): Reduce application code, ensure < 992KB
- Mode 0 (Full): Reduce total code, ensure < 1024KB

### Application Doesn't Start After Bootloader

**Cause**: Application built with wrong linker script or mode.

**Solution**:
- Verify Mode 3 build uses `STM32F407VGTX_FLASH_APP.ld`
- Verify `BOOTLOADER_MODE=3` is defined
- Check vector table starts at 0x08008000

### Bootloader Can't Jump to Application

**Cause**: Application not present at 0x08008000 or invalid.

**Solution**:
- Verify application is programmed at correct address
- Check application linker script origin is 0x08008000
- Use bootloader query command to check status

---

## Summary

| Mode | Name | Linker Script | Flash Origin | Flash Size | Define |
|------|------|---------------|--------------|------------|--------|
| 0 | Full Project | `STM32F407VGTX_FLASH.ld` | 0x08000000 | 1024KB | `BOOTLOADER_MODE=0` |
| 2 | Bootloader Only | `STM32F407VGTX_FLASH_BOOT.ld` | 0x08000000 | 32KB | `BOOTLOADER_MODE=2` |
| 3 | Application Only | `STM32F407VGTX_FLASH_APP.ld` | 0x08008000 | 992KB | `BOOTLOADER_MODE=3` |

**All linker scripts are validated and have no conflicts.**

For more information about the bootloader protocol and USB MIDI firmware updates, see:
- `README_BOOTLOADER.md` - Complete bootloader documentation
- `Tools/README.md` - Firmware upload tool documentation
