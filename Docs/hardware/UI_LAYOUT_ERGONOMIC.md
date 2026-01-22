# MidiCore LoopA - Ergonomic UI Layout

## Physical Interface Design

This document shows the recommended ergonomic layout for the MidiCore LoopA hardware interface.

### Front Panel Layout (ASCII Diagram)

```
╔═══════════════════════════════════════════════════════════════════════════╗
║                          LOOPA MIDI SEQUENCER                             ║
║                                                                           ║
║  ┌────────────────────────────────────────────────────────────────────┐  ║
║  │                                                                    │  ║
║  │                  SSD1322 OLED Display                             │  ║
║  │                     256 × 64 pixels                               │  ║
║  │                   (Grayscale 4-bit)                               │  ║
║  │                                                                    │  ║
║  │   ┌──────────┬──────────┬──────────┬──────────┬──────────┐       │  ║
║  │   │ TRACK 1  │ TRACK 2  │ TRACK 3  │ TRACK 4  │ TRACK 5  │       │  ║
║  │   │  ▓▓▓▓▓   │  ▓▓▓▓▓   │  ░░░░░   │  ▓▓▓▓▓   │  ░░░░░   │       │  ║
║  │   │  MIDI    │  DRUMS   │  BASS    │  LEAD    │  PAD     │       │  ║
║  │   └──────────┴──────────┴──────────┴──────────┴──────────┘       │  ║
║  │                                                                    │  ║
║  └────────────────────────────────────────────────────────────────────┘  ║
║                                                                           ║
║  ╭─────────────────────────╮              ╭─────────────────────────╮   ║
║  │   NAVIGATION CLUSTER    │              │   LOOPER CONTROLS       │   ║
║  │                         │              │                         │   ║
║  │        ┌─────┐          │              │  ┌─────┐    ┌─────┐    │   ║
║  │        │ UP  │          │              │  │ REC │    │PLAY │    │   ║
║  │        └──┬──┘          │              │  └──┬──┘    └──┬──┘    │   ║
║  │  ┌─────┐ │ ┌─────┐     │              │     │          │        │   ║
║  │  │LEFT ├─┼─┤RIGHT│     │              │  ┌──┴──┐    ┌──┴──┐    │   ║
║  │  └──┬──┘ │ └──┬──┘     │              │  │MUTE │    │CLEAR│    │   ║
║  │     │ ┌──┴──┐ │        │              │  └──┬──┘    └──┬──┘    │   ║
║  │     │ │DOWN │ │        │              │     │          │        │   ║
║  │     │ └─────┘ │        │              │  ┌──┴──┐    ┌──┴──┐    │   ║
║  │     │         │        │              │  │UNDO │    │SHIFT│    │   ║
║  │  ┌──┴──┐   ┌──┴──┐    │              │  └─────┘    └─────┘    │   ║
║  │  │EXIT │   │ SEL │    │              │                         │   ║
║  │  └─────┘   └─────┘    │              │                         │   ║
║  ╰─────────────────────────╯              ╰─────────────────────────╯   ║
║                                                                           ║
║  ╭─────────────────────────────────────────────────────────────────╮    ║
║  │                    ROTARY ENCODERS                              │    ║
║  │                                                                 │    ║
║  │      ╭───────╮                              ╭───────╮          │    ║
║  │      │  ╱─╲  │  ENC1                  ENC2  │  ╱─╲  │          │    ║
║  │      │ │   │ │  VALUE / TEMPO         NOTE  │ │   │ │          │    ║
║  │      │  ╲─╱  │  (Push: SELECT)     (Push: │  ╲─╱  │          │    ║
║  │      ╰───┬───╯                       ENTER) ╰───┬───╯          │    ║
║  │          │                                       │              │    ║
║  │      [  PUSH  ]                             [  PUSH  ]         │    ║
║  │                                                                 │    ║
║  ╰─────────────────────────────────────────────────────────────────╯    ║
║                                                                           ║
║  ╭─────────────────────────────────────────────────────────────────╮    ║
║  │              OPTIONAL TRANSPORT CONTROLS                        │    ║
║  │                                                                 │    ║
║  │    ┌─────┐  ┌─────┐  ┌─────┐  ┌─────┐         ╭──────╮        │    ║
║  │    │ |◄◄ │  │ ►|  │  │ ■   │  │ ●   │         │ TAP  │        │    ║
║  │    │ REW │  │ FF  │  │STOP │  │ REC │         │TEMPO │        │    ║
║  │    └─────┘  └─────┘  └─────┘  └─────┘         ╰──────╯        │    ║
║  │                                                                 │    ║
║  ╰─────────────────────────────────────────────────────────────────╯    ║
║                                                                           ║
║  ╭─────────────────────────────────────────────────────────────────╮    ║
║  │                    FOOTSWITCH INPUTS                            │    ║
║  │                                                                 │    ║
║  │        FS1          FS2          FS3          FS4              │    ║
║  │       ╱◯╲         ╱◯╲         ╱◯╲         ╱◯╲                │    ║
║  │      ( FS1 )     ( FS2 )     ( FS3 )     ( FS4 )              │    ║
║  │       ╲_╱         ╲_╱         ╲_╱         ╲_╱                │    ║
║  │                                                                 │    ║
║  ╰─────────────────────────────────────────────────────────────────╯    ║
║                                                                           ║
╚═══════════════════════════════════════════════════════════════════════════╝
```

