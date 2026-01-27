# USB Enumeration Failure - Analysis

**Date**: 2026-01-27  
**Issue**: Device failed enumeration with error code 43 (CM_PROB_FAILED_POST_START)  
**Error**: CONFIGURATION_DESCRIPTOR_VALIDATION_FAILURE

---

## Problem Summary

The STM32F407 USB device is failing to enumerate on Windows with the following issues:

1. **Connection Status**: DeviceFailedEnumeration
2. **Problem Code**: 43 (CM_PROB_FAILED_POST_START)
3. **Hardware IDs**: USB\CONFIGURATION_DESCRIPTOR_VALIDATION_FAILURE
4. **Root Cause**: Invalid USB configuration descriptor

---

## USB Device Information

- **Vendor ID**: 0x16C0 (Van Ooijen Technische Informatica)
- **Product ID**: 0x0489
- **USB Version**: 2.0 (Full-Speed, 12 Mbit/s)
- **Device Address**: 0x20 (32) - assigned but configuration failed

---

## Likely Causes

### 1. Invalid Configuration Descriptor Structure

The configuration descriptor likely has one or more of these issues:

- **Incorrect bLength values** - Descriptor lengths don't match actual size
- **Invalid bNumInterfaces** - Number of interfaces doesn't match actual interfaces
- **Incorrect endpoint descriptors** - Invalid endpoint addresses or types
- **Missing descriptors** - Interface or endpoint descriptors missing
- **Total length mismatch** - wTotalLength doesn't match actual descriptor chain
- **Reserved bits set** - bmAttributes or other fields have invalid values

### 2. CubeMX USB Configuration Issues

Common CubeMX USB problems:
- Multiple USB classes enabled incorrectly
- Endpoint conflicts
- Buffer size mismatches
- Descriptor template errors

---

## Investigation Steps

### 1. Check USB Descriptor Files

Files to examine:
```
USB_DEVICE/App/usbd_desc.c
USB_DEVICE/App/usbd_desc.h
USB_DEVICE/Target/usbd_conf.c
USB_DEVICE/Target/usbd_conf.h
Middlewares/ST/STM32_USB_Device_Library/Class/*/usbd_*.c
```

### 2. Verify Configuration Descriptor

The configuration descriptor should have this structure:
```c
Configuration Descriptor:
  bLength: 9
  bDescriptorType: 0x02 (CONFIGURATION)
  wTotalLength: <total bytes>
  bNumInterfaces: <count>
  bConfigurationValue: 1
  iConfiguration: 0
  bmAttributes: 0x80 or 0xC0 (bus-powered or self-powered)
  bMaxPower: <mA/2>

Interface Descriptor(s):
  bLength: 9
  bDescriptorType: 0x04 (INTERFACE)
  ...

Endpoint Descriptor(s):
  bLength: 7
  bDescriptorType: 0x05 (ENDPOINT)
  ...
```

### 3. Common Fixes

**Fix 1: Check wTotalLength**
```c
// In usbd_desc.c or USB class file
// Ensure wTotalLength matches actual descriptor size
#define USB_CONFIGURATION_DESC_SIZE  (9 + 9*NUM_INTERFACES + 7*NUM_ENDPOINTS)
```

**Fix 2: Verify Endpoint Addresses**
```c
// Endpoints must be unique and valid
#define MIDI_IN_EP   0x81  // IN endpoint 1
#define MIDI_OUT_EP  0x01  // OUT endpoint 1
#define HID_IN_EP    0x82  // IN endpoint 2 (if using HID too)
```

**Fix 3: Check bmAttributes**
```c
// Configuration descriptor bmAttributes
0x80  // Bus-powered, no remote wakeup
0xC0  // Self-powered, no remote wakeup
0xA0  // Bus-powered, remote wakeup supported
0xE0  // Self-powered, remote wakeup supported
```

---

## Files to Check in MidiCore

Based on the repository structure:

1. **USB_DEVICE/App/usbd_desc.c**
   - Configuration descriptor definition
   - String descriptors

2. **USB_DEVICE/Target/usbd_conf.c**
   - USB endpoint configuration
   - Buffer sizes

3. **Middlewares/ST/STM32_USB_Device_Library/Class/MIDI/Src/usbd_midi.c**
   - MIDI class descriptors
   - Interface descriptors

4. **Middlewares/ST/STM32_USB_Device_Library/Class/CustomHID/Src/usbd_customhid.c**
   - HID class descriptors (if using composite device)

---

## Recommended Actions

### Immediate Steps

1. **Capture USB Descriptor Data**
   - Use USB analyzer or Wireshark with USBPcap
   - Examine actual bytes sent during enumeration

2. **Check CubeMX Configuration**
   - Open `.ioc` file in CubeMX
   - Verify USB device class settings
   - Check endpoint configuration
   - Regenerate code if needed

3. **Validate Descriptor Manually**
   - Calculate expected wTotalLength
   - Verify all bLength fields
   - Check endpoint addresses don't conflict

4. **Test with Minimal Configuration**
   - Disable composite device (if using)
   - Use single USB class (MIDI only)
   - Confirm basic enumeration works

### Long-term Fix

1. Create USB descriptor validation tool
2. Add descriptor checksums
3. Implement USB descriptor self-test
4. Document correct descriptor structure

---

## USB Descriptor Debugging Commands

If CLI access is available:

```bash
# Dump USB descriptors (if implemented)
usb_dump_descriptors

# Check USB configuration
usb_status

# Force USB reset
usb_reset

# Check USB endpoints
usb_endpoints
```

---

## Windows USB Debugging

### Enable USB Tracing

1. Install Windows Driver Kit (WDK)
2. Use USB View tool (from WDK)
3. Capture with `tracelog` and `tracefmt`

### Check Event Viewer

```
Event Viewer > Windows Logs > System
Filter: Source = "USB"
```

Look for error messages about configuration descriptor validation.

---

## Related Issues

This USB issue is **separate from RAM optimization work**. The RAM optimizations don't touch USB code, so this is either:

1. A pre-existing issue
2. Related to CubeMX configuration
3. A USB cable/hardware issue
4. Windows driver issue

---

## Next Steps

1. ✅ Document the USB error (this file)
2. ⚠️ Investigate USB descriptor files (requires code review)
3. ⚠️ Fix invalid descriptor structure
4. ⚠️ Test enumeration on Windows
5. ⚠️ Verify MIDI functionality after fix

---

**Note**: This USB enumeration issue should be tracked separately from the RAM optimization PR. The RAM work is complete and successful - this is a different problem area.

---

**Status**: 📝 Documented, investigation required  
**Priority**: High (blocks USB functionality)  
**Scope**: USB device configuration, not RAM optimization
