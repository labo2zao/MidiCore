# OLED SSD1322 Display - Testing Protocol

**Version**: 2.0  
**Date**: 2026-01-23  
**Test Modes**: 15 comprehensive validation modes  
**Display**: SSD1322 256×64 grayscale OLED

---

## Test Environment Setup

### ⚠️ IMPORTANT: OLED Initialization Functions

**MidiCore has two OLED init functions**:

1. **`oled_init_newhaven()`** - **PRODUCTION USE ONLY**
   - Complete Newhaven NHD-3.12 initialization sequence from LoopA
   - Used in `app_init.c` for production firmware
   - Tested and validated for accordion hardware
   - **This is the default for production builds**

2. **`oled_init()`** - **TEST/DEBUG USE ONLY**
   - Simple MIOS32-style test initialization
   - Used in MODULE_TEST_OLED test mode only
   - Suitable for quick hardware verification
   - **NOT used in production mode**

**For this test protocol**: The OLED test page uses `oled_init()` for quick verification. If testing production firmware, verify `oled_init_newhaven()` is called in `app_init.c` instead.

### Hardware Requirements
- STM32F407VGT6 microcontroller with MidiCore firmware
- SSD1322 OLED display (256×64 pixels, 4-bit grayscale)
- 3-wire Software SPI connection (PA8/DC, PC8/SCL, PC11/SDA)
- Rotary encoders with push buttons (navigation)
- Stable 3.3V power supply (100-200mA)

### Software Requirements
- MidiCore firmware with MODULE_ENABLE_OLED=1
- OLED test page accessible via UI navigation
- Optional: Logic analyzer or oscilloscope for signal verification

### Pin Configuration Verification
```
PA8  → DC (Data/Command)
PC8  → SCL (Clock) - Software SPI
PC11 → SDA (Data/MOSI) - Software SPI
3.3V → VCC
GND  → GND
```

---

## Phase 1: Basic Display Tests (Modes 0-6)

### 1.1 Mode 0: Pattern Test
**Purpose**: Verify basic pixel rendering and display uniformity

**Test Cases**:
- [ ] T1.1.1: Horizontal stripes render solid lines at Y=40-48
- [ ] T1.1.2: Vertical stripes appear every 4 pixels from X=0
- [ ] T1.1.3: Checkerboard pattern displays alternating pixels at Y=60+
- [ ] T1.1.4: All patterns visible with full brightness (15)
- [ ] T1.1.5: No pixel defects or missing lines
- [ ] T1.1.6: Pattern remains stable (no flickering)

**Expected Result**: Clean geometric patterns demonstrating pixel-level accuracy

### 1.2 Mode 1: Grayscale Levels
**Purpose**: Test 4-bit grayscale capability (16 levels)

**Test Cases**:
- [ ] T1.2.1: 16 vertical bars displayed from X=0 to X=256
- [ ] T1.2.2: Each bar shows distinct gray level (0x0 to 0xF)
- [ ] T1.2.3: Hex labels (0-F) visible on each bar
- [ ] T1.2.4: Smooth gradient from black (0) to white (15)
- [ ] T1.2.5: No banding or missing levels
- [ ] T1.2.6: Brightness matches expected values

**Expected Result**: All 16 grayscale levels clearly distinguishable

### 1.3 Mode 2: Pixel Test
**Purpose**: Individual pixel control and gradient rendering

**Test Cases**:
- [ ] T1.3.1: Pixel grid displays from Y=40 to Y=64
- [ ] T1.3.2: Every 2nd pixel rendered in both X and Y
- [ ] T1.3.3: Brightness varies based on position (diagonal gradient)
- [ ] T1.3.4: No stuck or dead pixels visible
- [ ] T1.3.5: Pixel spacing uniform across display
- [ ] T1.3.6: Pattern repeats correctly (modulo 16)

**Expected Result**: Fine-grained pixel control with position-based brightness