### Detailed Component Breakdown

#### 1. OLED Display (Top)
- **Model**: SSD1322 (256×64 pixels, 4-bit grayscale)
- **Pins**: PA8 (RST), PC8 (SCL), PC11 (SDA) - Software SPI
- **Function**: Main UI display showing:
  - Track status and patterns
  - Parameter values
  - Menus and settings
  - Real-time feedback

#### 2. Navigation Cluster (Left Side)
```
     [UP]
      │
[LT]──┼──[RT]
      │
    [DOWN]

[EXIT]  [SELECT]
```

**Buttons**:
- **UP**: Navigate up in menus, increase values
- **DOWN**: Navigate down in menus, decrease values
- **LEFT**: Move cursor left, previous option
- **RIGHT**: Move cursor right, next option
- **EXIT**: Back/Cancel operation
- **SELECT**: Confirm/Enter selection

**GPIO**: Via SRIO shift registers (J10A/J10B connectors)

#### 3. Looper Controls (Right Side)
```
[REC ]  [PLAY]
[MUTE]  [CLEAR]
[UNDO]  [SHIFT]
```

**Buttons**:
- **REC**: Record/Arm track
- **PLAY**: Play/Stop track
- **MUTE**: Mute/Unmute track
- **CLEAR**: Clear track data
- **UNDO**: Undo last action
- **SHIFT**: Modifier key (500ms hold activates shift layer)

**GPIO**: Via SRIO shift registers

#### 4. Rotary Encoders (Center)
```
    ╭───────╮
    │  ╱─╲  │  ENC1: VALUE/TEMPO
    │ │   │ │  (Infinite rotation)
    │  ╲─╱  │  Push button: SELECT
    ╰───┬───╯
        │
    [  PUSH  ]
```

**ENC1** (Left Encoder):
- **Rotation**: Adjust value, set tempo, navigate parameters
- **Push**: Select/Confirm (alternate to SELECT button)
- **Function**: Primary parameter control

**ENC2** (Right Encoder):
- **Rotation**: Note selection, fine-tuning, secondary parameter
- **Push**: Enter/OK
- **Function**: Secondary parameter control

**GPIO**: Via SRIO shift registers
- Quadrature encoding (A/B phases)
- Debounced push buttons

#### 5. Optional Transport Controls (Bottom Center)
```
[|◄◄]  [►|]  [■]  [●]    [TAP TEMPO]
 REW    FF   STOP REC     (Large button)
```

**Buttons**:
- **REW**: Rewind/Previous section
- **FF**: Fast forward/Next section
- **STOP**: Stop all playback
- **REC**: Global record enable
- **TAP TEMPO**: Tap to set tempo (large, easy to hit)

**Optional**: Can be omitted if not needed

#### 6. Footswitch Inputs (Bottom)
```
  FS1      FS2      FS3      FS4
 ╱◯╲     ╱◯╲     ╱◯╲     ╱◯╲
( FS1 )  ( FS2 )  ( FS3 )  ( FS4 )
 ╲_╱     ╲_╱     ╲_╱     ╲_╱
```

