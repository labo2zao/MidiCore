# OLED SSD1322 Troubleshooting Guide

## Hardware Configuration

**Display**: SSD1322 256×64 OLED (4-bit grayscale)
**Interface**: 3-wire Software SPI (bit-bang)
**MCU**: STM32F407

### Pin Connections

| STM32 Pin | Function | OLED Module | Notes |
|-----------|----------|-------------|-------|
| PA8 | DC (RS) | Data/Command | LOW=Command, HIGH=Data |
| PC8 | SCL | Clock | Software bit-bang |
| PC11 | SDA | Data/MOSI | Software bit-bang |
| 3.3V | Power | VCC | Must be stable 3.3V |
| GND | Ground | GND | Common ground |

**NOT CONNECTED**:
- No RST wire (OLED module has on-board RC reset circuit)
- No CS wire (hardwired to GND on OLED module)

## Common Issues & Solutions

### 1. Display Stays Blank

**Check Power**:
```
☐ Measure 3.3V on VCC pin of OLED module
☐ Check GND connection is solid
☐ Verify power supply can provide enough current (typically 100-200mA)
```

**Check Wiring**:
```
☐ PA8  → DC/RS pin on OLED
☐ PC8  → Clock pin on OLED
☐ PC11 → Data pin on OLED
☐ All connections are secure (no loose wires)
☐ No short circuits between signal wires
```

**Check Software**:
```
☐ Firmware compiled with correct pin definitions
☐ GPIO pins initialized correctly (see Core/Src/main.c)
☐ oled_init() is called during startup
☐ oled_flush() is being called to update display
```

### 2. GPIO Initialization Problems

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

### 3. SPI Timing Issues

**Software SPI Characteristics**:
- Bit order: MSB first
- Clock idle: LOW
- Sample edge: Rising (clock LOW→HIGH)
- Setup time: ≥100ns (8 NOPs at 168MHz)
- Hold time: ≥100ns (8 NOPs at 168MHz)

**DC Signal Timing**:
- Change DC pin
- Wait 4 NOPs for stabilization
- Start SPI transmission

### 4. Test Pattern Not Showing

The driver sends a test pattern during initialization:
- Top 4 rows: WHITE (0xFF)
- Rows 4-63: GRAY (0x77)

**If test pattern doesn't appear**:
```
1. Power cycle the hardware completely
2. Ensure 300ms delay after power-on (for RC reset)
3. Check that Display ON command (0xAF) is sent
4. Verify framebuffer is being written correctly
5. Confirm flush function is being called
```

### 5. Using a Logic Analyzer

If available, a logic analyzer is invaluable for debugging:

**What to Check**:
```
☐ Clock (PC8) is toggling 0V → 3.3V
☐ Data (PC11) changes between bytes
☐ DC (PA8) is LOW for commands, HIGH for data
☐ All signals are clean (no ringing or noise)
☐ Timing meets SSD1322 specifications
```

**Expected Behavior During Init**:
1. 300ms delay (no activity)
2. Series of command bytes with DC=0
3. Some data bytes with DC=1
4. Display RAM write commands
5. Framebuffer data transfer (large burst)

### 6. Debugging Without Display Response

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

### 7. Common Mistakes

❌ **Wrong Pin Assignments**
- Using hardware SPI pins (PB13/PB15) instead of software SPI (PC8/PC11)
- Swapping SCL and SDA
- Wrong DC pin

❌ **GPIO Speed Too Low**
- Must be VERY_HIGH for reliable communication
- LOW or MEDIUM speed can cause timing violations

❌ **Missing Delays**
- No power-up delay (need 300ms for RC reset)
- No DC setup delay (need 4+ NOPs)
- No SPI bit delays (need 8+ NOPs)

❌ **Incorrect Initialization Sequence**
- Display must be ON (0xAF) before data can be displayed
- Commands must be sent in correct order
- Clock/MUX/addressing must be configured first

### 8. Verification Checklist

Before asking for help, verify:

```
Hardware:
☐ OLED module powers on (may see backlight or slight glow)
☐ All 5 connections present (PA8, PC8, PC11, VCC, GND)
☐ Measured 3.3V on VCC pin
☐ No physical damage to OLED or cables

Software:
☐ Latest firmware from PR branch
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

### 9. Last Resort: Scope/Analyzer Required

If everything above checks out but display still doesn't work:

**You need to see the actual signals** to diagnose further. Without a scope or logic analyzer, you cannot verify:
- Signal integrity (voltage levels, rise/fall times)
- Actual timing (is it really 100ns+?)
- Correct byte values being sent
- Display module receiving correctly

**What to capture**:
1. One complete initialization sequence
2. One framebuffer flush
3. All three signals simultaneously (DC, SCL, SDA)
4. Decode SPI to see actual bytes

## Success Criteria

When working correctly, you should see:
1. **1 second after power-on**: White bar (top 4 rows) + gray fill (rest)
2. **After 1 second**: Normal MidiCore UI appears
3. **During operation**: UI updates smoothly when buttons pressed

## Additional Resources

- **SSD1322 Datasheet**: Search for "SSD1322 datasheet pdf"
- **MIOS32 Reference**: github.com/midibox/mios32 (loopa application)
- **Pin Definitions**: `Config/oled_pins.h`
- **Driver Code**: `Hal/oled_ssd1322/oled_ssd1322.c`
- **GPIO Init**: `Core/Src/main.c` (MX_GPIO_Init function)

## Getting Help

When reporting issues, include:
1. Hardware setup (photo if possible)
2. Voltage measurements (VCC at OLED module)
3. Which troubleshooting steps completed
4. Any error messages during compilation
5. Logic analyzer capture if available

**Do NOT report "not working" without completing this troubleshooting guide first!**
