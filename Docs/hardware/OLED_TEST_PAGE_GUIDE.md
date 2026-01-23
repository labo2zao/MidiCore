# OLED Test Page Guide

Comprehensive guide to the enhanced OLED SSD1322 test page features in MidiCore.

## Overview

The OLED test page provides 15 different test modes to validate display functionality, performance, and visual quality. This is useful for:
- Verifying OLED driver implementation
- Testing display hardware
- Debugging rendering issues
- Benchmarking display performance
- Demonstrating graphics capabilities

## Test Modes

### Mode 0: Pattern Test
Tests basic rendering with geometric patterns.

**Features**:
- Horizontal stripes (solid lines)
- Vertical stripes (repeated pattern)
- Checkerboard pattern (pixel-level accuracy)

**Use Cases**:
- Verify basic pixel rendering
- Check horizontal/vertical line quality
- Test alternating pixel patterns
- Identify pixel-level defects

### Mode 1: Grayscale Levels
Tests all 16 grayscale levels of the SSD1322 display.

**Features**:
- 16 vertical bars showing levels 0-F (hex)
- Each level labeled with brightness value
- Smooth gradient demonstration

**Use Cases**:
- Verify 4-bit grayscale capability
- Check contrast and brightness
- Identify missing or incorrect gray levels
- Test display calibration

### Mode 2: Pixel Test
Tests individual pixel control and color gradients.

**Features**:
- Grid of pixels with varying brightness
- Diagonal gradient pattern
- Sub-pixel resolution testing

**Use Cases**:
- Verify pixel-level control
- Test gradient rendering
- Check pixel uniformity
- Identify stuck or dead pixels

### Mode 3: Text Rendering Test
Tests text rendering capabilities.

**Features**:
- Numbers: 0-9
- Uppercase: A-Z
- Lowercase: a-z
- Multiple brightness levels

**Use Cases**:
- Verify font rendering
- Test character spacing
- Check text readability
- Validate 5x7 font implementation

### Mode 4: Animation Test
Tests basic animation capabilities.

**Features**:
- Moving horizontal bar
- Pulsing square (size animation)
- Frame counter display

**Use Cases**:
- Verify smooth animation
- Test frame timing (100ms updates)
- Check display refresh rate
- Identify rendering lag

### Mode 5: Hardware Info
Displays OLED hardware information.

**Features**:
- Display model (SSD1322)
- Resolution (256×64)
- Pin configuration (PA8/PC8/PC11)

**Use Cases**:
- Quick hardware reference
- Verify display configuration
- Document pin assignments
- Hardware compatibility check

### Mode 6: Framebuffer Direct Test
Tests direct framebuffer access.

**Features**:
- Raw framebuffer write operations
- Alternating pattern generation
- Bypasses graphics library

**Use Cases**:
- Verify framebuffer memory access
- Test low-level rendering
- Benchmark direct write performance
- Debug framebuffer layout issues

### Mode 7: Scrolling Text ✨ NEW
Tests smooth horizontal text scrolling.

**Features**:
- Smooth scrolling text banner
- Configurable scroll speed (2px/50ms)
- Automatic wrap-around
- Speed indicator display

**Use Cases**:
- Verify smooth animation
- Test text rendering performance
- Check display update timing
- Demonstrate marquee effects

**Technical Details**:
- Update interval: 50ms
- Scroll speed: 2 pixels per update
- Total scroll distance: 300px + screen width
- Wraps from right to left automatically

### Mode 8: Bouncing Ball ✨ NEW
Tests physics-based animation with collision detection.

**Features**:
- 6×6 pixel ball with radial gradient
- Physics-based movement (velocity vectors)
- Wall collision detection and bounce
- Real-time position display

**Use Cases**:
- Verify complex animations
- Test rapid screen updates (30ms refresh)
- Check pixel-level precision
- Demonstrate gradient rendering

