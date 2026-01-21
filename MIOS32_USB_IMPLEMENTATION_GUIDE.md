# MIOS32 USB Implementation Guide for STM32F4

## Key Findings from MIOS32 Source Code Analysis

### 1. Device Descriptor (CRITICAL)

MIOS32 uses **simple, robust device descriptor**:

```c
// From mios32_usb.c
static const u8 MIOS32_USB_DeviceDescriptor[18] = {
  0x12,                       // bLength
  DSCR_DEVICE,               // bDescriptorType
  0x00, 0x02,                // bcdUSB = 2.00 (LSB, MSB)
  0x00,                      // bDeviceClass = 0 (Composite)
  0x00,                      // bDeviceSubClass
  0x00,                      // bDeviceProtocol
  0x40,                      // bMaxPacketSize = 64
  VID_LSB, VID_MSB,          // idVendor
  PID_LSB, PID_MSB,          // idProduct
  VER_LSB, VER_MSB,          // bcdDevice
  USBD_IDX_MFC_STR,          // iManufacturer
  USBD_IDX_PRODUCT_STR,      // iProduct
  USBD_IDX_SERIAL_STR,       // iSerialNumber
  0x01                       // bNumConfigurations
};
```

**Key Points**:
- bDeviceClass = 0x00 (Composite Device) - NOT 0x01 (Audio)
- bMaxPacketSize = 0x40 (64 bytes)
- Simple, standard format

### 2. Configuration Descriptor

MIOS32 uses **conditional compilation** for interfaces:

```c
#define MIOS32_USB_MIDI_NUM_PORTS        4  // or 8, configurable

// Size calculations
#define MIOS32_USB_MIDI_SIZ_CLASS_DESC   \
  (7 + MIOS32_USB_MIDI_NUM_PORTS*(6+6+9+9) + 9 + (4+MIOS32_USB_MIDI_NUM_PORTS) + \
   9 + (4+MIOS32_USB_MIDI_NUM_PORTS))

#define MIOS32_USB_MIDI_SIZ_CONFIG_DESC  \
  (9 + MIOS32_USB_MIDI_USE_AC_INTERFACE*(9+9) + MIOS32_USB_MIDI_SIZ_CLASS_DESC)
```

**MIOS32 uses Audio Control Interface** (AC):
- Interface 0: Audio Control (AC) - empty, no endpoints
- Interface 1: MIDI Streaming (MS) - 2 endpoints (IN/OUT)

### 3. MIDI Descriptor Structure

For **4 ports** (like MidiCore), MIOS32 creates:
- 4x MIDI IN Jack (Embedded) - 6 bytes each
- 4x MIDI IN Jack (External) - 6 bytes each  
- 4x MIDI OUT Jack (Embedded) - 9 bytes each
- 4x MIDI OUT Jack (External) - 9 bytes each
- 1x Bulk OUT Endpoint - 9 bytes + (4 + 4 ports) = 13 bytes
- 1x Bulk IN Endpoint - 9 bytes + (4 + 4 ports) = 13 bytes

**Total for 4 ports**: ~235 bytes for MIDI class descriptors

### 4. String Descriptors

MIOS32 uses **STM32 UID for serial number**:

```c
// In usbd_desc.c callback
uint32_t deviceserial0 = *(uint32_t*)(UID_BASE);
uint32_t deviceserial1 = *(uint32_t*)(UID_BASE + 4);
uint32_t deviceserial2 = *(uint32_t*)(UID_BASE + 8);
```

Formatted as **hex string**: "AABBCCDDEE001122..."

### 5. USB Initialization Sequence

MIOS32 initialization order:

1. **System clock** must be ready FIRST
2. **GPIO configuration** for USB pins
3. **USB_OTG peripheral init**
4. **Descriptor registration**
5. **Start USB** with `USBD_Start()`

### 6. Key Configuration Settings

From MIOS32:

```c
// USB endpoints
#define MIDI_DATA_OUT_EP   0x01  // Endpoint 1 OUT
#define MIDI_DATA_IN_EP    0x81  // Endpoint 1 IN

// Packet sizes
#define MIDI_DATA_FS_MAX_PACKET_SIZE  64  // Full Speed

// FIFO allocation (important!)
#define RX_FIFO_SIZE       128
#define TX0_FIFO_SIZE      64
#define TX1_FIFO_SIZE      64
```

## Our Implementation vs MIOS32

### Differences Found:

1. **Device Class**
   - MIOS32: `bDeviceClass = 0x00` (Composite)
   - Ours: `bDeviceClass = 0x00` ✅ CORRECT

2. **Audio Control Interface**
   - MIOS32: Uses AC interface (empty, no endpoints)
   - Ours: Also uses AC interface ✅ CORRECT

3. **VID/PID**
   - MIOS32: Configurable (e.g., 0x16C0/0x0489 for MIOS32)
   - Ours: Currently 0x0483/0x5740 (ST test) ⚠️ TEMPORARY

4. **String Descriptors**
   - MIOS32: Uses STM32 UID
   - Ours: Already implemented ✅ CORRECT

5. **FIFO Configuration**
   - MIOS32: Explicit FIFO sizes in usbd_conf.c
   - Ours: Need to verify

## Root Cause of VID_0000&PID_0000

Based on MIOS32 analysis, the problem is likely:

1. **Timing Issue**: USB PHY not ready when descriptors are requested
2. **FIFO Configuration**: RX/TX FIFOs not properly sized
3. **Initialization Order**: Something initializing before USB is ready

## Action Plan

### Priority 1: Verify FIFO Configuration

Check in `usbd_conf.c` that:
```c
HAL_PCDEx_SetRxFiFo(&hpcd_USB_OTG_FS, 128);
HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 0, 64);
HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 1, 64);
```

### Priority 2: Check Initialization Order

Ensure in `main.c`:
```c
SystemClock_Config();  // FIRST
HAL_Init();
MX_GPIO_Init();        // Before USB
MX_USB_DEVICE_Init();  // After GPIO, clocks
```

### Priority 3: Verify Descriptor Return Values

All descriptor callbacks must return **valid pointers**, never NULL.

### Priority 4: Add Debug Output

Add UART debug to see which descriptor Windows requests and what we return.

## MIOS32 Proven Configuration

Based on years of production use on STM32F4:
- VID: 0x16C0 (Voti - community VID)
- PID: 0x0489 (MIOS32 MIDI)
- 4-8 ports work perfectly
- Same HAL/USB OTG driver we're using

## Conclusion

Our implementation is **very close** to MIOS32. The VID_0000&PID_0000 error suggests:

1. Timing/initialization order problem
2. FIFO size issue
3. Descriptor callback returning NULL

**Next step**: Systematically verify each of these potential issues.

## References

- MIOS32 source: https://github.com/midibox/mios32/tree/master/mios32/STM32F4xx
- USB MIDI 1.0 Spec
- STM32F4 RM0090 Reference Manual