**Footswitches** (1/4" TRS jacks):
- **FS1**: REC/PLAY toggle
- **FS2**: MUTE current track
- **FS3**: UNDO last action
- **FS4**: User assignable

**Connection**: Via DIN module (J10A/J10B)

### Ergonomic Design Principles

#### Hand Position Zones
```
    ┌─────────────┐
    │   DISPLAY   │  ← Eyes
    └─────────────┘
         
    ┌──────┐ ┌──────┐
    │ NAV  │ │LOOPER│  ← Thumbs/Fingers
    └──────┘ └──────┘
         
    ┌──────────────┐
    │   ENCODERS   │  ← Hands
    └──────────────┘
         
    ┌──────────────┐
    │  TRANSPORT   │  ← Lower fingers
    └──────────────┘
         
      ╱◯╲  ╱◯╲
      FS    FS        ← Feet
```

#### Advantages:
1. **Display at eye level** - Easy to read without neck strain
2. **Most-used buttons in center** - NAV and LOOPER within easy thumb reach
3. **Encoders in natural hand position** - Arms relaxed, wrists straight
4. **Transport at bottom** - Less frequently used, still accessible
5. **Footswitches for hands-free operation** - Play/record while performing

### Button Mapping Summary

| Physical Button | Logical ID | Function | GPIO/SRIO |
|----------------|-----------|----------|-----------|
| UP | BTN_UP | Navigate up | SRIO |
| DOWN | BTN_DOWN | Navigate down | SRIO |
| LEFT | BTN_LEFT | Navigate left | SRIO |
| RIGHT | BTN_RIGHT | Navigate right | SRIO |
| EXIT | BTN_EXIT | Cancel/Back | SRIO |
| SELECT | BTN_SELECT | Confirm/Enter | SRIO |
| REC | BTN_REC | Record track | SRIO |
| PLAY | BTN_PLAY | Play/Stop | SRIO |
| MUTE | BTN_MUTE | Mute track | SRIO |
| CLEAR | BTN_CLEAR | Clear track | SRIO |
| UNDO | BTN_UNDO | Undo action | SRIO |
| SHIFT | BTN_SHIFT | Modifier key | SRIO |
| ENC1 | ENC1 | Value/Tempo | SRIO (A/B) |
| ENC1_PUSH | BTN_ENC1 | Select | SRIO |
| ENC2 | ENC2 | Note/Fine | SRIO (A/B) |
| ENC2_PUSH | BTN_ENC2 | Enter | SRIO |

### Hardware Requirements

**Minimum Configuration**:
- 1× SSD1322 OLED (256×64, grayscale)
- 6× Navigation buttons (UP, DOWN, LEFT, RIGHT, EXIT, SELECT)
- 6× Looper buttons (REC, PLAY, MUTE, CLEAR, UNDO, SHIFT)
- 2× Rotary encoders with push buttons (ENC1, ENC2)

**Recommended Configuration**:
- Above + 4× Transport buttons (REW, FF, STOP, REC)
- Above + 1× Large TAP TEMPO button
- Above + 2-4× Footswitch inputs (1/4" TRS jacks)

**GPIO Resources**:
- OLED: 3 pins (PA8/RST, PC8/SCL, PC11/SDA) - Software SPI, no SRIO needed
- Buttons/Encoders: SRIO shift registers via J10A/J10B (PE2-PE15, PC13-PC15)
- Debouncing: 20ms (configurable in `Services/input/input.h`)

### Physical Dimensions (Suggested)

```
Width:  250mm (10 inches)
Height: 180mm (7 inches) - front panel
Depth:   60mm (2.4 inches) - enclosure

Button spacing: 20mm center-to-center
Encoder spacing: 80mm center-to-center
Display area: 60mm × 15mm (approx)
```

### Enclosure Considerations

1. **Tilt angle**: 15-20° for optimal viewing and button access
2. **Button caps**: 12-15mm diameter, tactile feedback
3. **Encoder knobs**: 25-30mm diameter, knurled or rubberized
4. **Display bezel**: Recessed 2-3mm to protect screen
5. **Footswitch jacks**: Rear panel mounting
6. **MIDI/USB ports**: Rear panel
7. **Power input**: Rear panel (12V DC or USB-C)

### Color Coding (Optional)

```
Navigation: Blue buttons
Looper:     Green/Red buttons (REC=Red, others Green)
Transport:  Yellow buttons
SHIFT:      Orange button (modifier)
Encoders:   Silver/Aluminum knobs
Display:    White/Blue OLED
```

### Alternative Compact Layout

For smaller enclosure (170mm × 120mm):

```
╔═══════════════════════════════════════════════════╗
║  ┌──────────────────────────────────────────┐   ║
║  │      SSD1322 OLED (256×64)               │   ║
║  └──────────────────────────────────────────┘   ║
║                                                  ║
║    [UP]         ╭─────╮         [REC] [PLAY]   ║
║  [LT][RT]       │ ENC1│         [MUTE][CLEAR]  ║
║   [DOWN]        ╰─────╯         [UNDO][SHIFT]  ║
║                                                  ║
║  [EXIT][SEL]    ╭─────╮    [TAP TEMPO]         ║
║                 │ ENC2│                         ║
║                 ╰─────╯                         ║
╚═══════════════════════════════════════════════════╝
```

Compact layout trades transport controls and footswitches for smaller size.

### References

- Hardware definitions: `Core/Inc/main.h`
- Button mapping: `Docs/user-guides/SCS_BUTTONS_ANALYSIS.md`
- Input system: `Services/input/input.h`
- OLED driver: `Hal/oled_ssd1322/oled_ssd1322.c`
- Testing protocol: `Docs/testing/TESTING_PROTOCOL.md`
- Pin configuration: `Config/oled_pins.h`

---

**Document Version**: 1.0  
**Created**: 2026-01-22  
**For**: MidiCore LoopA Hardware
