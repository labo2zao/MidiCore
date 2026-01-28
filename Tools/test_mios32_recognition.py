#!/usr/bin/env python3
"""
MIOS32 Device Recognition Test Tool

This tool tests MIOS32 query/response protocol to diagnose why
MIOS Studio might not recognize a MidiCore device.

It sends MIOS32 queries via USB MIDI and checks if the device responds
correctly. This helps identify if the issue is in the query handler,
USB MIDI transmission, or device enumeration.

Requirements:
    pip install python-rtmidi

Usage:
    python3 test_mios32_recognition.py

Author: MidiCore Project
"""

import rtmidi
import time
import sys

# MIOS32 Query Protocol Constants
MIOS32_HEADER = [0xF0, 0x00, 0x00, 0x7E, 0x32]  # F0 00 00 7E 32
DEVICE_ID = 0x00  # Device instance ID
QUERY_CMD = 0x00  # Query command
RESPONSE_ACK = 0x0F  # ACK response code

# Query Types
QUERY_TYPES = {
    0x01: "Operating System",
    0x02: "Board Name",
    0x03: "Core Family",
    0x04: "Chip ID",
    0x05: "Serial Number",
    0x06: "Flash Size",
    0x07: "RAM Size",
    0x08: "Application Name (Line 1)",
    0x09: "Application Version (Line 2)",
}

class Colors:
    """ANSI color codes for terminal output"""
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    CYAN = '\033[96m'
    BOLD = '\033[1m'
    END = '\033[0m'

def print_header(text):
    """Print a formatted header"""
    print(f"\n{Colors.CYAN}{Colors.BOLD}{'='*70}")
    print(f"{text:^70}")
    print(f"{'='*70}{Colors.END}\n")

def print_success(text):
    """Print success message"""
    print(f"{Colors.GREEN}✓ {text}{Colors.END}")

def print_error(text):
    """Print error message"""
    print(f"{Colors.RED}✗ {text}{Colors.END}")

def print_warning(text):
    """Print warning message"""
    print(f"{Colors.YELLOW}⚠ {text}{Colors.END}")

def print_info(text):
    """Print info message"""
    print(f"{Colors.BLUE}ℹ {text}{Colors.END}")

def bytes_to_hex(data):
    """Convert byte array to hex string"""
    return ' '.join(f'{b:02x}' for b in data)

def build_query(query_type):
    """Build a MIOS32 query message"""
    # F0 00 00 7E 32 <dev_id> <cmd> <query_type> F7
    message = MIOS32_HEADER + [DEVICE_ID, QUERY_CMD, query_type, 0xF7]
    return message

def parse_response(data):
    """Parse MIOS32 response message"""
    if len(data) < 8:
        return None, "Response too short"
    
    # Check header: F0 00 00 7E 32
    if data[:5] != MIOS32_HEADER:
        return None, f"Invalid header: {bytes_to_hex(data[:5])}"
    
    # Check device ID
    device_id = data[5]
    
    # Check ACK code
    ack_code = data[6]
    if ack_code != RESPONSE_ACK:
        return None, f"Expected ACK (0x0F), got 0x{ack_code:02X}"
    
    # Extract response string (between ACK and F7)
    if data[-1] != 0xF7:
        return None, "Missing F7 terminator"
    
    # Response string is from index 7 to -1 (before F7)
    response_bytes = data[7:-1]
    
    # Convert to string (ASCII)
    try:
        response_str = bytes(response_bytes).decode('ascii')
    except:
        response_str = bytes_to_hex(response_bytes)
    
    return response_str, None

def list_midi_ports(midi_out, midi_in):
    """List available MIDI input and output ports"""
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

def find_midicore_ports(out_ports, in_ports):
    """Try to auto-detect MidiCore device"""
    keywords = ['MidiCore', 'midicore', 'MIDICORE', 'STM32', 'MIOS32']
    
    out_idx = None
    in_idx = None
    
    for i, port in enumerate(out_ports):
        if any(keyword in port for keyword in keywords):
            out_idx = i
            break
    
    for i, port in enumerate(in_ports):
        if any(keyword in port for keyword in keywords):
            in_idx = i
            break
    
    return out_idx, in_idx

