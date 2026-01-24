/**
 * @file nrpn_helper.c
 * @brief NRPN/RPN Helper implementation
 */

#include "Services/nrpn_helper/nrpn_helper.h"
#include <string.h>

// Parser state structure
typedef struct {
    nrpn_state_t state;
    nrpn_type_t type;
    uint8_t param_msb;
    uint8_t param_lsb;
    uint8_t data_msb;
    uint8_t data_lsb;
    uint8_t channel;
    uint8_t message_valid;
} nrpn_parser_t;

static nrpn_parser_t g_parsers[NRPN_HELPER_MAX_PARSERS];
static nrpn_complete_callback_t g_complete_callback = NULL;
static nrpn_cc_callback_t g_cc_callback = NULL;

/**
 * @brief Send CC message via callback
 */
static void send_cc(uint8_t cc_number, uint8_t cc_value, uint8_t channel) {
    if (g_cc_callback) {
        g_cc_callback(cc_number, cc_value, channel);
    }
}

/**
 * @brief Initialize NRPN helper module
 */
void nrpn_helper_init(void) {
    memset(g_parsers, 0, sizeof(g_parsers));
    g_complete_callback = NULL;
    g_cc_callback = NULL;
}

/**
 * @brief Set NRPN complete callback
 */
void nrpn_helper_set_callback(nrpn_complete_callback_t callback) {
    g_complete_callback = callback;
}

/**
 * @brief Set CC output callback
 */
void nrpn_helper_set_cc_callback(nrpn_cc_callback_t callback) {
    g_cc_callback = callback;
}

/**
 * @brief Send NRPN message
 */
void nrpn_helper_send_nrpn(uint8_t channel, uint16_t parameter, uint16_t value) {
    if (channel > 15) channel = 15;
    
    // Send NRPN parameter MSB (CC 99)
    send_cc(CC_NRPN_MSB, (parameter >> 7) & 0x7F, channel);
    
    // Send NRPN parameter LSB (CC 98)
    send_cc(CC_NRPN_LSB, parameter & 0x7F, channel);
    
    // Send data entry MSB (CC 6)
    send_cc(CC_DATA_ENTRY_MSB, (value >> 7) & 0x7F, channel);
    
    // Send data entry LSB (CC 38)
    send_cc(CC_DATA_ENTRY_LSB, value & 0x7F, channel);
}

/**
 * @brief Send RPN message
 */
void nrpn_helper_send_rpn(uint8_t channel, uint16_t parameter, uint16_t value) {
    if (channel > 15) channel = 15;
    
    // Send RPN parameter MSB (CC 101)
    send_cc(CC_RPN_MSB, (parameter >> 7) & 0x7F, channel);
    
    // Send RPN parameter LSB (CC 100)
    send_cc(CC_RPN_LSB, parameter & 0x7F, channel);
    
    // Send data entry MSB (CC 6)
    send_cc(CC_DATA_ENTRY_MSB, (value >> 7) & 0x7F, channel);
    
    // Send data entry LSB (CC 38)
    send_cc(CC_DATA_ENTRY_LSB, value & 0x7F, channel);
}

/**
 * @brief Send NRPN null
 */
void nrpn_helper_send_nrpn_null(uint8_t channel) {
    if (channel > 15) channel = 15;
    send_cc(CC_NRPN_MSB, 0x7F, channel);
    send_cc(CC_NRPN_LSB, 0x7F, channel);
}

/**
 * @brief Send RPN null
 */
void nrpn_helper_send_rpn_null(uint8_t channel) {
    if (channel > 15) channel = 15;
    send_cc(CC_RPN_MSB, 0x7F, channel);
    send_cc(CC_RPN_LSB, 0x7F, channel);
}

/**
 * @brief Send NRPN increment
 */
void nrpn_helper_send_nrpn_increment(uint8_t channel, uint16_t parameter) {
    if (channel > 15) channel = 15;
    
    // Send NRPN parameter
    send_cc(CC_NRPN_MSB, (parameter >> 7) & 0x7F, channel);
    send_cc(CC_NRPN_LSB, parameter & 0x7F, channel);
    
    // Send increment
    send_cc(CC_DATA_INCREMENT, 0, channel);
}

/**
 * @brief Send NRPN decrement
 */
void nrpn_helper_send_nrpn_decrement(uint8_t channel, uint16_t parameter) {
    if (channel > 15) channel = 15;
    
    // Send NRPN parameter
    send_cc(CC_NRPN_MSB, (parameter >> 7) & 0x7F, channel);
    send_cc(CC_NRPN_LSB, parameter & 0x7F, channel);
    
    // Send decrement
    send_cc(CC_DATA_DECREMENT, 0, channel);
}

/**
 * @brief Send RPN increment
 */
void nrpn_helper_send_rpn_increment(uint8_t channel, uint16_t parameter) {
    if (channel > 15) channel = 15;
    
    // Send RPN parameter
    send_cc(CC_RPN_MSB, (parameter >> 7) & 0x7F, channel);
    send_cc(CC_RPN_LSB, parameter & 0x7F, channel);
    
    // Send increment
    send_cc(CC_DATA_INCREMENT, 0, channel);
}

/**
 * @brief Send RPN decrement
 */
void nrpn_helper_send_rpn_decrement(uint8_t channel, uint16_t parameter) {
    if (channel > 15) channel = 15;
    
    // Send RPN parameter
    send_cc(CC_RPN_MSB, (parameter >> 7) & 0x7F, channel);
    send_cc(CC_RPN_LSB, parameter & 0x7F, channel);
    
    // Send decrement
    send_cc(CC_DATA_DECREMENT, 0, channel);
}

