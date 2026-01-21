# OLED Driver Configuration

MidiCore supports two OLED display drivers for LoopA/MBHP compatibility:

## Supported Displays

### SSD1306 (LoopA Compatible) - **Currently Active**
- **Resolution**: 128×64 pixels
- **Color**: Monochrome (1-bit per pixel)
- **Memory**: 1KB framebuffer
- **Use Case**: LoopA/MBHP hardware compatibility
- **Wiring**: Via J1 connector (pins 1, 2, 4, 5, 14, 15, 16)

### SSD1322 (MidiCore Default)
- **Resolution**: 256×64 pixels
- **Color**: 16-level grayscale (4-bit per pixel)
- **Memory**: 8KB framebuffer
- **Use Case**: Enhanced graphics capability
- **Wiring**: Via J1 connector (same pinout as SSD1306)

## Current Configuration

The system is currently configured for **SSD1306** (LoopA compatible):

```c
// In Config/module_config.h:
#define OLED_DRIVER_SSD1306 1  // 1=SSD1306, 0=SSD1322
```

## Switching Between Drivers

### To Use SSD1306 (LoopA/MBHP Compatible)

1. Edit `Config/module_config.h`:
   ```c
   #define OLED_DRIVER_SSD1306 1
   ```

2. Rebuild the project

3. Flash to board

4. Connect SSD1306 display to J1 connector (7 wires: pins 1, 2, 4, 5, 14, 15, 16)

### To Use SSD1322 (MidiCore Default)

1. Edit `Config/module_config.h`:
   ```c
   #define OLED_DRIVER_SSD1306 0
   ```

2. Rebuild the project

3. Flash to board

4. Connect SSD1322 display to J1 connector (same 7 wires)

## Driver Implementation

Both drivers expose the same API:

```c
void oled_init(void);           // Initialize display
uint8_t* oled_framebuffer(void); // Get framebuffer pointer
void oled_flush(void);          // Update display
void oled_clear(void);          // Clear framebuffer
```

The wrapper header `Hal/oled.h` automatically includes the correct driver based on `OLED_DRIVER_SSD1306` setting.

## Wiring

Both displays use the same J1 connector pinout:

| J1 Pin | Signal | STM32 Pin | Function |
|--------|--------|-----------|----------|
| 1 | GND | GND | Ground |
| 2 | VCC_IN | 3.3V | Power |
| 4 | CLK | PB13 | SPI clock |
| 5 | DIN | PB15 | SPI data |
| 14 | D/C# | PC4 | Data/Command |
| 15 | Res# | PC5 | Reset |
| 16 | CS# | PB12 | Chip Select |

See [Docs/hardware/OLED_WIRING_GUIDE.md](../../Docs/hardware/OLED_WIRING_GUIDE.md) for complete wiring details.

## Initialization Sequences

### SSD1306 (LoopA)
The SSD1306 driver uses the standard LoopA-compatible initialization:
- 128×64 resolution
- Horizontal addressing mode
- Charge pump enabled
- Segment remapping (0xA1)
- COM scan direction remapped (0xC8)

### SSD1322 (MidiCore)
The SSD1322 driver uses grayscale initialization:
- 256×64 resolution
- Linear gray scale table
- Normal display mode
- Full framebuffer update

## UI Compatibility

The UI code automatically adapts to the display resolution:
- **SSD1306**: UI renders in 128×64 monochrome
- **SSD1322**: UI renders in 256×64 grayscale

Graphics functions use `OLED_W` and `OLED_H` macros which adjust automatically based on the active driver.

## Notes

1. **Same Pinout**: Both displays use identical J1 connector wiring
2. **Auto-Detection**: The build system automatically selects the right driver
3. **No Code Changes**: UI code works with both displays without modification
4. **LoopA Hardware**: Use SSD1306 driver for MBHP/LoopA compatibility
5. **Performance**: SSD1306 uses less memory (1KB vs 8KB framebuffer)

## Testing

To test the OLED display:

1. Enable OLED module in `Config/module_config.h`:
   ```c
   #define MODULE_ENABLE_OLED 1
   #define OLED_DRIVER_SSD1306 1  // For LoopA hardware
   ```

2. Build and flash

3. Expected behavior:
   - Display powers on and initializes
   - UI pages render correctly
   - No artifacts or flickering

See [Docs/hardware/OLED_QUICK_TEST.md](../../Docs/hardware/OLED_QUICK_TEST.md) for testing procedures.

## Files

- `Hal/oled.h` - Wrapper header (auto-selects driver)
- `Hal/oled_ssd1306/` - SSD1306 driver (128×64 monochrome)
- `Hal/oled_ssd1322/` - SSD1322 driver (256×64 grayscale)
- `Config/oled_pins.h` - Pin definitions (shared by both drivers)
- `Config/module_config.h` - Driver selection

## Version

- **SSD1306 Driver**: Added 2026-01-21 for LoopA compatibility
- **SSD1322 Driver**: Original MidiCore driver
