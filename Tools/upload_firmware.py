#!/usr/bin/env python3
"""
MidiCore Bootloader Firmware Upload Utility

MIOS Studio Compatible - Uses Device ID 0x40 (standard MIOS32)

Requirements: pip install python-rtmidi
Usage: python3 upload_firmware.py firmware.bin [--port PORT_INDEX]
"""

import sys
import time
import argparse

try:
    import rtmidi
except ImportError:
    print("Error: python-rtmidi required. Install: pip install python-rtmidi")
    sys.exit(1)

# Bootloader protocol constants (MIOS32 compatible)
DEVICE_ID = 0x40  # Standard MIOS32 Device ID for MIOS Studio compatibility
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
    """Build SysEx message (MIOS Studio compatible)"""
    if data is None:
        data = []
    msg = [0xF0, 0x00, 0x00, 0x7E, DEVICE_ID, command] + data
    msg.append(checksum([command] + data))
    msg.append(0xF7)
    return msg

def select_port_interactive(midi_out):
    """Interactively select MIDI port with validation"""
    ports = midi_out.get_ports()
    
    if not ports:
        print("Error: No MIDI ports available")
        return None
    
    print("\nAvailable MIDI ports:")
    for i, port in enumerate(ports):
        print(f"  {i}: {port}")
    print()
    
    while True:
        try:
            selection = input("Select port number (or 'q' to quit): ").strip()
            if selection.lower() == 'q':
                return None
            
            port_idx = int(selection)
            if 0 <= port_idx < len(ports):
                return port_idx
            else:
                print(f"Invalid port number. Please choose 0-{len(ports)-1}")
        except ValueError:
            print("Invalid input. Please enter a number or 'q' to quit.")
        except KeyboardInterrupt:
            print("\nCancelled.")
            return None
    
def upload_firmware(firmware_path, port_index=None, block_size=64, erase_delay=3.0):
    """Upload firmware to bootloader
    
    Args:
        firmware_path: Path to .bin firmware file
        port_index: MIDI port index (None for interactive)
        block_size: Block size in bytes (default 64)
        erase_delay: Delay after erase in seconds (default 3.0)
    """
    midi_out = rtmidi.MidiOut()
    
    # Select MIDI port
    if port_index is None:
        port_index = select_port_interactive(midi_out)
        if port_index is None:
            return False
    
    try:
        midi_out.open_port(port_index)
    except (rtmidi.InvalidPortError, IndexError):
        print(f"Error: Invalid port index {port_index}")
        return False
    
    print(f"Connected to: {midi_out.get_port_name(port_index)}")
    
    # Read firmware
    try:
        with open(firmware_path, 'rb') as f:
            firmware = f.read()
    except IOError as e:
        print(f"Error reading firmware file: {e}")
        midi_out.close_port()
        return False
    
    print(f"Firmware size: {len(firmware)} bytes")
    
    # Erase
    print("Erasing flash (this may take a few seconds)...")
    midi_out.send_message(build_message(CMD_ERASE_APP))
    time.sleep(erase_delay)
    
    # Upload blocks
    print(f"Uploading in {block_size}-byte blocks...")
    offset = 0
    total_blocks = (len(firmware) + block_size - 1) // block_size
    block_num = 0
    
    while offset < len(firmware):
        block = firmware[offset:offset + block_size]
        
        # Build write command
        addr = encode_u32(offset)
        length = [(len(block) >> 7) & 0x7F, len(block) & 0x7F]
        data = addr + length + list(block)
        
        # Send block
        midi_out.send_message(build_message(CMD_WRITE_BLOCK, data))
        
        offset += len(block)
        block_num += 1
        progress = 100 * offset // len(firmware)
        
        # Progress indicator
        print(f"\rProgress: {progress}% [{block_num}/{total_blocks} blocks]", end='', flush=True)
        
        # Small delay between blocks
        time.sleep(0.02)
    
    print("\n\nUpload complete!")
    
    # Jump to application
    print("Starting application...")
    midi_out.send_message(build_message(CMD_JUMP_APP))
    time.sleep(0.5)
    
    midi_out.close_port()
    return True

def main():
    parser = argparse.ArgumentParser(
        description="MidiCore Bootloader Firmware Upload Utility",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Interactive port selection
  python3 upload_firmware.py firmware.bin
  
  # Specify port index
  python3 upload_firmware.py firmware.bin --port 0
  
  # Custom block size
  python3 upload_firmware.py firmware.bin --block-size 32
        """
    )
    
    parser.add_argument("firmware", help="Firmware binary file (.bin)")
    parser.add_argument("--port", type=int, help="MIDI port index (interactive if not specified)")
    parser.add_argument("--block-size", type=int, default=64, 
                       help="Block size for upload in bytes (default: 64)")
    parser.add_argument("--erase-delay", type=float, default=3.0,
                       help="Delay after erase command in seconds (default: 3.0)")
    
    args = parser.parse_args()
    
    # Validate arguments
    if args.block_size < 1 or args.block_size > 127:
        print("Error: Block size must be between 1 and 127 bytes")
        return 1
    
    if args.erase_delay < 0:
        print("Error: Erase delay must be non-negative")
        return 1
    
    # Upload firmware
    print("MidiCore Bootloader Upload Tool v1.0")
    print("=" * 50)
    
    success = upload_firmware(
        args.firmware,
        port_index=args.port,
        block_size=args.block_size,
        erase_delay=args.erase_delay
    )
    
    if success:
        print("\n✓ Firmware update successful!")
        return 0
    else:
        print("\n✗ Firmware update failed")
        return 1

if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        print("\n\nInterrupted by user.")
        sys.exit(130)
