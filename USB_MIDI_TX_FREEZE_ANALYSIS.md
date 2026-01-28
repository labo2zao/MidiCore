# USB MIDI TX Freeze - Deep Analysis

## Problem
MIOS Studio still freezes even after:
1. Re-enabling MIOS32 query responses
2. Fixing CIN preservation in packet transmission
3. Improving DataIn TX complete callback

User reports: "Windows may not even see the driver well"

## Investigation: USB MIDI Transmission Flow

### Current Implementation

**usb_midi_send_sysex() → usb_midi_send_packet() → USBD_LL_Transmit()**

```c
void usb_midi_send_sysex(const uint8_t *data, uint16_t length, uint8_t cable) {
  // Loops through data, calling usb_midi_send_packet() for each 4-byte chunk
  while (length > 0) {
    usb_midi_send_packet(cin, b0, b1, b2);  // Multiple calls in quick succession!
    length -= bytes_sent;
  }
}

void usb_midi_send_packet(uint8_t cin, uint8_t b0, uint8_t b1, uint8_t b2) {
  // Check if busy
  if (hUsbDeviceFS.ep_in[MIDI_IN_EP & 0x0F].status == USBD_BUSY) {
    return;  // Drop packet!
  }
  
  // Transmit
  USBD_LL_Transmit(&hUsbDeviceFS, MIDI_IN_EP, packet, 4);
}
```

### The Problem

**MIOS32 Query Response:** `F0 00 00 7E 32 00 0F "MIOS32" F7` (17 bytes)

Requires 5 USB-MIDI packets:
1. `[04 F0 00 00]` - Sent immediately
2. `[04 7E 32 00]` - Called immediately after #1
3. `[04 0F 4D 49]` - Called immediately after #2
4. `[04 4F 53 33]` - Called immediately after #3
5. `[06 32 F7 00]` - Called immediately after #4

**Timeline:**
- T=0: Packet 1 sent, endpoint BUSY
- T=1μs: Packet 2 attempted, endpoint BUSY → **DROPPED**
- T=2μs: Packet 3 attempted, endpoint BUSY → **DROPPED**
- T=3μs: Packet 4 attempted, endpoint BUSY → **DROPPED**
- T=4μs: Packet 5 attempted, endpoint BUSY → **DROPPED**
- T=1000μs: TX complete interrupt, endpoint OK (but too late!)

**Result:** MIOS Studio receives only `F0 00 00` → incomplete SysEx → **FREEZE**

### Root Cause

**We're calling USBD_LL_Transmit() in a tight loop without waiting for TX complete!**

The USB hardware can only transmit ONE packet at a time. Each 4-byte packet takes ~1ms to transmit at Full Speed USB (12 Mbit/s). But we're trying to send all 5 packets in microseconds!

## USB Full Speed Timing

- Bandwidth: 12 Mbit/s
- Max packet size: 64 bytes
- Actual MIDI packet: 4 bytes
- Transmission time per packet: ~33μs minimum
- Polling interval: 1ms (for bulk endpoints)
- **Effective throughput: 1 packet per 1ms**

## Solutions

### Option 1: Buffered TX with Queue (Best)
Implement a TX queue that buffers packets and sends them as TX completes:

```c
static uint8_t tx_queue[MAX_PACKETS][4];
static uint8_t tx_queue_head = 0;
static uint8_t tx_queue_tail = 0;

void usb_midi_send_packet(uint8_t cin, uint8_t b0, uint8_t b1, uint8_t b2) {
  // Add to queue
  enqueue_packet(cin, b0, b1, b2);
  
  // Try to send if endpoint not busy
  send_next_from_queue();
}

void USBD_MIDI_DataIn() {
  // TX complete - send next packet from queue
  send_next_from_queue();
}
```

### Option 2: Blocking with Timeout (Simpler, but risky)
Wait for TX complete before sending next packet:

```c
void usb_midi_send_packet(uint8_t cin, uint8_t b0, uint8_t b1, uint8_t b2) {
  uint32_t timeout = 1000; // 1ms
  while (hUsbDeviceFS.ep_in[MIDI_IN_EP & 0x0F].status == USBD_BUSY && timeout--) {
    // Wait (but this blocks!)
  }
  
  if (timeout > 0) {
    USBD_LL_Transmit(&hUsbDeviceFS, MIDI_IN_EP, packet, 4);
  }
}
```

**Problem:** Blocking in RTOS is bad! Could cause watchdog resets.

### Option 3: Rate Limiting with Delay (Hack)
Add small delay between packets:

```c
void usb_midi_send_sysex(...) {
  while (length > 0) {
    usb_midi_send_packet(...);
    osDelay(1); // Wait 1ms between packets
    length -= 3;
  }
}
```

**Problem:** Wastes time, not elegant, still might drop packets.

## Recommended Solution

**Implement Option 1: TX Queue**

1. Create circular buffer for TX packets (16-32 packets)
2. `usb_midi_send_packet()` adds to queue
3. If endpoint idle, start transmission
4. `USBD_MIDI_DataIn()` sends next packet from queue
5. Non-blocking, efficient, no packet loss

## Additional Debugging Needed

To confirm this is the issue, need to:
1. Add debug counters for packets sent vs dropped
2. Measure actual time between `usb_midi_send_packet()` calls
3. Verify TX complete interrupt is firing
4. Check Windows Device Manager for USB errors

## Next Steps

1. Implement TX queue in usb_midi.c
2. Test with MIOS Studio
3. Verify all 5 response packets are received
4. Confirm no freeze

---

**This explains why "Windows may not even see the driver well"** - if the device descriptor request itself gets corrupted due to dropped packets, Windows can't enumerate the device properly!
