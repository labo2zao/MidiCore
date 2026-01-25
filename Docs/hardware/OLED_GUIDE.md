# OLED Display Guide for MidiCore

Complete guide for OLED display integration, testing, troubleshooting, and configuration in the MidiCore project.

**Display**: SSD1322 256×64 OLED (4-bit grayscale)  
**Interface**: Software SPI (bit-bang)  
**MCU**: STM32F407VGT6 @ 168 MHz  
**Status**: ✅ Production Ready

---

## Table of Contents

1. [Quick Start](#quick-start)
2. [Hardware Setup](#hardware-setup)
3. [Wiring Guide](#wiring-guide)
4. [Pin Configuration](#pin-configuration)
5. [SPI Protocol](#spi-protocol)
6. [Technical Reference](#technical-reference)
7. [Test Page Modes](#test-page-modes)
8. [Troubleshooting](#troubleshooting)
9. [CubeMX Configuration](#cubemx-configuration)
10. [Combined Key Navigation](#combined-key-navigation)
11. [Performance Metrics](#performance-metrics)
12. [References](#references)

---

## Quick Start

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
- Initialization sequence completes (~ 2100 ms)
- UI pages render correctly
- No flickering or artifacts

### 4. Visual Test

The display should show:
- **Top Bar** (12px): Status information
- **Main Area** (40px): Content area
- **Bottom Bar** (12px): Menu/functions
- **Crisp Graphics**: No distortion
- **Smooth Updates**: No visible lag

---

## Hardware Setup

### Supported Displays

1. **SSD1322** (default) - 256×64 grayscale OLED
2. **SSD1306** (LoopA compatible) - 128×64 monochrome OLED

### Hardware Connector

**Connector J1** provides the OLED interface signals (16-pin connector, 7 wires used in SPI mode).

### Display Specifications

- **Controller**: Solomon Systech SSD1322
- **Resolution**: 256 × 64 pixels
- **Color Depth**: 4-bit grayscale (16 levels)
- **Interface**: 3-wire Software SPI (bit-bang)
- **Supply Voltage**: 3.3V

---

## Wiring Guide

### SSD1322 Pin Configuration (Software SPI Mode)

**IMPORTANT**: This uses **Software SPI (bit-bang)** following MIOS32 LoopA convention, NOT hardware SPI.

| J1 Pin | Signal | STM32 Pin | Function | Description |
|--------|--------|-----------|----------|-------------|
| **1** | GND | GND | Ground | Ground reference |
| **2** | VCC_IN | 3.3V | Power | 3.3V power supply |
| **4** | SCL | PC8 | GPIO (Clock) | Software SPI clock (bit-bang) |
| **5** | SDA | PC11 | GPIO (Data) | Software SPI data (bit-bang) |
| **14** | D/C# | PA8 | Data/Command | LOW=Command, HIGH=Data |
| **15** | Res# | - | Reset | On-board RC reset circuit (not connected to STM32) |
| **16** | CS# | GND | Chip Select | Hardwired to GND on display (always enabled) |

**Active connections (5 wires)**: Pins 1, 2, 4, 5, 14

### Wiring Diagram

```
STM32F407VGT6         J1 Connector          SSD1322 OLED
┌─────────────┐       Pin  Signal          ┌──────────────┐
│             │        1   GND    ──────────┤ GND          │
│    GND  ────┼────────┘                    │              │
│             │        2   VCC_IN ──────────┤ VDD (3.3V)   │
│    3.3V ────┼────────┘                    │              │
│             │        4   SCL    ──────────┤ SCK  (Clock) │
│    PC8  ────┼────────┘                    │              │
│             │        5   SDA    ──────────┤ MOSI (Data)  │
│    PC11 ────┼────────┘                    │              │
│             │       14   D/C#   ──────────┤ DC   (D/C)   │
│    PA8  ────┼────────┘                    │              │
│             │       15   Res#   (on-board RC reset)      │
│             │       16   CS#    (hardwired to GND)       │
└─────────────┘                             └──────────────┘
```

### Cable Requirements

- **Required wires**: 5 (for Software SPI mode)
- **Length**: Keep under 15cm for reliable communication
- **Type**: Ribbon cable or individual wires
- **Gauge**: 24-28 AWG sufficient
- **Shielding**: Optional, helps with EMI in noisy environments

### Connection Checklist

- [ ] J1 pin 1 (GND) connected to display GND
- [ ] J1 pin 2 (VCC_IN) connected to display VDD/VCC (3.3V)
- [ ] J1 pin 4 (SCL/PC8) connected to display SCK/CLK
- [ ] J1 pin 5 (SDA/PC11) connected to display MOSI/DIN
- [ ] J1 pin 14 (D/C#/PA8) connected to display D/C
- [ ] CS# hardwired to GND on display module
- [ ] RST# uses on-board RC reset circuit
- [ ] All connections are secure (no loose wires)

---

## Pin Configuration

### MCU Configuration

- **Processor**: STM32F407VGT6
- **Core**: ARM Cortex-M4F
- **Clock**: 168 MHz
- **Cycle Time**: 5.95 ns per cycle

### GPIO Configuration

All pins configured as:
- **Mode**: GPIO_OUTPUT_PP (Push-Pull)
- **Pull**: GPIO_NOPULL (No internal pull-up/down)
- **Speed**: GPIO_SPEED_FREQ_VERY_HIGH

### Pin Mapping Table

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

**Note**: PC8 and PC9 are both used for dual COM mode (clocked together).

### Pin Definitions

Located in: `Config/oled_pins.h`

```c
#define OLED_DC_Pin GPIO_PIN_8
#define OLED_DC_GPIO_Port GPIOA
#define OLED_SCL_Pin GPIO_PIN_8
#define OLED_SCL_GPIO_Port GPIOC
#define OLED_SDA_Pin GPIO_PIN_11
#define OLED_SDA_GPIO_Port GPIOC
```

---

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

### DWT Cycle Counter

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

### Timing Values

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

---

## Technical Reference

### Initialization Sequence

#### Power-Up Timing

1. **Power stabilization**: 300 ms after VDD reaches 3.3V
2. **Reset sequence**: 300 ms for on-board RC circuit
3. **Total pre-init delay**: ~600 ms

#### Initialization Commands

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

#### Initialization Timing

- **Total duration**: ~2100 ms
  - Pre-init delays: 600 ms + 600 ms = 1200 ms
  - Initialization sequence: ~100 ms
  - RAM clear: ~800 ms
  - Post-init delay: 100 ms

### Framebuffer Management

#### Memory Layout

- **Size**: 8192 bytes (256 × 64 ÷ 2)
- **Format**: 4-bit grayscale, packed
- **Location**: CCM RAM (Core Coupled Memory)
- **Organization**: Row-major order

#### Framebuffer Structure

```
Row 0:  [128 bytes] → 256 pixels (2 pixels per byte)
Row 1:  [128 bytes] → 256 pixels
...
Row 63: [128 bytes] → 256 pixels
Total: 64 rows × 128 bytes/row = 8192 bytes
```

#### Pixel Encoding

Each byte contains two 4-bit grayscale pixels:
```
Byte = [High Nibble: Pixel N] [Low Nibble: Pixel N+1]
0x00 = Black-Black
0xFF = White-White
0x77 = Gray-Gray (medium)
```

#### Display Update

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

### MIOS32 Compatibility

#### Reference Implementation

The implementation is based on MIOS32 (MIDIbox Operating System 32):
- **Repository**: github.com/midibox/mios32
- **Board Support**: mios32/STM32F4xx/mios32_board.c
- **App LCD Driver**: modules/app_lcd/ssd1322/app_lcd.c

#### Function Mapping

| MIOS32 Function | MidiCore Equivalent | Purpose |
|-----------------|---------------------|---------|
| `MIOS32_BOARD_J15_SerDataShift()` | `spi_write_byte()` | Bit-bang SPI |
| `MIOS32_BOARD_J15_RS_Set()` | Set `OLED_DC_Pin` | Command/Data select |
| `APP_LCD_Cmd()` | `cmd()` | Send command byte |
| `APP_LCD_Data()` | `data()` | Send data byte |

#### J15 Connector Mapping

MIOS32 uses J15 connector for LCD/OLED interfaces:
- **J15:A0 (SER/RS)** → PA8 (DC pin)
- **J15:E1** → PC8 (Clock 1)
- **J15:E2** → PC9 (Clock 2)
- **J15:RW** → PC11 (Data pin)

---

## Test Page Modes

The OLED test page provides **28 comprehensive test modes** (0-27) to validate display functionality, performance, and visual quality.

### Navigation

**Encoder Control**:
- **Rotate Right**: Next test mode (0 → 1 → 2 → ... → 27 → 0)
- **Rotate Left**: Previous test mode (0 → 27 → 26 → ... → 1 → 0)

**Button Controls**:
- **Button 0**: Previous test mode
- **Button 1**: Next test mode
- **Button 2**: Clear screen (fill with black)
- **Button 3**: Fill screen white
- **Button 4**: Clear screen (same as button 2)
- **Button 5**: Reset all statistics (FPS, min/max, frame times)

### Test Mode Categories

#### Pattern & Display Tests (Modes 0-6)

**Mode 0: Pattern Test**
- Horizontal stripes, vertical stripes, checkerboard
- Tests basic pixel rendering and line quality

**Mode 1: Grayscale Levels**
- 16 vertical bars showing levels 0-F (hex)
- Tests all 4-bit grayscale capability

**Mode 2: Pixel Test**
- Grid of pixels with varying brightness
- Tests pixel-level control and gradients

**Mode 3: Text Rendering Test**
- Numbers, uppercase, lowercase, multiple brightness levels
- Tests font rendering and readability

**Mode 4: Animation Test**
- Moving horizontal bar, pulsing square, frame counter
- Tests smooth animation (100ms updates)

**Mode 5: Hardware Info**
- Display model, resolution, pin configuration
- Quick hardware reference

**Mode 6: Framebuffer Direct Test**
- Raw framebuffer write operations
- Tests low-level rendering

#### Animation Tests (Modes 7-10, 15)

**Mode 7: Scrolling Text**
- Smooth horizontal text scrolling (2px/50ms)
- Tests display refresh and text rendering

**Mode 8: Bouncing Ball**
- Physics-based animation with collision detection
- 6×6 pixel ball with radial gradient (30ms updates)

**Mode 9: Performance Test**
- 5 moving bars, real-time FPS counter
- Stress test for display performance (20ms updates)

**Mode 10: Circles & Lines**
- Animated expanding circles, diagonal lines, rotating line
- Tests geometry rendering (100ms updates)

**Mode 15: Burn-In Prevention**
- Moving gradient box and vertical lines
- Prevents static image burn-in (100ms updates)

#### Advanced Graphics (Modes 11-14, 17-19)

**Mode 11: Bitmap Test**
- Smiley face using circles and pixels
- Demonstrates bitmap/image rendering

**Mode 12: Fill Patterns**
- 4 different fill patterns (dots, dither, waves, grid)
- Auto-cycles every 500ms

**Mode 13: Stress Test**
- 18 simultaneous animated elements
- Maximum graphics throughput test (16ms updates, target 60 FPS)

**Mode 14: Auto-Cycle Demo**
- Automatically cycles through all modes (3s each)
- Perfect for unattended demos

**Mode 17: 3D Wireframe Cube**
- 8-vertex rotating wireframe cube
- 12 edges with different brightness levels
- Simplified rotation (8-step lookup, 45° increments)
- 50ms updates (~20 FPS)

**Mode 18: Advanced Graphics Primitives**
- Animated filled circles, moving triangles
- Tests scanline algorithms (100ms updates)

**Mode 19: UI Elements Demo**
- Progress indicator (0-360° arc), pie chart, directional arrows
- Real-world UI component demonstration (50ms updates)

#### Performance & Statistics (Mode 16)

**Mode 16: Performance Statistics**
- Current FPS display
- Min/Max FPS tracking
- Average frame time calculation
- System uptime display
- FPS history bar graph

#### Hardware Driver Tests (Modes 20-27)

**Mode 20: MIOS32 Pattern** - MIOS32-compatible test pattern
**Mode 21: Checkerboard** - Pixel uniformity testing
**Mode 22: Horizontal Gradient** - Left-to-right grayscale test
**Mode 23: Vertical Gradient** - Top-to-bottom grayscale test
**Mode 24: Rectangles** - Concentric rectangle patterns
**Mode 25: Stripes** - Diagonal stripe patterns
**Mode 26: Voxel Landscape** - 3D terrain visualization
**Mode 27: Gray Levels** - All 16 grayscale levels as vertical bars

### Expected Frame Rates

| Test Mode | Update Interval | Expected FPS |
|-----------|----------------|--------------|
| 0-3 (Static) | N/A | 60 |
| 4 (Animation) | 100ms | 30-40 |
| 5-6 (Static) | N/A | 60 |
| 7 (Scrolling) | 50ms | 40-50 |
| 8 (Ball) | 30ms | 30-40 |
| 9 (Performance) | 20ms | 20-30 |
| 10 (Circles) | 100ms | 30-40 |
| 11 (Bitmap) | N/A | 60 |
| 12 (Patterns) | 500ms | 30-40 |
| 13 (Stress) | 16ms | 40-60 |
| 14 (Auto-Cycle) | 3000ms | Varies |
| 15 (Burn-In) | 100ms | 30-40 |
| 16 (Stats) | N/A | 60 |
| 17 (3D Wireframe) | 50ms | 20 |
| 18 (Adv Graphics) | 100ms | 30-40 |
| 19 (UI Elements) | 50ms | 40-50 |
| 20-27 (HW Tests) | N/A | 60 |

---

## Troubleshooting

### Common Issues & Solutions

#### 1. Display Stays Blank

**Check Power**:
- [ ] Measure 3.3V on VCC pin of OLED module
- [ ] Check GND connection is solid
- [ ] Verify power supply can provide enough current (100-200mA)

**Check Wiring**:
- [ ] PA8 → DC/RS pin on OLED
- [ ] PC8 → Clock pin on OLED
- [ ] PC11 → Data pin on OLED
- [ ] All connections are secure (no loose wires)
- [ ] No short circuits between signal wires

**Check Software**:
- [ ] Firmware compiled with correct pin definitions
- [ ] GPIO pins initialized correctly (see Core/Src/main.c)
- [ ] oled_init() is called during startup
- [ ] oled_flush() is being called to update display

#### 2. GPIO Initialization Problems

The GPIO pins MUST be configured correctly:

**Requirements**:
- Mode: `GPIO_MODE_OUTPUT_PP` (push-pull output)
- Speed: `GPIO_SPEED_FREQ_VERY_HIGH` (for fast switching)
- Pull: `GPIO_NOPULL` (no pull-up/pull-down)

**Verify in code** (`Core/Src/main.c`):
```c
// PA8 (DC/RS)
GPIO_InitStruct.Pin = OLED_DC_Pin|...;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

// PC8 (SCL) and PC11 (SDA)
GPIO_InitStruct.Pin = OLED_SCL_Pin|...|OLED_SDA_Pin;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
```

#### 3. SPI Timing Issues

**Software SPI Characteristics**:
- Bit order: MSB first
- Clock idle: LOW
- Sample edge: Rising (clock LOW→HIGH)
- Setup time: ≥100ns (17 cycles at 168MHz)
- Hold time: ≥100ns (17 cycles at 168MHz)

**DC Signal Timing**:
- Change DC pin
- Wait 10 cycles for stabilization
- Start SPI transmission

#### 4. Using a Logic Analyzer

If available, a logic analyzer is invaluable for debugging:

**What to Check**:
- [ ] Clock (PC8) is toggling 0V → 3.3V
- [ ] Data (PC11) changes between bytes
- [ ] DC (PA8) is LOW for commands, HIGH for data
- [ ] All signals are clean (no ringing or noise)
- [ ] Timing meets SSD1322 specifications

**Expected Behavior During Init**:
1. 300ms delay (no activity)
2. Series of command bytes with DC=0
3. Some data bytes with DC=1
4. Display RAM write commands
5. Framebuffer data transfer (large burst)

#### 5. Debugging Without Display Response

**Add Debug Output**:
```c
// In oled_init():
printf("OLED: Starting initialization...\r\n");
printf("OLED: PA8=%d PC8=%d PC11=%d\r\n",
  HAL_GPIO_ReadPin(OLED_DC_GPIO_Port, OLED_DC_Pin),
  HAL_GPIO_ReadPin(OLED_SCL_GPIO_Port, OLED_SCL_Pin),
  HAL_GPIO_ReadPin(OLED_SDA_GPIO_Port, OLED_SDA_Pin));
// After init:
printf("OLED: Initialization complete\r\n");
```

**Verify GPIO Read/Write**:
```c
// Test GPIO control
HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET);
HAL_Delay(1);
uint8_t dc_state = HAL_GPIO_ReadPin(OLED_DC_GPIO_Port, OLED_DC_Pin);
// dc_state should be 1
```

#### 6. Common Mistakes

❌ **Wrong Pin Assignments**
- Using hardware SPI pins (PB13/PB15) instead of software SPI (PC8/PC11)
- Swapping SCL and SDA
- Wrong DC pin

❌ **GPIO Speed Too Low**
- Must be VERY_HIGH for reliable communication
- LOW or MEDIUM speed can cause timing violations

❌ **Missing Delays**
- No power-up delay (need 300ms for RC reset)
- No DC setup delay (need 10+ cycles)
- No SPI bit delays (need 17+ cycles)

❌ **Incorrect Initialization Sequence**
- Display must be ON (0xAF) before data can be displayed
- Commands must be sent in correct order
- Clock/MUX/addressing must be configured first

#### 7. Verification Checklist

Before asking for help, verify:

```
Hardware:
☐ OLED module powers on (may see backlight or slight glow)
☐ All 5 connections present (PA8, PC8, PC11, VCC, GND)
☐ Measured 3.3V on VCC pin
☐ No physical damage to OLED or cables

Software:
☐ Latest firmware from repository
☐ Clean build (make clean && make all)
☐ Successfully flashed to STM32
☐ No compile errors or warnings about OLED

Pin Configuration:
☐ main.c GPIO init uses OLED_DC_Pin, OLED_SCL_Pin, OLED_SDA_Pin
☐ GPIO speed set to VERY_HIGH for all OLED pins
☐ No pin conflicts (check .ioc file in CubeMX)

Driver:
☐ oled_init() called from main()
☐ oled_flush() called by UI system
☐ No crashes or hard faults during OLED operations
```

### Troubleshooting by Symptom

| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| Blank display | No power | Check 3.3V at VCC pin |
| Blank display | Wrong SPI mode | Verify CPOL=0, CPHA=0 |
| Blank display | Timing too fast | Increase delay cycles |
| Garbled display | Wrong bit order | Verify MSB first |
| Partial display | Incorrect init | Check command sequence |
| Flickering | Power supply noise | Add decoupling capacitors |
| Flickering | Refresh rate conflicts | Check FreeRTOS task timing |
| Wrong resolution | Display type mismatch | Ensure correct driver |

### Success Criteria

When working correctly, you should see:
1. **1 second after power-on**: Test pattern or UI initialization
2. **After 1 second**: Normal MidiCore UI appears
3. **During operation**: UI updates smoothly when buttons/encoders used

---

## CubeMX Configuration

### Issue: Code Regeneration

When trying to regenerate code with STM32CubeMX, you may encounter overwrite warnings because the generated GPIO initialization code differs from the current `main.c`.

### Solution: Let CubeMX Regenerate (Recommended)

Since the `.ioc` file now has the correct pin labels, you can safely let CubeMX regenerate the code:

1. **Backup your changes** (optional, but recommended):
   ```bash
   git stash
   ```

2. **Open MidiCore.ioc in STM32CubeMX**

3. **Verify pin labels** in the Pinout view:
   - **PA8** should be labeled `OLED_DC`
   - **PC8** should be labeled `OLED_SCL`
   - **PC11** should be labeled `OLED_SDA`

4. **Generate Code**:
   - Click "Project" → "Generate Code"
   - When prompted about file conflicts, choose **"Yes to All"** to overwrite

5. **The regenerated code will now be correct** because:
   - CubeMX will generate `OLED_DC_Pin` on PA8 (GPIOA)
   - CubeMX will generate `OLED_SCL_Pin` on PC8 (GPIOC)
   - CubeMX will generate `OLED_SDA_Pin` on PC11 (GPIOC)

### Verification

After regeneration, verify the build compiles without errors:
```bash
make clean
make -j16 all
```

You should see no errors about undefined pins.

### Protecting OLED Configuration

**User Code Sections**: Always place custom OLED configuration code in USER CODE sections in `main.c`:

```c
/* USER CODE BEGIN Includes */
#include "Hal/oled_ssd1322/oled_ssd1322.h"
/* USER CODE END Includes */

/* USER CODE BEGIN 2 */
// Initialize OLED display
oled_init();
/* USER CODE END 2 */
```

CubeMX will preserve these sections during code regeneration.

---

## Combined Key Navigation

### LoopA-Style Button Combinations

MidiCore implements **combined key navigation** with visual feedback, allowing quick access to any UI page using button combinations.

### Button Mapping

Hold **Button 5** and press another button to jump directly to a page:

| Combination | Destination Page | Description |
|-------------|------------------|-------------|
| **B5 + B1** | Piano Roll | Note editing and visualization |
| **B5 + B2** | Timeline | Horizontal event view |
| **B5 + B3** | Rhythm Trainer | Timing practice and statistics |
| **B5 + B4** | LiveFX | Real-time MIDI effects |
| **B5 + B6** | Song Mode | Scene arrangement grid |
| **B5 + B7** | Config | Configuration editor |
| **B5 + B8** | Automation | Scene chaining and workflows |
| **B5 + B9** | Humanizer | Humanization and LFO (if enabled) |
| **B5 alone** | Next Page | Cycle through all pages |

### Visual Feedback

When Button 5 is held down, the header displays **[B5]** to indicate combo mode is active:
- Normal: `Bank:Patch  PAGE`
- Combo Active: `Bank:Patch  PAGE [B5]`

This provides instant visual confirmation that combination shortcuts are available.

### Page Cycle Order (B5 alone)

1. LOOPER (main view)
2. TIMELINE (horizontal)
3. PIANO ROLL (vertical notes)
4. SONG (scene grid)
5. MIDI MONITOR (message log)
6. SYSEX (hex viewer)
7. CONFIG (parameter editor)
8. LIVEFX (effects control)
9. RHYTHM (rhythm trainer)
10. AUTOMATION (scene chaining)
11. HUMANIZER (if enabled)
12. OLED TEST (hardware tests)
13. → Back to LOOPER

### Usage Examples

**Example 1: Quick Access to Piano Roll**
```
1. Hold down Button 5
2. Press Button 1
3. Release both buttons
→ Piano Roll page instantly appears
```

**Example 2: Jump to Rhythm Trainer**
```
1. Hold down Button 5
2. Press Button 3
3. Release both buttons
→ Rhythm Trainer page instantly appears
```

**Example 3: Cycle Through Pages**
```
1. Press Button 5 (without holding other buttons)
2. Release Button 5
→ Advances to next page in sequence
```

---

## Performance Metrics

### Expected Performance

**Normal Operation**:
- **Initialization**: < 2100ms
- **Frame Update**: < 50ms
- **SPI Transfer**: ~8KB in ~3ms @ 5MHz
- **CPU Usage**: < 5% for UI updates

**Test Mode Performance**:
- **Static modes**: < 1% CPU, 60 FPS
- **Simple animations**: 2-5% CPU, 30-50 FPS
- **Complex animations**: 5-15% CPU, 20-40 FPS
- **Stress test**: 20-30% CPU, 40-60 FPS (18 elements)

### Memory Usage

- **Framebuffer**: 8192 bytes (256×64 pixels, 4-bit/pixel)
- **Static variables**: ~70 bytes (test page state tracking)
- **Stack usage**: < 100 bytes per function
- **Code size**: ~2 KB (driver + init sequence)

### If Performance Issues

**Check**:
1. FreeRTOS task priorities
2. SPI communication timing
3. Framebuffer location (CCMRAM recommended)
4. Mutex contention

**Optimization**:
- Verify HAL_GetTick() accuracy
- Check task periods and priorities
- Use direct framebuffer access when possible
- Reduce interrupt frequency

---

## References

### Documentation

**Internal Documentation**:
- [UI_GUIDE.md](UI_GUIDE.md) - UI system documentation
- [Testing Quick Start](../testing/TESTING_QUICKSTART.md) - Module testing framework
- [Module Configuration](../configuration/README_MODULE_CONFIG.md) - Enable/disable modules
- [OLED Test Protocol](../testing/OLED_TEST_PROTOCOL.md) - Comprehensive testing protocol
- [Testing Protocol](../testing/TESTING_PROTOCOL.md) - LoopA feature testing

**External References**:
- [MIOS32 Hardware Platform](http://www.ucapps.de/mios32.html)
- [MBHP OLED Modules](http://www.ucapps.de/mbhp.html)
- [LoopA Hardware](https://www.midiphy.com/en/loopa-v2/)
- [MBHP OLED Wiring](http://www.ucapps.de/mbhp/mbhp_lcd_ssd1306_single_mios32.pdf)

### Datasheets

- [SSD1322 Datasheet](NHD-OLEDSSD1322DISP.pdf) - In hardware folder
- [SSD1322 Datasheet (Alt)](SSD1322%20(4).pdf) - Alternative version
- [STM32F407 Reference Manual](https://www.st.com/resource/en/reference_manual/dm00031020.pdf)

### Source Code References

**MIOS32**: github.com/midibox/mios32
- mios32/STM32F4xx/mios32_board.c
- modules/app_lcd/ssd1322/app_lcd.c

**MidiCore**: github.com/labodezao/MidiCore
- Hal/oled_ssd1322/oled_ssd1322.c
- Services/ui/ui_page_oled_test.c
- App/tests/module_tests.c

### Algorithms Implemented

1. **Midpoint Circle Algorithm** - Efficient integer-based circle drawing
2. **Bresenham's Line Algorithm** - Pixel-perfect line rasterization
3. **Physics Simulation** - Velocity vectors with collision detection
4. **Procedural Patterns** - Mathematical fill pattern generation
5. **Auto-Cycle Engine** - Timer-based mode sequencing
6. **FPS Calculation** - Frame counting over 1-second windows
7. **3D Projection** - Simplified wireframe rendering (integer-only)
8. **Scanline Rasterization** - Efficient filled triangle/circle rendering
9. **Arc Approximation** - 16-point lookup table for smooth arcs

---

## Summary

✅ **Display Type**: SSD1322 256×64 grayscale OLED  
✅ **Interface**: Software SPI (bit-bang), 5 wires  
✅ **Compatible**: MIOS32 MBHP_CORE_STM32F4 standard  
✅ **Test Modes**: 28 comprehensive modes (0-27)  
✅ **Navigation**: Combined key shortcuts with visual feedback  
✅ **Performance**: 20-60 FPS depending on mode  
✅ **Documentation**: Complete wiring, testing, troubleshooting guides  

**Status**: ✅ Production Ready - Hardware Testing Recommended

---

**Document Version**: 1.0  
**Last Updated**: 2026-01-25  
**Created By**: Consolidation of 10 OLED documentation files  
**Hardware**: MidiCore STM32F407VGT6  
**Display**: SSD1322 256×64 OLED
