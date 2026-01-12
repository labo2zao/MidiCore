# USB MIDI Bootloader for MidiCore

## Overview

The MidiCore bootloader enables fast firmware updates via USB MIDI using the MIOS32-compatible SysEx protocol. This provides a user-friendly way to update the firmware without opening the device or using JTAG/SWD programmers.

## Features

- **Fast USB MIDI Transfer**: 50-100x faster than traditional DIN MIDI (~ 1 MB in 30-60 seconds)
- **MIOS32-Compatible Protocol**: Uses standard SysEx messages for wide tool compatibility
- **Safe Updates**: CRC verification and flash verification ensure data integrity
- **Flexible Entry**: Multiple ways to enter bootloader mode
- **Fallback Protection**: Stays in bootloader if no valid application detected

## Memory Layout

The STM32F407VG flash is partitioned as follows:

```
0x08000000 ┌─────────────────────┐
           │   Bootloader        │ 32 KB (Sectors 0-1)
0x08008000 ├─────────────────────┤
           │                     │
           │   Application       │ 992 KB (Sectors 2-11)
           │                     │
0x08100000 └─────────────────────┘
```

## Entering Bootloader Mode

### Method 1: From Application (Recommended)
```c
#include "Services/bootloader/bootloader.h"

// Request bootloader entry and reset
bootloader_request_entry();
// System will reset and enter bootloader
```

### Method 2: No Valid Application
If the bootloader detects no valid application in flash (e.g., after erasing or corrupted firmware), it automatically enters bootloader mode.

### Method 3: Button Press (Hardware-Specific)
Hold a designated button during power-on or reset (requires hardware-specific implementation).

## SysEx Protocol

### Message Format

All bootloader SysEx messages follow this structure:

```
F0 00 00 7E 4E <command> <data...> <checksum> F7
│  └──┬───┘ │  └────┬────┘ └───┬───┘ └───┬──┘ │
│   MfgID  Dev     Cmd       Data      CS   End
│          ID
Start
```

- **Manufacturer ID**: `00 00 7E` (Universal Non-Realtime, MIDIbox)
- **Device ID**: `4E` ('N' for MIDIbox/MIOS32)
- **Checksum**: 7-bit two's complement checksum of data from command onwards

### Commands

#### 1. Query Bootloader Info (0x01)

**Request:**
```
F0 00 00 7E 4E 01 <checksum> F7
```

**Response:**
```
F0 00 00 7E 4E 0F 01 <ver_major> <ver_minor> <ver_patch> <flash_size:5> <app_addr:5> <checksum> F7
```

Example:
```python
# Query bootloader version and configuration
send_sysex([0xF0, 0x00, 0x00, 0x7E, 0x4E, 0x01, checksum, 0xF7])
```

#### 2. Erase Application (0x04)

**Request:**
```
F0 00 00 7E 4E 04 <checksum> F7
```

**Response:**
```
F0 00 00 7E 4E 0F 04 <address:5> <checksum> F7  # Success
F0 00 00 7E 4E 0E 04 <error_code> <checksum> F7  # Error
```

Example:
```python
# Erase application flash before uploading new firmware
send_sysex([0xF0, 0x00, 0x00, 0x7E, 0x4E, 0x04, checksum, 0xF7])
```

#### 3. Write Block (0x02)

**Request:**
```
F0 00 00 7E 4E 02 <address:5> <length:2> <data...> <checksum> F7
```

- **Address**: 32-bit offset from application start (0x08008000), encoded in 5 bytes of 7-bit data
- **Length**: 16-bit data length, encoded in 2 bytes of 7-bit data (max 127 bytes per block)
- **Data**: Binary firmware data (7-bit safe: MSB=0 for each byte)

**Response:**
```
F0 00 00 7E 4E 0F 02 <address:5> <checksum> F7  # Success
F0 00 00 7E 4E 0E 02 <error_code> <checksum> F7  # Error
```

Example:
```python
# Write 64 bytes at offset 0x0000
address_bytes = encode_u32(0x00000000)  # [0x00, 0x00, 0x00, 0x00, 0x00]
length_bytes = [(64 >> 7) & 0x7F, 64 & 0x7F]  # [0x00, 0x40]
data = [0x00, 0x01, 0x02, ...]  # 64 bytes of firmware
send_sysex([0xF0, 0x00, 0x00, 0x7E, 0x4E, 0x02] + address_bytes + length_bytes + data + [checksum, 0xF7])
```

#### 4. Jump to Application (0x05)

**Request:**
```
F0 00 00 7E 4E 05 <checksum> F7
```

**Response:**
```
F0 00 00 7E 4E 0F 05 <address:5> <checksum> F7  # Success (then jumps)
```

Example:
```python
# After successful firmware upload, jump to new application
send_sysex([0xF0, 0x00, 0x00, 0x7E, 0x4E, 0x05, checksum, 0xF7])
```

