# J1 OLED Connector Pinout

This document provides the detailed pinout for the J1 OLED connector on MidiCore hardware.

## Connector Overview

**Connector**: J1  
**Purpose**: OLED Display Interface  
**Standard**: MBHP/MIOS32 Compatible  
**Voltage**: 3.3V Logic  

## J1 Pinout Table

| Pin | Signal Name | STM32 Pin | Function | Direction | Description |
|-----|-------------|-----------|----------|-----------|-------------|
| 1 | VDD / VCC | 3.3V | Power | → | 3.3V Power Supply |
| 2 | GND | GND | Ground | - | Ground Reference |
| 3 | SCK / D0 | PB13 | SPI2_SCK | → | SPI Clock Signal |
| 4 | MOSI / D1 | PB15 | SPI2_MOSI | → | SPI Data Out (Master Out Slave In) |
| 5 | CS | PB12 | GPIO | → | Chip Select (Active Low) |
| 6 | DC / D/C | PC4 | GPIO | → | Data/Command Select (Low=Cmd, High=Data) |
| 7 | RST / RES | PC5 | GPIO | → | Reset (Active Low) |

## Pin Descriptions

### Power Pins

**Pin 1 - VDD/VCC (3.3V)**
- Supply voltage for OLED display
- Must be stable 3.3V ±5%
- Current draw: Typically 20-100mA depending on display
- Add 100nF decoupling capacitor near display

**Pin 2 - GND**
- Ground reference
- Connect to display ground
- Ensure solid ground connection for noise immunity

### SPI Communication Pins

**Pin 3 - SCK/D0 (PB13 - SPI2_SCK)**
- SPI clock signal
- Direction: STM32 → Display
- Clock frequency: Up to 21 MHz
- Idle state: LOW (CPOL=0)
- Logic level: 3.3V CMOS

**Pin 4 - MOSI/D1 (PB15 - SPI2_MOSI)**
- SPI data signal
- Direction: STM32 → Display (Master Out, Slave In)
- Data valid on clock rising edge (CPHA=0)
- 8-bit transfers, MSB first
- Logic level: 3.3V CMOS

### Control Pins

**Pin 5 - CS (PB12)**
- Chip Select / Slave Select
- Direction: STM32 → Display
- Active LOW (display selected when LOW)
- Idle state: HIGH
- Software controlled GPIO
- Pull-up resistor optional (10kΩ recommended for noise immunity)

**Pin 6 - DC/D/C (PC4)**
- Data/Command select
- Direction: STM32 → Display
- LOW = Command byte follows
- HIGH = Data byte(s) follow
- Software controlled GPIO
- Must be valid before SPI transfer starts

**Pin 7 - RST/RES (PC5)**
- Hardware reset signal
- Direction: STM32 → Display
- Active LOW (display resets when LOW)
- Idle state: HIGH
- Software controlled GPIO
- Pulsed LOW for minimum 2ms during initialization
- Followed by 5ms delay before first command

## Connection Diagram

```
         J1 Connector (7-pin header)
    ┌─────────────────────────────────┐
    │  1  VDD     ●●●●●●● 3.3V        │
    │  2  GND     ●●●●●●● GND         │
    │  3  SCK     ●●●●●●● PB13        │
    │  4  MOSI    ●●●●●●● PB15        │
    │  5  CS      ●●●●●●● PB12        │
    │  6  DC      ●●●●●●● PC4         │
    │  7  RST     ●●●●●●● PC5         │
    └─────────────────────────────────┘
           ↓↓↓↓↓↓↓
         OLED Display
```

## Cable Requirements

### Wire Specifications
- **Length**: Keep under 15cm for reliable high-speed SPI
- **Type**: Ribbon cable or twisted pairs
- **Gauge**: 24-28 AWG sufficient
- **Shielding**: Optional, helps with EMI in noisy environments

### Signal Integrity
- Keep SCK and MOSI traces close together
- Separate power and ground from signal lines
- Use twisted pair for SCK/MOSI if longer cables needed
- Add series resistors (22-47Ω) if ringing observed

## Compatible Displays

### SSD1322 (Default)
- Resolution: 256×64 pixels
- Color: 16-level grayscale (4-bit)
- Interface: 4-wire SPI + DC + RST + CS
- Typical supply current: 30-50mA

### SSD1306 (LoopA Compatible)
- Resolution: 128×64 pixels
- Color: Monochrome (1-bit)
- Interface: 4-wire SPI + DC + RST + CS
- Typical supply current: 15-25mA

## Initialization Sequence

1. **Power Up**
   - Apply 3.3V to VDD
   - Wait 1ms for power stabilization

2. **Hardware Reset**
   - RST = LOW for 2ms
   - RST = HIGH
   - Wait 5ms

3. **SPI Configuration**
   - CS = HIGH (idle)
   - Configure SPI: Mode 0 (CPOL=0, CPHA=0)
   - Clock speed: 21 MHz max

4. **Display Initialization**
   - Send initialization commands via SPI
   - DC = LOW for commands
   - DC = HIGH for data

## Electrical Characteristics

### Logic Levels (3.3V CMOS)
- VIH (Input High): > 2.0V
- VIL (Input Low): < 0.8V
- VOH (Output High): > 2.4V @ 2mA
- VOL (Output Low): < 0.4V @ 2mA

### Timing (SPI Mode 0)
- Clock frequency: 0-21 MHz
- Setup time: > 20ns
- Hold time: > 20ns
- CS to SCK: > 50ns

### Power Supply
- VDD: 3.3V ±5% (3.135V - 3.465V)
- Current (SSD1322): 30-50mA typical
- Current (SSD1306): 15-25mA typical
- Inrush current: < 100mA

## Troubleshooting

### No Display
1. Check VDD voltage (should be 3.3V ±5%)
2. Verify GND connection
3. Confirm RST pulse occurs on startup
4. Measure CS signal (should toggle during transfers)

### Garbled Display
1. Reduce SPI clock speed
2. Shorten cable length
3. Add decoupling capacitor (100nF) near display
4. Check for loose connections

### Flickering
1. Check power supply stability
2. Verify GND is solid
3. Add bulk capacitor (10µF) on VDD
4. Reduce refresh rate if possible

## MIOS32/MBHP Compatibility

This J1 connector pinout is compatible with:
- MBHP_CORE_STM32F4 OLED connector
- MIDIphy LoopA OLED interface
- Standard MIOS32 OLED shields
- ucapps.de MBHP modules

Reference: http://www.ucapps.de/mbhp/mbhp_lcd_ssd1306_single_mios32.pdf

## Notes

1. **Not I2C**: This is SPI mode, not I2C. Some displays support both; ensure SPI mode is selected.

2. **No MISO**: This is write-only SPI (no MISO/data in from display). This is standard for OLED displays.

3. **Software CS**: CS is GPIO-controlled, not hardware NSS from SPI peripheral.

4. **Pin Order**: Pin numbering may vary by display module. Always verify with display datasheet.

5. **Level Shifting**: If using 5V displays, add level shifters. Most modern OLED displays are 3.3V native.

## Related Documentation

- [OLED Wiring Guide](OLED_WIRING_GUIDE.md) - Complete wiring documentation
- [OLED Quick Test](OLED_QUICK_TEST.md) - Testing procedures
- [SPI Configuration Reference](../configuration/SPI_CONFIGURATION_REFERENCE.md) - SPI setup details

---

**Document Version**: 1.0  
**Last Updated**: 2026-01-21  
**Hardware**: MidiCore STM32F407VGT6  
**Connector**: J1 (OLED Interface)
