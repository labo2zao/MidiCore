# SSD1322 OLED Driver - Technical Reference

## Hardware Configuration

### Display Specifications
- **Controller**: Solomon Systech SSD1322
- **Resolution**: 256 × 64 pixels
- **Color Depth**: 4-bit grayscale (16 levels)
- **Interface**: 4-wire SPI (software bit-bang)
- **Supply Voltage**: 3.3V

### MCU Configuration
- **Processor**: STM32F407VGT6
- **Core**: ARM Cortex-M4F
- **Clock**: 168 MHz
- **Cycle Time**: 5.95 ns per cycle

### Pin Mapping

| MCU Pin | OLED Signal | MIOS32 Equivalent | Function |
|---------|-------------|-------------------|----------|
| PA8 | DC | J15_SER (RS) | Data/Command select |
| PC8 | SCL | J15_E1 | Clock line 1 (dual COM mode) |
| PC9 | SCL | J15_E2 | Clock line 2 (dual COM mode) |
| PC11 | SDA | J15_RW | Serial Data |
| - | CS# | - | Hardwired to GND on module |
| - | RST | - | On-board RC reset circuit |
| GND | GND | GND | Ground |
| 3.3V | VCC | VCC | Power supply |

### GPIO Configuration
All pins configured as:
- **Mode**: GPIO_OUTPUT_PP (Push-Pull)
- **Pull**: GPIO_NOPULL (No internal pull-up/down)
- **Speed**: GPIO_SPEED_FREQ_VERY_HIGH

## SPI Protocol

### SPI Mode 0 Specification
- **CPOL**: 0 (Clock Polarity = 0, clock idles LOW)
- **CPHA**: 0 (Clock Phase = 0, data sampled on first/rising edge)
- **Bit Order**: MSB First (Most Significant Bit transmitted first)
- **Clock Frequency**: ~5 MHz (200 ns period)

### SPI Communication Sequence

#### Single Byte Transmission
```
1. Set data line (MSB of byte)
2. Ensure clock is LOW (idle state)
3. Wait for setup time (101 ns)
4. Clock rises to HIGH → SSD1322 samples data on this edge
5. Wait for hold time (101 ns)
6. Return clock to LOW (prepare for next bit)
7. Shift byte left, repeat for all 8 bits
8. After byte complete: Ensure clock returns to idle LOW
```

#### Command vs Data Selection
- **Command Mode**: DC = LOW (0)
- **Data Mode**: DC = HIGH (1)
- **DC Setup Time**: 60 ns before SPI transmission

### Timing Implementation

#### DWT Cycle Counter
```c
// Initialize DWT (Data Watchpoint and Trace)
CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
DWT->CYCCNT = 0;
DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

// Delay function using cycle counter
static inline void delay_cycles(uint32_t cycles) {
  uint32_t start = DWT->CYCCNT;
  while ((DWT->CYCCNT - start) < cycles);
}
```

#### Timing Values
| Parameter | Cycles | Time @ 168 MHz | SSD1322 Requirement |
|-----------|--------|----------------|---------------------|
| Data setup | 17 | 101.2 ns | >15 ns ✅ |
| Data hold | 17 | 101.2 ns | >10 ns ✅ |
| Clock period | ~34 | ~202 ns | >100 ns ✅ |
| DC setup | 10 | 59.5 ns | Not specified ✅ |
| Clock frequency | - | ~5 MHz | <10 MHz max ✅ |

### Software SPI Bit-Bang Implementation

```c
static inline void spi_write_byte(uint8_t byte) {
  for (uint8_t i = 0; i < 8; i++) {
    // Step 1: Set data line (MSB first)
    if (byte & 0x80)
      HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_SET);
    else
      HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_RESET);
    
    // Step 2: Clock LOW, data setup time
    SCL_LOW();
    delay_cycles(17);  // 101 ns
    
    // Step 3: Clock rises HIGH - SSD1322 samples on this rising edge
    SCL_HIGH();
    delay_cycles(17);  // 101 ns hold time
    
    // Step 4: Next bit
    byte <<= 1;
  }
  
  // Step 5: Return clock to idle LOW (SPI Mode 0)
  SCL_LOW();
  SCL_LOW();  // Done twice for compatibility with MIOS32
}
```

## SSD1322 Initialization Sequence

### Power-Up Timing
1. **Power stabilization**: 300 ms after VDD reaches 3.3V
2. **Reset sequence**: 300 ms for on-board RC circuit
3. **Total pre-init delay**: ~600 ms

