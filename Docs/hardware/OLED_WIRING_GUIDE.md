# OLED Display Wiring Guide

This guide documents OLED display wiring configurations for MidiCore, including compatibility with MIOS32/LoopA hardware setups.

## Overview

MidiCore supports grayscale OLED displays for the user interface. The system is designed to be compatible with MIOS32/MBHP hardware standards while using the SSD1322 display by default.

### Supported Displays

1. **SSD1322** (default) - 256×64 grayscale OLED
2. **SSD1306** (LoopA compatible) - 128×64 monochrome OLED

## Current Configuration: SSD1322

The MidiCore default configuration uses an SSD1322 256×64 grayscale OLED display connected via SPI2.

### Hardware Connector

**Connector J1** on the board provides the OLED interface signals.

For detailed J1 pinout specifications, see [J1 Connector Pinout](J1_OLED_CONNECTOR_PINOUT.md).

### Pin Configuration

Located in: `Config/oled_pins.h`

| Signal | STM32 Pin | Connector | SPI Function | Description |
|--------|-----------|-----------|--------------|-------------|
| **SPI2_SCK** | PB13 | J1 | Clock | SPI clock signal |
| **SPI2_MOSI** | PB15 | J1 | Data Out | SPI data to display |
| **OLED_CS** | PB12 | J1 | Chip Select | Active low |
| **OLED_DC** | PC4 | J1 | Data/Command | Low=Command, High=Data |
| **OLED_RST** | PC5 | J1 | Reset | Active low reset |

### Wiring Diagram (SSD1322)

```
STM32F407VGT6    Connector J1        SSD1322 OLED
┌─────────────┐  ┌────────┐         ┌──────────────┐
│             │  │   J1   │         │              │
│    PB13 ────┼──┤   SCK  ├─────────┤ SCK  (CLK)   │
│    PB15 ────┼──┤  MOSI  ├─────────┤ MOSI (DIN)   │
│    PB12 ────┼──┤   CS   ├─────────┤ CS   (/CS)   │
│    PC4  ────┼──┤   DC   ├─────────┤ DC   (D/C)   │
│    PC5  ────┼──┤   RST  ├─────────┤ RST  (/RES)  │
│             │  │        │         │              │
│    GND  ────┼──┤  GND   ├─────────┤ GND          │
│    3.3V ────┼──┤  VDD   ├─────────┤ VDD          │
│             │  │        │         │              │
└─────────────┘  └────────┘         └──────────────┘
```

### SPI Configuration

- **Bus**: SPI2
- **Mode**: Master
- **Speed**: Up to 42 MHz (APB1/2)
- **Data Size**: 8-bit
- **CPOL**: 0 (clock idle low)
- **CPHA**: 0 (first edge)
- **NSS**: Software managed (GPIO CS)

### Code Reference

Driver implementation: `Hal/oled_ssd1322/oled_ssd1322.c`

```c
#define OLED_W 256
#define OLED_H 64

// Initialize display
void oled_init(void);

// Get framebuffer pointer (4bpp grayscale)
uint8_t* oled_framebuffer(void);

// Flush framebuffer to display
void oled_flush(void);

// Clear display
void oled_clear(void);
```

## LoopA Compatible Configuration: SSD1306

For MIOS32/LoopA compatibility with SSD1306 displays, use the wiring specified in the MBHP reference.

### Reference

**MIOS32 MBHP OLED Wiring**: http://www.ucapps.de/mbhp/mbhp_lcd_ssd1306_single_mios32.pdf

### Hardware Connector

**Connector J1** provides the standard MBHP/MIOS32 OLED interface.

### SSD1306 Pin Configuration

| Signal | STM32 Pin | Connector | Function | Description |
|--------|-----------|-----------|----------|-------------|
| **SCK** | PB13 (SPI2) | J1 | Clock | SPI clock |
| **MOSI** | PB15 (SPI2) | J1 | Data | SPI data |
| **CS** | PB12 | J1 | Chip Select | Active low |
| **DC** | PC4 | J1 | Data/Command | Low=Command, High=Data |
| **RST** | PC5 | J1 | Reset | Active low reset |

