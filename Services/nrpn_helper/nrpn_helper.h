/**
 * @file nrpn_helper.h
 * @brief NRPN/RPN Helper - simplify 14-bit NRPN/RPN message handling
 * 
 * Provides helper functions for sending and parsing NRPN (Non-Registered
 * Parameter Number) and RPN (Registered Parameter Number) messages.
 * Handles the complex multi-CC sequences required for 14-bit parameter control.
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NRPN_HELPER_MAX_PARSERS 4

/**
 * @brief NRPN/RPN type
 */
typedef enum {
    NRPN_TYPE_NRPN = 0,  // Non-Registered Parameter Number
    NRPN_TYPE_RPN,       // Registered Parameter Number
} nrpn_type_t;

/**
 * @brief Parser state
 */
typedef enum {
    NRPN_STATE_IDLE = 0,
    NRPN_STATE_MSB_RECEIVED,
    NRPN_STATE_LSB_RECEIVED,
    NRPN_STATE_DATA_MSB_RECEIVED,
    NRPN_STATE_COMPLETE
} nrpn_state_t;

/**
 * @brief NRPN message structure
 */
typedef struct {
    nrpn_type_t type;        // NRPN or RPN
    uint16_t parameter;      // 14-bit parameter number (0-16383)
    uint16_t value;          // 14-bit value (0-16383)
    uint8_t channel;         // MIDI channel (0-15)
} nrpn_message_t;

/**
 * @brief NRPN complete callback function type
 * @param parser_id Parser index
 * @param message Parsed NRPN message
 */
typedef void (*nrpn_complete_callback_t)(uint8_t parser_id, const nrpn_message_t* message);

/**
 * @brief CC output callback function type
 * @param cc_number CC number
 * @param cc_value CC value (0-127)
 * @param channel MIDI channel
 */
typedef void (*nrpn_cc_callback_t)(uint8_t cc_number, uint8_t cc_value, uint8_t channel);

/**
 * @brief Initialize NRPN helper module
 */
void nrpn_helper_init(void);

/**
 * @brief Set NRPN complete callback
 * @param callback Function to call when NRPN message is complete
 */
void nrpn_helper_set_callback(nrpn_complete_callback_t callback);

/**
 * @brief Set CC output callback (for sending)
 * @param callback Function to call for CC output
 */
void nrpn_helper_set_cc_callback(nrpn_cc_callback_t callback);

/**
 * @brief Send NRPN message
 * @param channel MIDI channel (0-15)
 * @param parameter 14-bit parameter number (0-16383)
 * @param value 14-bit value (0-16383)
 */
void nrpn_helper_send_nrpn(uint8_t channel, uint16_t parameter, uint16_t value);

/**
 * @brief Send RPN message
 * @param channel MIDI channel (0-15)
 * @param parameter 14-bit parameter number (0-16383)
 * @param value 14-bit value (0-16383)
 */
void nrpn_helper_send_rpn(uint8_t channel, uint16_t parameter, uint16_t value);

/**
 * @brief Send NRPN null (reset NRPN state)
 * @param channel MIDI channel (0-15)
 */
void nrpn_helper_send_nrpn_null(uint8_t channel);

/**
 * @brief Send RPN null (reset RPN state)
 * @param channel MIDI channel (0-15)
 */
void nrpn_helper_send_rpn_null(uint8_t channel);

/**
 * @brief Send NRPN increment
 * @param channel MIDI channel (0-15)
 * @param parameter 14-bit parameter number
 */
void nrpn_helper_send_nrpn_increment(uint8_t channel, uint16_t parameter);

/**
 * @brief Send NRPN decrement
 * @param channel MIDI channel (0-15)
 * @param parameter 14-bit parameter number
 */
void nrpn_helper_send_nrpn_decrement(uint8_t channel, uint16_t parameter);

/**
 * @brief Send RPN increment
 * @param channel MIDI channel (0-15)
 * @param parameter 14-bit parameter number
 */
void nrpn_helper_send_rpn_increment(uint8_t channel, uint16_t parameter);

/**
 * @brief Send RPN decrement
 * @param channel MIDI channel (0-15)
 * @param parameter 14-bit parameter number
 */
void nrpn_helper_send_rpn_decrement(uint8_t channel, uint16_t parameter);

/**
 * @brief Parse incoming CC message (updates state machine)
 * @param parser_id Parser index (0-3)
 * @param cc_number CC number
 * @param cc_value CC value (0-127)
 * @param channel MIDI channel
 * @return 1 if NRPN message is complete, 0 otherwise
 */
uint8_t nrpn_helper_parse_cc(uint8_t parser_id, uint8_t cc_number, 
                              uint8_t cc_value, uint8_t channel);

/**
 * @brief Get current parser state
 * @param parser_id Parser index (0-3)
 * @return Current parser state
 */
nrpn_state_t nrpn_helper_get_state(uint8_t parser_id);

/**
 * @brief Get last parsed message
 * @param parser_id Parser index (0-3)
 * @param message Output: parsed message
 * @return 1 if valid message available, 0 otherwise
 */
uint8_t nrpn_helper_get_message(uint8_t parser_id, nrpn_message_t* message);

/**
 * @brief Reset parser state
 * @param parser_id Parser index (0-3)
 */
void nrpn_helper_reset_parser(uint8_t parser_id);

/**
 * @brief Reset all parsers
 */
void nrpn_helper_reset_all(void);

/**
 * @brief Common RPN parameters (helper constants)
 */
#define RPN_PITCH_BEND_RANGE    0x0000  // Pitch bend sensitivity
#define RPN_FINE_TUNING         0x0001  // Fine tuning
#define RPN_COARSE_TUNING       0x0002  // Coarse tuning
#define RPN_TUNING_PROGRAM      0x0003  // Tuning program select
#define RPN_TUNING_BANK         0x0004  // Tuning bank select
#define RPN_NULL                0x7F7F  // RPN null value

/**
 * @brief MIDI CC numbers for NRPN/RPN
 */
#define CC_NRPN_LSB             98      // NRPN LSB
#define CC_NRPN_MSB             99      // NRPN MSB
#define CC_RPN_LSB              100     // RPN LSB
#define CC_RPN_MSB              101     // RPN MSB
#define CC_DATA_ENTRY_MSB       6       // Data Entry MSB
#define CC_DATA_ENTRY_LSB       38      // Data Entry LSB
#define CC_DATA_INCREMENT       96      // Data Increment
#define CC_DATA_DECREMENT       97      // Data Decrement

#ifdef __cplusplus
}
#endif
