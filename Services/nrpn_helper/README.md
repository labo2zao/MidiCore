# NRPN Helper Module

NRPN/RPN Helper for MidiCore MIDI FX suite.

## Overview

The nrpn_helper module simplifies handling of NRPN (Non-Registered Parameter Number) and RPN (Registered Parameter Number) messages. These are used for 14-bit parameter control in MIDI, requiring complex multi-CC sequences. This module handles both sending and parsing these sequences.

## Features

- **Send 14-bit NRPN/RPN messages** with single function call
- **Parse incoming NRPN/RPN** CC sequences with state machine
- **Increment/Decrement functions** for parameter adjustment
- **Null message sending** to reset NRPN/RPN state
- **State machine** for multi-CC parsing (4 independent parsers)
- **Helper constants** for common RPN parameters

## NRPN/RPN Message Format

NRPN and RPN messages consist of multiple CC messages:

### NRPN:
1. CC 99 (NRPN MSB) - Parameter number MSB
2. CC 98 (NRPN LSB) - Parameter number LSB
3. CC 6 (Data Entry MSB) - Value MSB
4. CC 38 (Data Entry LSB) - Value LSB

### RPN:
1. CC 101 (RPN MSB) - Parameter number MSB
2. CC 100 (RPN LSB) - Parameter number LSB
3. CC 6 (Data Entry MSB) - Value MSB
4. CC 38 (Data Entry LSB) - Value LSB

## Usage Example

### Sending NRPN Messages

```c
#include "Services/nrpn_helper/nrpn_helper.h"

// CC output callback
void my_cc_callback(uint8_t cc_number, uint8_t cc_value, uint8_t channel) {
    MIDI_SendCC(channel, cc_number, cc_value);
}

// Initialize
nrpn_helper_init();
nrpn_helper_set_cc_callback(my_cc_callback);

// Send NRPN: parameter 500, value 8000, channel 0
nrpn_helper_send_nrpn(0, 500, 8000);

// Send RPN: Pitch Bend Range to ±2 semitones
nrpn_helper_send_rpn(0, RPN_PITCH_BEND_RANGE, 2 << 7);

// Send NRPN null (reset state)
nrpn_helper_send_nrpn_null(0);

// Increment/Decrement
nrpn_helper_send_nrpn_increment(0, 500);
nrpn_helper_send_nrpn_decrement(0, 500);
```

### Parsing NRPN Messages

```c
// NRPN complete callback
void my_nrpn_callback(uint8_t parser_id, const nrpn_message_t* message) {
    printf("NRPN received: param=%d, value=%d, channel=%d\n",
           message->parameter, message->value, message->channel);
}

// Initialize
nrpn_helper_init();
nrpn_helper_set_callback(my_nrpn_callback);

// Process incoming CC messages
// Parser 0 for channel 0
void on_midi_cc(uint8_t channel, uint8_t cc_number, uint8_t cc_value) {
    if (channel == 0) {
        uint8_t complete = nrpn_helper_parse_cc(0, cc_number, cc_value, channel);
        if (complete) {
            // NRPN message complete, callback was called
            nrpn_message_t msg;
            if (nrpn_helper_get_message(0, &msg)) {
                // Use the message
            }
        }
    }
}
```

## API Reference

### Initialization

- `nrpn_helper_init()` - Initialize module
- `nrpn_helper_set_callback()` - Set NRPN complete callback
- `nrpn_helper_set_cc_callback()` - Set CC output callback

### Sending

- `nrpn_helper_send_nrpn()` - Send NRPN message
- `nrpn_helper_send_rpn()` - Send RPN message
- `nrpn_helper_send_nrpn_null()` - Send NRPN null
- `nrpn_helper_send_rpn_null()` - Send RPN null
- `nrpn_helper_send_nrpn_increment()` - Increment NRPN
- `nrpn_helper_send_nrpn_decrement()` - Decrement NRPN
- `nrpn_helper_send_rpn_increment()` - Increment RPN
- `nrpn_helper_send_rpn_decrement()` - Decrement RPN

### Parsing

- `nrpn_helper_parse_cc()` - Parse incoming CC (returns 1 when complete)
- `nrpn_helper_get_state()` - Get current parser state
- `nrpn_helper_get_message()` - Get last parsed message
- `nrpn_helper_reset_parser()` - Reset parser state
- `nrpn_helper_reset_all()` - Reset all parsers

## Common RPN Parameters

The module provides constants for standard RPN parameters:

- `RPN_PITCH_BEND_RANGE` (0x0000) - Pitch bend sensitivity
- `RPN_FINE_TUNING` (0x0001) - Fine tuning
- `RPN_COARSE_TUNING` (0x0002) - Coarse tuning
- `RPN_TUNING_PROGRAM` (0x0003) - Tuning program select
- `RPN_TUNING_BANK` (0x0004) - Tuning bank select
- `RPN_NULL` (0x7F7F) - RPN null value

## CC Numbers

Standard CC numbers for NRPN/RPN:

- `CC_NRPN_LSB` (98) - NRPN LSB
- `CC_NRPN_MSB` (99) - NRPN MSB
- `CC_RPN_LSB` (100) - RPN LSB
- `CC_RPN_MSB` (101) - RPN MSB
- `CC_DATA_ENTRY_MSB` (6) - Data Entry MSB
- `CC_DATA_ENTRY_LSB` (38) - Data Entry LSB
- `CC_DATA_INCREMENT` (96) - Data Increment
- `CC_DATA_DECREMENT` (97) - Data Decrement

## Parser State Machine

The parser tracks incoming CC messages through these states:

1. **IDLE** - Waiting for parameter MSB
2. **MSB_RECEIVED** - Parameter MSB received
3. **LSB_RECEIVED** - Parameter LSB received
4. **DATA_MSB_RECEIVED** - Data MSB received
5. **COMPLETE** - Full message received

## Integration

1. Include header in your project
2. Call `nrpn_helper_init()` at startup
3. Set CC callback for sending
4. Set complete callback for parsing
5. Use send functions to output NRPN/RPN
6. Feed incoming CC messages to parser

## Common Applications

- **Synth parameter control**: Deep editing with 14-bit resolution
- **Effects parameters**: High-resolution modulation
- **Pitch bend range**: Set via RPN
- **Custom NRPN mappings**: Device-specific parameters
- **Multi-device control**: Use separate parsers per device

## Notes

- 4 independent parsers allow simultaneous parsing on multiple channels
- Null messages (0x7F7F) are used to reset NRPN/RPN state
- Some devices only use MSB (7-bit), LSB can be 0
- Parser state is per-parser, not global
- Callback is called when message is complete (after data LSB)

## License

Part of MidiCore project
