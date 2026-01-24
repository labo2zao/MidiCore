# MIDI DIN Module Test with LiveFX and MIDI Learn

## Overview

The `MODULE_TEST_MIDI_DIN` test provides a comprehensive demonstration of MIDI processing capabilities in MidiCore, combining:

1. **MIDI I/O** - Bidirectional MIDI communication over DIN ports
2. **LiveFX Transform** - Real-time MIDI message transformation
3. **MIDI Learn** - Dynamic parameter mapping via CC messages

This test allows you to:
- Receive MIDI from a controller via DIN IN
- Apply real-time effects (transpose, velocity scale, force-to-scale)
- Control effects parameters using MIDI CC messages
- Send transformed MIDI to DIN OUT for monitoring

## Hardware Requirements

- **STM32F407VGT6** microcontroller (or compatible)
- **MIDI DIN IN/OUT** interface (UART-based)
- **MIDI Controller** (keyboard, pad controller, etc.)
- **MIDI Device** (synth, DAW, or MIDI monitor)
- **UART Debug Connection** (115200 baud, 8N1)

## Wiring

### MIDI DIN Connections
```
Controller → DIN IN1 (UART3 RX, PB11)
DIN OUT1 (UART3 TX, PB10) → Synth/DAW
```

### Debug UART
```
Debug UART2 (PA2/PA3) → USB-Serial adapter @ 115200 baud
```

## Building and Flashing

### Using STM32CubeIDE

1. Open project in STM32CubeIDE
2. Go to Project Properties → C/C++ Build → Settings → MCU GCC Compiler → Preprocessor
3. Add define: `MODULE_TEST_MIDI_DIN`
4. Build and flash the project

### Using Command Line (if Makefile available)

```bash
make CFLAGS+="-DMODULE_TEST_MIDI_DIN"
```

### Optional: Select UART Port

If you need to use a different UART for DIN1:

```bash
make CFLAGS+="-DMODULE_TEST_MIDI_DIN -DTEST_MIDI_DIN_UART_PORT=2"
```

## Running the Test

### 1. Initial Setup

1. Flash the firmware with `MODULE_TEST_MIDI_DIN` enabled
2. Connect MIDI controller to DIN IN1
3. Connect DIN OUT1 to synth or MIDI monitor
4. Open serial terminal at 115200 baud
5. Reset the board

### 2. Test Sequence

The test starts with LiveFX **disabled** by default. MIDI messages pass through unmodified.

#### Step 1: Verify MIDI I/O

1. Play notes on your MIDI controller
2. Notes should pass through to DIN OUT1 unchanged
3. Debug UART shows received messages:
   ```
   [RX] DIN1: 90 3C 64 NOTE_ON Ch:1 Note:60 Vel:100
   ```

#### Step 2: Enable LiveFX

1. Send **CC 20** with value **127** (Channel 1)
2. Debug output confirms:
   ```
   [RX] DIN1: B0 14 7F CC Ch:1 CC:20 Val:127
   [LEARN] LiveFX ENABLED
   ```

#### Step 3: Transpose Notes

1. Send **CC 22** (transpose up) multiple times
2. Each CC 22 increases transpose by +1 semitone
3. Debug output shows:
   ```
   [RX] DIN1: B0 16 7F CC Ch:1 CC:22 Val:127
   [LEARN] Transpose: +1
   ```
4. Play note C4 (MIDI note 60)
5. Output will be C#4 (MIDI note 61):
   ```
   [RX] DIN1: 90 3C 64 NOTE_ON Ch:1 Note:60 Vel:100
   [TX] DIN OUT1 (transformed): 90 3D 64 Note:60→61 Vel:100→100
   ```

#### Step 4: Adjust Velocity

1. Send **CC 25** (velocity up) twice for +20%
2. Debug output:
   ```
   [LEARN] Velocity Scale: 110%
   [LEARN] Velocity Scale: 120%
   ```
3. Play note at velocity 100
4. Output will be velocity 120:
   ```
   [TX] DIN OUT1 (transformed): 90 3C 78 Note:60→60 Vel:100→120
   ```

