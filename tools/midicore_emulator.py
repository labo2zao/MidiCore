#!/usr/bin/env python3
"""
MidiCore Emulator - MIOS Studio Terminal Tester
================================================

This script EMULATES MidiCore firmware to test MIOS Studio terminal.
It creates a virtual MIDI device that responds to queries and sends debug messages,
allowing you to verify that MIOS Studio terminal is working correctly.

Requirements:
    pip install python-rtmidi

Usage:
    python midicore_emulator.py                     # Start emulator
    python midicore_emulator.py --verbose           # Show debug info
    python midicore_emulator.py --port "Virtual"    # Use specific port

Then in MIOS Studio:
    1. Device should appear in device list
    2. Open Terminal window (View → Terminal)
    3. You should see test messages appearing

Author: GitHub Copilot
Date: 2026-01-28
"""

import sys
import time
import argparse
import threading
import platform
from typing import List, Optional

try:
    import rtmidi
except ImportError:
    print("ERROR: python-rtmidi not installed!")
    print("Install with: pip install python-rtmidi")
    sys.exit(1)


class MIOS32Protocol:
    """MIOS32 Protocol Implementation"""
    
    # MIOS32 Manufacturer ID
    MIOS32_MANUF_ID = [0x00, 0x00, 0x7E]
    
    # MIOS32 Device ID
    MIOS32_DEVICE_ID = 0x32
    
    # MIOS32 Commands
    CMD_QUERY = 0x00
    CMD_DEBUG_MESSAGE = 0x0D
    CMD_ACK_RESPONSE = 0x0F
    
    # Query Types
    QUERY_TYPE_OS = 0x01
    QUERY_TYPE_BOARD = 0x02
    QUERY_TYPE_CORE = 0x03
    QUERY_TYPE_CHIP_ID = 0x04
    QUERY_TYPE_SERIAL = 0x05
    QUERY_TYPE_FLASH = 0x06
    QUERY_TYPE_RAM = 0x07
    QUERY_TYPE_APP_NAME1 = 0x08
    QUERY_TYPE_APP_NAME2 = 0x09
    
    @classmethod
    def build_debug_message(cls, text: str) -> List[int]:
        """Build a MIOS32 debug message SysEx"""
        message = [
            0xF0,  # SysEx start
            *cls.MIOS32_MANUF_ID,
            cls.MIOS32_DEVICE_ID,
            0x00,  # Device instance 0
            cls.CMD_DEBUG_MESSAGE,
        ]
        
        # Add ASCII text
        message.extend([ord(c) for c in text])
        
        # SysEx end
        message.append(0xF7)
        
        return message
    
    @classmethod
    def build_query_response(cls, device_id: int, response_text: str) -> List[int]:
        """Build a MIOS32 query response SysEx"""
        message = [
            0xF0,  # SysEx start
            *cls.MIOS32_MANUF_ID,
            cls.MIOS32_DEVICE_ID,
            device_id,
            cls.CMD_ACK_RESPONSE,
        ]
        
        # Add ASCII text
        message.extend([ord(c) for c in response_text])
        
        # SysEx end
        message.append(0xF7)
        
        return message
    
    @classmethod
    def parse_query(cls, data: List[int]) -> Optional[tuple]:
        """Parse a MIOS32 query SysEx message"""
        if len(data) < 9:
            return None
        
        if data[0] != 0xF0 or data[-1] != 0xF7:
            return None
        
        # Check manufacturer ID
        if data[1:4] != cls.MIOS32_MANUF_ID:
            return None
        
        # Check device ID
        if data[4] != cls.MIOS32_DEVICE_ID:
            return None
        
        device_instance = data[5]
        command = data[6]
        
        if command == cls.CMD_QUERY and len(data) >= 9:
            query_type = data[7]
            return (device_instance, query_type)
        
        return None


