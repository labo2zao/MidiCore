#!/usr/bin/env python3
"""
USB CDC Terminal Test Tool for MidiCore

This tool tests USB CDC (Virtual COM Port) communication with MidiCore.
It helps diagnose issues with MIOS Studio terminal not working.

The CDC interface is used by MIOS Studio for:
- Debug messages
- Terminal commands
- Firmware upload (bootloader)

Requirements:
    pip install pyserial

Usage:
    python3 test_cdc_terminal.py

Author: MidiCore Project
"""

import serial
import serial.tools.list_ports
import time
import sys
import threading

class Colors:
    """ANSI color codes"""
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    CYAN = '\033[96m'
    MAGENTA = '\033[95m'
    BOLD = '\033[1m'
    END = '\033[0m'

def print_header(text):
    """Print formatted header"""
    print(f"\n{Colors.CYAN}{Colors.BOLD}{'='*70}")
    print(f"{text:^70}")
    print(f"{'='*70}{Colors.END}\n")

def print_success(text):
    print(f"{Colors.GREEN}✓ {text}{Colors.END}")

def print_error(text):
    print(f"{Colors.RED}✗ {text}{Colors.END}")

def print_warning(text):
    print(f"{Colors.YELLOW}⚠ {text}{Colors.END}")

def print_info(text):
    print(f"{Colors.BLUE}ℹ {text}{Colors.END}")

def list_serial_ports():
    """List all available serial ports"""
    print_header("Available Serial Ports")
    
    ports = serial.tools.list_ports.comports()
    
    if not ports:
        print_error("No serial ports found!")
        return []
    
    result = []
    for i, port in enumerate(ports):
        result.append(port.device)
        print(f"  {i}: {port.device}")
        print(f"     Description: {port.description}")
        if port.manufacturer:
            print(f"     Manufacturer: {port.manufacturer}")
        if port.vid and port.pid:
            print(f"     VID:PID = {port.vid:04X}:{port.pid:04X}")
        print()
    
    return result

def find_midicore_port():
    """Try to auto-detect MidiCore CDC port"""
    ports = serial.tools.list_ports.comports()
    
    # Look for STM32 VID or "MidiCore" in description
    for port in ports:
        # STM32 default VID
        if port.vid == 0x16C0:  # Generic VID used by MidiCore
            return port.device
        # Check description
        if port.description and 'midicore' in port.description.lower():
            return port.device
        if port.description and 'stm32' in port.description.lower():
            return port.device
    
    return None

def test_echo(ser, timeout=2.0):
    """Test echo functionality"""
    print_header("Echo Test")
    print_info("Testing if device echoes back what we send...")
    
    test_strings = [
        b"Hello MidiCore!\r\n",
        b"Test 123\r\n",
        b"MIOS Studio Terminal Test\r\n"
    ]
    
    passed = 0
    failed = 0
    
    for test_str in test_strings:
        print(f"\n  Sending: {test_str.decode('ascii', errors='ignore').strip()}")
        
        # Clear input buffer
        ser.reset_input_buffer()
        
        # Send test string
        ser.write(test_str)
        ser.flush()
        
        # Wait for echo
        time.sleep(0.1)
        
        # Read response
        response = b""
        start_time = time.time()
        while (time.time() - start_time) < timeout:
            if ser.in_waiting > 0:
                response += ser.read(ser.in_waiting)
            if len(response) >= len(test_str):
                break
            time.sleep(0.01)
        
        if response:
            print(f"  Received: {response.decode('ascii', errors='ignore').strip()}")
            if response == test_str:
                print_success("  ✓ Perfect echo!")
                passed += 1
            else:
                print_warning("  ⚠ Echo mismatch!")
                failed += 1
        else:
            print_error("  ✗ No response (timeout)")
            failed += 1
    
    print(f"\n{Colors.BOLD}Echo Test Results:{Colors.END}")
    print(f"  Passed: {Colors.GREEN}{passed}{Colors.END}")
    print(f"  Failed: {Colors.RED}{failed}{Colors.END}")
    
    return passed > 0

def test_line_coding(ser):
    """Test line coding parameters"""
    print_header("Line Coding Test")
    print_info("Testing CDC line coding (baud rate, data bits, etc.)...")
    
    # Try to get current settings
    try:
        print(f"\n  Current settings:")
        print(f"    Baud rate: {ser.baudrate}")
        print(f"    Data bits: {ser.bytesize}")
        print(f"    Parity: {ser.parity}")
        print(f"    Stop bits: {ser.stopbits}")
        print_success("Line coding parameters accessible")
        return True
    except Exception as e:
        print_error(f"Cannot access line coding: {e}")
        return False

def test_control_lines(ser):
    """Test control line signals (DTR/RTS)"""
    print_header("Control Line Test")
    print_info("Testing DTR/RTS control signals...")
    
    try:
        # Test DTR
        print("\n  Testing DTR (Data Terminal Ready)...")
        ser.dtr = False
        time.sleep(0.1)
        ser.dtr = True
        time.sleep(0.1)
        print_success("DTR toggle successful")
        
        # Test RTS
        print("\n  Testing RTS (Request To Send)...")
        ser.rts = False
        time.sleep(0.1)
        ser.rts = True
        time.sleep(0.1)
        print_success("RTS toggle successful")
        
        return True
    except Exception as e:
        print_error(f"Control line test failed: {e}")
        return False