### Initialization Commands
```c
cmd(0xFD); data(0x12);              // 1. Unlock OLED driver IC
cmd(0xAE);                          // 2. Display OFF during config
cmd(0x15); data(0x1C); data(0x5B); // 3. Column Address 0x1C-0x5B (256px)
cmd(0x75); data(0x00); data(0x3F); // 4. Row Address 0x00-0x3F (64 rows)
cmd(0xCA); data(0x3F);              // 5. MUX ratio = 64
cmd(0xA0); data(0x14); data(0x11); // 6. Remap (dual COM, dual line)
cmd(0xB3); data(0x00); data(0x0C); // 7. Clock Div & Oscillator
cmd(0xC1); data(0xFF);              // 8. Segment Output Current (max)
cmd(0xC7); data(0x0F);              // 9. Master Current Control (max)
cmd(0xB9);                          // 10. Use linear gray scale table
cmd(0x00);                          //     (0x00 enables linear table)
cmd(0xB1); data(0x56);              // 11. Phase Length
cmd(0xBB); data(0x00);              // 12. Pre-charge Voltage
cmd(0xB6); data(0x08);              // 13. Second Pre-charge Period
cmd(0xBE); data(0x00);              // 14. VCOMH Voltage
cmd(0xA6);                          // 15. Normal Display mode

// 16. Clear display RAM
for (uint8_t row = 0; row < 64; row++) {
  cmd(0x15); data(0x1C);           // Set column start
  cmd(0x75); data(row);            // Set row
  cmd(0x5C);                       // Write RAM command
  for (uint16_t i = 0; i < 128; i++) {
    data(0x00);                    // Clear 256 pixels (128 bytes)
  }
}

cmd(0xAF);                         // 17. Display ON
```

### Initialization Timing
- **Total duration**: ~2100 ms
  - Pre-init delays: 600 ms + 600 ms = 1200 ms
  - Initialization sequence: ~100 ms
  - RAM clear: ~800 ms
  - Post-init delay: 100 ms

## Framebuffer Management

### Memory Layout
- **Size**: 8192 bytes (256 × 64 ÷ 2)
- **Format**: 4-bit grayscale, packed
- **Location**: CCM RAM (Core Coupled Memory)
- **Organization**: Row-major order

### Framebuffer Structure
```
Row 0:  [128 bytes] → 256 pixels (2 pixels per byte)
Row 1:  [128 bytes] → 256 pixels
...
Row 63: [128 bytes] → 256 pixels
Total: 64 rows × 128 bytes/row = 8192 bytes
```

### Pixel Encoding
Each byte contains two 4-bit grayscale pixels:
```
Byte = [High Nibble: Pixel N] [Low Nibble: Pixel N+1]
0x00 = Black-Black
0xFF = White-White
0x77 = Gray-Gray (medium)
```

### Display Update
```c
void oled_flush(void) {
  for (uint8_t row = 0; row < 64; row++) {
    cmd(0x15); data(0x1C);       // Set column start
    cmd(0x75); data(row);        // Set current row
    cmd(0x5C);                   // Write RAM command
    
    uint8_t *row_ptr = &fb[row * 128];
    for (uint16_t i = 0; i < 128; i++) {
      data(row_ptr[i]);          // Transfer row data
    }
  }
}
```

## MIOS32 Compatibility

### Reference Implementation
The implementation is based on MIOS32 (MIDIbox Operating System 32):
- **Repository**: github.com/midibox/mios32
- **Board Support**: mios32/STM32F4xx/mios32_board.c
- **App LCD Driver**: modules/app_lcd/ssd1322/app_lcd.c

### MIOS32 Function Mapping

| MIOS32 Function | MidiCore Equivalent | Purpose |
|-----------------|---------------------|---------|
| `MIOS32_BOARD_J15_SerDataShift()` | `spi_write_byte()` | Bit-bang SPI |
| `MIOS32_BOARD_J15_RS_Set()` | Set `OLED_DC_Pin` | Command/Data select |
| `APP_LCD_Cmd()` | `cmd()` | Send command byte |
| `APP_LCD_Data()` | `data()` | Send data byte |

### J15 Connector Mapping
MIOS32 uses J15 connector for LCD/OLED interfaces:
- **J15:A0 (SER/RS)** → PA8 (DC pin)
- **J15:E1** → PC8 (Clock 1)
- **J15:E2** → PC9 (Clock 2)
- **J15:RW** → PC11 (Data pin)

