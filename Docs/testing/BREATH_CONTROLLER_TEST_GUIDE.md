# Breath Controller Test Guide (MODULE_TEST_BREATH)

## Overview

The `MODULE_TEST_BREATH` test provides comprehensive validation of the complete breath controller signal chain in MidiCore, from I2C pressure sensor reading through to MIDI CC output. This test is essential for:

- **Wind Controller Setup**: Validating breath-controlled synthesizers
- **Accordion Bellows**: Testing bidirectional pressure sensing (push/pull)
- **Hardware Validation**: Verifying I2C communication and sensor operation
- **Calibration**: Tuning pressure-to-MIDI mapping curves
- **Troubleshooting**: Diagnosing sensor, I2C, or configuration issues

## Hardware Requirements

### Pressure Sensor Options

1. **XGZP6847D** (Recommended)
   - 24-bit high-resolution I2C ADC
   - Differential pressure sensor
   - Range: ±40 kPa (configurable)
   - I2C address: 0x6D (7-bit)
   - Supply: 3.3V
   - Excellent for breath/bellows sensing

2. **Generic 16-bit I2C Sensors**
   - MCP3425/MCP3426/MCP3427/MCP3428
   - Generic U16/S16 big-endian sensors
   - I2C addresses: varies by sensor

### Electrical Connections

```
STM32F407          Pressure Sensor
-----------        ---------------
SCL (I2C1/2)  -->  SCL
SDA (I2C1/2)  <->  SDA
3.3V          -->  VCC
GND           -->  GND

Pull-up resistors: 4.7kΩ on SCL and SDA (required!)
```

### Debug Output

- **UART2** (PA2/PA3): 115200 baud, 8N1
- Connect USB-UART adapter to view real-time sensor data

## Test Features

The breath controller test demonstrates:

1. **Pressure Sensor I2C Communication**
   - Reads raw sensor values (24-bit or 16-bit)
   - Converts to pressure units (Pascal)
   - Handles I2C errors gracefully

2. **Pressure-to-12bit Conversion**
   - Maps sensor range to 0-4095 (12-bit)
   - Supports multiple mapping modes:
     - Clamp (0-4095): Full range mapping
     - Center (0 Pa = 2048): Bidirectional mapping

3. **Expression Module Integration**
   - Applies curve transformations (Linear, Exponential, S-curve)
   - Implements smoothing (EMA filter)
   - Deadband and hysteresis for stable output
   - Rate limiting (configurable update rate)