**Technical Details**:
- Ball size: 6×6 pixels
- Update interval: 30ms
- Initial velocity: dx=2, dy=1 pixels/frame
- Gradient: Center bright (15) to edge dim (calculated by distance)

### Mode 9: Performance Test ✨ NEW
Tests display performance with multiple simultaneous animations.

**Features**:
- 5 moving bars at different speeds
- Real-time FPS (Frames Per Second) counter
- Frame number display
- Stress test mode

**Use Cases**:
- Benchmark display performance
- Measure actual frame rate
- Identify performance bottlenecks
- Optimize rendering code

**Technical Details**:
- Update interval: 20ms
- Multiple animated elements
- FPS calculation: frames counted per 1 second window
- Expected FPS: 30-60 depending on system load

### Mode 10: Circles & Lines ✨ NEW
Tests advanced geometry rendering with circles and lines.

**Features**:
- Animated expanding circles (midpoint circle algorithm)
- Diagonal scrolling lines (Bresenham's line algorithm)
- Rotating line from center
- Multiple overlapping animations

**Use Cases**:
- Verify circle rendering accuracy
- Test line drawing quality
- Check anti-aliasing (if supported)
- Demonstrate geometric capabilities

**Technical Details**:
- Circle algorithm: Midpoint circle (optimized for integer math)
- Line algorithm: Bresenham's line (sub-pixel accuracy)
- Update interval: 100ms
- 3 animated circles with different radii

### Mode 11: Bitmap Test ✨ NEW
Tests bitmap/image rendering capabilities with graphical demo.

**Features**:
- Smiley face rendered using circles and pixels
- Demonstrates combining primitives for complex graphics
- Simple bitmap graphics example

**Use Cases**:
- Verify bitmap rendering capability
- Test combining multiple drawing primitives
- Demonstrate image-like graphics
- Prototype icon/logo display

**Technical Details**:
- Uses ui_gfx_circle() for face and eyes
- Parabolic curve approximation for smile
- Centered at display midpoint

### Mode 12: Fill Patterns ✨ NEW
Tests various fill patterns with automatic cycling.

**Features**:
- 4 different fill patterns (cycles every 500ms)
- Pattern 1: Dots (varied brightness based on position)
- Pattern 2: Dither (checkerboard black/white)
- Pattern 3: Waves (diagonal gradient waves)
- Pattern 4: Grid (8×8 pixel grid lines)
- Pattern counter display

**Use Cases**:
- Test fill algorithms
- Verify pixel uniformity across patterns
- Check pattern rendering performance
- Demonstrate texture/background effects

**Technical Details**:
- Update interval: 500ms per pattern
- Auto-cycles through 4 patterns
- Uses mathematical formulas for procedural patterns

### Mode 13: Stress Test ✨ NEW
Maximum graphics throughput stress test with 18 animated elements.

**Features**:
- 10 moving rectangles at different speeds
- 3 expanding circles with varying radii
- 5 diagonal scrolling lines
- Real-time element count display
- Target frame rate: 60 FPS

**Use Cases**:
- Benchmark maximum graphics performance
- Test system under heavy rendering load
- Identify performance bottlenecks
- Verify display can handle complex scenes

**Technical Details**:
- Update interval: 16ms (~60 FPS target)
- 18 simultaneous animated elements
- Tests rectangles, circles, and lines together
- Measures actual vs target FPS

### Mode 14: Auto-Cycle Demo ✨ NEW
Automatically cycles through all test modes for demonstrations.

**Features**:
- 3-second dwell time per mode
- Visual progress bar showing time remaining
- Countdown timer in milliseconds
- Exits on any button press or encoder rotation
- Cycles modes 0-13 automatically

**Use Cases**:
- Unattended demonstration mode
- Trade show/exhibition displays
- Automated testing sequence
- Quick overview of all features

**Technical Details**:
- 3000ms cycle interval
- Progress bar width: proportional to time elapsed
- Automatically skips self (mode 14) in cycle
- Resets animation state on each mode change

## Navigation

### Encoder Control
- **Rotate Right**: Next test mode (0 → 1 → 2 → ... → 14 → 0)
- **Rotate Left**: Previous test mode (0 → 14 → 13 → ... → 1 → 0)
- Animation state resets on mode change
- Exits auto-cycle mode (14) immediately

### Button Controls
- **Button 0**: Previous test mode
- **Button 1**: Next test mode
- **Button 2**: Clear screen (fill with black)
- **Button 3**: Fill screen white (all pixels max brightness)
- **Button 4**: Clear screen (same as button 2)
- **Button 5**: Reset FPS counter ✨ NEW
- **Any button in Mode 14**: Exit auto-cycle and return to mode 0

## Status Display

### Header Information
Always displayed at the top of the screen:
- Current test mode number (0-14)
- Instruction: "Use ENC" (rotate encoder to change modes)

### Footer Information ✨ NEW
Always displayed at the top right:
- **MS**: Current milliseconds counter (system uptime)
- **FPS**: Frames per second (updated every 1 second)

## Performance Metrics

### Expected Frame Rates
- **Static Modes** (0-3, 5-6, 11): 60 FPS
- **Animation Modes** (4, 7-10, 12): 30-50 FPS
- **Performance Test** (Mode 9): 20-40 FPS
- **Stress Test** (Mode 13): 40-60 FPS
- **Auto-Cycle** (Mode 14): Varies by current mode

### Performance Factors
Frame rate is affected by:
1. **Rendering Complexity**: More graphics = lower FPS
2. **Update Frequency**: How often content changes
3. **System Load**: Other tasks running on MCU
4. **SPI Speed**: Display communication speed
5. **Framebuffer Location**: RAM vs CCMRAM affects speed

## New Graphics Functions ✨

### ui_gfx_circle()
Draws a circle using the midpoint circle algorithm.

```c
void ui_gfx_circle(int cx, int cy, int radius, uint8_t gray);
```

**Parameters**:
- `cx`, `cy`: Center coordinates
- `radius`: Circle radius in pixels
- `gray`: Brightness level (0-15)

**Features**:
- Integer-only math (no floating point)
- Draws all 8 octants symmetrically
- Optimized for embedded systems

### ui_gfx_line()
Draws a line using Bresenham's line algorithm.

```c
void ui_gfx_line(int x0, int y0, int x1, int y1, uint8_t gray);
```

**Parameters**:
- `x0`, `y0`: Start coordinates
- `x1`, `y1`: End coordinates
- `gray`: Brightness level (0-15)

**Features**:
- Integer-only math
- Handles all angles and slopes
- Efficient pixel-perfect rendering
- Supports negative coordinates

## Troubleshooting

### Issue: Low FPS (< 20)
**Possible Causes**:
1. System overloaded with other tasks
2. SPI communication too slow
3. Framebuffer in slow memory region
4. Excessive interrupts

**Solutions**:
- Check FreeRTOS task priorities
- Verify SPI speed configuration
- Move framebuffer to CCMRAM
- Reduce interrupt frequency

### Issue: Animations Stuttering
**Possible Causes**:
1. Inconsistent timing
2. Task scheduling issues
3. Memory access conflicts

**Solutions**:
- Verify HAL_GetTick() accuracy
- Check task periods and priorities
- Use DMA for SPI transfers

### Issue: Graphics Corruption
**Possible Causes**:
1. Framebuffer memory corruption
2. Concurrent access without mutex
3. Buffer overflow

**Solutions**:
- Verify framebuffer size allocation
- Add mutex protection if needed
- Check array bounds in rendering code

### Issue: Missing Circles/Lines
**Possible Causes**:
1. Coordinates out of bounds
2. Brightness level 0 (invisible)
3. Clipping issues

**Solutions**:
- Check coordinate calculations
- Verify brightness values (1-15 for visible)
- Review clipping logic

## Technical Implementation

### Memory Usage
- **Framebuffer**: 8192 bytes (256×64 pixels, 4-bit/pixel)
- **Static Variables**: ~40 bytes (animation state, counters)
- **Stack Usage**: Minimal (< 100 bytes per function)

### CPU Usage
Approximate CPU usage per mode:
- **Static Modes**: < 1% CPU
- **Simple Animations**: 2-5% CPU
- **Complex Animations**: 5-15% CPU
- **Stress Test**: 15-25% CPU

### Update Timing
| Mode | Update Interval | Expected FPS |
|------|----------------|--------------|
| 0-3  | N/A (static)   | 60           |
| 4    | 100ms          | 30-40        |
| 5-6  | N/A (static)   | 60           |
| 7    | 50ms           | 40-50        |
| 8    | 30ms           | 30-40        |
| 9    | 20ms           | 20-30        |
| 10   | 100ms          | 30-40        |
| 11   | N/A (static)   | 60           |
| 12   | 500ms          | 30-40        |
| 13   | 16ms           | 40-60        |
| 14   | 3000ms         | Varies       |

## Use Cases

### Development Testing
1. **Initial Hardware Bring-Up**: Use Mode 0-2 to verify basic display function
2. **Driver Development**: Use Mode 6 to test direct framebuffer access
3. **Performance Tuning**: Use Modes 9 and 13 to benchmark optimizations
4. **Graphics Library**: Use Modes 10 and 11 to test new drawing functions

### Quality Assurance
1. **Manufacturing Test**: Run all modes to verify display quality
2. **Burn-In Test**: Leave Mode 4, 9, or 13 running for extended period
3. **Pixel Defect Check**: Use Mode 2 with all brightness levels
4. **Grayscale Calibration**: Use Mode 1 to verify gray levels
5. **Automated Testing**: Use Mode 14 for hands-free testing cycle

### Demonstration
1. **Customer Demo**: Show capabilities with Modes 8, 10, and 11
2. **Trade Show**: Use Mode 14 for automatic continuous demo
3. **Technical Presentation**: Use Modes 9 and 13 to show performance metrics
4. **Feature Showcase**: Mode 14 automatically cycles through all modes

## Future Enhancements

Potential additions for future versions:
- [x] Bitmap image display test (Mode 11 ✅)
- [ ] Custom pattern upload via SD card
- [x] Automatic mode cycling (Mode 14 ✅)
- [ ] Screen burn-in prevention mode with moving patterns
- [ ] Touch/gesture testing (if hardware supports)
- [ ] Color test mode (for future RGB displays)
- [ ] QR code rendering test
- [ ] Font size variations (small/medium/large)
- [ ] 3D wireframe cube rendering
- [ ] Video playback test (frame buffering)

## Related Documentation

- [OLED Test Protocol](../testing/OLED_TEST_PROTOCOL.md) - Comprehensive testing protocol matching LoopA/UI standards
- [OLED Quick Test](OLED_QUICK_TEST.md) - Basic OLED testing procedures
- [OLED Troubleshooting](OLED_TROUBLESHOOTING.md) - Hardware troubleshooting guide
- [OLED Wiring Guide](OLED_WIRING_GUIDE.md) - Complete wiring documentation
- [Module Testing](../testing/README_MODULE_TESTING.md) - Testing framework documentation
- [Testing Protocol](../testing/TESTING_PROTOCOL.md) - LoopA feature testing (300+ tests)

## References

- **Midpoint Circle Algorithm**: Efficient integer-based circle drawing
- **Bresenham's Line Algorithm**: Pixel-perfect line rasterization
- **SSD1322 Datasheet**: Solomon Systech OLED driver specifications
- **MIOS32 Compatibility**: Based on MIDIbox MIOS32 OLED implementation

---

**Document Version**: 2.0  
**Last Updated**: 2026-01-23  
**Feature Status**: ✅ Production Ready  
**Test Modes**: 15 (expanded from 11)  
**Tested On**: STM32F407VGT6 with SSD1322 256×64 OLED