### 1.4 Mode 3: Text Rendering Test
**Purpose**: Verify font rendering and text readability

**Test Cases**:
- [ ] T1.4.1: Numbers "0123456789" display clearly at Y=38
- [ ] T1.4.2: Uppercase alphabet A-Z readable at Y=50
- [ ] T1.4.3: Lowercase alphabet a-z readable at Y=62
- [ ] T1.4.4: Character spacing consistent (6 pixels)
- [ ] T1.4.5: Multiple brightness levels (8, 10, 12, 15) distinguishable
- [ ] T1.4.6: 5×7 font renders without artifacts

**Expected Result**: Clear, readable text at various brightness levels

### 1.5 Mode 4: Animation Test
**Purpose**: Basic animation and frame timing

**Test Cases**:
- [ ] T1.5.1: Horizontal bar moves smoothly across screen
- [ ] T1.5.2: Pulsing square animates size changes (10-20 pixels)
- [ ] T1.5.3: Frame counter increments continuously
- [ ] T1.5.4: Update interval approximately 100ms
- [ ] T1.5.5: No tearing or visual artifacts
- [ ] T1.5.6: Animation smooth and consistent

**Expected Result**: Smooth 10 FPS animation without stuttering

### 1.6 Mode 5: Hardware Info
**Purpose**: Display configuration verification

**Test Cases**:
- [ ] T1.6.1: Display model shows "SSD1322"
- [ ] T1.6.2: Resolution displays "256×64"
- [ ] T1.6.3: Pin configuration shows "PA8/PC8/PC11"
- [ ] T1.6.4: Text readable at brightness 12
- [ ] T1.6.5: Static display (no animation)
- [ ] T1.6.6: Information accurate to hardware

**Expected Result**: Static information page with correct hardware details

### 1.7 Mode 6: Framebuffer Direct Test
**Purpose**: Low-level framebuffer access verification

**Test Cases**:
- [ ] T1.7.1: Direct framebuffer writes succeed
- [ ] T1.7.2: Alternating pattern displays at Y=40-64
- [ ] T1.7.3: Pattern formula (col + row) & 0xFF applied correctly
- [ ] T1.7.4: Bypasses graphics library (raw memory access)
- [ ] T1.7.5: Framebuffer size 8192 bytes (256×64/2)
- [ ] T1.7.6: Memory writes stable (no corruption)

**Expected Result**: Raw framebuffer access produces expected pattern

---

## Phase 2: Advanced Animation Tests (Modes 7-10)

### 2.1 Mode 7: Scrolling Text
**Purpose**: Smooth horizontal scrolling and text animation

**Test Cases**:
- [ ] T2.1.1: Text scrolls smoothly from right to left
- [ ] T2.1.2: Scroll speed exactly 2 pixels per 50ms
- [ ] T2.1.3: Text wraps around after 300 pixels + screen width
- [ ] T2.1.4: Speed indicator displays "2px/50ms"
- [ ] T2.1.5: No stuttering or frame drops
- [ ] T2.1.6: Long text string fully visible during scroll

**Expected Result**: Smooth marquee effect at 40-50 FPS

### 2.2 Mode 8: Bouncing Ball
**Purpose**: Physics simulation and collision detection

**Test Cases**:
- [ ] T2.2.1: Ball (6×6 pixels) displays with radial gradient
- [ ] T2.2.2: Ball bounces off all four walls correctly
- [ ] T2.2.3: Velocity vectors (dx=±2, dy=±1) applied
- [ ] T2.2.4: Collision detection accurate at boundaries
- [ ] T2.2.5: Position display shows X and Y coordinates
- [ ] T2.2.6: Gradient center bright (15) to edge dim (calculated)
- [ ] T2.2.7: Update interval 30ms (approximately 33 FPS)
- [ ] T2.2.8: Ball motion continuous and smooth

**Expected Result**: Realistic physics with proper collision handling

### 2.3 Mode 9: Performance Test
**Purpose**: FPS measurement and multi-element rendering

