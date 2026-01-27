# Terminal Setup Guide for DIN MIDI Test Mode

## Quick Start

### Hardware Connection
**Debug UART**: UART5 on STM32F407
- **TX Pin**: PC12 (pin 45)
- **RX Pin**: PD2 (pin 83)
- Connect to USB-to-Serial adapter (e.g., FTDI, CP2102)
  - PC12 → RX on adapter
  - PD2 → TX on adapter
  - GND → GND

### Terminal Software Options

#### Option 1: TeraTerm (Windows - RECOMMENDED)
1. **Download**: https://ttssh2.osdn.jp/
2. **Install** and launch TeraTerm
3. **Setup**:
   - File → New Connection
   - Select "Serial"
   - Choose your COM port (e.g., COM3, COM4)
   - Click OK
4. **Configure**:
   - Setup → Serial Port
   - Speed: **115200**
   - Data: 8 bit
   - Parity: none
   - Stop: 1 bit
   - Flow control: none
   - Click OK
5. **Save Setup** (optional):
   - Setup → Save Setup
   - Save as `midicore.ini`

#### Option 2: PuTTY (Windows/Linux)
1. **Download**: https://www.putty.org/
2. **Configure**:
   - Connection type: Serial
   - Serial line: COM3 (or your port)
   - Speed: **115200**
   - Click "Open"

#### Option 3: minicom (Linux)
```bash
sudo minicom -s
# Configure:
# - Serial Device: /dev/ttyUSB0 (or your device)
# - Bps: 115200
# - Hardware Flow Control: No
# - Software Flow Control: No
# Save setup as "midicore"
# Exit and run: minicom midicore
```

#### Option 4: screen (Linux/Mac)
```bash
screen /dev/ttyUSB0 115200
# Exit: Ctrl-A then K
```

#### Option 5: CoolTerm (Mac/Windows/Linux)
1. **Download**: https://freeware.the-meiers.org/
2. **Configure**:
   - Port: Select your serial port
   - Baudrate: **115200**
   - Data Bits: 8
   - Parity: None
   - Stop Bits: 1
   - Click "Connect"

## Troubleshooting

### Problem: Garbled/Garbage Characters

**Cause**: Terminal baudrate doesn't match UART baudrate (115200)

**Solutions**:

1. **Verify Terminal Settings**:
   - Baudrate: **115200** (NOT 31250, NOT 9600)
   - Data bits: 8
   - Parity: None
   - Stop bits: 1
   - Flow control: None

2. **Check USB Driver**:
   - Windows: Install FTDI/CP2102/CH340 driver
   - Linux: Should work out-of-box (check `dmesg | tail`)
   - Mac: Install driver if needed

3. **Verify COM Port**:
   - Windows: Device Manager → Ports (COM & LPT)
   - Linux: `ls /dev/ttyUSB* /dev/ttyACM*`
   - Mac: `ls /dev/tty.*`

4. **Test with Simple Echo**:
   - Try typing in terminal
   - If you see garbled echo, baudrate is wrong

### Problem: Terminal Shows Nothing (Empty)

**Possible Causes**:

1. **Wrong COM Port**:
   - Close terminal
   - Unplug/replug USB adapter
   - Check which port appears/disappears
   - Open correct port

2. **Test Code Not Running**:
   - Verify DIN MIDI test mode is compiled
   - Check that `test_debug_init()` is called
   - Press RESET button on board

3. **Hardware Connection**:
   - Verify TX/RX not swapped
   - Check GND connection
   - Test with multimeter: PC12 should show ~3.3V idle

4. **Driver Issue**:
   - Check Device Manager (Windows) for yellow exclamation marks
   - Reinstall USB-to-Serial driver

### Problem: "Access Denied" or "Permission Denied"

**Linux**:
```bash
# Add user to dialout group
sudo usermod -a -G dialout $USER
# Logout and login again
# Or use sudo:
sudo minicom midicore
```

**Windows**:
- Close any other programs using the COM port
- Check if another terminal is already connected

## Expected Output

When working correctly, you should see:

```
==============================================
DIN MIDI Test Mode
==============================================

Initializing OLED... OK
Initializing CLI... OK
  Type 'help' for available commands
  Type 'router matrix' to view routing

Initializing MIDI Router... OK
Initializing SD and DIN mapping... OK (config loaded from SD)
MIDI routing via router (hal_uart_midi skipped to preserve 115200 debug baud)... OK
Initializing SRIO... OK

==============================================
Test running. Press DIN buttons to send MIDI.
Use CLI commands to control routing.

** UART DEBUG @ 115200 BAUD **
  Port: UART5 (PC12/PD2)
  Verify your terminal is set to 115200 baud!

Available commands:
  help          - Show all commands
  router matrix - Show routing matrix
  router enable IN OUT - Enable route
  router disable IN OUT - Disable route
==============================================

>
```

## Testing the Setup

### Test 1: Send a Command
Type in terminal:
```
help
```
Press Enter. You should see a list of commands.

### Test 2: Check Router
Type:
```
router matrix
```
You should see the MIDI routing matrix.

### Test 3: Enable a Route
Type:
```
router enable 0 1
```
You should see:
```
Enabled route: 0 -> 1
```

## Alternative: Use OLED Debug Instead

If UART terminal is problematic, you can view debug output on the OLED screen:

1. Compile with `MODULE_ENABLE_OLED=1`
2. Debug messages appear on OLED display
3. All 4 MIDI DIN ports available at 31250 baud
4. No terminal needed!

## Pin Reference

### STM32F407VGT6 UART Pins

| UART   | TX Pin | RX Pin | Default Use        | Test Mode Use     |
|--------|--------|--------|--------------------|-------------------|
| USART1 | PA9    | PA10   | USB OTG (reserved) | USB OTG (reserved)|
| USART2 | PA2    | PA3    | MIDI DIN1 (31250)  | MIDI DIN1 (31250) |
| USART3 | PD8    | PD9    | MIDI DIN2 (31250)  | MIDI DIN2 (31250) |
| UART5  | PC12   | PD2    | MIDI DIN4 (31250)  | **Debug (115200)**|

### USB-to-Serial Adapter Connection

```
STM32F407          USB-Serial Adapter
PC12 (TX) -------> RX
PD2  (RX) <------- TX
GND       -------> GND
```

**IMPORTANT**: Do NOT connect 5V! STM32 uses 3.3V logic.

## Common Terminal Shortcuts

### TeraTerm
- Clear screen: Ctrl+L (or Edit → Clear Screen)
- Paste: Right-click or Shift+Insert
- Copy: Select text then Ctrl+C

### PuTTY
- Clear screen: Right-click → Clear Scrollback
- Paste: Right-click
- Copy: Select text (auto-copies)

### minicom
- Exit: Ctrl-A then X
- Clear: Ctrl-A then C
- Help: Ctrl-A then Z

## Need Help?

If you still see garbage or empty terminal:

1. Double-check: **115200 baud, 8N1, no flow control**
2. Try different COM port
3. Try different terminal software
4. Check hardware connections with multimeter
5. Verify code is running (LED blinking?)
6. Switch to OLED debug mode instead

---

**Note**: This guide is for DIN MIDI test mode only. Production mode uses all UARTs at 31250 baud for MIDI.