### Wiring Diagram (SSD1306 - LoopA Style)

```
STM32F407VGT6    Connector J1        SSD1306 OLED
┌─────────────┐  ┌────────┐         ┌──────────────┐
│             │  │   J1   │         │              │
│    PB13 ────┼──┤   SCK  ├─────────┤ D0   (SCK)   │
│    PB15 ────┼──┤  MOSI  ├─────────┤ D1   (MOSI)  │
│    PB12 ────┼──┤   CS   ├─────────┤ CS   (/CS)   │
│    PC4  ────┼──┤   DC   ├─────────┤ DC   (D/C)   │
│    PC5  ────┼──┤   RST  ├─────────┤ RES  (/RES)  │
│             │  │        │         │              │
│    GND  ────┼──┤  GND   ├─────────┤ GND          │
│    3.3V ────┼──┤  VDD   ├─────────┤ VCC (3.3V)   │
│             │  │        │         │              │
└─────────────┘  └────────┘         └──────────────┘
```

### MBHP Compatibility Notes

1. **Connector J1**: Standard OLED connector for MBHP/MIOS32 compatibility
2. **Pin Assignment**: Uses the same SPI2 pins as MIOS32 MBHP_CORE_STM32F4
3. **Signal Levels**: 3.3V logic compatible
4. **LoopA Standard**: Matches the wiring used in MIDIphy LoopA hardware
5. **MBHP Shield Compatible**: Can use standard MBHP OLED modules

### SSD1306 vs SSD1322 Differences

| Feature | SSD1306 | SSD1322 |
|---------|---------|---------|
| **Resolution** | 128×64 | 256×64 |
| **Color Depth** | 1-bit (monochrome) | 4-bit (16 grayscale) |
| **Memory Size** | 1KB framebuffer | 8KB framebuffer |
| **Typical Use** | MIOS32/LoopA projects | MidiCore default |
| **Pin Compatibility** | ✅ Same SPI pins | ✅ Same SPI pins |

## Hardware Setup

### MBHP_CORE_STM32F4 Compatibility

MidiCore is designed to be compatible with MBHP_CORE_STM32F4 hardware:

- **Connector J1**: OLED display connector (standard MBHP assignment)
- **J10A/J10B Pins**: SPI2 bus signals routed to J1
- **Control Signals**: Standard D/C and RST pins on J1
- **Power**: 3.3V from STM32 board via J1

### Connection Checklist

- [ ] SPI2 pins connected (SCK, MOSI)
- [ ] Chip Select (CS) connected to PB12
- [ ] Data/Command (DC) connected to PC4
- [ ] Reset (RST) connected to PC5
- [ ] Ground (GND) connected
- [ ] Power (VCC/VDD) connected to 3.3V
- [ ] Pull-up resistors on CS line (optional, helps noise immunity)

### Testing

To test OLED functionality:

```c
// In your test code (module_config.h)
#define MODULE_ENABLE_OLED 1
#define MODULE_TEST_UI 1

// Expected behavior:
// - Display initializes on startup
// - UI pages render correctly
// - No visual artifacts or flickering
```

See [TESTING_QUICKSTART.md](../testing/TESTING_QUICKSTART.md) for module testing procedures.

## CubeMX Configuration

### SPI2 Setup

In STM32CubeMX (MidiCore.ioc):

1. **SPI2 Settings**:
   - Mode: Full-Duplex Master
   - Hardware NSS: Disabled (software CS)
   - Frame Format: Motorola
   - Data Size: 8 bits
   - First Bit: MSB First

2. **GPIO Configuration**:
   - PB13: SPI2_SCK (Alternate Function)
   - PB15: SPI2_MOSI (Alternate Function)
   - PB12: GPIO_Output (OLED_CS)
   - PC4: GPIO_Output (OLED_DC)
   - PC5: GPIO_Output (OLED_RST)

3. **Clock Configuration**:
   - SPI2 Clock Source: APB1 (42 MHz max)
   - Prescaler: /2 (21 MHz) recommended

### Note on Pin Naming

In `main.h`, you'll see:
```c
#define OLED_CS_Pin GPIO_PIN_12
#define OLED_CS_GPIO_Port GPIOB
```