**Test Cases**:
- [ ] T2.3.1: FPS counter displays in header
- [ ] T2.3.2: FPS value updates every 1 second
- [ ] T2.3.3: FPS range 20-30 under load
- [ ] T2.3.4: 5 moving bars animate simultaneously
- [ ] T2.3.5: Each bar moves at different speed
- [ ] T2.3.6: Frame counter increments continuously
- [ ] T2.3.7: Update interval 20ms (50 FPS target)
- [ ] T2.3.8: No memory leaks (FPS stable over time)

**Expected Result**: Stable performance metrics with multi-element animation

### 2.4 Mode 10: Circles & Lines
**Purpose**: Geometry primitive validation

**Test Cases**:
- [ ] T2.4.1: Three circles expand/contract smoothly
- [ ] T2.4.2: Circles rendered using midpoint algorithm
- [ ] T2.4.3: Circles appear round (not oval or distorted)
- [ ] T2.4.4: Diagonal lines scroll smoothly
- [ ] T2.4.5: Lines rendered using Bresenham's algorithm
- [ ] T2.4.6: Lines straight (no stairstepping artifacts)
- [ ] T2.4.7: Rotating line from center cycles through 8 directions
- [ ] T2.4.8: All elements animate at 100ms intervals
- [ ] T2.4.9: Overlapping elements display correctly

**Expected Result**: Clean geometric primitives with accurate rendering

---

## Phase 3: Advanced Graphics Tests (Modes 11-13)

### 3.1 Mode 11: Bitmap Test
**Purpose**: Bitmap/image rendering capability

**Test Cases**:
- [ ] T3.1.1: Smiley face displays centered on screen
- [ ] T3.1.2: Face outline circle (radius 15) renders correctly
- [ ] T3.1.3: Left eye circle (radius 2) positioned at correct offset
- [ ] T3.1.4: Right eye circle (radius 2) positioned at correct offset
- [ ] T3.1.5: Smile parabolic curve rendered smoothly
- [ ] T3.1.6: All elements use appropriate brightness levels
- [ ] T3.1.7: Combining primitives produces recognizable image
- [ ] T3.1.8: Static display (no animation)

**Expected Result**: Clear smiley face demonstrating bitmap-style graphics

### 3.2 Mode 12: Fill Patterns
**Purpose**: Procedural pattern generation

**Test Cases**:
- [ ] T3.2.1: Pattern cycles every 500ms automatically
- [ ] T3.2.2: Pattern 1 (Dots): Varied brightness based on position
- [ ] T3.2.3: Pattern 2 (Dither): Checkerboard black/white
- [ ] T3.2.4: Pattern 3 (Waves): Diagonal gradient waves
- [ ] T3.2.5: Pattern 4 (Grid): 8×8 pixel grid lines
- [ ] T3.2.6: Pattern counter displays "Pattern X/4"
- [ ] T3.2.7: All 4 patterns display correctly
- [ ] T3.2.8: Transitions between patterns clean (no artifacts)

**Expected Result**: Four distinct procedural patterns cycling automatically

### 3.3 Mode 13: Stress Test
**Purpose**: Maximum graphics throughput benchmark

**Test Cases**:
- [ ] T3.3.1: 10 rectangles animate simultaneously
- [ ] T3.3.2: 3 circles expand/contract with different radii
- [ ] T3.3.3: 5 diagonal lines scroll across screen
- [ ] T3.3.4: Total 18 animated elements displayed
- [ ] T3.3.5: Update interval 16ms (60 FPS target)
- [ ] T3.3.6: FPS maintains 40-60 range
- [ ] T3.3.7: No visual corruption or artifacts
- [ ] T3.3.8: Element count "18" displayed
- [ ] T3.3.9: Performance stable over extended period (5+ minutes)
- [ ] T3.3.10: CPU usage reasonable (<30%)

**Expected Result**: Sustained high-performance rendering with 18 elements