### Error Codes

| Code | Name | Description |
|------|------|-------------|
| 0x00 | NONE | No error |
| 0x01 | INVALID_CMD | Unknown or unsupported command |
| 0x02 | INVALID_LEN | Invalid message or data length |
| 0x03 | INVALID_ADDR | Address out of range |
| 0x04 | WRITE_FAILED | Flash write operation failed |
| 0x05 | ERASE_FAILED | Flash erase operation failed |
| 0x06 | VERIFY_FAILED | Flash verification failed |
| 0x07 | CHECKSUM | Checksum mismatch |

## Data Encoding

### 32-bit Values (5 bytes)

32-bit values are encoded into 5 bytes of 7-bit MIDI data:

```c
void encode_u32(uint32_t value, uint8_t* output) {
  output[0] = (value >> 28) & 0x0F;  // Top 4 bits
  output[1] = (value >> 21) & 0x7F;  // Bits 21-27
  output[2] = (value >> 14) & 0x7F;  // Bits 14-20
  output[3] = (value >> 7) & 0x7F;   // Bits 7-13
  output[4] = value & 0x7F;          // Bits 0-6
}
```

### Checksum Calculation

The checksum is a 7-bit two's complement sum:

```c
uint8_t checksum(const uint8_t* data, uint32_t len) {
  uint8_t sum = 0;
  for (uint32_t i = 0; i < len; i++) {
    sum += data[i];
  }
  return (-sum) & 0x7F;  // Two's complement, 7-bit
}
```

Calculate the checksum over all bytes from the command byte up to (but not including) the checksum byte itself.

## Firmware Update Workflow

### 1. Connect to Device
- Connect USB cable
- Device should enumerate as USB MIDI device
- Open MIDI connection to bootloader

### 2. Enter Bootloader Mode
- From application: Send bootloader entry command via SysEx, or call `bootloader_request_entry()`
- From cold boot: Hold button or have no valid application

### 3. Query Bootloader Info (Optional)
```
Send: F0 00 00 7E 4E 01 <CS> F7
Recv: F0 00 00 7E 4E 0F 01 <version> <flash_size> <app_addr> <CS> F7
```

### 4. Erase Application Flash
```
Send: F0 00 00 7E 4E 04 <CS> F7
Recv: F0 00 00 7E 4E 0F 04 <addr> <CS> F7
```

### 5. Upload Firmware Blocks
For each block of firmware (typically 64-128 bytes):
```
Send: F0 00 00 7E 4E 02 <addr:5> <len:2> <data...> <CS> F7
Recv: F0 00 00 7E 4E 0F 02 <addr:5> <CS> F7
```

Repeat until entire firmware is uploaded.

### 6. Jump to Application
```
Send: F0 00 00 7E 4E 05 <CS> F7
Recv: F0 00 00 7E 4E 0F 05 <addr:5> <CS> F7
```

Device will restart and run the new application.

## Example Python Script

```python
#!/usr/bin/env python3
"""
Simple bootloader firmware update script
Requires: python-rtmidi
"""

import rtmidi
import struct
import time

def encode_u32(value):
    """Encode 32-bit value to 5 bytes of 7-bit data"""
    return [
        (value >> 28) & 0x0F,
        (value >> 21) & 0x7F,
        (value >> 14) & 0x7F,
        (value >> 7) & 0x7F,
        value & 0x7F
    ]

def checksum(data):
    """Calculate 7-bit checksum"""
    return (-(sum(data))) & 0x7F

def send_bootloader_sysex(midi_out, command, data=[]):
    """Send bootloader SysEx message"""
    msg = [0xF0, 0x00, 0x00, 0x7E, 0x4E, command] + data
    msg.append(checksum(msg[5:]))  # Checksum from command onwards
    msg.append(0xF7)
    midi_out.send_message(msg)

def upload_firmware(midi_out, firmware_path):
    """Upload firmware to bootloader"""
    # Read firmware file
    with open(firmware_path, 'rb') as f:
        firmware = f.read()
    
    print(f"Firmware size: {len(firmware)} bytes")
    
    # Erase application
    print("Erasing flash...")
    send_bootloader_sysex(midi_out, 0x04)
    time.sleep(2)  # Wait for erase (can take time)
    
    # Upload in blocks
    block_size = 64
    offset = 0
    
    while offset < len(firmware):
        block = firmware[offset:offset + block_size]
        
        # Build write command
        addr_bytes = encode_u32(offset)
        len_bytes = [(len(block) >> 7) & 0x7F, len(block) & 0x7F]
        data = addr_bytes + len_bytes + list(block)
        
        send_bootloader_sysex(midi_out, 0x02, data)
        
        offset += len(block)
        print(f"Progress: {offset}/{len(firmware)} bytes ({100*offset//len(firmware)}%)")
        
        time.sleep(0.01)  # Small delay between blocks
    
    print("Upload complete!")
    
    # Jump to application
    print("Starting application...")
    send_bootloader_sysex(midi_out, 0x05)

if __name__ == "__main__":
    import sys
    
    if len(sys.argv) < 2:
        print("Usage: python upload_firmware.py <firmware.bin>")
        sys.exit(1)
    
    # Open MIDI port
    midi_out = rtmidi.MidiOut()
    ports = midi_out.get_ports()
    
    print("Available MIDI ports:")
    for i, port in enumerate(ports):
        print(f"  {i}: {port}")
    
    # Select MidiCore bootloader port
    port_idx = int(input("Select port: "))
    midi_out.open_port(port_idx)
    
    # Upload firmware
    upload_firmware(midi_out, sys.argv[1])
    
    midi_out.close_port()
```