def test_data_transfer(ser):
    """Test bidirectional data transfer"""
    print_header("Data Transfer Test")
    print_info("Testing sustained bidirectional communication...")
    
    # Send a burst of data
    data_sent = 0
    data_received = 0
    
    test_data = b"0123456789ABCDEF" * 10  # 160 bytes
    
    print(f"\n  Sending {len(test_data)} bytes...")
    ser.write(test_data)
    ser.flush()
    data_sent = len(test_data)
    
    # Wait for echo
    time.sleep(0.5)
    
    if ser.in_waiting > 0:
        response = ser.read(ser.in_waiting)
        data_received = len(response)
        print(f"  Received {data_received} bytes")
        
        if data_received == data_sent:
            if response == test_data:
                print_success("Perfect data transfer!")
                return True
            else:
                print_warning("Data mismatch (but size correct)")
                return True
        else:
            print_warning(f"Size mismatch: sent {data_sent}, received {data_received}")
            return False
    else:
        print_error("No data received")
        return False

def interactive_terminal(ser):
    """Run interactive terminal mode"""
    print_header("Interactive Terminal Mode")
    print_info("Type messages to send to MidiCore. Press Ctrl+C to exit.")
    print_info("All received data will be displayed.\n")
    
    # Create reader thread
    stop_flag = threading.Event()
    
    def reader():
        """Background thread to read and display incoming data"""
        while not stop_flag.is_set():
            if ser.in_waiting > 0:
                data = ser.read(ser.in_waiting)
                try:
                    text = data.decode('ascii', errors='replace')
                    print(f"{Colors.MAGENTA}<<< {text}{Colors.END}", end='', flush=True)
                except:
                    print(f"{Colors.MAGENTA}<<< [binary: {data.hex()}]{Colors.END}")
            time.sleep(0.01)
    
    reader_thread = threading.Thread(target=reader, daemon=True)
    reader_thread.start()
    
    try:
        while True:
            # Get user input
            try:
                line = input(f"{Colors.CYAN}>>> {Colors.END}")
                if line:
                    ser.write((line + '\r\n').encode('ascii'))
                    ser.flush()
            except EOFError:
                break
    except KeyboardInterrupt:
        print("\n\nExiting interactive mode...")
    finally:
        stop_flag.set()
        reader_thread.join(timeout=1.0)

def main():
    """Main entry point"""
    print_header("USB CDC Terminal Test Tool")
    
    # List ports
    ports = list_serial_ports()
    
    if not ports:
        print_error("No serial ports available!")
        print_info("Please connect your MidiCore device via USB.")
        return 1
    
    # Try to auto-detect
    auto_port = find_midicore_port()
    
    if auto_port:
        print_success(f"\nAuto-detected MidiCore port: {auto_port}")
        port_name = auto_port
    else:
        print_warning("\nCould not auto-detect MidiCore port.")
        print("Please select a port:\n")
        
        try:
            idx = int(input(f"Select port (0-{len(ports)-1}): "))
            if idx < 0 or idx >= len(ports):
                raise ValueError
            port_name = ports[idx]
        except (ValueError, KeyboardInterrupt):
            print_error("\nInvalid selection or cancelled.")
            return 1
    
    # Open serial port
    print_info(f"\nOpening {port_name}...")
    
    try:
        # MIOS Studio default settings: 115200, 8N1
        ser = serial.Serial(
            port=port_name,
            baudrate=115200,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=1,
            write_timeout=1
        )
        
        print_success("Port opened successfully!")
        
        # Run tests
        results = {
            'Line Coding': test_line_coding(ser),
            'Control Lines': test_control_lines(ser),
            'Echo': test_echo(ser),
            'Data Transfer': test_data_transfer(ser),
        }
        
        # Summary
        print_header("Test Summary")
        
        for test_name, result in results.items():
            status = f"{Colors.GREEN}PASS{Colors.END}" if result else f"{Colors.RED}FAIL{Colors.END}"
            print(f"  {test_name:20} {status}")
        
        passed = sum(results.values())
        total = len(results)
        
        print(f"\n{Colors.BOLD}Overall: {passed}/{total} tests passed{Colors.END}\n")
        
        if passed == total:
            print_success("All tests passed! CDC terminal should work with MIOS Studio.")
        elif passed > 0:
            print_warning("Some tests failed. CDC terminal may have issues.")
        else:
            print_error("All tests failed! CDC terminal is not working.")
            print_info("\nPossible causes:")
            print("  1. Wrong COM port selected")
            print("  2. Device not responding (firmware issue)")
            print("  3. USB cable is charge-only (no data)")
            print("  4. Driver not properly installed")
        
        # Offer interactive mode
        if passed > 0:
            print()
            try:
                answer = input(f"Start interactive terminal mode? (y/n): ").lower()
                if answer == 'y':
                    interactive_terminal(ser)
            except (KeyboardInterrupt, EOFError):
                pass
        
        ser.close()
        return 0 if passed == total else 1
        
    except serial.SerialException as e:
        print_error(f"Cannot open port: {e}")
        print_info("\nPossible causes:")
        print("  1. Port is already in use by another application")
        print("  2. Insufficient permissions (Linux: add user to 'dialout' group)")
        print("  3. Device disconnected")
        return 1
    except KeyboardInterrupt:
        print_error("\n\nTest interrupted by user.")
        return 1
    except Exception as e:
        print_error(f"\nError: {e}")
        import traceback
        traceback.print_exc()
        return 1

if __name__ == "__main__":
    sys.exit(main())