**Important**: The OLED_CS pin (PB12) is also used as SRIO DOUT RCLK in some configurations. See `Services/srio/srio_user_config.h` for details.

## Troubleshooting

### Display Not Initializing

1. **Check Wiring**: Verify all connections match the pinout above
2. **Power**: Ensure 3.3V supply is stable and sufficient
3. **SPI Bus**: Use oscilloscope to verify SCK and MOSI signals
4. **Reset**: Check RST line has proper reset pulse on startup

### Display Artifacts or Flickering

1. **SPI Speed**: Reduce SPI clock if seeing data corruption
2. **Cable Length**: Keep wires short (<15cm) for high-speed SPI
3. **Ground**: Ensure solid ground connection
4. **Power Supply**: Check for voltage drops during operation

### No Content Visible

1. **Contrast**: Some displays need contrast adjustment
2. **Backlight**: Verify backlight/power LEDs are lit
3. **Framebuffer**: Verify `oled_flush()` is being called
4. **Display Mode**: Check display is not in sleep mode

### MIOS32 Compatibility Issues

1. **Pin Assignment**: Verify pins match MBHP_CORE_STM32F4 standard
2. **Voltage Levels**: Both systems use 3.3V logic
3. **SPI Mode**: Ensure CPOL=0, CPHA=0 (Mode 0)
4. **Shield Adapter**: Use proper connector if using MBHP shields

## Module Configuration

To enable OLED support in your build:

```c
// In Config/module_config.h
#define MODULE_ENABLE_OLED 1
#define MODULE_ENABLE_SPI_BUS 1  // Required dependency
```

Note: OLED module requires SPI_BUS module to be enabled.

## Migration Guide

### From SSD1306 to SSD1322

If migrating from SSD1306 (LoopA) to SSD1322 (MidiCore):

1. **Wiring**: Pin connections remain the same
2. **Resolution**: UI code updated for 256×64 instead of 128×64
3. **Grayscale**: Can now use 16 gray levels instead of binary
4. **Framebuffer**: Larger memory footprint (8KB vs 1KB)
5. **Driver**: Replace driver in `Hal/oled_ssd1322/`

### From SSD1322 to SSD1306

If you want to use SSD1306 (LoopA compatible):

1. **Keep Wiring**: Pin connections are the same
2. **Replace Driver**: Copy SSD1306 driver to `Hal/oled_ssd1306/`
3. **Update UI**: Adjust for 128×64 resolution
4. **Framebuffer**: Binary mode (1bpp) instead of grayscale
5. **Testing**: Verify display init sequence

## Resources

### Documentation
- [MIOS32 Hardware Platform](http://www.ucapps.de/mios32.html)
- [MBHP OLED Modules](http://www.ucapps.de/mbhp.html)
- [LoopA Hardware](https://www.midiphy.com/en/loopa-v2/)

### Datasheets
- [SSD1322 Datasheet](https://www.solomon-systech.com/en/product/advanced-display/oled-display-driver-ic/ssd1322/)
- [SSD1306 Datasheet](https://www.solomon-systech.com/en/product/advanced-display/oled-display-driver-ic/ssd1306/)

### MidiCore Documentation
- [Module Configuration](../configuration/README_MODULE_CONFIG.md)
- [SPI Configuration](../configuration/SPI_CONFIGURATION_REFERENCE.md)
- [MIOS32 Compatibility](../mios32/MIOS32_COMPATIBILITY.md)
- [LoopA Compatibility](../development/LOOPA_COMPATIBILITY_REPORT.md)

## Summary

✅ **Default Configuration**: SSD1322 256×64 grayscale OLED on SPI2  
✅ **LoopA Compatible**: Same pinout works with SSD1306 128×64  
✅ **MBHP Compatible**: Standard MIOS32 MBHP_CORE_STM32F4 pin assignment  
✅ **Wiring Reference**: http://www.ucapps.de/mbhp/mbhp_lcd_ssd1306_single_mios32.pdf  

The OLED wiring is designed for maximum compatibility with MIOS32/LoopA hardware while providing enhanced display capabilities with the SSD1322.