#### Step 5: Force-to-Scale

1. Send **CC 28** with value **1** (Major scale)
2. Send **CC 29** with value **0** (C root)
3. Send **CC 27** with value **127** (enable force-to-scale)
4. Debug output:
   ```
   [LEARN] Scale Type: 1
   [LEARN] Scale Root: 0
   [LEARN] Force-to-Scale: ON
   ```
5. Play C# (note 61) - it will snap to D (note 62)
6. Play D# (note 63) - it will snap to E (note 64)
7. All notes will be quantized to C Major scale

#### Step 6: Reset Parameters

1. Send **CC 23** to reset transpose to 0
2. Send **CC 26** to reset velocity scale to 100%
3. Send **CC 27** with value **0** to disable force-to-scale
4. Send **CC 20** with value **0** to disable LiveFX

## MIDI Learn Command Reference

All commands use **MIDI Channel 1** (status byte 0xB0).

| CC Number | Function | Value Range | Description |
|-----------|----------|-------------|-------------|
| 20 | LiveFX Enable | 0-127 | > 64 = Enabled, ≤ 64 = Disabled |
| 21 | Transpose Down | Any | Decreases transpose by 1 semitone |
| 22 | Transpose Up | Any | Increases transpose by 1 semitone |
| 23 | Transpose Reset | Any | Sets transpose to 0 |
| 24 | Velocity Down | Any | Decreases velocity scale by 10% |
| 25 | Velocity Up | Any | Increases velocity scale by 10% |
| 26 | Velocity Reset | Any | Sets velocity scale to 100% |
| 27 | Force-to-Scale | 0-127 | > 64 = Enabled, ≤ 64 = Disabled |
| 28 | Scale Type | 0-11 | 0=Chromatic, 1=Major, 2=Minor, etc. |
| 29 | Scale Root | 0-11 | 0=C, 1=C#, 2=D, ..., 11=B |

## LiveFX Parameters

### Transpose
- **Range**: -12 to +12 semitones
- **Default**: 0
- **Effect**: Shifts all notes up or down by semitones
- **Use case**: Change key without rerecording

### Velocity Scale
- **Range**: 0% to 200% (0-255 internal, 128 = 100%)
- **Default**: 100%
- **Effect**: Multiplies note velocity by scale factor
- **Use case**: Adjust dynamics, make notes louder/softer

### Force-to-Scale
- **Scale Types**: 0-11 (Chromatic, Major, Minor, etc.)
- **Scale Root**: 0-11 (C to B)
- **Default**: Disabled
- **Effect**: Quantizes notes to nearest scale degree
- **Use case**: Ensure melody stays in key, create modal effects

## Debug Output Format

### Received Messages
```
[RX] DIN1: 90 3C 64 NOTE_ON Ch:1 Note:60 Vel:100
```

### Transformed Messages
```
[TX] DIN OUT1 (transformed): 90 3D 78 Note:60→61 Vel:100→120
```

### MIDI Learn Events
```
[LEARN] LiveFX ENABLED
[LEARN] Transpose: +2
[LEARN] Velocity Scale: 120%
[LEARN] Force-to-Scale: ON
```

### Status Updates (every 10 seconds)
```
--- LiveFX Status ---
Enabled: YES | Transpose: +2 | Velocity: 120% | Scale: ON
---------------------
```

## Troubleshooting

### No MIDI Input
- Check MIDI cable connections
- Verify UART port configuration (PB11 for UART3)
- Check MIDI controller is sending on Channel 1
- Monitor debug UART for activity

### No MIDI Output
- Check DIN OUT1 connection (PB10 for UART3)
- Verify router is enabled (`MODULE_ENABLE_ROUTER=1`)
- Check receiving device is listening

### LiveFX Not Working
- Verify `MODULE_ENABLE_LIVEFX=1` in Config/module_config.h
- Send CC 20 (value 127) to enable LiveFX
- Check debug output for [LEARN] messages

