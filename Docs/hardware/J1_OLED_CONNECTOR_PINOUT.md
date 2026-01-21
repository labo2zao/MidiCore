# J1 OLED Connector Pinout

This document provides the detailed pinout for the J1 OLED connector on MidiCore hardware.

## Connector Overview

**Connector**: J1  
**Type**: 16-pin header  
**Purpose**: OLED Display Interface  
**Modes**: SPI (4-wire) or Parallel (8080/6800)  
**Standard**: MBHP/MIOS32 Compatible  
**Voltage**: 3.3V Logic  

## J1 Complete Pinout Table

| Pin | Signal Name | STM32 Pin | Function | Mode | Description |
|-----|-------------|-----------|----------|------|-------------|
| 1 | GND | GND | Ground | Both | Ground Reference |
| 2 | VCC_IN | 3.3V | Power | Both | 3.3V Power Supply |
| 3 | NC | - | Not Connected | - | No connection |
| 4 | CLK | PB13 | SPI2_SCK | SPI | SPI Clock Signal |
| 5 | DIN | PB15 | SPI2_MOSI | SPI | SPI Data In (MOSI) |
| 6 | D2 | - | Data Bit 2 | Parallel | Parallel data bus bit 2 |
| 7 | D3 | - | Data Bit 3 | Parallel | Parallel data bus bit 3 |
| 8 | D4 | - | Data Bit 4 | Parallel | Parallel data bus bit 4 |
| 9 | D5 | - | Data Bit 5 | Parallel | Parallel data bus bit 5 |
| 10 | - | - | - | - | (Not specified) |
| 11 | - | - | - | - | (Not specified) |
| 12 | E/RD# | - | Enable/Read | Parallel | Parallel interface enable/read |
| 13 | R/W# | - | Read/Write | Parallel | Parallel interface read/write |
| 14 | D/C# | PC4 | Data/Command | Both | Data/Command select (active low) |
| 15 | Res# | PC5 | Reset | Both | Reset signal (active low) |
| 16 | CS# | PB12 | Chip Select | Both | Chip select (active low) |

## Operating Modes

### SPI Mode (Default - Used by MidiCore)

MidiCore uses **4-wire SPI mode** for OLED communication. Only the following pins are actively used:

| Pin | Signal | STM32 Pin | Description |
|-----|--------|-----------|-------------|
| 1 | GND | GND | Ground |
| 2 | VCC_IN | 3.3V | Power supply |
| 4 | CLK | PB13 (SPI2_SCK) | SPI clock |
| 5 | DIN | PB15 (SPI2_MOSI) | SPI data (MOSI) |
| 14 | D/C# | PC4 | Data/Command select |
| 15 | Res# | PC5 | Reset |
| 16 | CS# | PB12 | Chip Select |

**Unused pins in SPI mode**: 3, 6-13

### Parallel Mode (Not Used by MidiCore)

