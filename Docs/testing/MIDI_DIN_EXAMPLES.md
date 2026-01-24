# MODULE_TEST_MIDI_DIN Quick Start Examples

## Example 1: Basic MIDI Echo (LiveFX Disabled)

**Setup:**
1. Flash firmware with `MODULE_TEST_MIDI_DIN` enabled
2. Connect MIDI keyboard to DIN IN1
3. Connect DIN OUT1 to synth
4. Open serial monitor at 115200 baud

**Test:**
1. Play C4 (MIDI note 60) on keyboard
2. You should see on serial monitor:
   ```
   [RX] DIN1: 90 3C 64 NOTE_ON Ch:1 Note:60 Vel:100
   ```
3. Synth should play C4 (unchanged)

**Result:** MIDI passes through unmodified ✓

---

## Example 2: Transpose Up by 5 Semitones

**Setup:** Same as Example 1

**Test:**
1. Send CC 20 with value 127 (enable LiveFX)
   ```
   MIDI: B0 14 7F
   ```
   Monitor shows: `[LEARN] LiveFX ENABLED`

2. Send CC 22 (transpose up) five times:
   ```
   MIDI: B0 16 7F  (1st time)
   MIDI: B0 16 7F  (2nd time)
   MIDI: B0 16 7F  (3rd time)
   MIDI: B0 16 7F  (4th time)
   MIDI: B0 16 7F  (5th time)
   ```
   Monitor shows:
   ```
   [LEARN] Transpose: +1
   [LEARN] Transpose: +2
   [LEARN] Transpose: +3
   [LEARN] Transpose: +4
   [LEARN] Transpose: +5
   ```

3. Play C4 (note 60) on keyboard
4. Monitor shows:
   ```
   [RX] DIN1: 90 3C 64 NOTE_ON Ch:1 Note:60 Vel:100
   [TX] DIN OUT1 (transformed): 90 41 64 Note:60→65 Vel:100→100
   ```
5. Synth plays F4 (note 65 = 60 + 5)

**Result:** Notes transposed up 5 semitones ✓

---

## Example 3: Increase Velocity by 50%

**Setup:** Same as Example 1, LiveFX enabled

**Test:**
1. Send CC 25 (velocity up) five times for +50%
   ```
   MIDI: B0 19 7F (×5 times)
   ```
   Monitor shows:
   ```
   [LEARN] Velocity Scale: 110%
   [LEARN] Velocity Scale: 120%
   [LEARN] Velocity Scale: 130%
   [LEARN] Velocity Scale: 140%
   [LEARN] Velocity Scale: 150%
   ```

2. Play C4 at velocity 80
3. Monitor shows:
   ```
   [RX] DIN1: 90 3C 50 NOTE_ON Ch:1 Note:60 Vel:80
   [TX] DIN OUT1 (transformed): 90 3C 78 Note:60→60 Vel:80→120
   ```
4. Synth plays C4 at velocity 120 (80 × 1.5 = 120)

**Result:** Notes 50% louder ✓

---

## Example 4: Force to C Major Scale

**Setup:** Same as Example 1, LiveFX enabled, transpose reset to 0

**Test:**
1. Set scale type to Major (1):
   ```
   MIDI: B0 1C 01
   ```
   Monitor: `[LEARN] Scale Type: 1`

2. Set scale root to C (0):
   ```
   MIDI: B0 1D 00
   ```
   Monitor: `[LEARN] Scale Root: 0`

3. Enable force-to-scale:
   ```
   MIDI: B0 1B 7F
   ```
   Monitor: `[LEARN] Force-to-Scale: ON`

4. Play C# (note 61) - it snaps to D (note 62)
   Monitor:
   ```
   [RX] DIN1: 90 3D 64 NOTE_ON Ch:1 Note:61 Vel:100
   [TX] DIN OUT1 (transformed): 90 3E 64 Note:61→62 Vel:100→100
   ```

5. Play D# (note 63) - it snaps to E (note 64)
   Monitor:
   ```
   [RX] DIN1: 90 3F 64 NOTE_ON Ch:1 Note:63 Vel:100
   [TX] DIN OUT1 (transformed): 90 40 64 Note:63→64 Vel:100→100
   ```

**Result:** All notes quantized to C Major scale (C, D, E, F, G, A, B) ✓

**C Major Scale Notes:**
- C (60) → C (60) ✓
- C# (61) → D (62)
- D (62) → D (62) ✓
- D# (63) → E (64)
- E (64) → E (64) ✓
- F (65) → F (65) ✓
- F# (66) → G (67)
- G (67) → G (67) ✓
- G# (68) → A (69)
- A (69) → A (69) ✓
- A# (70) → B (71)
- B (71) → B (71) ✓

