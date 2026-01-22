# SSD1322 OLED Driver Test Suite

Simple standalone test for validating the SSD1322 OLED driver with UART debug output.

## Features

- **GPIO Control Test**: Verifies all pins (PA8, PC8, PC9, PC11) can be read/written
- **SPI Communication Test**: Validates software bit-bang SPI implementation  
- **OLED Initialization**: Tests complete SSD1322 init sequence
- **Display Patterns**: Shows various test patterns (white, black, checkerboard, stripes, gradient)
- **UART Debug Output**: Detailed progress and diagnostics via UART

## Quick Start

### 1. Add to your main.c

```c
#include "App/tests/app_test_oled_ssd1322.h"

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART2_UART_Init();  // Or your debug UART
  
  // Run OLED test
  test_oled_init();
  test_oled_run();
  
  while (1) {
    // Your application code
  }
}
```

### 2. Monitor UART Output

Connect to your debug UART (default UART2) at 115200 baud and you'll see:

```
=====================================
  SSD1322 OLED Driver Test Suite
=====================================
Version: 1.0
Target: STM32F407VGT6 @ 168 MHz
Display: SSD1322 256x64 OLED

=== Pin Mapping (MIOS32 Compatible) ===
PA8  = DC   (Data/Command)
PC8  = SCL  (Clock 1)
PC9  = SCL  (Clock 2, dual COM)
PC11 = SDA  (Data)

=== GPIO Control Test ===
Testing PA8 (DC pin)...
  PA8 LOW=0, HIGH=1 [PASS]
...

Overall: [SUCCESS]
```

## Test Functions

### test_oled_run()

Complete test sequence that validates all functionality:
1. GPIO pin control
2. SPI communication
3. OLED initialization  
4. Display test patterns

### test_oled_minimal()

Minimal test that only sends:
- Unlock command (0xFD 0x12)
- Display ON (0xAF)

Use this to quickly check if OLED hardware responds.

## Expected Display Behavior

During `test_oled_run()`, you should see on the OLED:

1. **Init sequence** (1 second):
   - White bar on top 4 rows
   - Gray fill on remaining rows

2. **Pattern tests** (2 seconds each):
   - All white
   - All black
   - Checkerboard pattern
   - Horizontal stripes
   - Grayscale gradient

3. **Final state**: Clear/blank display

## Troubleshooting

### Display stays blank

Check UART output for specific failures:

- **GPIO test fails**: Pin configuration problem in CubeMX
- **SPI test fails**: Software bit-bang timing issue
- **Init completes but no display**: 
  - Power: Measure 3.3V at OLED VCC
  - Wiring: Verify all 5 connections
  - Module: May be incompatible OLED variant

### Use logic analyzer

The test provides timing information for verification:
- Clock frequency: ~5 MHz (200 ns period)
- Data setup: 101 ns before rising edge
- Data hold: 101 ns during clock HIGH
- SPI Mode 0: Clock idle LOW, sample on rising edge

## Configuration

Edit `test_debug.h` to select your debug UART:

```c
#define TEST_DEBUG_UART_PORT 1  // 0=UART1, 1=UART2, 2=UART3, 3=UART5
```

## Files

- `app_test_oled_ssd1322.c/h` - Main test implementation
- `test_debug.c/h` - UART debug output functions
- `oled_ssd1322.c/h` - Driver being tested

## Technical Details

### SPI Mode 0 (MIOS32 Compatible)

- **CPOL = 0**: Clock idle LOW
- **CPHA = 0**: Data sampled on rising edge
- **Bit order**: MSB first
- **Timing**: Verified against SSD1322 datasheet

### Pin Mapping

Matches MIOS32/LoopA standard:
- `J15_SER` (PA8) = DC pin
- `J15_E1` (PC8) = Clock 1  
- `J15_E2` (PC9) = Clock 2 (dual COM mode)
- `J15_RW` (PC11) = Data pin

### Timing

- Data setup: 17 cycles @ 168 MHz = 101.2 ns (meets >15 ns requirement)
- Data hold: 17 cycles @ 168 MHz = 101.2 ns (meets >10 ns requirement)
- Clock period: ~200 ns (meets >100 ns requirement)

## License

Compatible with MidiCore project license.