class MidiCoreEmulator:
    """MidiCore Device Emulator"""
    
    def __init__(self, port_name: str = "MidiCore Emulator", verbose: bool = False):
        self.port_name = port_name
        self.verbose = verbose
        self.midi_in = None
        self.midi_out = None
        self.running = False
        self.message_count = 0
        
        # Device info (matching MidiCore firmware)
        self.device_info = {
            0x01: "MIOS32",           # Operating System
            0x02: "STM32F407VGT6",    # Board
            0x03: "STM32F4",          # Core Family
            0x04: "00000000",         # Chip ID (placeholder)
            0x05: "000001",           # Serial Number
            0x06: "1048576",          # Flash Size (1MB)
            0x07: "131072",           # RAM Size (128KB)
            0x08: "MidiCore",         # App Name Line 1
            0x09: "v1.0",             # App Name Line 2
        }
    
    def list_ports(self) -> List[tuple]:
        """List available MIDI ports"""
        midi_out = rtmidi.MidiOut()
        ports = []
        for i in range(midi_out.get_port_count()):
            name = midi_out.get_port_name(i)
            ports.append((i, name))
        return ports
    
    def find_port(self, pattern: str = None) -> Optional[int]:
        """Find MIDI port by name pattern"""
        ports = self.list_ports()
        
        if not ports:
            print("ERROR: No MIDI ports found!")
            print("\nOn Windows, you need to create a loopMIDI port first:")
            print("  1. Open loopMIDI application")
            print("  2. Click '+' to create a new port")
            print("  3. Name it (e.g., 'loopMIDI Port')")
            print("  4. Run this script again with: --use-existing 'loopMIDI'")
            return None
        
        # If no pattern, return first port
        if pattern is None:
            return 0
        
        # Search for matching port
        pattern_lower = pattern.lower()
        for idx, name in ports:
            if pattern_lower in name.lower():
                return idx
        
        print(f"ERROR: Port matching '{pattern}' not found!")
        print("Available ports:")
        for idx, name in ports:
            print(f"  {idx}: {name}")
        return None
    
    def start_with_existing_port(self, port_idx: int) -> bool:
        """Start emulator using existing MIDI port (e.g., loopMIDI on Windows)"""
        try:
            # Open existing input port
            self.midi_in = rtmidi.MidiIn()
            if port_idx >= self.midi_in.get_port_count():
                print(f"ERROR: Port index {port_idx} out of range!")
                return False
            
            port_name = self.midi_in.get_port_name(port_idx)
            self.midi_in.open_port(port_idx)
            self.midi_in.set_callback(self._midi_callback)
            
            # Open existing output port
            self.midi_out = rtmidi.MidiOut()
            self.midi_out.open_port(port_idx)
            
            print(f"✓ MidiCore Emulator started")
            print(f"✓ Using existing MIDI port: '{port_name}'")
            print(f"\nIn MIOS Studio:")
            print(f"  1. Device '{port_name}' should appear in device list")
            print(f"  2. Select it and click 'Query'")
            print(f"  3. Open Terminal window (View → Terminal)")
            print(f"  4. You should see test messages!\n")
            
            self.running = True
            return True
            
        except Exception as e:
            print(f"ERROR connecting to port: {e}")
            return False
    
    def start_with_virtual_port(self) -> bool:
        """Start emulator - creates virtual MIDI ports (macOS/Linux)"""
        try:
            # Create virtual MIDI INPUT port (receives queries from MIOS Studio)
            self.midi_in = rtmidi.MidiIn()
            self.midi_in.open_virtual_port(self.port_name)
            self.midi_in.set_callback(self._midi_callback)
            
            # Create virtual MIDI OUTPUT port (sends responses and debug to MIOS Studio)
            self.midi_out = rtmidi.MidiOut()
            self.midi_out.open_virtual_port(self.port_name)
            
            print(f"✓ MidiCore Emulator started")
            print(f"✓ Created virtual MIDI port: '{self.port_name}'")
            print(f"\nIn MIOS Studio:")
            print(f"  1. Device '{self.port_name}' should appear in device list")
            print(f"  2. Select it and click 'Query'")
            print(f"  3. Open Terminal window (View → Terminal)")
            print(f"  4. You should see test messages!\n")
            
            self.running = True
            return True
            
        except Exception as e:
            print(f"ERROR creating virtual port: {e}")
            print("\n" + "="*60)
            print("Virtual port creation not supported on this system!")
            print("="*60)
            print("\nOn Windows:")
            print("  1. Install and open loopMIDI")
            print("  2. Create a port (click '+')")
            print("  3. Run: python midicore_emulator.py --use-existing 'loopMIDI'")
            print("\nOn macOS:")
            print("  Virtual ports should work. Check your python-rtmidi installation.")
            print("\nOn Linux:")
            print("  Virtual ports should work. Check your python-rtmidi installation.")
            print("="*60)
            return False
    
    def stop(self):
        """Stop emulator"""
        self.running = False
        if self.midi_in:
            self.midi_in.close_port()
            self.midi_in = None
        if self.midi_out:
            self.midi_out.close_port()
            self.midi_out = None
    
    def _midi_callback(self, event, data=None):
        """MIDI input callback - handles queries from MIOS Studio"""
        message, delta_time = event
        
        if self.verbose:
            print(f"[RX] {' '.join(f'{b:02X}' for b in message)}")
        
        # Check if it's a query
        if message[0] == 0xF0:
            result = MIOS32Protocol.parse_query(message)
            
            if result:
                device_id, query_type = result
                
                print(f"[QUERY] Type=0x{query_type:02X} ({self._query_type_name(query_type)})")
                
                # Send response
                response_text = self.device_info.get(query_type, "Unknown")
                self.send_query_response(device_id, response_text)
    
    def _query_type_name(self, query_type: int) -> str:
        """Get human-readable query type name"""
        names = {
            0x01: "OS",
            0x02: "Board",
            0x03: "Core",
            0x04: "Chip ID",
            0x05: "Serial",
            0x06: "Flash",
            0x07: "RAM",
            0x08: "App Name 1",
            0x09: "App Name 2",
        }
        return names.get(query_type, f"Unknown-{query_type:02X}")
    
    def send_query_response(self, device_id: int, text: str):
        """Send query response to MIOS Studio"""
        if not self.midi_out:
            return
        
        message = MIOS32Protocol.build_query_response(device_id, text)
        
        if self.verbose:
            print(f"[TX RESPONSE] {text}")
            print(f"[TX] {' '.join(f'{b:02X}' for b in message)}")
        
        self.midi_out.send_message(message)
    
    def send_debug_message(self, text: str):
        """Send debug message to MIOS Studio terminal"""
        if not self.midi_out:
            return
        
        message = MIOS32Protocol.build_debug_message(text)
        
        if self.verbose:
            print(f"[TX DEBUG] {repr(text)}")
            print(f"[TX] {' '.join(f'{b:02X}' for b in message)}")
        else:
            print(f"[SENT] {text.rstrip()}")
        
        self.midi_out.send_message(message)
        self.message_count += 1
    
    def run_test_sequence(self):
        """Run test sequence - sends various messages to test MIOS Studio terminal"""
        print("\n" + "="*60)
        print("STARTING TEST SEQUENCE")
        print("="*60)
        print("\n⏳ Waiting 5 seconds for you to connect MIOS Studio...")
        print("   1. Open MIOS Studio")
        print("   2. Select 'MidiCore Emulator' device")
        print("   3. Click 'Query' button")
        print("   4. Open Terminal window (View → Terminal)")
        print()
        
        # Countdown
        for i in range(5, 0, -1):
            print(f"   Starting in {i} seconds...")
            time.sleep(1)
        
        print("\n" + "="*60)
        print("Sending test messages to MIOS Studio terminal...")
        print("Watch MIOS Studio Terminal window for these messages:")
        print("="*60 + "\n")
        
        # Send initial test messages
        test_messages = [
            "\r\n*** MidiCore Emulator Started ***\r\n",
            "MIOS Studio Terminal Test\r\n",
            "If you see this, terminal is WORKING!\r\n",
            "\r\n",
        ]
        
        for msg in test_messages:
            self.send_debug_message(msg)
            time.sleep(0.2)
        
        # Send numbered test messages
        for i in range(1, 6):
            self.send_debug_message(f"Test Message #{i}\r\n")
            time.sleep(0.5)
        
        self.send_debug_message("\r\n")
        self.send_debug_message("All test messages sent!\r\n")
        self.send_debug_message("If you see all 5 messages above, terminal is working perfectly.\r\n")
        self.send_debug_message("\r\n")
        
        print("\nInitial test sequence complete!")
        print(f"Sent {self.message_count} messages.")
        print("\nIf you saw the 5 numbered test messages in MIOS Studio terminal,")
        print("then terminal is WORKING! Now starting continuous test...\n")
    
    def run(self):
        """Main emulator loop"""
        # Send initial test sequence
        self.run_test_sequence()
        
        # Send continuous test messages
        test_count = 0
        try:
            print("\n" + "="*60)
            print("CONTINUOUS TEST MODE")
            print("="*60)
            print("Sending test messages continuously every 2 seconds...")
            print("Press Ctrl+C to stop.")
            print("="*60 + "\n")
            
            while self.running:
                time.sleep(2)
                test_count += 1
                
                # Alternate between different message types
                if test_count % 3 == 0:
                    self.send_debug_message(f"[Test #{test_count}] Continuous terminal test - all working!\r\n")
                elif test_count % 3 == 1:
                    self.send_debug_message(f"[Test #{test_count}] MIOS Studio terminal receiving messages OK\r\n")
                else:
                    self.send_debug_message(f"[Test #{test_count}] Heartbeat - {self.message_count} messages sent\r\n")
        
        except KeyboardInterrupt:
            print("\n\nStopping emulator...")
        
        finally:
            print(f"\nTotal messages sent: {self.message_count}")
            print(f"Test messages: {test_count}")
            print(f"Duration: ~{test_count * 2} seconds")


