# MIOS32 USB MIDI Descriptor Analysis

## Problem: VID_0000&PID_0000

Windows reports `VID_0000&PID_0000` which means USB descriptors are not being read correctly.

## Root Cause Analysis

### Possible Issues:

1. **Descriptor Structure Too Complex**
   - Current implementation has very detailed 4-port MIDI descriptors
   - May be timing out during enumeration
   - Windows can't read descriptors fast enough

2. **Missing or Incorrect USB String Descriptors**
   - Serial number generation may be failing
   - String descriptor callback may be returning NULL

3. **MIDI Class Descriptor Size**
   - Configuration descriptor `wTotalLength` may be incorrect
   - Endpoints may not be properly aligned

## MIOS32 Approach (Proven on STM32F4)

MIOS32 uses a **simpler, more robust** approach:

### Key Differences:

1. **Simpler Initial Descriptors**
   - Device descriptor is minimal
   - Configuration descriptor is correctly sized
   - Uses standard USB enumeration sequence

2. **String Descriptor Generation**
   - Uses STM32 unique ID for serial number
   - Manufacturer and Product strings are simple ASCII
   - No complex Unicode conversion

3. **MIDI Descriptors**
   - Standard compliant USB MIDI 1.0
   - Properly sized jack descriptors
   - Correct endpoint associations

## Solution: Simplify and Fix String Descriptors

### Priority Fixes:

1. **Fix Serial Number Generation** (CRITICAL)
   - Use STM32 96-bit unique ID
   - Format as hex string: "AABBCCDDEE001122"
   
2. **Simplify String Descriptors**
   - Manufacturer: "MidiCore"
   - Product: "MidiCore 4x4"
   - Serial: From unique ID

3. **Verify Configuration Descriptor Size**
   - Calculate exact size
   - Ensure wTotalLength is correct

4. **Test with Minimal MIDI Config**
   - Start with 1 port
   - Once working, expand to 4 ports

## Implementation Plan

1. Fix `USBD_FS_SerialStrDescriptor()` - use STM32 UID
2. Verify all string descriptors return valid pointers
3. Add debug output to see which descriptor Windows is requesting
4. Test enumeration step-by-step

## References

- MIOS32: Uses STM32 UID for serial @ 0x1FFF7A10
- USB MIDI 1.0: Standard descriptor format
- STM32F4 RM0090: Unique ID register location
