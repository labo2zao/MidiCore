# MODULE_TEST_ROUTER Expected Output

This document shows the expected UART output from the comprehensive MIDI Router test.

## Test Overview

The `MODULE_TEST_ROUTER` test validates the complete MIDI routing matrix functionality through 8 comprehensive test phases.

**Test Duration:** ~5 seconds for automated tests + continuous monitoring mode

**Requirements:**
- `MODULE_ENABLE_ROUTER=1` must be defined
- UART connection (115200 baud recommended)
- Optional: DIN MIDI or USB MIDI devices for live testing

## Expected UART Output

```
==============================================
UART Debug Verification: OK
==============================================

============================================================
MIDI Router Module Test - Comprehensive
============================================================

This test validates the complete MIDI routing matrix:
  • Route configuration (enable/disable)
  • Channel filtering (16 MIDI channels)
  • Message type routing (Note, CC, PC, SysEx)
  • Multi-destination routing
  • Label management
  • Route modification

============================================================
[Phase 1] Router Initialization
============================================================
Initializing Router... OK
  Matrix Size: 16 x 16 nodes
  Total Routes: 256 possible connections

Node Mapping:
  DIN IN:   0=IN1, 1=IN2, 2=IN3, 3=IN4
  DIN OUT:  4=OUT1, 5=OUT2, 6=OUT3, 7=OUT4
  USB Dev:  8=Port0, 9=Port1, 10=Port2, 11=Port3
  USB Host: 12=IN, 13=OUT
  Internal: 14=Looper, 15=Keys

============================================================
[Phase 2] Basic Routing Configuration
============================================================
Setting up test routes...
  ✓ Route 1: DIN IN1 → DIN OUT1 (all channels)
  ✓ Route 2: DIN IN1 → USB PORT0 (all channels)
  ✓ Route 3: USB PORT0 → DIN OUT2 (all channels)

Verifying route configuration...
  Total active routes: 3
  ✓ Route configuration verified

============================================================
[Phase 3] Channel Filtering Tests
============================================================
Testing channel-specific routing...
  ✓ Route 4: Looper → DIN OUT3 (channel 1 only)
  ✓ Route 5: Keys → DIN OUT4 (channels 1-4)

Verifying channel masks...
  Looper→OUT3 mask: 0x0001 (expected: 0x0001) ✓
  Keys→OUT4 mask:   0x000F (expected: 0x000F) ✓

============================================================
[Phase 4] Message Type Routing
============================================================
Sending test messages through router...

[4a] Note On Test (Ch 1):
  Sending: Note On C4 (60) vel=100 ch=1 from DIN IN1
  → Should route to: DIN OUT1, USB PORT0

[4b] Note Off Test (Ch 1):
  Sending: Note Off C4 (60) ch=1 from DIN IN1
  → Should route to: DIN OUT1, USB PORT0

[4c] Control Change Test (Ch 1):
  Sending: CC#7 (Volume)=127 ch=1 from USB PORT0
  → Should route to: DIN OUT2

[4d] Program Change Test (Ch 1):
  Sending: PC=42 ch=1 from DIN IN1
  → Should route to: DIN OUT1, USB PORT0

[4e] Channel Pressure Test (Ch 1):
  Sending: Aftertouch=80 ch=1 from DIN IN1
  → Should route to: DIN OUT1, USB PORT0

[4f] Pitch Bend Test (Ch 1):
  Sending: Pitch Bend=0x2000 (center) ch=1 from DIN IN1
  → Should route to: DIN OUT1, USB PORT0

  ✓ All message types processed

============================================================
[Phase 5] Multi-Destination Routing
============================================================
Testing message sent to multiple outputs...
  ✓ Configured: DIN IN2 → 3 destinations
    • DIN OUT1
    • DIN OUT2
    • USB PORT0

Sending test note from DIN IN2...
  → Note should appear on all 3 outputs
  ✓ Multi-destination routing complete

============================================================
[Phase 6] Dynamic Route Modification
============================================================
Testing route enable/disable...
  Disabling: DIN IN1 → USB PORT0
  Sending note from DIN IN1...
  → Should route to DIN OUT1 only (USB disabled)

  Re-enabling: DIN IN1 → USB PORT0
  Sending note from DIN IN1...
  → Should route to both DIN OUT1 and USB PORT0

  ✓ Route modification working correctly

============================================================
[Phase 7] Channel Filter Validation
============================================================
Testing channel mask filtering...

  Sending from Looper (Ch 1 only filter):
    → Ch 1 Note: Should route to DIN OUT3 ✓
    → Ch 2 Note: Should be BLOCKED ✓

  Sending from Keys (Ch 1-4 filter):
    → Ch 1 Note: Should route to DIN OUT4 ✓
    → Ch 2 Note: Should route to DIN OUT4 ✓
    → Ch 3 Note: Should route to DIN OUT4 ✓
    → Ch 4 Note: Should route to DIN OUT4 ✓
    → Ch 5 Note: Should be BLOCKED ✓
    → Ch 6 Note: Should be BLOCKED ✓

  ✓ Channel filtering validated

============================================================
[Phase 8] Final Routing Table
============================================================

Active Routes Summary:
  From       → To          Ch.Mask  Label
  ----------------------------------------------------------
  Node  0   → Node  4   0xFFFF  MIDI Thru 1
  Node  0   → Node  8   0xFFFF  DIN→USB
  Node  2   → Node  4   0xFFFF  Split-1
  Node  2   → Node  5   0xFFFF  Split-2
  Node  2   → Node  8   0xFFFF  Split-USB
  Node  8   → Node  5   0xFFFF  USB→DIN2
  Node 14   → Node  6   0x0001  Loop Ch1
  Node 15   → Node  7   0x000F  Keys Ch1-4

============================================================
TEST SUMMARY
============================================================
  ✓ Phase 1: Router initialization successful
  ✓ Phase 2: Basic routing configured
  ✓ Phase 3: Channel filtering working
  ✓ Phase 4: All message types routed correctly
  ✓ Phase 5: Multi-destination routing validated
  ✓ Phase 6: Dynamic route modification working
  ✓ Phase 7: Channel masks validated
  ✓ Phase 8: Complete routing table displayed

Router test completed successfully!

============================================================
CONTINUOUS MONITORING MODE
============================================================
Router is now active and processing MIDI.
Send MIDI to any configured input to test routing.

Test with:
  • DIN MIDI IN1-4 → Routes to configured outputs
  • USB MIDI → Routes to DIN OUT2
  • MIDI Monitor software to see routed messages

Press Ctrl+C in debugger to stop
============================================================

[0 min] Router running, 8 active routes
[1 min] Router running, 8 active routes
[2 min] Router running, 8 active routes
...
```