def main():
    parser = argparse.ArgumentParser(
        description='MidiCore Emulator - MIOS Studio Terminal Tester',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
This script EMULATES MidiCore to test MIOS Studio terminal.

IMPORTANT FOR WINDOWS USERS:
  On Windows, virtual port creation doesn't work. You MUST use loopMIDI:
  1. Install and open loopMIDI
  2. Create a port (click '+' button)
  3. Run: python midicore_emulator.py --use-existing "loopMIDI"

For macOS/Linux:
  Virtual ports work automatically:
  python midicore_emulator.py

Setup (Windows with loopMIDI):
  1. Open loopMIDI and create a port
  2. Run: python midicore_emulator.py --use-existing "loopMIDI"
  3. Wait for 5-second countdown
  4. Open MIOS Studio
  5. Device "loopMIDI Port" should appear in device list
  6. Select it and click Query
  7. Open Terminal window (View → Terminal)
  8. You should see continuous test messages!

Setup (macOS/Linux):
  1. Run: python midicore_emulator.py
  2. Wait for 5-second countdown
  3. Open MIOS Studio
  4. Device "MidiCore Emulator" should appear in device list
  5. Select it and click Query
  6. Open Terminal window (View → Terminal)
  7. You should see continuous test messages!

If messages appear → MIOS Studio terminal is WORKING
If no messages → MIOS Studio terminal has a problem

Examples:
  %(prog)s --use-existing "loopMIDI"
      Connect to existing loopMIDI port (Windows)
  
  %(prog)s --use-existing "IAC"
      Connect to existing IAC port (macOS)
  
  %(prog)s --list
      List available MIDI ports
  
  %(prog)s
      Create virtual port (macOS/Linux only)
  
  %(prog)s --verbose
      Show detailed MIDI message debug info
        """
    )
    
    parser.add_argument('--list', action='store_true',
                        help='List available MIDI ports and exit')
    parser.add_argument('--use-existing', type=str, metavar='PORT_PATTERN',
                        help='Use existing MIDI port (required for Windows/loopMIDI)')
    parser.add_argument('--name', type=str, default="MidiCore Emulator",
                        help='Virtual MIDI port name (only for macOS/Linux)')
    parser.add_argument('--verbose', action='store_true',
                        help='Show verbose debug output')
    
    args = parser.parse_args()
    
    # Create emulator
    emulator = MidiCoreEmulator(port_name=args.name, verbose=args.verbose)
    
    # List ports mode
    if args.list:
        print("\nAvailable MIDI Ports:")
        print("-" * 60)
        ports = emulator.list_ports()
        if not ports:
            print("  No ports found!")
            print("\nOn Windows: Create a loopMIDI port first")
            print("On macOS: Enable IAC Driver in Audio MIDI Setup")
            print("On Linux: Run 'sudo modprobe snd-virmidi'")
        else:
            for idx, name in ports:
                print(f"  {idx}: {name}")
        print("-" * 60)
        return 0
    
    # Determine startup mode
    if args.use_existing:
        # Explicit flag - use specified port
        port_idx = emulator.find_port(args.use_existing)
        if port_idx is None:
            return 1
        
        if not emulator.start_with_existing_port(port_idx):
            return 1
    elif platform.system() == 'Windows':
        # Windows - auto-detect loopMIDI (virtual ports don't work on Windows)
        print("✓ Detected Windows - searching for loopMIDI port...")
        port_idx = emulator.find_port("loop")
        if port_idx is None:
            print("\n" + "=" * 60)
            print("ERROR: loopMIDI port not found!")
            print("=" * 60)
            print("\nPlease:")
            print("  1. Open loopMIDI application")
            print("  2. Create a port (click '+' button)")
            print("  3. Keep loopMIDI running")
            print("  4. Run this script again")
            print("\nOr specify a different port:")
            print("  python midicore_emulator.py --use-existing 'YourPortName'")
            print("=" * 60)
            return 1
        
        if not emulator.start_with_existing_port(port_idx):
            return 1
    else:
        # macOS/Linux - try to create virtual port
        if not emulator.start_with_virtual_port():
            print("\nHint: Try --use-existing with an existing port")
            print("Example: python midicore_emulator.py --use-existing 'IAC'")
            return 1
    
    try:
        # Run test sequence
        emulator.run()
    
    finally:
        emulator.stop()
        print("\nEmulator stopped.")
    
    return 0


if __name__ == '__main__':
    sys.exit(main())
