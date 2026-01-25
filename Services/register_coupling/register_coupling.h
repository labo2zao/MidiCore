/**
 * @file register_coupling.h
 * @brief Register Coupling - Automatic voice combination management for accordion
 * 
 * Manages accordion register switches and voice combinations. Automatically
 * couples/decouples reed sets, handles register changes, and simulates
 * mechanical register switching behavior.
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REG_COUPLING_MAX_TRACKS 4
#define REG_COUPLING_MAX_REGISTERS 16

/**
 * @brief Standard accordion registers (right hand)
 */
typedef enum {
    REG_MASTER = 0,         // All reeds (L-L-M-M-H)
    REG_MUSETTE,            // L-M-H (wet musette)
    REG_BANDONEON,          // L-M (dry, dark)
    REG_ACCORDION,          // M-M (slightly wet)
    REG_VIOLIN,             // M-H (bright)
    REG_CLARINET,           // M (single reed, clear)
    REG_BASSOON,            // L (single reed, dark)
    REG_PICCOLO,            // H (single reed, bright)
    REG_ORGAN,              // M-M-H (organ-like)
    REG_HARMONIUM,          // L-M-M-H (full, slightly wet)
    REG_OBOE,               // L-H (hollow)
    REG_FLUTE,              // M-H-H (airy)
    REG_CUSTOM_1,           // User-defined
    REG_CUSTOM_2,
    REG_CUSTOM_3,
    REG_CUSTOM_4,
    REG_COUNT
} accordion_register_t;

/**
 * @brief Reed set configuration (which reeds are active)
 */
typedef struct {
    uint8_t low_1;          // Bassoon reed (L)
    uint8_t low_2;          // Second bassoon reed (L)
    uint8_t mid_1;          // Clarinet reed (M)
    uint8_t mid_2;          // Detuned clarinet (M+)
    uint8_t high;           // Piccolo reed (H)
} reed_set_config_t;

/**
 * @brief Initialize register coupling module
 */
void reg_coupling_init(void);

/**
 * @brief Set current register
 * @param track Track index (0-3)
 * @param reg Register type
 */
void reg_coupling_set_register(uint8_t track, accordion_register_t reg);

/**
 * @brief Get current register
 * @param track Track index (0-3)
 * @return Current register
 */
accordion_register_t reg_coupling_get_register(uint8_t track);

/**
 * @brief Get reed configuration for a register
 * @param track Track index (0-3)
 * @param reg Register type
 * @param config Output: reed configuration
 */
void reg_coupling_get_reed_config(uint8_t track, accordion_register_t reg, reed_set_config_t* config);

/**
 * @brief Set custom reed configuration
 * @param track Track index (0-3)
 * @param reg Custom register slot (REG_CUSTOM_1-4)
 * @param config Reed configuration
 */
void reg_coupling_set_custom_config(uint8_t track, accordion_register_t reg, const reed_set_config_t* config);

/**
 * @brief Enable/disable smooth register transitions
 * @param track Track index (0-3)
 * @param enabled 1 for smooth (crossfade), 0 for instant
 */
void reg_coupling_set_smooth_transition(uint8_t track, uint8_t enabled);

/**
 * @brief Check if smooth transitions are enabled
 * @param track Track index (0-3)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t reg_coupling_is_smooth_transition(uint8_t track);

/**
 * @brief Set transition time for register changes
 * @param track Track index (0-3)
 * @param ms Transition time in milliseconds (10-500ms)
 */
void reg_coupling_set_transition_time(uint8_t track, uint16_t ms);

/**
 * @brief Get transition time
 * @param track Track index (0-3)
 * @return Transition time in milliseconds
 */
uint16_t reg_coupling_get_transition_time(uint8_t track);

/**
 * @brief Enable/disable register memory (remembers last register per key range)
 * @param track Track index (0-3)
 * @param enabled 1 to enable, 0 to disable
 */
void reg_coupling_set_memory_enabled(uint8_t track, uint8_t enabled);

/**
 * @brief Check if register memory is enabled
 * @param track Track index (0-3)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t reg_coupling_is_memory_enabled(uint8_t track);

/**
 * @brief Process note with current register configuration
 * @param track Track index (0-3)
 * @param note MIDI note number
 * @param velocity Note velocity (0 = note off)
 * @param channel MIDI channel
 */
void reg_coupling_process_note(uint8_t track, uint8_t note, uint8_t velocity, uint8_t channel);

/**
 * @brief Cycle to next register (for foot switch control)
 * @param track Track index (0-3)
 */
void reg_coupling_next_register(uint8_t track);

/**
 * @brief Cycle to previous register
 * @param track Track index (0-3)
 */
void reg_coupling_prev_register(uint8_t track);

/**
 * @brief Get register name
 * @param reg Register type
 * @return Register name string
 */
const char* reg_coupling_get_register_name(accordion_register_t reg);

/**
 * @brief Called every 1ms for smooth transitions
 */
void reg_coupling_tick_1ms(void);

/**
 * @brief Callback for outputting notes with reed set info
 * @param track Track index
 * @param note MIDI note
 * @param velocity Velocity (0 = note off)
 * @param channel MIDI channel
 * @param reed_index Which reed (0-4 for L1, L2, M1, M2, H)
 */
typedef void (*reg_coupling_output_cb_t)(uint8_t track, uint8_t note, uint8_t velocity,
                                         uint8_t channel, uint8_t reed_index);

/**
 * @brief Set output callback
 * @param callback Callback function
 */
void reg_coupling_set_output_callback(reg_coupling_output_cb_t callback);

#ifdef __cplusplus
}
#endif
