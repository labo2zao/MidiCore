#!/usr/bin/env python3
"""
Simple MIDI Loopback Test

Tests if USB MIDI is working by sending simple MIDI messages and checking for any response.
This helps isolate whether the issue is with USB MIDI or specifically with MIOS32 queries.

Requirements:
    pip install python-rtmidi

Usage:
    python3 test_midi_loopback.py
"""

import rtmidi
import time
import sys

class Colors:
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    CYAN = '\033[96m'
    BOLD = '\033[1m'
    END = '\033[0m'

def print_header(text):
    print(f"\n{Colors.CYAN}{Colors.BOLD}{'='*70}")
    print(f"{text:^70}")
    print(f"{'='*70}{Colors.END}\n")

def print_success(text):
    print(f"{Colors.GREEN}✓ {text}{Colors.END}")

def print_error(text):
    print(f"{Colors.RED}✗ {text}{Colors.END}")

def print_info(text):
    print(f"{Colors.BLUE}ℹ {text}{Colors.END}")

def bytes_to_hex(data):
    return ' '.join(f'{b:02x}' for b in data)

def list_ports(midi_out, midi_in):
    """List available MIDI ports"""
    print_header("Available MIDI Ports")
    
    out_ports = midi_out.get_ports()
    in_ports = midi_in.get_ports()
    
    print(f"{Colors.BOLD}Output Ports:{Colors.END}")
    for i, port in enumerate(out_ports):
        print(f"  {i}: {port}")
    
    print(f"\n{Colors.BOLD}Input Ports:{Colors.END}")
    for i, port in enumerate(in_ports):
        print(f"  {i}: {port}")
    
    return out_ports, in_ports

def find_midicore(out_ports, in_ports):
    """Auto-detect MidiCore"""
    keywords = ['MidiCore', 'midicore', 'MIDICORE']
    
    out_idx = None
    in_idx = None
    
    for i, port in enumerate(out_ports):
        if any(kw in port for kw in keywords):
            out_idx = i
            break
    
    for i, port in enumerate(in_ports):
        if any(kw in port for kw in keywords):
            in_idx = i
            break
    
    return out_idx, in_idx

def test_simple_sysex(midi_out, midi_in):
    """Send a simple SysEx and listen for anything"""
    print_header("Simple SysEx Test")
    print_info("Sending a simple SysEx message and listening for ANY response...")
    
    # Clear input
    while midi_in.get_message():
        pass
    
    # Send a simple SysEx: F0 7D 00 01 F7 (manufacturer ID 0x7D = educational use)
    test_msg = [0xF0, 0x7D, 0x00, 0x01, 0xF7]
    print(f"  Sending: {bytes_to_hex(test_msg)}")
    midi_out.send_message(test_msg)
    
    # Wait and listen
    time.sleep(0.5)
    
    response_count = 0
    while True:
        msg = midi_in.get_message()
        if not msg:
            break
        response_count += 1
        print(f"  Response {response_count}: {bytes_to_hex(msg[0])}")
    
    if response_count > 0:
        print_success(f"Received {response_count} response(s)")
        return True
    else:
        print_error("No response received")
        return False

def test_note_on_off(midi_out, midi_in):
    """Send Note On/Off and listen"""
    print_header("Note On/Off Test")
    print_info("Sending Note On/Off and listening for ANY response...")
    
    # Clear input
    while midi_in.get_message():
        pass
    
    # Send Note On: 90 3C 7F (channel 1, middle C, velocity 127)
    note_on = [0x90, 0x3C, 0x7F]
    print(f"  Sending Note On: {bytes_to_hex(note_on)}")
    midi_out.send_message(note_on)
    
    time.sleep(0.1)
    
    # Send Note Off: 80 3C 00
    note_off = [0x80, 0x3C, 0x00]
    print(f"  Sending Note Off: {bytes_to_hex(note_off)}")
    midi_out.send_message(note_off)
    
    time.sleep(0.3)
    
    response_count = 0
    while True:
        msg = midi_in.get_message()
        if not msg:
            break
        response_count += 1
        print(f"  Response {response_count}: {bytes_to_hex(msg[0])}")
    
    if response_count > 0:
        print_success(f"Received {response_count} response(s)")
        return True
    else:
        print_info("No response (this might be normal if no loopback)")
        return False

