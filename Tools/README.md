# MidiCore Tools

## Diagnostic Tools

### MIDI Loopback Test (`test_midi_loopback.py`)

**NEW:** Simple diagnostic tool to test if USB MIDI is working at all.

Tests three things:
1. **Simple SysEx** - Basic USB MIDI RX/TX test
2. **Note On/Off** - MIDI routing test
3. **MIOS32 Query** - Query handler test

**Usage:**
```bash
pip install python-rtmidi
python3 test_midi_loopback.py
```

**When to use:**
- MIOS32 queries are not responding
- Need to isolate if issue is USB MIDI or query handler
- Want to verify firmware is actually running

**Results:**
- Simple SysEx FAILS → USB driver/hardware issue
- Simple SysEx PASS but MIOS32 FAILS → Firmware doesn't have query handler (need to flash)
- All PASS → Firmware is correct, issue elsewhere

### MIOS32 Recognition Test (`test_mios32_recognition.py`)

Tests all 9 MIOS32 query types (as used by MIOS Studio).

**Usage:**
```bash
pip install python-rtmidi
python3 test_mios32_recognition.py
```

### CDC Terminal Test (`test_cdc_terminal.py`)

Tests USB CDC (Virtual COM port) communication.

**Usage:**
```bash
pip install pyserial
python3 test_cdc_terminal.py
```

## Firmware Upload Tool

The `upload_firmware.py` script allows you to upload new firmware to MidiCore devices via USB MIDI using the bootloader.

### Installation

```bash
pip install python-rtmidi
```

### Usage

1. Connect your MidiCore device via USB
2. Enter bootloader mode (see README_BOOTLOADER.md)
3. Run the upload script:

```bash
python3 upload_firmware.py path/to/firmware.bin
```

The script will:
- List available MIDI ports
- Erase the application flash
- Upload the firmware in blocks
- Start the new application

### Example

```bash
$ python3 upload_firmware.py MidiCore_v2.0.bin
MIDI ports:
  0: MidiCore Bootloader
  1: Other MIDI Device
Select port: 0
Firmware: 245760 bytes
Erasing...
Progress: 100%
Done! Starting app...
```

## Building Firmware for Bootloader

To build firmware that works with the bootloader:

### Option 1: Using STM32CubeIDE

1. Open the project in STM32CubeIDE
2. In Project Properties → C/C++ Build → Settings
3. Under MCU GCC Linker → General:
   - Change Linker Script to: `../STM32F407VGTX_FLASH_APP.ld`
4. Build the project
5. Convert ELF to BIN:
   ```bash
   arm-none-eabi-objcopy -O binary Debug/MidiCore.elf firmware.bin
   ```

### Option 2: Using Makefile

If using a Makefile-based build:

1. Set `LDSCRIPT = STM32F407VGTX_FLASH_APP.ld` in Makefile
2. Build: `make`
3. The .bin file will be generated automatically

## Memory Layout

When using the bootloader:

```
Flash Memory Map:
0x08000000 ├─────────────────────┐
           │   Bootloader        │ 32 KB
0x08008000 ├─────────────────────┤
           │   Application       │ 992 KB
           │   (Your firmware)   │
0x08100000 └─────────────────────┘
```

The application linker script (`STM32F407VGTX_FLASH_APP.ld`) ensures your firmware starts at the correct address (0x08008000).

## Troubleshooting

### "No MIDI ports available"

- Ensure the device is connected via USB
- Check that the device is in bootloader mode
- On Linux, you may need to add udev rules for USB MIDI access

### Upload fails or hangs

- Try reducing block size in the script (default is 64 bytes)
- Ensure stable USB connection
- Reset the device and try again

### Application doesn't start after upload

- Verify you used the correct linker script (APP version)
- Check that the firmware binary is valid
- Try erasing and uploading again

## Advanced Usage

### Custom Block Size

Edit `upload_firmware.py` and change `block_size = 64` to a smaller value (e.g., 32) if experiencing upload issues.

### Verification

The bootloader automatically verifies each block after writing. If verification fails, the bootloader will send an error response and the upload will stop.

## See Also

- [README_BOOTLOADER.md](../README_BOOTLOADER.md) - Complete bootloader documentation
- [MIOS32 Documentation](http://www.ucapps.de/mios32.html) - Original MIOS32 reference