---

## Phase 4: Auto-Cycle Demo (Mode 14)

### 4.1 Mode 14: Auto-Cycle Demo
**Purpose**: Automated demonstration mode

**Test Cases**:
- [ ] T4.1.1: Automatically cycles through modes 0-13
- [ ] T4.1.2: Dwell time exactly 3000ms per mode
- [ ] T4.1.3: Progress bar displays at Y=60
- [ ] T4.1.4: Progress bar width proportional to time elapsed
- [ ] T4.1.5: Countdown timer shows milliseconds remaining
- [ ] T4.1.6: "Press any button to exit" message displays
- [ ] T4.1.7: Any button press exits to mode 0
- [ ] T4.1.8: Encoder rotation exits to mode 0
- [ ] T4.1.9: Mode transitions clean (animation state resets)
- [ ] T4.1.10: Complete cycle through all 14 modes
- [ ] T4.1.11: Cycles continuously until interrupted

**Expected Result**: Unattended demo mode for exhibitions/trade shows

---

## Phase 5: Navigation and Control Tests

### 5.1 Encoder Navigation
**Test Cases**:
- [ ] T5.1.1: Rotate right increments mode (0→1→2→...→14→0)
- [ ] T5.1.2: Rotate left decrements mode (0→14→13→...→1→0)
- [ ] T5.1.3: Mode wraps around at boundaries
- [ ] T5.1.4: Animation state resets on mode change
- [ ] T5.1.5: Encoder responsive (no lag)
- [ ] T5.1.6: Mode changes immediate (no delay)

### 5.2 Button Controls
**Test Cases**:
- [ ] T5.2.1: Button 0 triggers previous mode
- [ ] T5.2.2: Button 1 triggers next mode
- [ ] T5.2.3: Button 2 clears screen (fills black)
- [ ] T5.2.4: Button 3 fills screen white (brightness 15)
- [ ] T5.2.5: Button 4 clears screen (same as button 2)
- [ ] T5.2.6: Button 5 resets FPS counter to 0
- [ ] T5.2.7: All buttons debounced (no double-triggers)

### 5.3 Display Header
**Test Cases**:
- [ ] T5.3.1: Current mode number displays (0-14)
- [ ] T5.3.2: "Use ENC" instruction visible
- [ ] T5.3.3: Millisecond counter increments (MS:xxxxx)
- [ ] T5.3.4: FPS counter displays current rate
- [ ] T5.3.5: Header always visible (not cleared)
- [ ] T5.3.6: Text readable at brightness 10

---

## Phase 6: Performance and Reliability Tests

### 6.1 Frame Rate Verification
**Test Cases**:
- [ ] T6.1.1: Static modes (0-3, 5-6, 11) achieve 60 FPS
- [ ] T6.1.2: Animation modes (4, 7-10, 12) achieve 30-50 FPS
- [ ] T6.1.3: Performance mode (9) achieves 20-30 FPS
- [ ] T6.1.4: Stress test (13) achieves 40-60 FPS
- [ ] T6.1.5: FPS stable over 10 minute test
- [ ] T6.1.6: No frame rate degradation

### 6.2 Memory Stability
**Test Cases**:
- [ ] T6.2.1: Framebuffer allocated in CCMRAM (8192 bytes)
- [ ] T6.2.2: Static variables <60 bytes overhead
- [ ] T6.2.3: Stack usage <100 bytes per function
- [ ] T6.2.4: No memory leaks during mode cycling
- [ ] T6.2.5: Extended operation (1+ hour) stable
- [ ] T6.2.6: No stack overflow or corruption

### 6.3 Timing Accuracy
**Test Cases**:
- [ ] T6.3.1: Mode 7 scroll updates every 50ms ±5ms
- [ ] T6.3.2: Mode 8 ball updates every 30ms ±5ms
- [ ] T6.3.3: Mode 9 updates every 20ms ±5ms
- [ ] T6.3.4: Mode 13 updates every 16ms ±5ms
- [ ] T6.3.5: Mode 14 cycle exactly 3000ms ±50ms
- [ ] T6.3.6: FPS counter accuracy within ±2 FPS