---

## Example 5: Combine All Effects

**Setup:** Same as Example 1

**Test:**
1. Enable LiveFX:
   ```
   MIDI: B0 14 7F
   ```

2. Transpose up by 2 semitones (2× CC 22):
   ```
   MIDI: B0 16 7F
   MIDI: B0 16 7F
   ```

3. Increase velocity by 20% (2× CC 25):
   ```
   MIDI: B0 19 7F
   MIDI: B0 19 7F
   ```

4. Enable C Major force-to-scale:
   ```
   MIDI: B0 1C 01  (Major scale)
   MIDI: B0 1D 00  (C root)
   MIDI: B0 1B 7F  (Enable)
   ```

5. Play C4 (note 60) at velocity 100
6. Processing:
   - Transpose: 60 + 2 = 62 (D)
   - Force-to-scale: 62 (D) → 62 (D) ✓ (D is in C Major)
   - Velocity: 100 × 1.2 = 120

7. Monitor shows:
   ```
   [RX] DIN1: 90 3C 64 NOTE_ON Ch:1 Note:60 Vel:100
   [TX] DIN OUT1 (transformed): 90 3E 78 Note:60→62 Vel:100→120
   ```

8. Synth plays D4 at velocity 120

**Result:** All effects applied in sequence ✓

---

## Example 6: Reset All Parameters

**Test:**
1. Reset transpose to 0:
   ```
   MIDI: B0 17 00  (CC 23)
   ```

2. Reset velocity scale to 100%:
   ```
   MIDI: B0 1A 00  (CC 26)
   ```

3. Disable force-to-scale:
   ```
   MIDI: B0 1B 00  (CC 27, value ≤ 64)
   ```

4. Disable LiveFX:
   ```
   MIDI: B0 14 00  (CC 20, value ≤ 64)
   ```

5. Play any note - it passes through unchanged

**Result:** All parameters reset ✓

---

## Example 7: Status Monitoring

The test automatically prints status every 10 seconds:

```
--- LiveFX Status ---
Enabled: YES | Transpose: +2 | Velocity: 120% | Scale: ON
---------------------
```

You can also trigger immediate status by sending any MIDI message.

---

## MIDI Message Reference

### Status Bytes (Channel 1)
- `90` = Note On, Channel 1
- `80` = Note Off, Channel 1
- `B0` = Control Change, Channel 1

### CC Numbers (Hex → Decimal)
- `14` (20) = LiveFX Enable
- `15` (21) = Transpose Down
- `16` (22) = Transpose Up
- `17` (23) = Transpose Reset
- `18` (24) = Velocity Down
- `19` (25) = Velocity Up
- `1A` (26) = Velocity Reset
- `1B` (27) = Force-to-Scale Toggle
- `1C` (28) = Scale Type
- `1D` (29) = Scale Root

### Common Values
- `00` = 0
- `40` = 64 (threshold for on/off toggles)
- `7F` = 127 (maximum)

---

## Troubleshooting Common Issues

### Issue: No [LEARN] messages
**Solution:** Make sure you're sending CC messages on Channel 1 (status byte 0xB0)

### Issue: Notes not transformed
**Solution:** Send CC 20 with value 127 to enable LiveFX

### Issue: Wrong transpose amount
**Solution:** Send CC 23 to reset, then send CC 22 the desired number of times

### Issue: Scale quantization not working
**Solution:** 
1. Verify force-to-scale is ON (CC 27, value 127)
2. Check scale type and root are set correctly
3. Remember: notes already in the scale won't change

---

## Testing with MIDI Monitoring Tools

### Windows: MIDI Monitor (MIDI-OX)
1. Configure MIDI IN/OUT ports
2. View raw MIDI bytes in hex
3. Send test messages manually

### macOS: MIDI Monitor
1. Select MIDI device
2. Enable "Display events numerically"
3. View incoming/outgoing messages

### Linux: aseqdump / amidi
```bash
# Monitor MIDI port
aseqdump -p 14:0

# Send CC 20 (enable LiveFX)
amidi -p hw:1,0 -S 'B0 14 7F'
```

### Web-based: WebMIDI
Use online MIDI monitors to visualize messages in real-time.

---

## Next Steps

- Experiment with different scale types (Minor, Dorian, etc.)
- Try negative transpose values (CC 21)
- Combine with MIDI looper for recording transformed sequences
- Create custom MIDI learn mappings for your workflow

See [MIDI_DIN_LIVEFX_TEST.md](MIDI_DIN_LIVEFX_TEST.md) for complete documentation.
