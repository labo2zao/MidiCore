# Configuration

> 🇫🇷 [Version française disponible](README_FR.md)

System configuration and setup guides for MidiCore.

## Available Guides

### Module Configuration
- **[Module Configuration](README_MODULE_CONFIG.md)** - Enable/disable modules and configure features

### UART & Serial Configuration
- **[MIOS32 UART Configuration](README_MIOS32_UART_CONFIG.md)** - UART and debug setup for MIOS32 compatibility

### Hardware Configuration
- **[SPI Configuration Reference](SPI_CONFIGURATION_REFERENCE.md)** - SPI parameters and setup
- **[CubeMX Regeneration Guide](CUBEMX_REGENERATION_GUIDE.md)** - Protect custom code from CubeMX regeneration

### System Protection
- **[FreeRTOS Protection Guide](FREERTOS_PROTECTION_GUIDE.md)** - Protect FreeRTOS tasks from CubeMX regeneration

## Configuration Overview

MidiCore uses a modular configuration system that allows you to:
- Enable/disable hardware modules (AINSER64, SRIO, OLED, etc.)
- Configure communication interfaces (UART, SPI, I2C)
- Protect custom code from CubeMX regeneration
- Set up MIOS32-compatible UART mapping

## Quick Start

1. **Module Configuration**: Edit `Config/module_config.h` to enable desired modules
2. **Pin Configuration**: Configure pins in CubeMX according to module requirements
3. **Regeneration Protection**: Follow guides to protect custom code before regenerating with CubeMX

## Related Documentation

- **[Getting Started](../getting-started/)** - Quick start and integration
- **[Hardware](../hardware/)** - Hardware setup and wiring
- **[Development](../development/)** - Technical implementation details