## Integration with Application

### Enable Bootloader Module

In `Config/module_config.h`:
```c
#define MODULE_ENABLE_BOOTLOADER 1
```

### Request Bootloader Entry

From your application code:
```c
#include "Services/bootloader/bootloader.h"

void enter_bootloader_via_button(void) {
    // User pressed bootloader entry button
    bootloader_request_entry();
    // System will reset into bootloader
}
```

### Handle Bootloader SysEx in Application (Optional)

If you want to support bootloader entry via SysEx from running application:
```c
#include "Services/bootloader/bootloader.h"

void handle_sysex_message(const uint8_t* data, uint32_t len) {
    // Check if it's a bootloader command
    if (len >= 6 && 
        data[1] == 0x00 && data[2] == 0x00 && 
        data[3] == 0x7E && data[4] == 0x4E) {
        // Bootloader SysEx detected
        bootloader_request_entry();
    }
}
```

## Linker Script Modifications

To support the bootloader, the application linker script must be modified to start at `0x08008000` instead of `0x08000000`.

### Application Linker Script (STM32F407VGTX_FLASH_APP.ld)

```ld
MEMORY
{
  CCMRAM    (xrw)    : ORIGIN = 0x10000000,   LENGTH = 64K
  RAM       (xrw)    : ORIGIN = 0x20000000,   LENGTH = 128K
  FLASH     (rx)     : ORIGIN = 0x8008000,    LENGTH = 992K  /* Start at 32KB offset */
}
```

### Bootloader Linker Script (STM32F407VGTX_FLASH_BOOT.ld)

```ld
MEMORY
{
  CCMRAM    (xrw)    : ORIGIN = 0x10000000,   LENGTH = 64K
  RAM       (xrw)    : ORIGIN = 0x20000000,   LENGTH = 128K
  FLASH     (rx)     : ORIGIN = 0x8000000,    LENGTH = 32K   /* Bootloader area */
}
```

## Building

### Build Application for Bootloader

1. Modify linker script to start at 0x08008000
2. Rebuild application
3. Convert ELF to binary: `arm-none-eabi-objcopy -O binary app.elf app.bin`

### Build Standalone Bootloader

1. Create bootloader project with linker script at 0x08000000
2. Include bootloader sources
3. Build bootloader
4. Flash bootloader via JTAG/SWD (one-time)

## MIOS32 Compatibility

This bootloader is designed to be compatible with MIOS32 tools and workflows:

- **Protocol**: Uses MIOS32 SysEx message structure
- **Transfer Speed**: Similar USB MIDI performance (50-100x faster than DIN)
- **Memory Layout**: Compatible memory partitioning approach
- **Tools**: Can be adapted to work with MIOS Studio or similar tools

## Troubleshooting

### Device Doesn't Enter Bootloader

1. Check that bootloader is properly flashed at 0x08000000
2. Verify reset happens after calling `bootloader_request_entry()`
3. Check if button entry method is properly configured

### Flash Write Errors

1. Ensure application area is erased before writing
2. Verify data length is multiple of 4 bytes (word aligned)
3. Check that address is within valid application range

### Verification Failures

1. Ensure data is correctly encoded (7-bit safe)
2. Check for transmission errors (try lower block sizes)
3. Verify checksum calculation matches

### Application Won't Start

1. Verify complete firmware was uploaded
2. Check that vector table is correctly placed at 0x08008000
3. Ensure application linker script uses correct origin

## References

- MIOS32 Bootloader: http://www.ucapps.de/mios32_bootstrap.html
- STM32F4 Flash Programming: ST AN2606
- USB MIDI Class Specification: USB.org

## License

This bootloader is part of MidiCore and follows the project's license.

## Version History

- v1.0.0 (2026-01-12): Initial implementation
  - MIOS32-compatible SysEx protocol
  - USB MIDI support
  - Fast firmware updates
  - Safe flash operations with verification
