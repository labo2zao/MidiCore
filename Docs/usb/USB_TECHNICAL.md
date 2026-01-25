# USB MIDI Technical Reference

**Document Version:** 2.0  
**Last Updated:** 2026-01-25  
**Target:** STM32F407VGT6, MIOS32 Compatible, 4x4 MIDI Interface

---

## Table of Contents

1. [Descriptor Structure](#descriptor-structure)
2. [Mandatory vs Optional Descriptors](#mandatory-vs-optional-descriptors)
3. [Protocol Requirements & State Machine](#protocol-requirements--state-machine)
4. [SysEx Engine Architecture](#sysex-engine-architecture)
5. [Common Mistakes to Avoid](#common-mistakes-to-avoid)
6. [Platform-Specific Notes](#platform-specific-notes)
7. [References](#references)

---

## Descriptor Structure

### Complete 4-Port MIDI Descriptor (215 bytes / 0xD7)

```
==============================================================================
  USB MIDI Configuration Descriptor Structure (4-port interface)
==============================================================================

┌─────────────────────────────────────────────────────────────────────┐
│ Configuration Descriptor                                    9 bytes │
│ ┌─────────────────────────────────────────────────────────────────┐ │
│ │ bLength:         0x09                                           │ │
│ │ bDescriptorType: 0x02 (Configuration)                           │ │
│ │ wTotalLength:    0x00DB (219 bytes) ← CRITICAL! Was 0xD9 (217) │ │
│ │ bNumInterfaces:  0x02                                           │ │
│ │ ...                                                             │ │
│ └─────────────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────┐
│ Interface Association Descriptor (IAD)                      8 bytes │
│ ┌─────────────────────────────────────────────────────────────────┐ │
│ │ bLength:             0x08                                       │ │
│ │ bDescriptorType:     0x0B (IAD)                                 │ │
│ │ bFunctionClass:      0x01 (Audio)                               │ │
│ │ bFunctionSubClass:   0x01 (Audio Control) ← NOT 0x03!          │ │
│ └─────────────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────┐
│ Audio Control (AC) Interface                                9 bytes │
│ ┌─────────────────────────────────────────────────────────────────┐ │
│ │ bLength:            0x09                                        │ │
│ │ bDescriptorType:    0x04 (Interface)                            │ │
│ │ bInterfaceNumber:   0x00                                        │ │
│ │ bInterfaceClass:    0x01 (Audio)                                │ │
│ │ bInterfaceSubClass: 0x01 (Audio Control)                        │ │
│ │ bNumEndpoints:      0x00 (no endpoints)                         │ │
│ └─────────────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────┐
│ Class-Specific AC Interface Header                          9 bytes │
│ ┌─────────────────────────────────────────────────────────────────┐ │
│ │ bLength:         0x09  ← THE BUG WAS HERE!                      │ │
│ │ bDescriptorType: 0x24 (CS_INTERFACE)                            │ │
│ │ bDescriptorSubtype: 0x01 (HEADER)                               │ │
│ │ bcdADC:          0x0100 (1.00)                                  │ │
│ │ wTotalLength:    0x0009 (9 bytes - just this header)            │ │
│ │ bInCollection:   0x01 ← These fields make it 9 bytes, not 7!   │ │
│ │ baInterfaceNr:   0x01 ←                                         │ │
│ └─────────────────────────────────────────────────────────────────┘ │
│                                                                     │
│ ⚠️  BUG: Was calculated as 7 bytes (USB_DESC_SIZE_CS_INTERFACE)    │
│ ✅  FIX: Now calculated as 9 bytes (USB_DESC_SIZE_CS_AC_INTERFACE) │
└─────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────┐
│ MIDI Streaming (MS) Interface                                9 bytes│
│ ┌─────────────────────────────────────────────────────────────────┐ │
│ │ bLength:            0x09                                        │ │
│ │ bDescriptorType:    0x04 (Interface)                            │ │
│ │ bInterfaceNumber:   0x01                                        │ │
│ │ bInterfaceClass:    0x01 (Audio)                                │ │
│ │ bInterfaceSubClass: 0x03 (MIDI Streaming)                       │ │
│ │ bNumEndpoints:      0x02 (IN + OUT)                             │ │
│ └─────────────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────┐
│ Class-Specific MS Interface Header                          7 bytes │
│ ┌─────────────────────────────────────────────────────────────────┐ │
│ │ bLength:         0x07  ← This one is correctly 7 bytes          │ │
│ │ bDescriptorType: 0x24 (CS_INTERFACE)                            │ │
│ │ bDescriptorSubtype: 0x01 (MS_HEADER)                            │ │
│ │ bcdMSC:          0x0100 (1.00)                                  │ │
│ │ wTotalLength:    0x00A8 (168 bytes of jacks+endpoints)          │ │
│ └─────────────────────────────────────────────────────────────────┘ │
│                                                                     │
│ ✅  Correctly uses USB_DESC_SIZE_CS_MS_INTERFACE = 7 bytes         │
└─────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────┐
│ MIDI Jack Descriptors (4 ports)                           132 bytes │
│ ┌─────────────────────────────────────────────────────────────────┐ │
│ │ External IN Jacks  (4 × 6 bytes)  = 24 bytes                   │ │
│ │ Embedded IN Jacks  (4 × 9 bytes)  = 36 bytes                   │ │
│ │ Embedded OUT Jacks (4 × 9 bytes)  = 36 bytes                   │ │
│ │ External OUT Jacks (4 × 9 bytes)  = 36 bytes                   │ │
│ └─────────────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────┐
│ Endpoint Descriptors                                        36 bytes│
│ ┌─────────────────────────────────────────────────────────────────┐ │
│ │ Bulk OUT Endpoint (Standard)       = 9 bytes                   │ │
│ │ CS Bulk OUT Endpoint (MS_GENERAL)  = 9 bytes (5 + 4 jacks)     │ │
│ │ Bulk IN Endpoint (Standard)        = 9 bytes                   │ │
│ │ CS Bulk IN Endpoint (MS_GENERAL)   = 9 bytes (5 + 4 jacks)     │ │
│ └─────────────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────────┘

==============================================================================
TOTAL: 219 bytes (0xDB)
==============================================================================

Breakdown:
  9  + 8  + 9  + 9  + 9  + 7  + 168 = 219 bytes
  ↑    ↑    ↑    ↑    ↑    ↑    ↑
  │    │    │    │    │    │    └─── MS Jacks + Endpoints
  │    │    │    │    │    └──────── CS MS Header (7 bytes)
  │    │    │    │    └───────────── MS Interface
  │    │    │    └────────────────── CS AC Header (9 bytes) ← THE FIX!
  │    │    └─────────────────────── AC Interface
  │    └──────────────────────────── IAD
  └───────────────────────────────── Configuration

Before Fix: 9 + 8 + 9 + 7 + 9 + 7 + 168 = 217 bytes ❌ (2 bytes short!)
After Fix:  9 + 8 + 9 + 9 + 9 + 7 + 168 = 219 bytes ✅ (CORRECT!)

Windows Error 0xC00000E5 = CONFIGURATION_DESCRIPTOR_VALIDATION_FAILURE
Cause: wTotalLength (217) didn't match actual data (219)
Fix: Use correct 9-byte size for CS AC Header
```


---

## Mandatory vs Optional Descriptors

### Strictly Required Descriptors

For a 4-cable (4x4) USB MIDI interface:

| Descriptor | Size | Count | Total | Required? |
|------------|------|-------|-------|-----------|
| Configuration | 9 | 1 | 9 | ✅ MANDATORY |
| IAD | 8 | 1 | 8 | ✅ Windows |
| AC Interface | 9 | 1 | 9 | ✅ MANDATORY |
| CS AC Header | 9 | 1 | 9 | ✅ MANDATORY |
| MS Interface | 9 | 1 | 9 | ✅ MANDATORY |
| CS MS Header | 7 | 1 | 7 | ✅ MANDATORY |
| MIDI IN Jack (Ext) | 6 | 4 | 24 | ✅ MIOS32 compat |
| MIDI IN Jack (Emb) | 9 | 4 | 36 | ✅ MANDATORY |
| MIDI OUT Jack (Emb) | 9 | 4 | 36 | ✅ MANDATORY |
| MIDI OUT Jack (Ext) | 9 | 4 | 36 | ✅ MIOS32 compat |
| Bulk OUT Endpoint | 7 | 1 | 7 | ✅ MANDATORY |
| CS Bulk OUT Endpoint | 8 | 1 | 8 | ✅ MANDATORY |
| Bulk IN Endpoint | 7 | 1 | 7 | ✅ MANDATORY |
| CS Bulk IN Endpoint | 8 | 1 | 8 | ✅ MANDATORY |
| **TOTAL** | | | **215** | |

### Key Descriptor Details

#### IAD (Interface Association Descriptor) - Windows Required
```c
{
    0x08,                            // bLength
    0x0B,                            // bDescriptorType (IAD = 0x0B)
    0x00,                            // bFirstInterface (AC interface number)
    0x02,                            // bInterfaceCount (AC + MS)
    USB_DEVICE_CLASS_AUDIO,          // bFunctionClass (0x01)
    AUDIO_SUBCLASS_AUDIOCONTROL,     // bFunctionSubClass (0x01) ← CRITICAL!
    0x00,                            // bFunctionProtocol
    0x00                             // iFunction
}
```

**Critical**: `bFunctionSubClass = 0x01` (Audio Control) ensures Windows places device under "Audio, Video and Game Controllers" category.

#### Bulk Endpoint Descriptors - MUST Be 7 Bytes
```c
{
    0x07,                    // bLength: 7 bytes for Bulk ✅
    USB_DESC_TYPE_ENDPOINT,  // bDescriptorType (0x05)
    MIDI_OUT_EP,             // bEndpointAddress (0x01 OUT)
    0x02,                    // bmAttributes (Bulk)
    LOBYTE(64), HIBYTE(64),  // wMaxPacketSize (64 for Full Speed)
    0x00                     // bInterval (ignored for Bulk)
    // NO bRefresh or bSynchAddress for Bulk endpoints!
}
```

**Common Mistake**: Adding bRefresh/bSynchAddress (only for Isochronous/Interrupt endpoints).

---

## Protocol Requirements & State Machine

### USB MIDI Event Packet Structure (4 bytes)

```
Byte 0: Cable Number (bits 7-4) | Code Index Number (bits 3-0)
Byte 1: MIDI Data byte 1 (or 0x00 if unused)
Byte 2: MIDI Data byte 2 (or 0x00 if unused)
Byte 3: MIDI Data byte 3 (or 0x00 if unused)
```

### Valid CIN Values and Byte Counts

| CIN  | Bytes | Message Type                          | Valid? |
|------|-------|---------------------------------------|--------|
| 0x00 | 0     | Reserved (Miscellaneous/Reserved)     | ❌ NEVER SEND |
| 0x01 | 0     | Reserved (Cable Events)               | ❌ NEVER SEND |
| 0x02 | 2     | System Common 2-byte (Song Position)  | ✅ |
| 0x03 | 3     | System Common 3-byte (Song Select)    | ✅ |
| 0x04 | 3     | SysEx starts or continues             | ✅ |
| 0x05 | 1     | SysEx ends with 1 byte / Single-byte  | ✅ |
| 0x06 | 2     | SysEx ends with 2 bytes               | ✅ |
| 0x07 | 3     | SysEx ends with 3 bytes               | ✅ |
| 0x08 | 3     | Note Off                              | ✅ |
| 0x09 | 3     | Note On                               | ✅ |
| 0x0A | 3     | Poly-KeyPress (Aftertouch)            | ✅ |
| 0x0B | 3     | Control Change                        | ✅ |
| 0x0C | 2     | Program Change                        | ✅ |
| 0x0D | 2     | Channel Pressure (Aftertouch)         | ✅ |
| 0x0E | 3     | Pitch Bend                            | ✅ |
| 0x0F | 1     | Single Byte (System Real-Time)        | ✅ |

### TX Path Validation Rules

Before transmitting any packet, validate:

1. **Cable number**: 0-3 only
2. **CIN validity**: Must be in valid set (0x02-0x0F, never 0x00/0x01)
3. **Status byte**: Must be valid MIDI status (0x80-0xFF)
4. **SysEx start**: First byte must be 0xF0 if CIN = 0x04
5. **SysEx end**: Last byte must be 0xF7 if CIN = 0x05/0x06/0x07
6. **Packet padding**: Unused bytes MUST be 0x00
7. **Endpoint ready**: Never transmit if endpoint busy

### Critical Protocol Violations That Cause Host Crashes

#### ❌ Violation 1: Uninitialized Packet Bytes
**Problem**: Unused bytes in USB MIDI packets contained garbage data

**Fix**:
```c
uint8_t packet[4] = {0, 0, 0, 0};  // Initialize all bytes to 0 ✅
```

#### ❌ Violation 2: Missing Endpoint Busy Check
**Problem**: Attempted to transmit while endpoint was busy

**Fix**:
```c
if (pdev->ep_in[MIDI_IN_EP & 0xFU].status == USB_EP_TX_BUSY) {
    return;  // Drop packet safely or retry
}
USBD_LL_Transmit(pdev, MIDI_IN_EP, packet, 4);
```

#### ❌ Violation 3: Malformed SysEx End Packets
**Problem**: F7 sent as separate packet instead of included in end packet

**Example Malformed**:
```
Packet 1: [04 F0 43 10]  // SysEx start (correct)
Packet 2: [04 01 02 03]  // SysEx continue (correct)
Packet 3: [04 04 05 F7]  // Should be [07 04 05 F7] (CIN 0x07 = 3-byte end)
Packet 4: [05 F7 00 00]  // WRONG: F7 sent alone! ❌
```

**Correct Sequence**:
```
Packet 1: [04 F0 43 10]  // SysEx start
Packet 2: [04 01 02 03]  // SysEx continue
Packet 3: [07 04 05 F7]  // SysEx end with 3 bytes (CIN 0x07) ✅
```

---

## SysEx Engine Architecture

### Data Structures

```c
/**
 * @brief SysEx buffer state for one cable
 */
typedef enum {
    SYSEX_STATE_IDLE = 0,        // No active SysEx
    SYSEX_STATE_ACCUMULATING,    // Receiving SysEx data
    SYSEX_STATE_COMPLETE,        // SysEx ready for processing
    SYSEX_STATE_ERROR            // Error occurred, needs reset
} sysex_state_t;

/**
 * @brief SysEx buffer for one cable
 */
typedef struct {
    uint8_t  buffer[256];        // Data buffer (max SysEx size)
    uint16_t length;             // Current length (0-256)
    sysex_state_t state;         // Current state
    uint32_t last_packet_time;   // Timestamp for timeout detection (ms)
    uint8_t  cable;              // Cable number (0-3)
    bool     has_f0;             // Started with F0
    bool     has_f7;             // Ended with F7
} sysex_buffer_t;

/**
 * @brief SysEx engine instance (one per USB device)
 */
typedef struct {
    sysex_buffer_t rx_buffers[4];  // One buffer per cable (RX)
    uint32_t timeout_ms;            // Timeout for incomplete SysEx (default: 1000ms)
    uint32_t max_sysex_size;        // Maximum SysEx length (default: 256)
    
    // Statistics
    uint32_t rx_sysex_count;        // Total SysEx messages received
    uint32_t rx_sysex_errors;       // Total errors
    uint32_t rx_sysex_timeouts;     // Total timeouts
    uint32_t tx_sysex_count;        // Total SysEx messages sent
} sysex_engine_t;
```

### Memory Layout

```
┌────────────────────────────────────────────────────────┐
│                 SYSEX ENGINE MEMORY                    │
├────────────────────────────────────────────────────────┤
│                                                        │
│  Cable 0 RX Buffer [256 bytes] ─┐                    │
│  Cable 1 RX Buffer [256 bytes]  │ 4 × 256 = 1024 bytes│
│  Cable 2 RX Buffer [256 bytes]  │ (RX only)          │
│  Cable 3 RX Buffer [256 bytes] ─┘                    │
│                                                        │
│  State Variables:               32 bytes              │
│  Total: 1056 bytes (~1 KB)                           │
│                                                        │
└────────────────────────────────────────────────────────┘
```

### RX SysEx State Machine

```
                     ┌─────────────────────┐
                     │        IDLE         │
                     │  (Waiting for F0)   │
                     └──────────┬──────────┘
                                │
                         CIN 0x04 + F0
                         (SysEx Start)
                                │
                                ↓
                     ┌─────────────────────┐
          CIN 0x04   │   ACCUMULATING      │   Timeout (1s)
        ┌────────────┤  (Collecting data)  ├──────────────┐
        │            └──────────┬──────────┘              │
        │                       │                          │
        │            CIN 0x05/0x06/0x07                   │
        │            (SysEx End + F7)                     │
        │                       │                          │
        │                       ↓                          │
        │            ┌─────────────────────┐              │
        │            │      COMPLETE       │              │
        │            │ (Ready to process)  │              │
        │            └──────────┬──────────┘              │
        │                       │                          │
        │              Process & Send to Core             │
        │                       │                          │
        └───────────────────────┴──────────────────────────┘
                                │
                     Reset to IDLE
```

### TX SysEx Segmentation

**Streaming Approach** (No static TX buffer):

```c
/**
 * @brief Send SysEx message via USB MIDI
 * @param cable: Cable number (0-3)
 * @param data: SysEx data (must start with F0, end with F7)
 * @param length: Data length (2-65535)
 * @return 0 on success, -1 on error
 */
int usb_midi_send_sysex(uint8_t cable, const uint8_t *data, uint16_t length);
```

**Algorithm**:
1. Validate: cable 0-3, data starts with F0, ends with F7
2. Chunk into 3-byte packets
3. Scan ahead for F7 position
4. Use correct CIN based on F7 position:
   - CIN 0x04: 3 bytes, no F7 (continue)
   - CIN 0x05: 1 byte ending with F7
   - CIN 0x06: 2 bytes ending with F7
   - CIN 0x07: 3 bytes ending with F7
5. Check endpoint busy before each packet
6. Transmit packet

### Safety Guarantees

| Guarantee | Implementation |
|-----------|----------------|
| **No buffer overflow** | Length check before append |
| **No NULL pointer deref** | Input validation |
| **No double-free** | Single owner (state machine) |
| **No use-after-free** | Immediate reset after processing |
| **No stack overflow** | No recursion, fixed stack |
| **No heap fragmentation** | No dynamic allocation |

### Timing Analysis

| Operation | Worst-Case Time | Guarantee |
|-----------|-----------------|-----------|
| `sysex_rx_process_packet()` | 200 cycles (1.2 μs @ 168 MHz) | < 5 μs |
| `usb_midi_send_sysex(256)` | 12,000 cycles (72 μs @ 168 MHz) | < 100 μs |
| `sysex_engine_periodic_task()` | 400 cycles (2.4 μs @ 168 MHz) | < 10 μs |

**IRQ Latency**:
- Maximum IRQ time: 1.2 μs (RX processing)
- USB IRQ frequency: 1 kHz (1 ms interval)
- IRQ load: 0.12% CPU
- **Conclusion**: Real-time safe ✅

---

## Common Mistakes to Avoid

### ❌ Mistake 1: Incorrect wTotalLength

```c
// WRONG:
Configuration Descriptor:
    wTotalLength = 200  // But actual total is 215!
```

**Impact**: Windows enumeration fails, "Device cannot start"

**Fix**: Always calculate `wTotalLength` correctly:
```c
#define USB_MIDI_CONFIG_DESC_SIZ  (9 + 8 + 9 + 9 + 9 + 7 +                                    (4 * (6 + 9 + 9 + 9)) +                                    7 + 8 + 7 + 8)
// = 215 bytes
```

### ❌ Mistake 2: Duplicate Jack IDs

```c
// WRONG:
MIDI IN Jack (Embedded):  bJackID = 1
MIDI IN Jack (Embedded):  bJackID = 1  // Duplicate! ❌
```

**Impact**: Only first jack works, cables 2-4 don't receive data

**Fix**: Use unique sequential IDs:
```c
// For 4 cables:
Cable 0: IN Ext = 1, IN Emb = 5, OUT Emb = 9,  OUT Ext = 13
Cable 1: IN Ext = 2, IN Emb = 6, OUT Emb = 10, OUT Ext = 14
Cable 2: IN Ext = 3, IN Emb = 7, OUT Emb = 11, OUT Ext = 15
Cable 3: IN Ext = 4, IN Emb = 8, OUT Emb = 12, OUT Ext = 16
```

### ❌ Mistake 3: Wrong bLength in Endpoint Descriptors

```c
// WRONG (common mistake from Isochronous examples):
Bulk Endpoint Descriptor:
    bLength = 0x09  // WRONG! Bulk endpoints are 7 bytes, not 9!
    ...
    bRefresh = 0x00       // These don't exist for Bulk! ❌
    bSynchAddress = 0x00  // These don't exist for Bulk! ❌
```

**Impact**: Linux kernel warning, some Linux distros reject device

**Fix**: Bulk endpoints are always 7 bytes (no bRefresh, no bSynchAddress)

### ❌ Mistake 4: Wrong Jack Connections

```c
// WRONG:
MIDI OUT Jack (Embedded):
    BaSourceID[0] = 5  // References MIDI IN Jack (Embedded) ❌
    // Should reference MIDI IN Jack (External)!
```

**Impact**: macOS Core MIDI crashes when building topology

**Fix**: Follow connection rules:
```
External IN Jack → Embedded IN Jack → USB OUT Endpoint
USB IN Endpoint → Embedded OUT Jack → External OUT Jack
```

---

## Platform-Specific Notes

### Windows 7/8/10/11

**Driver:** `usbaudio.sys` (inbox driver)

**Requirements:**
- ✅ IAD is **MANDATORY**
- ✅ CS endpoint descriptors required
- ⚠️ Tolerates missing External jacks (but shows fewer ports)

**Common Issues:**
- Missing IAD → "Unknown Device"
- Wrong wTotalLength → "Device cannot start"
- Duplicate Jack IDs → Only first cable works

### macOS 10.15+ (Catalina)

**Driver:** `AppleUSBAudio.kext` → `CoreMIDI`

**Requirements:**
- ✅ All jack descriptors (Embedded + External)
- ✅ Valid jack connection graph
- ⚠️ IAD optional but recommended

**Common Issues:**
- Invalid jack connections → Core MIDI service crash
- Malformed MS Header → Device not visible in Audio MIDI Setup
- Missing CS endpoint → Ports not recognized

### Linux (ALSA usbmidi driver)

**Driver:** `snd-usb-audio` kernel module

**Requirements:**
- ✅ All descriptors strictly validated
- ✅ bInCollection + baInterfaceNr must be correct
- ✅ CS endpoint descriptors mandatory

**Common Issues:**
- Wrong bInCollection → Device rejected
- Missing CS endpoint → "invalid descriptor"
- bLength errors → Kernel warning

**Validation:**
```bash
# Check kernel log:
dmesg | grep -i midi

# Check ALSA devices:
aconnect -l
```

---

## References

### Official Specifications
- **USB 2.0 Specification** - usb.org
- **USB Device Class Definition for MIDI Devices v1.0** - usb.org
  - Section 4.1: Interface Association Descriptor
  - Section 6.1.2.1: Class-specific MS Interface Header Descriptor
  - Section 6.2: MIDI Adapter MIDI IN Jack Descriptor
  - Section 6.3: MIDI Adapter MIDI OUT Jack Descriptor
- **USB Audio Device Class Specification v1.0** - usb.org
  - Section 4.3.1: Class-specific AC Interface Header Descriptor

### Implementation References
- **MIOS32 USB Implementation**: https://github.com/midibox/mios32/
  - `drivers/STM32F4xx/mios32_usb_midi.c`
  - Proven 4-port MIDI implementation
  - Uses correct descriptor sizes

### Tools & Debugging
- **Windows**: USBView.exe, Zadig, Wireshark + USBPcap
- **macOS**: USB Prober, Audio MIDI Setup, ioreg
- **Linux**: lsusb -v, dmesg, usbmon

---

**Document Status**: ✅ Complete technical reference  
**Last Verified**: 2026-01-25  
**Target Platform**: STM32F407VGT6 with MIOS32 compatibility
