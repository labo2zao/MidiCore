#!/usr/bin/env python3
"""
MidiCore Bootloader Firmware Upload Utility

Requirements: pip install python-rtmidi
Usage: python3 upload_firmware.py firmware.bin
"""

import sys
import time

try:
    import rtmidi
except ImportError:
    print("Error: python-rtmidi required. Install: pip install python-rtmidi")
    sys.exit(1)

# Bootloader protocol constants
CMD_QUERY = 0x01
CMD_WRITE_BLOCK = 0x02
CMD_ERASE_APP = 0x04
CMD_JUMP_APP = 0x05

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

def build_message(command, data=None):
    """Build SysEx message"""
    if data is None:
        data = []
    msg = [0xF0, 0x00, 0x00, 0x7E, 0x4E, command] + data
    msg.append(checksum([command] + data))
    msg.append(0xF7)
    return msg

def upload_firmware(firmware_path):
    """Upload firmware to bootloader"""
    midi_out = rtmidi.MidiOut()
    
    # Select MIDI port
    ports = midi_out.get_ports()
    if not ports:
        print("No MIDI ports available")
        return False
    
    print("MIDI ports:")
    for i, port in enumerate(ports):
        print(f"  {i}: {port}")
    
    port_idx = int(input("Select port: "))
    midi_out.open_port(port_idx)
    
    # Read firmware
    with open(firmware_path, 'rb') as f:
        firmware = f.read()
    
    print(f"Firmware: {len(firmware)} bytes")
    
    # Erase
    print("Erasing...")
    midi_out.send_message(build_message(CMD_ERASE_APP))
    time.sleep(2)
    
    # Upload blocks
    block_size = 64
    offset = 0
    while offset < len(firmware):
        block = firmware[offset:offset + block_size]
        addr = encode_u32(offset)
        length = [(len(block) >> 7) & 0x7F, len(block) & 0x7F]
        data = addr + length + list(block)
        midi_out.send_message(build_message(CMD_WRITE_BLOCK, data))
        offset += len(block)
        print(f"\rProgress: {100*offset//len(firmware)}%", end='')
        time.sleep(0.02)
    
    print("\nDone! Starting app...")
    midi_out.send_message(build_message(CMD_JUMP_APP))
    midi_out.close_port()
    return True

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 upload_firmware.py firmware.bin")
        sys.exit(1)
    upload_firmware(sys.argv[1])