The connector also supports parallel 8080/6800 interface mode using pins 6-9 (D2-D5) and 12-13 (E/RD#, R/W#). This mode is not currently implemented in MidiCore firmware.

## Pin Descriptions (SPI Mode)

### Power Pins

**Pin 1 - GND**
- Ground reference
- Connect to display ground
- Ensure solid ground connection for noise immunity

**Pin 2 - VCC_IN (3.3V)**
- Supply voltage for OLED display
- Must be stable 3.3V ±5%
- Current draw: Typically 20-100mA depending on display
- Add 100nF decoupling capacitor near display

### SPI Communication Pins

**Pin 4 - CLK (PB13 - SPI2_SCK)**
- SPI clock signal
- Direction: STM32 → Display
- Clock frequency: Up to 21 MHz
- Idle state: LOW (CPOL=0)
- Logic level: 3.3V CMOS

**Pin 5 - DIN (PB15 - SPI2_MOSI)**
- SPI data signal
- Direction: STM32 → Display (Master Out, Slave In)
- Data valid on clock rising edge (CPHA=0)
- 8-bit transfers, MSB first
- Logic level: 3.3V CMOS
- Data valid on clock rising edge (CPHA=0)
- 8-bit transfers, MSB first
- Logic level: 3.3V CMOS

### Control Pins

**Pin 14 - D/C# (PC4)**
- Data/Command select
- Direction: STM32 → Display
- Active LOW for commands (marked with # suffix)
- LOW = Command byte follows
- HIGH = Data byte(s) follow
- Software controlled GPIO
- Must be valid before SPI transfer starts

**Pin 15 - Res# (PC5)**
- Hardware reset signal
- Direction: STM32 → Display
- Active LOW (display resets when LOW)
- Idle state: HIGH
- Software controlled GPIO
- Pulsed LOW for minimum 2ms during initialization
- Followed by 5ms delay before first command

**Pin 16 - CS# (PB12)**
- Chip Select / Slave Select
- Direction: STM32 → Display
- Active LOW (display selected when LOW)
- Idle state: HIGH
- Software controlled GPIO
- Pull-up resistor optional (10kΩ recommended for noise immunity)

### Unused Pins in SPI Mode

**Pin 3 - NC**
- Not connected
- Leave floating or tie to ground

**Pins 6-9 - D2, D3, D4, D5**
- Parallel data bus bits
- Not used in SPI mode
- Leave disconnected

**Pins 12-13 - E/RD#, R/W#**
- Parallel interface control signals
- Not used in SPI mode
- Leave disconnected

## Connection Diagram (SPI Mode)

```
         J1 Connector (16-pin header)
    ┌─────────────────────────────────────┐
    │  Pin  Signal    STM32               │
    │  ──────────────────────────────     │
    │   1   GND      ●●●●●●● GND          │
    │   2   VCC_IN   ●●●●●●● 3.3V         │
    │   3   NC       ●                    │
    │   4   CLK      ●●●●●●● PB13 (SCK)   │
    │   5   DIN      ●●●●●●● PB15 (MOSI)  │
    │   6   D2       ●       (unused)     │
    │   7   D3       ●       (unused)     │
    │   8   D4       ●       (unused)     │
    │   9   D5       ●       (unused)     │
    │  10   -        ●                    │
    │  11   -        ●                    │
    │  12   E/RD#    ●       (unused)     │
    │  13   R/W#     ●       (unused)     │
    │  14   D/C#     ●●●●●●● PC4          │
    │  15   Res#     ●●●●●●● PC5          │
    │  16   CS#      ●●●●●●● PB12         │
    └─────────────────────────────────────┘
           ↓↓↓↓↓↓↓
      OLED Display (SPI Mode)
      
      Required connections:
      - Pins 1,2,4,5,14,15,16 (7 wires)
```

## Simplified SPI Connection

For SPI mode operation, only these 7 connections are needed:

```
STM32F407VGT6          J1 Pin    OLED Display
┌─────────────┐                  ┌──────────────┐
│             │                  │              │
│    GND  ────┼───── 1 ──────────┤ GND          │
│    3.3V ────┼───── 2 ──────────┤ VCC/VDD      │
│    PB13 ────┼───── 4 ──────────┤ CLK/SCK/D0   │
│    PB15 ────┼───── 5 ──────────┤ DIN/MOSI/D1  │
│    PC4  ────┼──── 14 ──────────┤ D/C#         │
│    PC5  ────┼──── 15 ──────────┤ Res#/RST     │
│    PB12 ────┼──── 16 ──────────┤ CS#          │
│             │                  │              │
└─────────────┘                  └──────────────┘
```

## Cable Requirements

### Wire Specifications
- **Required wires**: 7 (for SPI mode)
- **Length**: Keep under 15cm for reliable high-speed SPI
- **Type**: Ribbon cable or individual wires
- **Gauge**: 24-28 AWG sufficient
- **Shielding**: Optional, helps with EMI in noisy environments

### Pin-to-Pin Connection

When connecting to an OLED display, map J1 pins to display pins according to display datasheet:

| J1 Pin | J1 Signal | Display Signal (typical) |
|--------|-----------|-------------------------|
| 1 | GND | GND |
| 2 | VCC_IN | VDD or VCC (3.3V) |
| 4 | CLK | SCK, CLK, D0, or SCLK |
| 5 | DIN | MOSI, DIN, D1, SDA, or SDIN |
| 14 | D/C# | D/C, DC, or A0 |
| 15 | Res# | RST, RES, or RESET |
| 16 | CS# | CS or /CS |

**Note**: Display signal names vary by manufacturer. Consult your specific OLED module datasheet.

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