def test_mios32_query(midi_out, midi_in):
    """Send MIOS32 query and listen"""
    print_header("MIOS32 Query Test")
    print_info("Sending MIOS32 query type 0x01 (Operating System)...")
    
    # Clear input
    while midi_in.get_message():
        pass
    
    # Send MIOS32 query: F0 00 00 7E 32 00 00 01 F7
    query = [0xF0, 0x00, 0x00, 0x7E, 0x32, 0x00, 0x00, 0x01, 0xF7]
    print(f"  Sending: {bytes_to_hex(query)}")
    midi_out.send_message(query)
    
    # Wait for response
    time.sleep(1.0)
    
    response_count = 0
    while True:
        msg = midi_in.get_message()
        if not msg:
            break
        response_count += 1
        data = msg[0]
        print(f"  Response {response_count}: {bytes_to_hex(data)}")
        
        # Try to parse as MIOS32 response
        if len(data) >= 7 and data[0:5] == [0xF0, 0x00, 0x00, 0x7E, 0x32]:
            if data[6] == 0x0F:  # ACK
                response_str = bytes(data[7:-1]).decode('ascii', errors='ignore')
                print_success(f"  MIOS32 ACK received: \"{response_str}\"")
    
    if response_count > 0:
        print_success(f"Received {response_count} response(s)")
        return True
    else:
        print_error("No response to MIOS32 query")
        return False

def monitor_mode(midi_in):
    """Monitor all incoming MIDI messages"""
    print_header("Monitor Mode")
    print_info("Monitoring all incoming MIDI messages...")
    print_info("Send MIDI from your device. Press Ctrl+C to stop.\n")
    
    message_count = 0
    try:
        while True:
            msg = midi_in.get_message()
            if msg:
                message_count += 1
                data = msg[0]
                print(f"  Message {message_count}: {bytes_to_hex(data)}")
            time.sleep(0.01)
    except KeyboardInterrupt:
        print(f"\n\nMonitored {message_count} messages")

def main():
    print_header("MIDI Loopback Test Tool")
    
    midi_out = rtmidi.MidiOut()
    midi_in = rtmidi.MidiIn()
    
    try:
        # List ports
        out_ports, in_ports = list_ports(midi_out, midi_in)
        
        if not out_ports or not in_ports:
            print_error("No MIDI ports available!")
            return 1
        
        # Auto-detect or prompt
        out_idx, in_idx = find_midicore(out_ports, in_ports)
        
        if out_idx is not None and in_idx is not None:
            print_success(f"\nAuto-detected MidiCore:")
            print(f"  Output: {out_ports[out_idx]}")
            print(f"  Input:  {in_ports[in_idx]}")
        else:
            print_info("\nSelect MIDI ports:")
            try:
                out_idx = int(input(f"Output port (0-{len(out_ports)-1}): "))
                in_idx = int(input(f"Input port (0-{len(in_ports)-1}): "))
            except (ValueError, KeyboardInterrupt):
                print_error("\nCancelled")
                return 1
        
        # Open ports
        print_info("\nOpening MIDI ports...")
        midi_out.open_port(out_idx)
        midi_in.open_port(in_idx)
        print_success("Ports opened\n")
        
        # Run tests
        results = []
        results.append(("Simple SysEx", test_simple_sysex(midi_out, midi_in)))
        time.sleep(0.5)
        results.append(("Note On/Off", test_note_on_off(midi_out, midi_in)))
        time.sleep(0.5)
        results.append(("MIOS32 Query", test_mios32_query(midi_out, midi_in)))
        
        # Summary
        print_header("Test Summary")
        for name, result in results:
            status = f"{Colors.GREEN}PASS{Colors.END}" if result else f"{Colors.RED}FAIL{Colors.END}"
            print(f"  {name:20} {status}")
        
        # Offer monitor mode
        print()
        try:
            answer = input("Start monitor mode? (y/n): ").lower()
            if answer == 'y':
                monitor_mode(midi_in)
        except (KeyboardInterrupt, EOFError):
            pass
        
        midi_out.close_port()
        midi_in.close_port()
        
        return 0
        
    except Exception as e:
        print_error(f"\nError: {e}")
        import traceback
        traceback.print_exc()
        return 1
    finally:
        del midi_out
        del midi_in

if __name__ == "__main__":
    sys.exit(main())