### Dual COM Mode
Both PC8 and PC9 are clocked together for dual COM line operation, which doubles the refresh rate by driving two rows simultaneously.

## Testing and Validation

### Module Test Framework
```c
// Test selection
#define MODULE_TEST_OLED_SSD1322 1

// Test execution
int result = module_test_oled_ssd1322_run();
```

### Test Coverage

#### 1. GPIO Control Test
Validates each pin can be set HIGH/LOW and read back correctly:
- PA8 (DC)
- PC8 (Clock 1)
- PC9 (Clock 2)
- PC11 (Data)

#### 2. SPI Timing Verification
- DWT cycle counter initialization
- Delay precision measurement
- Clock frequency calculation

#### 3. OLED Initialization Test
- Measures initialization duration
- Verifies test pattern display
- Expected: ~2100 ms total time

#### 4. Display Pattern Tests
Five visual tests, 2 seconds each:
1. All WHITE (0xFF) - Full brightness test
2. All BLACK (0x00) - Contrast test
3. CHECKERBOARD - Pixel resolution test
4. HORIZONTAL STRIPES - Row addressing test
5. GRAYSCALE GRADIENT - Gray level test

### Troubleshooting Protocol

#### Power Verification
```bash
# Measure at OLED module
VCC = 3.3V ± 0.1V
GND = 0V
```

#### GPIO Read-Back Test
```c
HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET);
uint8_t state = HAL_GPIO_ReadPin(OLED_DC_GPIO_Port, OLED_DC_Pin);
// Expected: state == 1
```

#### Logic Analyzer Checkpoints
- **PC8/PC9**: Should toggle 0V ↔ 3.3V during transmission
- **PC11**: Data should change between bytes
- **PA8**: LOW for commands, HIGH for data
- **Frequency**: ~5 MHz clock rate

#### Common Issues

| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| Blank display | No power | Check 3.3V at VCC pin |
| Blank display | Wrong SPI mode | Verify CPOL=0, CPHA=0 |
| Blank display | Timing too fast | Increase delay cycles |
| Garbled display | Wrong bit order | Verify MSB first |
| Partial display | Incorrect init | Check command sequence |

## Performance Considerations

### CPU Usage
- **Bit-bang overhead**: ~400 ns per bit (3.2 μs per byte)
- **Frame update**: ~8192 bytes × 3.2 μs = ~26 ms per frame
- **Max frame rate**: ~38 FPS (theoretical)
- **Typical frame rate**: 20-30 FPS (with processing time)

### Memory Usage
- **Framebuffer**: 8192 bytes in CCM RAM
- **Code size**: ~2 KB (driver + init sequence)
- **Stack usage**: Minimal (<100 bytes)

### Optimization Opportunities
1. **DMA**: Not applicable for software SPI
2. **Hardware SPI**: Could reduce CPU usage but requires pin remapping
3. **Partial updates**: Only flush changed regions
4. **Compression**: RLE for solid color areas

## References

### Datasheets
1. **SSD1322**: Solomon Systech SSD1322 OLED Controller Datasheet
   - Section 8.1.3: MCU Serial Interface
   - Section 10: Command Table
   - Section 11: Electrical Characteristics

2. **STM32F407**: STM32F407xx Reference Manual
   - Section 8: GPIO
   - Section 33: Debug Support (DWT)
   - Section 6: Clock Tree

### Source Code References
1. **MIOS32**: github.com/midibox/mios32
   - mios32/STM32F4xx/mios32_board.c
   - modules/app_lcd/ssd1322/app_lcd.c

2. **MidiCore**: github.com/labodezao/MidiCore
   - Hal/oled_ssd1322/oled_ssd1322.c
   - App/tests/module_tests.c (OLED test)

### Documentation
- OLED_WIRING_GUIDE.md
- OLED_TROUBLESHOOTING.md
- J1_OLED_CONNECTOR_PINOUT.md
- module_tests.h (test documentation)

## Revision History

| Version | Date | Description |
|---------|------|-------------|
| 1.0 | 2026-01 | Initial implementation with Mode 3 (incorrect) |
| 2.0 | 2026-01 | Fixed to Mode 0, MIOS32-compatible |
| 2.1 | 2026-01 | Added DWT timing, fixed pin naming |
| 2.2 | 2026-01 | Integrated module test framework |

## License
This implementation is part of the MidiCore project and follows the project's licensing terms.
