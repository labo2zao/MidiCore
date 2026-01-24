/**
 * @file program_change_mgr.h
 * @brief Program Change/Bank Select Manager
 * 
 * Manages program change and bank select messages with preset storage.
 * Stores complete program + bank configurations and recalls them by
 * slot number or name, sending proper CC 0, CC 32, and PC sequences.
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PROGRAM_CHANGE_MAX_SLOTS 128
#define PROGRAM_CHANGE_MAX_NAME_LEN 32

/**
 * @brief Program preset structure
 */
typedef struct {
    uint8_t program;              // Program number (0-127)
    uint8_t bank_msb;             // Bank MSB (CC 0, 0-127)
    uint8_t bank_lsb;             // Bank LSB (CC 32, 0-127)
    uint8_t channel;              // MIDI channel (0-15)
    char name[PROGRAM_CHANGE_MAX_NAME_LEN];  // Preset name
    uint8_t valid;                // 1 if slot contains valid data
} program_preset_t;

/**
 * @brief CC output callback function type
 * @param cc_number CC number
 * @param cc_value CC value (0-127)
 * @param channel MIDI channel
 */
typedef void (*program_change_cc_callback_t)(uint8_t cc_number, uint8_t cc_value, uint8_t channel);

/**
 * @brief Program Change output callback function type
 * @param program Program number (0-127)
 * @param channel MIDI channel
 */
typedef void (*program_change_pc_callback_t)(uint8_t program, uint8_t channel);

/**
 * @brief Initialize program change manager
 */
void program_change_mgr_init(void);

/**
 * @brief Set CC output callback
 * @param callback Function to call for CC output
 */
void program_change_mgr_set_cc_callback(program_change_cc_callback_t callback);

/**
 * @brief Set PC output callback
 * @param callback Function to call for program change output
 */
void program_change_mgr_set_pc_callback(program_change_pc_callback_t callback);

/**
 * @brief Store a program preset
 * @param slot Preset slot number (0-127)
 * @param program Program number (0-127)
 * @param bank_msb Bank MSB (0-127)
 * @param bank_lsb Bank LSB (0-127)
 * @param channel MIDI channel (0-15)
 * @param name Preset name (max 31 chars + null terminator)
 * @return 1 if stored successfully, 0 on error
 */
uint8_t program_change_mgr_store(uint8_t slot, uint8_t program, uint8_t bank_msb,
                                  uint8_t bank_lsb, uint8_t channel, const char* name);

/**
 * @brief Recall a program preset by slot number
 * @param slot Preset slot number (0-127)
 * @return 1 if recalled successfully, 0 if slot empty or invalid
 */
uint8_t program_change_mgr_recall(uint8_t slot);

/**
 * @brief Recall a program preset by name
 * @param name Preset name to search for
 * @return 1 if found and recalled, 0 if not found
 */
uint8_t program_change_mgr_recall_by_name(const char* name);

/**
 * @brief Send program change with bank select
 * @param program Program number (0-127)
 * @param bank_msb Bank MSB (0-127)
 * @param bank_lsb Bank LSB (0-127)
 * @param channel MIDI channel (0-15)
 */
void program_change_mgr_send(uint8_t program, uint8_t bank_msb, 
                             uint8_t bank_lsb, uint8_t channel);

/**
 * @brief Send program change only (no bank select)
 * @param program Program number (0-127)
 * @param channel MIDI channel (0-15)
 */
void program_change_mgr_send_program(uint8_t program, uint8_t channel);

/**
 * @brief Send bank select only (no program change)
 * @param bank_msb Bank MSB (0-127)
 * @param bank_lsb Bank LSB (0-127)
 * @param channel MIDI channel (0-15)
 */
void program_change_mgr_send_bank(uint8_t bank_msb, uint8_t bank_lsb, uint8_t channel);

/**
 * @brief Get preset from slot
 * @param slot Preset slot number (0-127)
 * @param preset Output: preset data
 * @return 1 if slot contains valid data, 0 if empty
 */
uint8_t program_change_mgr_get_preset(uint8_t slot, program_preset_t* preset);

/**
 * @brief Clear a preset slot
 * @param slot Preset slot number (0-127)
 */
void program_change_mgr_clear_slot(uint8_t slot);

/**
 * @brief Clear all preset slots
 */
void program_change_mgr_clear_all(void);

/**
 * @brief Check if a slot contains valid data
 * @param slot Preset slot number (0-127)
 * @return 1 if valid, 0 if empty
 */
uint8_t program_change_mgr_is_slot_valid(uint8_t slot);

/**
 * @brief Find preset by name
 * @param name Preset name to search for
 * @return Slot number if found, -1 if not found
 */
int16_t program_change_mgr_find_by_name(const char* name);

/**
 * @brief Get number of valid presets
 * @return Number of slots with valid data
 */
uint8_t program_change_mgr_get_preset_count(void);

/**
 * @brief Get list of valid preset slots
 * @param slots Output array for slot numbers (must have space for 128 entries)
 * @return Number of valid slots
 */
uint8_t program_change_mgr_get_valid_slots(uint8_t* slots);

/**
 * @brief Copy preset from one slot to another
 * @param src_slot Source slot number (0-127)
 * @param dst_slot Destination slot number (0-127)
 * @return 1 if copied successfully, 0 on error
 */
uint8_t program_change_mgr_copy_preset(uint8_t src_slot, uint8_t dst_slot);

/**
 * @brief Rename a preset
 * @param slot Preset slot number (0-127)
 * @param new_name New preset name
 * @return 1 if renamed successfully, 0 on error
 */
uint8_t program_change_mgr_rename_preset(uint8_t slot, const char* new_name);

#ifdef __cplusplus
}
#endif