### Transformed Notes Incorrect
- Send CC 23 to reset transpose
- Send CC 26 to reset velocity scale
- Send CC 27 (value 0) to disable force-to-scale
- Verify parameters in status output

## Technical Details

### Module Dependencies

This test requires the following modules to be enabled:

- `MODULE_ENABLE_MIDI_DIN` - MIDI DIN I/O
- `MODULE_ENABLE_ROUTER` - MIDI message routing
- `MODULE_ENABLE_LIVEFX` - Real-time effects
- `MODULE_ENABLE_SCALE` - Scale quantization (for force-to-scale)

### Code Architecture

```
MIDI IN → midi_din_tick() → Parse Messages → Check CC (MIDI Learn)
                                           ↓
                              Apply LiveFX Transform (if enabled)
                                           ↓
                              midi_din_send() → MIDI OUT
```

### Performance

- **Latency**: < 1ms typical (depends on UART speed and processing)
- **Max Notes**: Limited by UART buffer size
- **CC Processing**: Immediate parameter updates
- **Status Updates**: Every 10 seconds (non-blocking)

## Example Session

```
==============================================
UART Debug Verification: OK
==============================================

╔══════════════════════════════════════════════════════════════════════════════╗
║                MIDI DIN Module Test with LiveFX & MIDI Learn                 ║
╚══════════════════════════════════════════════════════════════════════════════╝

Initializing MIDI DIN service... OK
Initializing MIDI Router... OK
Router configured: DIN IN1 → DIN OUT1
Initializing LiveFX... OK

──────────────────────────────────────────────────────────────────────────────
MIDI DIN I/O Test with LiveFX Transform & MIDI Learn
──────────────────────────────────────────────────────────────────────────────

Features:
  1. MIDI I/O: Receives from DIN IN1, sends to DIN OUT1
  2. LiveFX: Transpose, velocity scale, force-to-scale
  3. MIDI Learn: Map CC messages to LiveFX parameters

MIDI Learn Commands (Channel 1):
  CC 20 (val>64) = Enable LiveFX
  CC 21 = Transpose Down (-1 semitone)
  CC 22 = Transpose Up (+1 semitone)
  ...

Current LiveFX Settings:
  Enabled: NO
  Transpose: +0 semitones
  Velocity Scale: 100% (128/128)
  Force-to-Scale: OFF (Type:0 Root:0)

──────────────────────────────────────────────────────────────────────────────
Monitoring MIDI activity...
──────────────────────────────────────────────────────────────────────────────

[RX] DIN1: B0 14 7F CC Ch:1 CC:20 Val:127
[LEARN] LiveFX ENABLED

[RX] DIN1: B0 16 7F CC Ch:1 CC:22 Val:127
[LEARN] Transpose: +1

[RX] DIN1: 90 3C 64 NOTE_ON Ch:1 Note:60 Vel:100
[TX] DIN OUT1 (transformed): 90 3D 64 Note:60→61 Vel:100→100

--- LiveFX Status ---
Enabled: YES | Transpose: +1 | Velocity: 100% | Scale: OFF
---------------------
```

## Advanced Usage

### Custom MIDI Learn Mappings

To add your own MIDI learn mappings, edit the CC switch statement in `module_test_midi_din_run()`:

```c
case 30:  // Your custom CC
  // Your custom parameter control
  break;
```

### Multiple Tracks

The test currently uses Track 0. To extend for multiple tracks:

```c
uint8_t track = (cc - 20) / 10;  // CC 20-29 = Track 0, CC 30-39 = Track 1, etc.
livefx_set_enabled(track, val > 64);
```

### Integration with Looper

Combine with looper module for recording transformed MIDI:

```c
#if MODULE_ENABLE_LOOPER
looper_on_router_msg(0, &msg);
#endif
```

## See Also

- [Module Testing Guide](README_MODULE_TESTING.md)
- [LiveFX Documentation](../../Services/livefx/README.md)
- [MIDI Router Documentation](../../Services/router/README.md)
- [Scale Module Documentation](../../Services/scale/README.md)

## License

This test is part of the MidiCore project. See main LICENSE file for details.
