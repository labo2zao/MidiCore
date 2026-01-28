# Windows MIDI Port Quirks

## Overview

On Windows, `python-rtmidi` with loopMIDI and other virtual MIDI drivers has TWO major quirks that cause errors. Both are now handled by the emulator.

## Quirk #1: Different Port Indices for Input vs. Output

### The Problem

**The same logical MIDI port can have different numerical indices for input vs. output**.

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

---

## Quirk #2: Asymmetric Input/Output Port Lists

### The Problem

**Windows MIDI drivers can expose DIFFERENT ports in input vs. output lists!**

This caused the error:
```
✓ Found: LoopBe Internal MIDI 3
ERROR: Input port 'LoopBe Internal MIDI 3' not found!
Available input ports:
  0: LoopBe Internal MIDI 0
  1: USB MIDI Interface 1
  2: loopMIDI Port 2
```

### What Happened

Port "LoopBe Internal MIDI 3" exists as an **OUTPUT** port but NOT as an **INPUT** port!

**Real Example (LoopBe Internal MIDI):**
```
Output Ports:
  0: LoopBe Internal MIDI 0
  1: LoopBe Internal MIDI 1
  2: LoopBe Internal MIDI 2
  3: LoopBe Internal MIDI 3  ← This one exists!

Input Ports:
  0: LoopBe Internal MIDI 0
  1: LoopBe Internal MIDI 1
  2: LoopBe Internal MIDI 2
  (no port 3!)  ← But not here!
```

### Why It Happens

Windows MIDI drivers can:
- Expose different numbers of input vs. output ports
- Have asymmetric routing configurations
- Use different port capabilities for TX vs. RX
- Show ports in different orders

### The Solution

**Find a port that exists in BOTH input AND output lists!**

```python
# CORRECT (new code):
def find_port_name(pattern):
    # Get both lists
    input_ports = get_input_ports()
    output_ports = get_output_ports()
    
    # Find ports in BOTH lists
    input_names = {name for idx, name in input_ports}
    output_names = {name for idx, name in output_ports}
    common_ports = input_names & output_names
    
    # Search pattern in common ports only
    for name in common_ports:
        if pattern in name.lower():
            return name  # Guaranteed to exist in both!
```

### What User Sees

**Before (BROKEN):**
```
✓ Found: LoopBe Internal MIDI 3
ERROR: Input port 'LoopBe Internal MIDI 3' not found!
```

**After (WORKING):**
```
✓ Detected Windows - searching for loopMIDI port...
✓ Found: LoopBe Internal MIDI 0  ← Common to both lists!
✓ MidiCore Emulator started
```

### Error Handling

If NO common ports exist:
```
ERROR: No MIDI ports found that exist in BOTH input and output!

Input ports:
  0: LoopBe Internal MIDI 0
  1: USB MIDI Interface 1

Output ports:
  0: Other Device 1
  1: Other Device 2

Note: The emulator needs a port that works for both input AND output.
```

---

## Quirk #3: rtmidi Port Naming Suffixes

### The Problem

**rtmidi (the Python MIDI library) automatically appends the port index to port names!**

This caused the error:
```
ERROR: No MIDI ports found that exist in BOTH input and output!
Input: loopMIDI Port 2
Output: loopMIDI Port 5
```

Even though both are the same logical port ("loopMIDI Port"), they appear with different suffixes!

### What Happened

```
Logical port: "loopMIDI Port" (created in loopMIDI app)

After rtmidi processing:
  Input port at index 2  → "loopMIDI Port 2"
  Output port at index 5 → "loopMIDI Port 5"
```

The emulator tried to find "loopMIDI Port 2" in the output list but failed because output has "loopMIDI Port 5"!

### Real Example

```
Input ports:
  0: LoopBe Internal MIDI 0
  1: USB MIDI Interface 1
  2: loopMIDI Port 2          ← This is loopMIDI

Output ports:
  0: Microsoft GS Wavetable Synth 0
  3: LoopBe Internal MIDI 3
  4: USB MIDI Interface 4
  5: loopMIDI Port 5          ← Same loopMIDI but different suffix!
```

