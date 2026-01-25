# Hardware Documentation

> 🇫🇷 [Version française disponible](README_FR.md)

This section contains hardware setup, wiring guides, and pinout information for MidiCore.

## Hardware Guides

### Display
- **[OLED Wiring Guide](OLED_WIRING_GUIDE.md)** - Complete OLED display wiring for SSD1322 and SSD1306 (LoopA compatible)
- **[J1 Connector Pinout](J1_OLED_CONNECTOR_PINOUT.md)** - Detailed J1 OLED connector pinout and specifications
- **[OLED Quick Test](OLED_QUICK_TEST.md)** - Quick testing guide for OLED displays

## Hardware Compatibility

MidiCore is designed to be compatible with MIOS32/MBHP hardware standards:

- **MBHP_CORE_STM32F4**: Pin-compatible with standard MIOS32 hardware
- **LoopA Hardware**: Compatible wiring for MIDIphy LoopA displays
- **MBHP Modules**: Works with standard MBHP shields and modules

## Quick Reference

### Primary MCU
- **STM32F407VGT6** (100-pin LQFP)
- **Clock**: 168 MHz
- **Flash**: 1 MB
- **RAM**: 192 KB (128 KB + 64 KB CCMRAM)

### Display Options
- **SSD1322**: 256×64 grayscale OLED (default)
- **SSD1306**: 128×64 monochrome OLED (LoopA compatible)

### Communication Interfaces
- **SPI2**: OLED, SD Card, AINSER64
- **UART**: MIDI DIN (4 ports)
- **USB**: Device and Host MIDI
- **I2C**: Optional pressure sensors

### GPIO Modules
- **SRIO**: Shift register I/O (74HC165/595)
- **DIN**: Digital inputs with debouncing
- **DOUT**: Digital outputs (LEDs)

## Wiring Standards

MidiCore follows MIOS32 wiring standards:

1. **SPI Bus**: Standard MIOS32 SPI pin assignment
2. **UART**: 31.25 kbaud MIDI DIN (5-pin DIN connectors)
3. **GPIO**: MBHP-compatible shift register pinout
4. **Power**: 3.3V logic levels throughout

## See Also

- [MIOS32 Compatibility](../mios32/MIOS32_COMPATIBILITY.md) - Software compatibility
- [Module Configuration](../configuration/README_MODULE_CONFIG.md) - Enable/disable hardware modules
- [SPI Configuration](../configuration/SPI_CONFIGURATION_REFERENCE.md) - SPI bus setup
- [Testing Guide](../testing/TESTING_QUICKSTART.md) - Hardware testing procedures

## External Resources

- [MIOS32 Hardware](http://www.ucapps.de/mios32.html)
- [MBHP Modules](http://www.ucapps.de/mbhp.html)
- [MIDIphy LoopA](https://www.midiphy.com/en/loopa-v2/)
- [MIDIbox Wiki](http://wiki.midibox.org/)