/**
 * @brief Parse incoming CC message
 */
uint8_t nrpn_helper_parse_cc(uint8_t parser_id, uint8_t cc_number,
                              uint8_t cc_value, uint8_t channel) {
    if (parser_id >= NRPN_HELPER_MAX_PARSERS) return 0;
    
    nrpn_parser_t* parser = &g_parsers[parser_id];
    
    // Handle NRPN/RPN parameter selection
    if (cc_number == CC_NRPN_MSB) {
        parser->type = NRPN_TYPE_NRPN;
        parser->param_msb = cc_value;
        parser->state = NRPN_STATE_MSB_RECEIVED;
        parser->channel = channel;
        parser->message_valid = 0;
        return 0;
    }
    else if (cc_number == CC_NRPN_LSB) {
        if (parser->state == NRPN_STATE_MSB_RECEIVED && 
            parser->type == NRPN_TYPE_NRPN &&
            parser->channel == channel) {
            parser->param_lsb = cc_value;
            parser->state = NRPN_STATE_LSB_RECEIVED;
        }
        return 0;
    }
    else if (cc_number == CC_RPN_MSB) {
        parser->type = NRPN_TYPE_RPN;
        parser->param_msb = cc_value;
        parser->state = NRPN_STATE_MSB_RECEIVED;
        parser->channel = channel;
        parser->message_valid = 0;
        return 0;
    }
    else if (cc_number == CC_RPN_LSB) {
        if (parser->state == NRPN_STATE_MSB_RECEIVED && 
            parser->type == NRPN_TYPE_RPN &&
            parser->channel == channel) {
            parser->param_lsb = cc_value;
            parser->state = NRPN_STATE_LSB_RECEIVED;
        }
        return 0;
    }
    
    // Handle data entry
    else if (cc_number == CC_DATA_ENTRY_MSB) {
        if (parser->state == NRPN_STATE_LSB_RECEIVED && parser->channel == channel) {
            parser->data_msb = cc_value;
            parser->state = NRPN_STATE_DATA_MSB_RECEIVED;
        }
        return 0;
    }
    else if (cc_number == CC_DATA_ENTRY_LSB) {
        if ((parser->state == NRPN_STATE_LSB_RECEIVED || 
             parser->state == NRPN_STATE_DATA_MSB_RECEIVED) &&
            parser->channel == channel) {
            parser->data_lsb = cc_value;
            parser->state = NRPN_STATE_COMPLETE;
            parser->message_valid = 1;
            
            // Call callback
            if (g_complete_callback) {
                nrpn_message_t msg;
                msg.type = parser->type;
                msg.parameter = ((uint16_t)parser->param_msb << 7) | parser->param_lsb;
                msg.value = ((uint16_t)parser->data_msb << 7) | parser->data_lsb;
                msg.channel = parser->channel;
                g_complete_callback(parser_id, &msg);
            }
            
            return 1;  // Message complete
        }
        return 0;
    }
    
    // Handle increment/decrement
    else if (cc_number == CC_DATA_INCREMENT || cc_number == CC_DATA_DECREMENT) {
        if (parser->state == NRPN_STATE_LSB_RECEIVED && parser->channel == channel) {
            // Increment/decrement uses previously set data value
            // Note: Application must track current values to apply inc/dec properly
            // We report the last known data value and let the application handle it
            parser->state = NRPN_STATE_COMPLETE;
            parser->message_valid = 1;
            
            if (g_complete_callback) {
                nrpn_message_t msg;
                msg.type = parser->type;
                msg.parameter = ((uint16_t)parser->param_msb << 7) | parser->param_lsb;
                // Use data MSB/LSB from previous data entry, or 0 if none
                msg.value = ((uint16_t)parser->data_msb << 7) | parser->data_lsb;
                msg.channel = parser->channel;
                g_complete_callback(parser_id, &msg);
            }
            
            return 1;
        }
    }
    
    return 0;
}

/**
 * @brief Get current parser state
 */
nrpn_state_t nrpn_helper_get_state(uint8_t parser_id) {
    if (parser_id >= NRPN_HELPER_MAX_PARSERS) return NRPN_STATE_IDLE;
    return g_parsers[parser_id].state;
}

/**
 * @brief Get last parsed message
 */
uint8_t nrpn_helper_get_message(uint8_t parser_id, nrpn_message_t* message) {
    if (parser_id >= NRPN_HELPER_MAX_PARSERS || !message) return 0;
    
    nrpn_parser_t* parser = &g_parsers[parser_id];
    
    if (!parser->message_valid) return 0;
    
    message->type = parser->type;
    message->parameter = ((uint16_t)parser->param_msb << 7) | parser->param_lsb;
    message->value = ((uint16_t)parser->data_msb << 7) | parser->data_lsb;
    message->channel = parser->channel;
    
    return 1;
}

/**
 * @brief Reset parser state
 */
void nrpn_helper_reset_parser(uint8_t parser_id) {
    if (parser_id >= NRPN_HELPER_MAX_PARSERS) return;
    memset(&g_parsers[parser_id], 0, sizeof(nrpn_parser_t));
}

/**
 * @brief Reset all parsers
 */
void nrpn_helper_reset_all(void) {
    memset(g_parsers, 0, sizeof(g_parsers));
}