### 6.4 Display Quality
**Test Cases**:
- [ ] T6.4.1: No flickering in any mode
- [ ] T6.4.2: No tearing during updates
- [ ] T6.4.3: No ghosting or persistence
- [ ] T6.4.4: Contrast appropriate (not washed out)
- [ ] T6.4.5: Brightness levels distinct
- [ ] T6.4.6: No visual artifacts or corruption

---

## Phase 7: Graphics Primitives Tests

### 7.1 ui_gfx_circle() Function
**Test Cases**:
- [ ] T7.1.1: Circles render as perfect circles (not ovals)
- [ ] T7.1.2: All radii 1-30 render correctly
- [ ] T7.1.3: Center coordinates accurate
- [ ] T7.1.4: Brightness levels 0-15 applied correctly
- [ ] T7.1.5: Integer-only math (no floating point)
- [ ] T7.1.6: 8 octants drawn symmetrically
- [ ] T7.1.7: No gaps in circle outline
- [ ] T7.1.8: Performance <1ms for radius 15

### 7.2 ui_gfx_line() Function
**Test Cases**:
- [ ] T7.2.1: Lines straight (no stairstepping)
- [ ] T7.2.2: All angles 0-360° render correctly
- [ ] T7.2.3: Horizontal lines perfect
- [ ] T7.2.4: Vertical lines perfect
- [ ] T7.2.5: Diagonal lines accurate
- [ ] T7.2.6: Brightness levels 0-15 applied correctly
- [ ] T7.2.7: Handles negative coordinates
- [ ] T7.2.8: Performance <2ms for 256-pixel line

### 7.3 Basic Drawing Functions
**Test Cases**:
- [ ] T7.3.1: ui_gfx_pixel() sets individual pixels
- [ ] T7.3.2: ui_gfx_rect() fills rectangles
- [ ] T7.3.3: ui_gfx_hline() draws horizontal lines
- [ ] T7.3.4: ui_gfx_vline() draws vertical lines
- [ ] T7.3.5: ui_gfx_text() renders 5×7 font
- [ ] T7.3.6: All functions clip at boundaries
- [ ] T7.3.7: All functions handle invalid inputs gracefully

---

## Phase 8: Hardware Interface Tests

### 8.1 SPI Communication
**Test Cases**:
- [ ] T8.1.1: Software SPI bit-bang functional
- [ ] T8.1.2: Clock (PC8) toggles 0V ↔ 3.3V
- [ ] T8.1.3: Data (PC11) changes between bytes
- [ ] T8.1.4: DC (PA8) LOW for commands, HIGH for data
- [ ] T8.1.5: MSB transmitted first
- [ ] T8.1.6: Clock idle LOW (Mode 0)
- [ ] T8.1.7: Data sampled on rising edge
- [ ] T8.1.8: Timing meets SSD1322 specifications (≥100ns)

### 8.2 Display Initialization
**Test Cases**:
- [ ] T8.2.1: Power-up delay 300ms observed
- [ ] T8.2.2: Unlock command (0xFD 0x12) sent
- [ ] T8.2.3: Display OFF during configuration
- [ ] T8.2.4: Column address (0x15 0x1C 0x5B) set
- [ ] T8.2.5: Row address (0x75 0x00 0x3F) set
- [ ] T8.2.6: MUX ratio (0xCA 0x3F) configured
- [ ] T8.2.7: Remap (0xA0 0x14 0x11) for dual COM mode
- [ ] T8.2.8: Linear gray scale (0xB9) enabled
- [ ] T8.2.9: Display ON (0xAF) command sent
- [ ] T8.2.10: Initialization completes <200ms

