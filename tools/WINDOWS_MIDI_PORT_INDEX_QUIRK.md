# Windows MIDI Port Index Quirk

## The Problem

On Windows, when using `python-rtmidi` with loopMIDI or other virtual MIDI drivers, **the same logical MIDI port can have different numerical indices for input vs. output**.

This caused the error:
```
✓ Detected Windows - searching for loopMIDI port...
ERROR: Port index 9 out of range!
```

## Technical Details

### What Happened (Before Fix)

```python
# Old code (BROKEN on Windows)
midi_out = rtmidi.MidiOut()
ports = []
for i in range(midi_out.get_port_count()):
    ports.append((i, midi_out.get_port_name(i)))
# Returns: [(9, "loopMIDI Port")]

# Later...
port_idx = 9  # From above enumeration
midi_in = rtmidi.MidiIn()
midi_in.open_port(port_idx)  # CRASH! Index 9 doesn't exist for input!
```

### Why It Happens

On Windows, `rtmidi` enumerates MIDI devices separately for input and output. The same device appears in both lists, but potentially at **different indices**.

**Example:**
```
Input Ports (MidiIn):
  0: Microsoft GS Wavetable Synth
  1: loopMIDI Port
  
Output Ports (MidiOut):
  0: Microsoft GS Wavetable Synth
  1: Some Other Device
  ...
  9: loopMIDI Port
```

So "loopMIDI Port" is at:
- **Input index: 1**
- **Output index: 9**

### The Solution

**Use port NAME as the stable identifier, then look up indices separately:**

```python
# New code (WORKS on all platforms)

# Step 1: Find the port NAME
def find_port_name(pattern):
    midi_out = rtmidi.MidiOut()
    for i in range(midi_out.get_port_count()):
        name = midi_out.get_port_name(i)
        if pattern.lower() in name.lower():
            return name  # Returns "loopMIDI Port"
    return None

# Step 2: Find INPUT index for that name
def find_input_index(port_name):
    midi_in = rtmidi.MidiIn()
    for i in range(midi_in.get_port_count()):
        if midi_in.get_port_name(i) == port_name:
            return i  # Returns 1
    return None

# Step 3: Find OUTPUT index for that name
def find_output_index(port_name):
    midi_out = rtmidi.MidiOut()
    for i in range(midi_out.get_port_count()):
        if midi_out.get_port_name(i) == port_name:
            return i  # Returns 9
    return None

# Step 4: Open ports with correct indices
port_name = find_port_name("loop")      # "loopMIDI Port"
input_idx = find_input_index(port_name)  # 1
output_idx = find_output_index(port_name) # 9

midi_in = rtmidi.MidiIn()
midi_in.open_port(input_idx)   # ✅ Opens index 1

midi_out = rtmidi.MidiOut()
midi_out.open_port(output_idx)  # ✅ Opens index 9
```

## Platform Differences

### Windows (loopMIDI)
- ❌ Virtual port creation not supported
- ✅ Must use existing ports (loopMIDI)
- ⚠️ **Input/output indices can differ**
- ✅ Port names are consistent

### macOS (IAC Driver)
- ✅ Virtual port creation supported
- ✅ Can use existing ports
- ✅ Input/output indices usually match
- ✅ Port names are consistent

### Linux (virmidi)
- ✅ Virtual port creation supported
- ✅ Can use existing ports
- ✅ Input/output indices usually match
- ✅ Port names are consistent

## How the Emulator Handles It

The MidiCore emulator now:

1. **Finds port by NAME** (not index)
2. **Looks up INPUT index** separately
3. **Looks up OUTPUT index** separately
4. **Opens both** with correct indices
5. **Shows both indices** in output for debugging

Example output:
```
✓ Detected Windows - searching for loopMIDI port...
✓ Found: loopMIDI Port
✓ MidiCore Emulator started
✓ Using existing MIDI port: 'loopMIDI Port'
  - Input port index: 1
  - Output port index: 9

In MIOS Studio:
  1. Device 'loopMIDI Port' should appear in device list
  2. Select it and click 'Query'
  3. Open Terminal window (View → Terminal)
  4. You should see test messages!
```

## Debugging Tips

If you get "Port index X out of range":

1. **List all ports:**
   ```bash
   python midicore_emulator.py --list
   ```
   
   This shows:
   ```
   Output Ports:
     0: Microsoft GS Wavetable Synth
     9: loopMIDI Port
   
   Input Ports:
     0: Microsoft GS Wavetable Synth
     1: loopMIDI Port
   ```

2. **Check if loopMIDI is running**
   - Open loopMIDI application
   - Verify port is created
   - Keep loopMIDI running

3. **Use explicit port name:**
   ```bash
   python midicore_emulator.py --use-existing "loopMIDI Port"
   ```

## For Developers

When working with `python-rtmidi` on Windows:

### ❌ DON'T DO THIS:
```python
# Get index from MidiOut
ports = midi_out.list_ports()
idx = find_index(ports, "My Port")

# Use same index with MidiIn
midi_in.open_port(idx)  # May crash!
```

### ✅ DO THIS:
```python
# Get port NAME
ports = midi_out.list_ports()
name = find_name(ports, "My Port")

# Find index specifically for MidiIn
idx_in = find_input_index(name)
midi_in.open_port(idx_in)

# Find index specifically for MidiOut
idx_out = find_output_index(name)
midi_out.open_port(idx_out)
```

## References

- rtmidi documentation: https://pypi.org/project/python-rtmidi/
- loopMIDI: https://www.tobias-erichsen.de/software/loopmidi.html
- MIOS Studio: http://www.ucapps.de/mios_studio.html

## Summary

**Problem:** Input and output MIDI ports can have different indices on Windows.

**Solution:** Use port NAME as stable identifier, look up indices separately for input/output.

**Result:** Emulator works reliably on Windows with loopMIDI!