4. **MIDI CC Output**
   - Unidirectional: Single CC (typically CC#2 for breath)
   - Bidirectional: Separate CCs for push/pull (accordion mode)
   - Outputs via MIDI Router to USB/DIN

5. **Real-time Monitoring**
   - Live value display via UART (5 Hz update rate)
   - Shows: Raw, Pressure (Pa), 12-bit, CC#, CC value
   - Status indicators (OK, I2C_ERR, DISABLED, EXPR_OFF)

## Running the Test

### Step 1: Enable the Test

In STM32CubeIDE:
1. Right-click project → **Properties**
2. **C/C++ Build → Settings → MCU GCC Compiler → Preprocessor**
3. Add define: `MODULE_TEST_BREATH`
4. Click **Apply and Close**

### Step 2: Build and Flash

```bash
# In STM32CubeIDE
Project → Build Project (Ctrl+B)
Run → Debug (F11) or Run (Ctrl+F11)
```

### Step 3: Connect UART Terminal

Use any serial terminal (PuTTY, Tera Term, Arduino IDE Serial Monitor):
- Port: COM port of your USB-UART adapter
- Baud: 115200
- Data bits: 8
- Parity: None
- Stop bits: 1

### Step 4: Observe Output

The test will display:

```
==============================================
UART Debug Verification: OK
==============================================

=== Breath Controller Module Test ===

This test demonstrates the complete breath controller signal chain:
  Pressure Sensor (I2C) → Expression Mapping → MIDI CC Output → USB/DIN

=== Pressure Sensor Configuration ===
  Enabled:     YES
  I2C Bus:     1
  I2C Address: 0x6D
  Sensor Type: XGZP6847D 24-bit
  Map Mode:    Center at 0 Pa
  Range:       -40000 to 40000 Pa
  Atm Zero:    101325 Pa
  Interval:    20 ms

=== Expression/MIDI CC Configuration ===
  Enabled:     YES
  MIDI Ch:     1
  Mode:        UNIDIRECTIONAL
  CC Number:   2 (Breath Controller)
  Curve:       Exponential (gamma=1.80)
  Output:      0 to 127 (7-bit MIDI)
  Raw Input:   0 to 4095 (12-bit)
  Rate:        20 ms
  Smoothing:   200 (0=none, 255=max)
  Deadband:    2 CC steps
  Hysteresis:  1 CC steps

========================================
Starting continuous monitoring...
Blow/suck on breath sensor to see values change
Press Ctrl+C to stop
========================================

Time(s) | Raw Value | Pressure(Pa) | 12-bit | CC# | CC Val | Status
--------|-----------|--------------|--------|-----|--------|--------
    0.2 |         0 |           +0 |   2048 |   2 |     63 | OK
    0.4 |      3250 |       +3250 |   2412 |   2 |     76 | OK
    0.6 |      8500 |       +8500 |   2915 |   2 |     93 | OK
    0.8 |     12000 |      +12000 |   3250 |   2 |    104 | OK
    1.0 |      8200 |       +8200 |   2898 |   2 |     92 | OK
    1.2 |      2100 |       +2100 |   2256 |   2 |     71 | OK
    1.4 |         0 |           +0 |   2048 |   2 |     63 | OK
```

### Step 5: Test Breath Input

**For Wind Controllers:**
- Blow gently into the sensor tube
- Observe increasing positive pressure values
- CC value should increase proportionally
- Release breath, watch values return to baseline

**For Accordion Bellows (Bidirectional):**
- Push bellows: positive pressure → CC#11 (push)
- Pull bellows: negative pressure → CC#2 (pull)
- Neutral position: near 0 Pa, deadband prevents flipping

## Output Column Descriptions

| Column | Description | Example |
|--------|-------------|---------|
| **Time(s)** | Elapsed seconds since test start | 1.2 |
| **Raw Value** | Direct sensor reading (24-bit or 16-bit) | 8500 |
| **Pressure(Pa)** | Pressure in Pascal (signed, relative to atm) | +8500 |
| **12-bit** | Mapped value 0-4095 | 2915 |
| **CC#** | MIDI CC number being sent | 2 |
| **CC Val** | MIDI CC value 0-127 | 93 |
| **Status** | Test status (OK, I2C_ERR, DISABLED, EXPR_OFF) | OK |

## Configuration Files

The test reads configuration from SD card files (or uses defaults if not found):

### pressure.ngc

Pressure sensor hardware configuration:

```
# Sensor enable
PRESSURE_ENABLE=1

# I2C configuration
PRESSURE_I2C_BUS=1         # 1=I2C1, 2=I2C2
PRESSURE_ADDR7=0x6D        # 7-bit I2C address (0x6D for XGZP6847D)

# Sensor type
PRESSURE_TYPE=2            # 0=Generic U16, 1=Generic S16, 2=XGZP6847D

# Mapping mode
PRESSURE_MAP_MODE=1        # 0=Clamp 0-4095, 1=Center at 0 Pa

# XGZP6847D range (Pa)
PRESSURE_PMIN_PA=-40000    # Minimum pressure
PRESSURE_PMAX_PA=40000     # Maximum pressure
PRESSURE_ATM0_PA=101325    # Atmospheric baseline

# Polling interval
PRESSURE_INTERVAL_MS=20    # 20ms = 50 Hz update rate
```

### expression.ngc

MIDI CC mapping configuration:

```
# Expression enable
EXPRESSION_ENABLE=1

# MIDI output
EXPRESSION_MIDI_CH=0       # 0=Channel 1
EXPRESSION_CC_NUM=2        # CC#2 = Breath Controller

# Bidirectional mode (for accordion)
EXPRESSION_BIDIR=0         # 0=Unidirectional, 1=Bidirectional
EXPRESSION_CC_PUSH=11      # CC for push/inhale (if BIDIR=1)
EXPRESSION_CC_PULL=2       # CC for pull/exhale (if BIDIR=1)
EXPRESSION_ZERO_DEADBAND_PA=500  # ±500 Pa deadband

# Input range (12-bit)
EXPRESSION_RAW_MIN=0
EXPRESSION_RAW_MAX=4095

# Output range (7-bit MIDI)
EXPRESSION_OUT_MIN=0
EXPRESSION_OUT_MAX=127

# Update rate
EXPRESSION_RATE_MS=20      # 20ms = 50 Hz

# Smoothing (0-255)
EXPRESSION_SMOOTHING=200   # 200 = heavy smoothing, reduces jitter

# Deadband/hysteresis
EXPRESSION_DEADBAND_CC=2   # Minimum change threshold (CC steps)
EXPRESSION_HYST_CC=1       # Hysteresis when reversing direction

# Curve type
EXPRESSION_CURVE=1         # 0=Linear, 1=Exponential, 2=S-curve
EXPRESSION_CURVE_PARAM=180 # Gamma*100 for expo (180 = 1.80)
```

## Troubleshooting

### Problem: "I2C_ERR" Status

**Causes:**
- Incorrect I2C address
- Missing pull-up resistors on SCL/SDA
- Sensor not powered
- Wrong I2C bus selected
- SCL/SDA lines shorted

**Solutions:**
1. Verify I2C address matches sensor datasheet
   - XGZP6847D: 0x6D (7-bit)
   - MCP342x: 0x68-0x6F (configurable)
2. Add 4.7kΩ pull-ups on SCL and SDA
3. Check 3.3V power at sensor VCC pin
4. Verify I2C bus in `pressure.ngc` (1 or 2)
5. Use I2C scanner to detect sensor address

### Problem: "DISABLED" Status

**Cause:** Pressure module not enabled in configuration

**Solutions:**
1. Check `pressure.ngc`: Set `PRESSURE_ENABLE=1`
2. Or in `Config/module_config.h`: Ensure `MODULE_ENABLE_PRESSURE 1`
3. Rebuild and reflash

### Problem: "EXPR_OFF" Status

**Cause:** Expression module disabled

**Solutions:**
1. Check `expression.ngc`: Set `EXPRESSION_ENABLE=1`
2. Or in `Config/module_config.h`: Ensure `MODULE_ENABLE_EXPRESSION 1`
3. Rebuild and reflash

### Problem: Values Don't Change

**Causes:**
- Sensor not connected to breath tube
- Sensor damaged or faulty
- No breath input detected
- Sensor range misconfigured

**Solutions:**
1. Verify physical connection to sensor port
2. Try blowing harder into sensor
3. Check `PRESSURE_PMIN_PA` and `PRESSURE_PMAX_PA` match sensor specs
4. Test with known working sensor

### Problem: Erratic/Noisy Values

**Causes:**
- Electrical noise on I2C lines
- Insufficient smoothing
- Mechanical vibration

**Solutions:**
1. Increase `EXPRESSION_SMOOTHING` (e.g., 240)
2. Add shielded cable for I2C
3. Mount sensor away from vibration sources
4. Increase `EXPRESSION_DEADBAND_CC` (e.g., 3-4)

### Problem: Wrong CC Number Sent

**Cause:** CC mapping misconfigured

**Solutions:**
1. Check `expression.ngc`: Set desired `EXPRESSION_CC_NUM`
   - CC#2 = Breath Controller (standard for wind instruments)
   - CC#11 = Expression (general purpose)
2. For bidirectional: Set `EXPRESSION_CC_PUSH` and `EXPRESSION_CC_PULL`

## Performance Characteristics

| Metric | Value | Notes |
|--------|-------|-------|
| **Latency** | <5ms | Sensor read + mapping + MIDI output |
| **Resolution** | 12-bit → 7-bit | 4096 levels → 128 MIDI values |
| **Update Rate** | 5-50 Hz | Configurable via `PRESSURE_INTERVAL_MS` |
| **Smoothing** | EMA filter | Reduces jitter, `SMOOTHING` parameter |
| **Deadband** | 2 CC steps | Prevents micro-fluctuations |
| **Hysteresis** | 1 CC step | Prevents oscillation on direction change |

## Sensor Specifications

### XGZP6847D

- **Type**: Differential pressure, I2C
- **Resolution**: 24-bit ADC
- **Range**: ±40 kPa (configurable)
- **I2C Address**: 0x6D (7-bit)
- **Supply Voltage**: 2.7-5.5V (use 3.3V)
- **Supply Current**: <2mA
- **Response Time**: <10ms
- **Accuracy**: ±0.5% FS
- **Temperature Range**: -40°C to +85°C

### MCP342x Series

- **Type**: General-purpose ADC, I2C
- **Resolution**: 12/14/16/18-bit (configurable)
- **Channels**: 1-4 (model dependent)
- **I2C Address**: 0x68-0x6F (configurable)
- **Supply Voltage**: 2.7-5.5V
- **PGA Gain**: 1x, 2x, 4x, 8x
- **Sample Rate**: 3.75-240 SPS

## Use Cases

### 1. Wind Controller (Saxophone, Flute, etc.)

**Configuration:**
- Unidirectional mode (`EXPRESSION_BIDIR=0`)
- CC#2 (Breath Controller)
- Exponential curve (gamma=1.80) for natural breath feel
- High smoothing (200-240) for stable output

**Test Procedure:**
1. Blow gently: CC value should increase smoothly
2. Blow hard: CC should reach maximum (127)
3. Release: CC should decay to minimum (0)
4. Rapid changes: Verify no jitter or overshoot

### 2. Accordion Bellows

**Configuration:**
- Bidirectional mode (`EXPRESSION_BIDIR=1`)
- CC#11 (push), CC#2 (pull)
- Center mapping (`PRESSURE_MAP_MODE=1`)
- Deadband ±500 Pa to prevent flip at neutral position

**Test Procedure:**
1. Push bellows: CC#11 should increase (positive pressure)
2. Release: CC#11 should return near 0
3. Pull bellows: CC#2 should increase (negative pressure)
4. Neutral position: Both CCs near 63 (center)

### 3. Expression Pedal Alternative

**Configuration:**
- Unidirectional mode
- CC#11 (Expression)
- Linear curve for predictable response
- Lower smoothing (50-100) for responsive feel

**Test Procedure:**
1. Vary pressure gradually
2. Verify linear relationship between pressure and CC value
3. Check for dead zones at extremes

## Advanced Topics

### Curve Selection

**Linear (`EXPRESSION_CURVE=0`):**
- Direct proportional mapping
- Best for: Expression pedal simulation, simple control
- Feel: Constant sensitivity across range

**Exponential (`EXPRESSION_CURVE=1`):**
- More sensitive at low pressures
- Best for: Natural breath feel, wind controllers
- Parameter: Gamma (100-500, default=180 = 1.80)
- Feel: Easy to play soft, hard blow for loud

**S-Curve (`EXPRESSION_CURVE=2`):**
- Smooth start and end, steep middle
- Best for: Smooth transitions, cinematic control
- Feel: Gentle at extremes, responsive in middle

### Calibration Procedure

1. **Find Atmospheric Zero:**
   - Leave sensor at rest (no breath)
   - Note raw value displayed
   - Set `PRESSURE_ATM0_PA` to this value

2. **Find Minimum/Maximum:**
   - Blow as hard as comfortable: Note max raw value
   - Suck as hard as comfortable: Note min raw value
   - Set `PRESSURE_PMIN_PA` and `PRESSURE_PMAX_PA` accordingly

3. **Tune Smoothing:**
   - Start with `EXPRESSION_SMOOTHING=200`
   - If too sluggish: Decrease to 150
   - If too jittery: Increase to 240

4. **Adjust Deadband:**
   - Set `EXPRESSION_DEADBAND_CC=2`
   - If MIDI CC flickers: Increase to 3-4
   - If too unresponsive: Decrease to 1

### Integration with MIDI Router

The breath controller test uses the MIDI Router to send CC messages. Ensure:

1. **Router Enabled:** `MODULE_ENABLE_ROUTER=1` in `module_config.h`
2. **Route Configured:** Expression node routed to USB/DIN outputs
3. **Channel Mask:** Set to allow all channels (0xFFFF)

Example routing:
```c
router_set_route(ROUTER_NODE_KEYS, ROUTER_NODE_USB_OUT, 1);
router_set_chanmask(ROUTER_NODE_KEYS, ROUTER_NODE_USB_OUT, 0xFFFF);
```

## Testing Checklist

- [ ] UART terminal connected at 115200 baud
- [ ] Pressure sensor wired correctly (SCL, SDA, VCC, GND)
- [ ] Pull-up resistors on I2C (4.7kΩ)
- [ ] `MODULE_TEST_BREATH` defined in build settings
- [ ] Project built and flashed to STM32
- [ ] Test header displayed in terminal
- [ ] Sensor configuration shows "Enabled: YES"
- [ ] Expression configuration shows "Enabled: YES"
- [ ] Status shows "OK" (not I2C_ERR)
- [ ] Raw value changes when blowing/sucking
- [ ] Pressure (Pa) value changes correspondingly
- [ ] 12-bit value changes (0-4095)
- [ ] CC value changes smoothly (0-127)
- [ ] No excessive jitter or noise
- [ ] Values return to baseline when breath stops

## Related Documentation

- **[Module Configuration](../configuration/README_MODULE_CONFIG.md)**: Module enable/disable
- **[Testing Quickstart](TESTING_QUICKSTART.md)**: Quick test examples
- **[Module Testing](README_MODULE_TESTING.md)**: Comprehensive testing guide
- **[Breath Controller Hardware](../hardware/)**: Wiring diagrams and sensor specs

## Summary

The `MODULE_TEST_BREATH` test provides a comprehensive validation tool for breath controller hardware and configuration. Use it to:

1. **Verify Hardware**: Confirm sensor is working and wired correctly
2. **Debug I2C**: Identify communication issues early
3. **Tune Configuration**: Adjust curves, smoothing, and mapping
4. **Validate Performance**: Ensure <5ms latency and smooth output
5. **Troubleshoot**: Diagnose sensor, configuration, or routing issues

This test is essential before integrating breath control into production patches or performances.