def test_query(midi_out, midi_in, query_type, timeout=1.0):
    """Send a MIOS32 query and wait for response"""
    query_name = QUERY_TYPES.get(query_type, f"Unknown (0x{query_type:02X})")
    
    # Build and send query
    query_msg = build_query(query_type)
    print(f"\n{Colors.BOLD}Testing Query Type 0x{query_type:02X}: {query_name}{Colors.END}")
    print(f"  Sending: {bytes_to_hex(query_msg)}")
    
    # Clear any pending messages
    while midi_in.get_message():
        pass
    
    # Send query
    midi_out.send_message(query_msg)
    
    # Wait for response
    start_time = time.time()
    response = None
    
    while (time.time() - start_time) < timeout:
        msg = midi_in.get_message()
        if msg:
            response = msg[0]
            break
        time.sleep(0.01)
    
    if not response:
        print_error(f"No response (timeout after {timeout}s)")
        return False
    
    print(f"  Received: {bytes_to_hex(response)}")
    
    # Parse response
    response_str, error = parse_response(response)
    
    if error:
        print_error(f"Invalid response: {error}")
        return False
    
    print_success(f"Valid response: \"{response_str}\"")
    return True

def run_diagnostic(midi_out, midi_in):
    """Run comprehensive diagnostic tests"""
    print_header("MIOS32 Device Recognition Diagnostic")
    
    print_info("This tool tests if your device responds to MIOS32 queries.")
    print_info("MIOS Studio uses these queries to detect and identify devices.\n")
    
    # Test each query type
    results = {}
    for query_type in sorted(QUERY_TYPES.keys()):
        success = test_query(midi_out, midi_in, query_type, timeout=1.0)
        results[query_type] = success
        time.sleep(0.1)  # Small delay between queries
    
    # Summary
    print_header("Test Summary")
    
    total = len(results)
    passed = sum(results.values())
    failed = total - passed
    
    print(f"{Colors.BOLD}Results:{Colors.END}")
    print(f"  Total queries: {total}")
    print(f"  Passed: {Colors.GREEN}{passed}{Colors.END}")
    print(f"  Failed: {Colors.RED}{failed}{Colors.END}")
    
    if passed == total:
        print_success("\nAll tests passed! Device should be recognized by MIOS Studio.")
    elif passed > 0:
        print_warning(f"\nPartial success. {failed} queries failed.")
        print_info("Device might be partially recognized by MIOS Studio.")
    else:
        print_error("\nAll tests failed! Device will NOT be recognized by MIOS Studio.")
        print_info("\nPossible causes:")
        print("  1. Device firmware does not have MIOS32 query handler compiled in")
        print("  2. USB MIDI is not working correctly")
        print("  3. Wrong MIDI port selected")
        print("  4. Device is in a different mode (bootloader, etc.)")
    
    return passed == total

def main():
    """Main entry point"""
    print_header("MIOS32 Device Recognition Test Tool")
    
    # Initialize MIDI
    midi_out = rtmidi.MidiOut()
    midi_in = rtmidi.MidiIn()
    
    try:
        # List available ports
        out_ports, in_ports = list_midi_ports(midi_out, midi_in)
        
        if not out_ports or not in_ports:
            print_error("No MIDI ports available!")
            print_info("Please connect your MidiCore device via USB.")
            return 1
        
        # Try to auto-detect MidiCore
        out_idx, in_idx = find_midicore_ports(out_ports, in_ports)
        
        if out_idx is not None and in_idx is not None:
            print_success(f"\nAuto-detected MidiCore device:")
            print(f"  Output: {out_ports[out_idx]}")
            print(f"  Input:  {in_ports[in_idx]}")
            print()
        else:
            print_warning("\nCould not auto-detect MidiCore device.")
            print("Please select ports manually:\n")
            
            # Prompt for output port
            try:
                out_idx = int(input(f"Select output port (0-{len(out_ports)-1}): "))
                if out_idx < 0 or out_idx >= len(out_ports):
                    raise ValueError
            except (ValueError, KeyboardInterrupt):
                print_error("\nInvalid selection or cancelled.")
                return 1
            
            # Prompt for input port
            try:
                in_idx = int(input(f"Select input port (0-{len(in_ports)-1}): "))
                if in_idx < 0 or in_idx >= len(in_ports):
                    raise ValueError
            except (ValueError, KeyboardInterrupt):
                print_error("\nInvalid selection or cancelled.")
                return 1
        
        # Open ports
        print_info(f"\nOpening MIDI ports...")
        midi_out.open_port(out_idx)
        midi_in.open_port(in_idx)
        print_success("MIDI ports opened successfully")
        
        # Run diagnostic
        success = run_diagnostic(midi_out, midi_in)
        
        # Close ports
        midi_out.close_port()
        midi_in.close_port()
        
        return 0 if success else 1
        
    except KeyboardInterrupt:
        print_error("\n\nTest interrupted by user.")
        return 1
    except Exception as e:
        print_error(f"\nError: {e}")
        import traceback
        traceback.print_exc()
        return 1
    finally:
        # Cleanup
        del midi_out
        del midi_in

if __name__ == "__main__":
    sys.exit(main())