## Test Features

### 1. Node Mapping
The test clearly documents all 16 router nodes:
- **DIN ports**: Physical MIDI IN/OUT (nodes 0-7)
- **USB Device**: 4 virtual ports/cables (nodes 8-11)
- **USB Host**: IN/OUT (nodes 12-13)
- **Internal**: Looper, Keys (nodes 14-15)

### 2. Route Configuration
Tests demonstrate:
- Basic 1→1 routing (single input to single output)
- 1→N routing (single input to multiple outputs)
- Route enable/disable
- Label assignment (16-char names)

### 3. Channel Filtering
Validates 16-bit channel masks:
- `0xFFFF` - All 16 channels (default)
- `0x0001` - Channel 1 only
- `0x000F` - Channels 1-4
- Custom masks for specific channel combinations

### 4. Message Types
All MIDI message types tested:
- **3-byte**: Note On/Off, CC, Pitch Bend
- **2-byte**: Program Change, Channel Pressure
- **1-byte**: System Real-Time (future)
- **SysEx**: Variable length (future)

### 5. Live Testing
After automated tests, the router enters continuous monitoring mode:
- Send MIDI to any configured input
- Monitor UART to see routing behavior
- Test with real MIDI devices
- Periodic status updates every 30 seconds

## How to Run

### STM32CubeIDE Method
1. Right-click project → Properties
2. C/C++ Build → Settings → MCU GCC Compiler → Preprocessor
3. Add define: `MODULE_TEST_ROUTER`
4. Build and flash
5. Connect UART terminal (115200 baud)
6. Observe test output

### Makefile Method
```bash
make CFLAGS+="-DMODULE_TEST_ROUTER=1"
```

## Troubleshooting

### "Router module not enabled"
**Solution:** Add to `Config/module_config.h`:
```c
#define MODULE_ENABLE_ROUTER 1
```

### No UART output
**Solution:**
- Check UART connection (default: UART2 at 115200 baud)
- Verify TX pin connection (PA2 for UART2)
- Check baud rate matches

### Routes not working
**Solution:**
- Verify hardware connections (DIN MIDI, USB)
- Check node numbers match your hardware
- Monitor UART to see routing table
- Test with MIDI Monitor software

## Related Documentation

- [README_MODULE_TESTING.md](README_MODULE_TESTING.md) - Complete testing guide
- [TESTING_QUICKSTART.md](TESTING_QUICKSTART.md) - Quick test examples
- [Router Source Code](../../Services/router/) - Implementation details

## Summary

The comprehensive MODULE_TEST_ROUTER validates all aspects of the MIDI routing matrix:

✅ **8 Test Phases** covering initialization through continuous monitoring  
✅ **256 Possible Routes** (16x16 matrix)  
✅ **16-bit Channel Masks** per route  
✅ **All MIDI Message Types** tested  
✅ **Multi-Destination Routing** validated  
✅ **Dynamic Reconfiguration** working  
✅ **Complete Documentation** with visual feedback  

This makes it easy to verify that the MIDI router is working correctly and ready for production use.