### Why It Happens

rtmidi (and the underlying Windows MIDI API) appends port indices to make port names unique in the system. Different input/output indices mean different suffixes.

### The Solution

**Port Name Normalization:**

```python
import re

def normalize_port_name(name):
    """Strip trailing space + digits"""
    # "loopMIDI Port 5" → "loopMIDI Port"
    return re.sub(r'\s+\d+$', '', name)

# Compare normalized names
input_base = normalize("loopMIDI Port 2")   # → "loopMIDI Port"
output_base = normalize("loopMIDI Port 5")  # → "loopMIDI Port"

if input_base == output_base:
    # Match! Same logical port despite different suffixes
```

**Implementation:**

```python
# Build map of base names to actual port names
port_map = {}

for idx, name in input_ports:
    base = normalize_port_name(name)
    port_map[base] = {
        'input': name,
        'output': port_map.get(base, {}).get('output')
    }

for idx, name in output_ports:
    base = normalize_port_name(name)
    if base in port_map:
        port_map[base]['output'] = name
    else:
        port_map[base] = {'input': None, 'output': name}

# Find ports in BOTH lists
common = {base: names for base, names in port_map.items()
          if names['input'] and names['output']}
```

### Interactive Selection Fallback

If automatic matching still fails, the emulator offers an interactive menu:

```
Try interactive selection? (y/n): y

============================================================
INTERACTIVE PORT SELECTION
============================================================

Available ports (select by number):

  1. LoopBe Internal MIDI          [✓ BOTH]
      Input:  LoopBe Internal MIDI 0
      Output: LoopBe Internal MIDI 3

  2. loopMIDI Port                  [✓ BOTH]
      Input:  loopMIDI Port 2
      Output: loopMIDI Port 5

  3. USB MIDI Interface             [✓ BOTH]
      Input:  USB MIDI Interface 1
      Output: USB MIDI Interface 4

  4. Microsoft GS Wavetable Synth   [⚠ OUTPUT ONLY]
      Output: Microsoft GS Wavetable Synth 0

Enter port number (or 'q' to quit): 2

✓ Selected: loopMIDI Port
```

### User Experience

**Before (BROKEN):**
```
ERROR: No MIDI ports found that exist in BOTH input and output!
```

**After (AUTOMATIC):**
```
✓ Detected Windows - searching for loopMIDI port...
✓ Found common port: loopMIDI Port
  Input: loopMIDI Port 2
  Output: loopMIDI Port 5
✓ MidiCore Emulator started
```

**After (INTERACTIVE if needed):**
```
Try interactive selection? (y/n): y
[Shows menu with all ports]
Enter port number: 2
✓ Selected: loopMIDI Port
```

### Why This Works

1. **Automatic**: Normalizes all port names before comparing
2. **Robust**: Finds matches despite rtmidi suffixes  
3. **Safe**: Interactive fallback if normalization fails
4. **User-friendly**: Clear status indicators (✓ BOTH, ⚠ INPUT ONLY, etc.)

---

## References

- rtmidi documentation: https://pypi.org/project/python-rtmidi/
- loopMIDI: https://www.tobias-erichsen.de/software/loopmidi.html
- LoopBe Internal MIDI: https://nerds.de/en/loopbe1.html
- MIOS Studio: http://www.ucapps.de/mios_studio.html

## Summary

### Quirk #1: Different Indices
**Problem:** Input and output MIDI ports can have different indices on Windows.

**Solution:** Use port NAME as stable identifier, look up indices separately for input/output.

### Quirk #2: Asymmetric Port Lists
**Problem:** A port might exist in output list but NOT in input list (or vice versa).

**Solution:** Find intersection of input and output port names, only use ports in BOTH lists.

### Quirk #3: rtmidi Port Naming Suffixes
**Problem:** rtmidi appends port indices to names, making same port appear different.

**Solution:** Normalize port names by stripping numeric suffixes before comparing. Interactive selection as fallback.

**Result:** Emulator works reliably on Windows with any MIDI driver, any port configuration!
