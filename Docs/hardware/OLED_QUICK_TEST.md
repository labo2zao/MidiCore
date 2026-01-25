# OLED Quick Test Guide

Quick reference for testing OLED display functionality on MidiCore.

## Quick Test Steps

### 1. Enable OLED Module

Edit `Config/module_config.h`:

```c
#define MODULE_ENABLE_OLED 1
#define MODULE_ENABLE_SPI_BUS 1  // Required dependency
```

### 2. Build and Flash

Build the project in STM32CubeIDE and flash to your board.

### 3. Expected Behavior

On startup, you should see:
- Display powers on
- Initialization sequence completes
- UI pages render correctly
- No flickering or artifacts

### 4. Visual Test

The display should show:
- **Top Bar** (12px): Status information
- **Main Area** (40px): Content area
- **Bottom Bar** (12px): Menu/functions
- **Crisp Graphics**: No distortion
- **Smooth Updates**: No visible lag

## Quick Wiring Verification

### Check These Connections

✅ **Power**:
- [ ] 3.3V connected to VDD/VCC
- [ ] GND connected

✅ **SPI Signals**:
- [ ] PB13 → SCK (Clock)
- [ ] PB15 → MOSI (Data)

✅ **Control Signals**:
- [ ] PB12 → CS (Chip Select)
- [ ] PC4 → DC (Data/Command)
- [ ] PC5 → RST (Reset)

## Common Issues and Fixes

### Issue: Blank Display

**Check**:
1. Power supply (3.3V stable?)
2. Wiring connections (loose wires?)
3. Display contrast (some displays need adjustment)
4. Module enabled in config

**Fix**:
```c
// Verify in Config/module_config.h
#define MODULE_ENABLE_OLED 1
```

### Issue: Garbled Display

**Check**:
1. SPI clock speed (too fast?)
2. Wire length (keep < 15cm)
3. Ground connection (solid?)

**Fix**: Reduce SPI speed in CubeMX:
- Change prescaler from /2 to /4 or /8

### Issue: Flickering

**Check**:
1. Power supply noise
2. Refresh rate conflicts
3. Framebuffer corruption

**Fix**: Add decoupling capacitors (100nF) near display power pins

### Issue: Wrong Resolution

**Check display type**:
- SSD1322: 256×64 (default)
- SSD1306: 128×64 (LoopA)

**Fix**: Ensure correct driver for your display type

## Testing Specific Features

### Test UI Pages

Use encoders or buttons to navigate:
- Song Mode (scenes grid)
- MIDI Monitor
- Config Editor
- LiveFX Control
- Rhythm Trainer

### Test Grayscale (SSD1322)

The display should show:
- 16 gray levels
- Smooth gradients
- Anti-aliased text

### Test Response Time

The display should:
- Update < 50ms
- No tearing
- Smooth animations

## Hardware Compatibility

### MBHP Compatible

✅ Works with MBHP_CORE_STM32F4 shields  
✅ Standard MIOS32 pin assignment  
✅ LoopA hardware compatible  

Reference: http://www.ucapps.de/mbhp/mbhp_lcd_ssd1306_single_mios32.pdf

### Tested Displays

| Display | Resolution | Compatibility | Notes |
|---------|------------|---------------|-------|
| SSD1322 | 256×64 | ✅ Default | 16 grayscale levels |
| SSD1306 | 128×64 | ✅ Compatible | Monochrome, LoopA standard |

## Module Test Mode

For isolated OLED testing:

```c
// In build settings, add preprocessor define:
MODULE_TEST_UI
```

This tests:
- Display initialization
- Framebuffer operations
- SPI communication
- UI rendering

## Measurement Points

Use oscilloscope/logic analyzer to verify:

1. **SCK (PB13)**: Should see clock pulses during SPI transfer
2. **MOSI (PB15)**: Should see data bits
3. **CS (PB12)**: Should go low during transfer
4. **DC (PC4)**: Should toggle between command/data
5. **RST (PC5)**: Should pulse low on startup

## Expected Signals

### SPI Clock (SCK)
- Frequency: Up to 21 MHz
- Idle: Low
- Active: Square wave

### Data (MOSI)
- Changes on clock edge
- MSB first
- 8-bit transfers

### Chip Select (CS)
- Idle: High
- Active: Low during transfer

### Data/Command (DC)
- Low = Command byte
- High = Data bytes

## Performance Metrics

### Normal Operation

- **Initialization**: < 100ms
- **Frame Update**: < 50ms
- **SPI Transfer**: ~8KB in ~3ms @ 21MHz
- **CPU Usage**: < 5% for UI updates

### If Performance Issues

Check:
1. FreeRTOS task priorities
2. SPI DMA configuration
3. Framebuffer location (CCMRAM recommended)
4. Mutex contention

## Debug Output

Enable debug UART for diagnostic messages:

```c
#define MODULE_ENABLE_UART_DEBUG 1
```

Look for:
```
[OLED] Init starting...
[OLED] Reset pulse complete
[OLED] Display configured
[OLED] Init complete
[UI] UI system initialized
```

## Success Criteria

✅ Display lights up on power-on  
✅ No visible artifacts or distortion  
✅ UI pages render correctly  
✅ Encoders/buttons navigate properly  
✅ Text is readable  
✅ Graphics are crisp  
✅ No flickering or tearing  

## Next Steps

After successful OLED test:
1. Test UI navigation
2. Verify encoder input
3. Check button responses
4. Test all UI pages
5. Verify looper visualization

## Resources

- [OLED Wiring Guide](OLED_WIRING_GUIDE.md) - Complete wiring documentation
- [UI Testing Guide](../user-guides/UI_PAGE_TESTING_GUIDE.md) - UI page testing
- [Testing Quick Start](../testing/TESTING_QUICKSTART.md) - Module testing framework
- [Module Configuration](../configuration/README_MODULE_CONFIG.md) - Enable/disable modules

## Support

If issues persist after following this guide:
1. Check wiring against [OLED_WIRING_GUIDE.md](OLED_WIRING_GUIDE.md)
2. Verify power supply stability
3. Test with known-good display
4. Review SPI bus configuration
5. Check for hardware conflicts

---

**Quick Reference Created**: 2026-01-21  
**For**: MidiCore OLED Testing  
**Display Types**: SSD1322 (default) / SSD1306 (LoopA compatible)