### 8.3 Framebuffer Flush
**Test Cases**:
- [ ] T8.3.1: All 64 rows transferred correctly
- [ ] T8.3.2: 128 bytes per row transmitted
- [ ] T8.3.3: Column address set per row (0x15 0x1C)
- [ ] T8.3.4: Row address set per row (0x75 row)
- [ ] T8.3.5: Write RAM command (0x5C) precedes data
- [ ] T8.3.6: Data bytes sent immediately after 0x5C
- [ ] T8.3.7: Flush completes <5ms @ 21MHz SPI
- [ ] T8.3.8: No data corruption during transfer

---

## Test Execution Guidelines

### Test Sequence
1. **Power Cycle**: Reset hardware before starting tests
2. **Visual Inspection**: Manually verify each mode
3. **Automated Cycle**: Use Mode 14 for quick overview
4. **Detailed Testing**: Execute all test cases systematically
5. **Performance Testing**: Run extended tests (10 min each)
6. **Signal Verification**: Use logic analyzer for critical signals

### Pass Criteria
- ✅ **Pass**: Test case meets all requirements
- ⚠️ **Warning**: Minor issues that don't affect functionality
- ❌ **Fail**: Test case does not meet requirements
- 🚫 **Blocked**: Cannot test due to hardware/software limitations

### Test Documentation
For each test:
- Record pass/fail status
- Note any visual artifacts or anomalies
- Capture FPS values for performance tests
- Document any workarounds applied
- Include logic analyzer captures for signal tests

### Known Limitations
- Software SPI limited to ~21 MHz
- FPS depends on system load and FreeRTOS scheduling
- Some modes may show slight variations in timing
- Performance affected by other active modules

---

## Troubleshooting Guide

### Issue: Low FPS (< 20)
**Checks**:
- [ ] Verify no other tasks consuming CPU
- [ ] Check SPI speed configuration
- [ ] Confirm framebuffer in CCMRAM
- [ ] Review FreeRTOS task priorities

### Issue: Display Artifacts
**Checks**:
- [ ] Verify power supply stable (3.3V)
- [ ] Check wire connections not loose
- [ ] Confirm proper pin assignments
- [ ] Test with reduced SPI speed

### Issue: No Display Output
**Checks**:
- [ ] Measure 3.3V at OLED VCC pin
- [ ] Verify GPIO initialization correct
- [ ] Check oled_init() called
- [ ] Confirm oled_flush() being called
- [ ] Test with Mode 3 (Fill screen white)

### Issue: Animation Stuttering
**Checks**:
- [ ] Verify HAL_GetTick() accurate
- [ ] Check no interrupt blocking
- [ ] Review task scheduling
- [ ] Confirm no memory allocation in render loop

---

## Integration with LoopA Testing Protocol

This OLED testing protocol complements the main **LoopA Feature Implementation - Testing Protocol** (TESTING_PROTOCOL.md) and should be executed as part of:

- **Phase 1: UI Pages Testing** - Verify OLED display working before UI page tests
- **Hardware Validation** - Confirm display operational before feature testing
- **Production Testing** - Include in manufacturing test sequence
- **Regression Testing** - Run after firmware updates

### Cross-References
- See [TESTING_PROTOCOL.md](TESTING_PROTOCOL.md) for UI page testing
- See [OLED_TEST_PAGE_GUIDE.md](../hardware/OLED_TEST_PAGE_GUIDE.md) for user guide
- See [OLED_TROUBLESHOOTING.md](../hardware/OLED_TROUBLESHOOTING.md) for hardware issues
- See [OLED_WIRING_GUIDE.md](../hardware/OLED_WIRING_GUIDE.md) for wiring details

---

**Test Protocol Version**: 2.0  
**Last Updated**: 2026-01-23  
**Compatibility**: MidiCore firmware with 15-mode OLED test page  
**Hardware**: STM32F407VGT6 + SSD1322 256×64 OLED  
**Status**: ✅ Complete - Ready for systematic testing
