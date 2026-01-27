# MIDI Monitor Service

Real-time MIDI message inspection service inspired by MIOS32 MIDI Monitor.

## Features

- **Circular Buffer**: Stores last N MIDI messages (configurable size)
- **Comprehensive Decoding**: Human-readable message type, channel, note names, etc.
- **Multi-Source Support**: Monitors DIN MIDI, USB MIDI, internal routing
- **Flexible Filtering**: By node, channel, message type
- **UART Debug Output**: Real-time message logging to debug console
- **UI Integration**: Can be used by OLED display or other UI systems
- **Statistics Tracking**: Message counts by type, dropped messages, etc.

## Usage

### Basic Initialization

```c
#include "Services/midi_monitor/midi_monitor.h"

// Initialize (call once at startup)
midi_monitor_init();

// Enable monitor
midi_monitor_set_enabled(1);

// Enable UART debug output
midi_monitor_set_uart_output(1);
```

### Capturing Messages

The monitor is automatically integrated with the router via tap hooks:

```c
// In router_hooks or midi_din service:
void router_tap_hook(uint8_t node, const router_msg_t* msg)
{
  uint32_t timestamp_ms = HAL_GetTick();
  
  if (msg->type == ROUTER_MSG_1B) {
    midi_monitor_capture_short(node, &msg->b0, 1, timestamp_ms);
  } else if (msg->type == ROUTER_MSG_2B) {
    uint8_t data[2] = {msg->b0, msg->b1};
    midi_monitor_capture_short(node, data, 2, timestamp_ms);
  } else if (msg->type == ROUTER_MSG_3B) {
    uint8_t data[3] = {msg->b0, msg->b1, msg->b2};
    midi_monitor_capture_short(node, data, 3, timestamp_ms);
  } else if (msg->type == ROUTER_MSG_SYSEX) {
    midi_monitor_capture_sysex(node, msg->data, msg->len, timestamp_ms);
  }
}
```

### Filtering Messages

```c
midi_monitor_config_t config;
midi_monitor_get_config(&config);

// Filter by specific node (e.g., DIN_IN1 only)
config.filter_node = ROUTER_NODE_DIN_IN1;

// Filter by specific channel (e.g., channel 1 only, 0-indexed)
config.filter_channel = 0;

// Filter by message type (e.g., Note On/Off only)
config.filter_msg_type = MIDI_MON_MSG_NOTE_ON;

// Disable SysEx display
config.show_sysex = 0;

// Disable realtime messages
config.show_realtime = 0;

midi_monitor_set_config(&config);
```

### Reading Events

```c
uint16_t count = midi_monitor_get_count();

for (uint16_t i = 0; i < count; i++) {
  midi_monitor_event_t event;
  if (midi_monitor_get_event(i, &event)) {
    char node_name[16];
    char msg_decode[64];
    
    midi_monitor_get_node_name(event.node, node_name, sizeof(node_name));
    midi_monitor_decode_message(event.data, event.len, msg_decode, sizeof(msg_decode));
    
    printf("[%lu] %s >> %s\r\n", event.timestamp_ms, node_name, msg_decode);
  }
}
```

### Viewing Statistics

```c
midi_monitor_stats_t stats;
midi_monitor_get_stats(&stats);

printf("Total Messages: %lu\r\n", stats.total_messages);
printf("Note On: %lu\r\n", stats.note_on_count);
printf("Note Off: %lu\r\n", stats.note_off_count);
printf("CC: %lu\r\n", stats.cc_count);
printf("SysEx: %lu\r\n", stats.sysex_count);
printf("Dropped: %lu\r\n", stats.dropped_messages);
```

## UART Output Format

When UART output is enabled, messages are printed in this format:

```
[timestamp] NODE >> Decoded Message | Raw Bytes

Examples:
[1234] DIN_IN1 >> NoteOn Ch:1 C4(60) Vel:100 | 90 3C 64
[1235] DIN_IN1 >> CC Ch:1 #7=127 | B0 07 7F
[1236] DIN_IN1 >> NoteOff Ch:1 C4(60) Vel:0 | 80 3C 00
[1237] USB_DEV0 >> SysEx (24 bytes) | F0 00 00 7E 40 ... (truncated)
```

## Configuration

### Buffer Size

```c
// In Config/module_config.h or project defines:
#define MIDI_MONITOR_BUFFER_SIZE 128  // Must be power of 2
```

### UART Output

```c
// Enable/disable UART output at compile time:
#define MIDI_MONITOR_ENABLE_UART_OUTPUT 1
```

## Integration with Tests

```c
// In MODULE_TEST_ROUTER or other tests:
void test_midi_routing(void)
{
  // Enable monitor with UART output
  midi_monitor_init();
  midi_monitor_set_enabled(1);
  midi_monitor_set_uart_output(1);
  
  // Clear any previous events
  midi_monitor_clear();
  
  // Send test MIDI message
  router_msg_t msg;
  msg.type = ROUTER_MSG_3B;
  msg.b0 = 0x90;  // Note On
  msg.b1 = 0x3C;  // Middle C
  msg.b2 = 0x64;  // Velocity 100
  
  router_process(ROUTER_NODE_DIN_IN1, &msg);
  
  // Messages will appear automatically in UART debug output
  // and can be retrieved from buffer for verification
}
```

## Performance

- **Memory**: ~2KB for 64-event buffer (16 bytes per event)
- **CPU**: Minimal overhead, only processes messages that pass filters
- **Thread Safety**: Uses volatile for head/tail pointers, safe for single producer/consumer

## Compatibility

- **MIOS32**: Inspired by MIOS32's MIDI monitor design and output format
- **STM32**: Uses HAL functions, portable across STM32 families
- **FreeRTOS**: Safe for use in RTOS environment with proper locking

## See Also

- `Services/router/router.h` - Message routing system
- `Services/router_hooks/` - Router tap hook integration
- `Services/ui/ui_page_midi_monitor.h` - UI display page
- `Config/router_config.h` - Router node definitions
