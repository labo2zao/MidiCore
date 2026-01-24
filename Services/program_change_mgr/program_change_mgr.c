/**
 * @file program_change_mgr.c
 * @brief Program Change/Bank Select Manager implementation
 */

#include "Services/program_change_mgr/program_change_mgr.h"
#include <string.h>

#define CC_BANK_SELECT_MSB 0
#define CC_BANK_SELECT_LSB 32

static program_preset_t g_presets[PROGRAM_CHANGE_MAX_SLOTS];
static program_change_cc_callback_t g_cc_callback = NULL;
static program_change_pc_callback_t g_pc_callback = NULL;

/**
 * @brief Send CC message via callback
 */
static void send_cc(uint8_t cc_number, uint8_t cc_value, uint8_t channel) {
    if (g_cc_callback) {
        g_cc_callback(cc_number, cc_value, channel);
    }
}

/**
 * @brief Send Program Change via callback
 */
static void send_pc(uint8_t program, uint8_t channel) {
    if (g_pc_callback) {
        g_pc_callback(program, channel);
    }
}

/**
 * @brief Initialize program change manager
 */
void program_change_mgr_init(void) {
    memset(g_presets, 0, sizeof(g_presets));
    g_cc_callback = NULL;
    g_pc_callback = NULL;
}

/**
 * @brief Set CC output callback
 */
void program_change_mgr_set_cc_callback(program_change_cc_callback_t callback) {
    g_cc_callback = callback;
}

/**
 * @brief Set PC output callback
 */
void program_change_mgr_set_pc_callback(program_change_pc_callback_t callback) {
    g_pc_callback = callback;
}

/**
 * @brief Store a program preset
 */
uint8_t program_change_mgr_store(uint8_t slot, uint8_t program, uint8_t bank_msb,
                                  uint8_t bank_lsb, uint8_t channel, const char* name) {
    if (slot >= PROGRAM_CHANGE_MAX_SLOTS) return 0;
    if (program > 127) program = 127;
    if (bank_msb > 127) bank_msb = 127;
    if (bank_lsb > 127) bank_lsb = 127;
    if (channel > 15) channel = 15;
    
    program_preset_t* preset = &g_presets[slot];
    preset->program = program;
    preset->bank_msb = bank_msb;
    preset->bank_lsb = bank_lsb;
    preset->channel = channel;
    preset->valid = 1;
    
    // Copy name (ensure null termination)
    if (name) {
        strncpy(preset->name, name, PROGRAM_CHANGE_MAX_NAME_LEN - 1);
        preset->name[PROGRAM_CHANGE_MAX_NAME_LEN - 1] = '\0';
    } else {
        preset->name[0] = '\0';
    }
    
    return 1;
}

/**
 * @brief Recall a program preset by slot number
 */
uint8_t program_change_mgr_recall(uint8_t slot) {
    if (slot >= PROGRAM_CHANGE_MAX_SLOTS) return 0;
    
    program_preset_t* preset = &g_presets[slot];
    if (!preset->valid) return 0;
    
    program_change_mgr_send(preset->program, preset->bank_msb,
                           preset->bank_lsb, preset->channel);
    
    return 1;
}

/**
 * @brief Recall a program preset by name
 */
uint8_t program_change_mgr_recall_by_name(const char* name) {
    if (!name) return 0;
    
    int16_t slot = program_change_mgr_find_by_name(name);
    if (slot < 0) return 0;
    
    return program_change_mgr_recall((uint8_t)slot);
}

/**
 * @brief Send program change with bank select
 */
void program_change_mgr_send(uint8_t program, uint8_t bank_msb,
                             uint8_t bank_lsb, uint8_t channel) {
    if (channel > 15) channel = 15;
    if (program > 127) program = 127;
    if (bank_msb > 127) bank_msb = 127;
    if (bank_lsb > 127) bank_lsb = 127;
    
    // Send bank select MSB (CC 0)
    send_cc(CC_BANK_SELECT_MSB, bank_msb, channel);
    
    // Send bank select LSB (CC 32)
    send_cc(CC_BANK_SELECT_LSB, bank_lsb, channel);
    
    // Send program change
    send_pc(program, channel);
}

/**
 * @brief Send program change only
 */
void program_change_mgr_send_program(uint8_t program, uint8_t channel) {
    if (channel > 15) channel = 15;
    if (program > 127) program = 127;
    
    send_pc(program, channel);
}

/**
 * @brief Send bank select only
 */
void program_change_mgr_send_bank(uint8_t bank_msb, uint8_t bank_lsb, uint8_t channel) {
    if (channel > 15) channel = 15;
    if (bank_msb > 127) bank_msb = 127;
    if (bank_lsb > 127) bank_lsb = 127;
    
    send_cc(CC_BANK_SELECT_MSB, bank_msb, channel);
    send_cc(CC_BANK_SELECT_LSB, bank_lsb, channel);
}

/**
 * @brief Get preset from slot
 */
uint8_t program_change_mgr_get_preset(uint8_t slot, program_preset_t* preset) {
    if (slot >= PROGRAM_CHANGE_MAX_SLOTS || !preset) return 0;
    
    if (!g_presets[slot].valid) return 0;
    
    *preset = g_presets[slot];
    return 1;
}

/**
 * @brief Clear a preset slot
 */
void program_change_mgr_clear_slot(uint8_t slot) {
    if (slot >= PROGRAM_CHANGE_MAX_SLOTS) return;
    memset(&g_presets[slot], 0, sizeof(program_preset_t));
}

/**
 * @brief Clear all preset slots
 */
void program_change_mgr_clear_all(void) {
    memset(g_presets, 0, sizeof(g_presets));
}

/**
 * @brief Check if a slot contains valid data
 */
uint8_t program_change_mgr_is_slot_valid(uint8_t slot) {
    if (slot >= PROGRAM_CHANGE_MAX_SLOTS) return 0;
    return g_presets[slot].valid;
}

/**
 * @brief Find preset by name
 */
int16_t program_change_mgr_find_by_name(const char* name) {
    if (!name) return -1;
    
    for (uint16_t i = 0; i < PROGRAM_CHANGE_MAX_SLOTS; i++) {
        if (g_presets[i].valid && strcmp(g_presets[i].name, name) == 0) {
            return (int16_t)i;
        }
    }
    
    return -1;
}

/**
 * @brief Get number of valid presets
 */
uint8_t program_change_mgr_get_preset_count(void) {
    uint8_t count = 0;
    
    for (uint16_t i = 0; i < PROGRAM_CHANGE_MAX_SLOTS; i++) {
        if (g_presets[i].valid) {
            count++;
        }
    }
    
    return count;
}

/**
 * @brief Get list of valid preset slots
 */
uint8_t program_change_mgr_get_valid_slots(uint8_t* slots) {
    if (!slots) return 0;
    
    uint8_t count = 0;
    
    for (uint16_t i = 0; i < PROGRAM_CHANGE_MAX_SLOTS; i++) {
        if (g_presets[i].valid) {
            slots[count++] = (uint8_t)i;
        }
    }
    
    return count;
}

/**
 * @brief Copy preset from one slot to another
 */
uint8_t program_change_mgr_copy_preset(uint8_t src_slot, uint8_t dst_slot) {
    if (src_slot >= PROGRAM_CHANGE_MAX_SLOTS || dst_slot >= PROGRAM_CHANGE_MAX_SLOTS) {
        return 0;
    }
    
    if (!g_presets[src_slot].valid) return 0;
    
    g_presets[dst_slot] = g_presets[src_slot];
    return 1;
}

/**
 * @brief Rename a preset
 */
uint8_t program_change_mgr_rename_preset(uint8_t slot, const char* new_name) {
    if (slot >= PROGRAM_CHANGE_MAX_SLOTS || !new_name) return 0;
    
    if (!g_presets[slot].valid) return 0;
    
    strncpy(g_presets[slot].name, new_name, PROGRAM_CHANGE_MAX_NAME_LEN - 1);
    g_presets[slot].name[PROGRAM_CHANGE_MAX_NAME_LEN - 1] = '\0';
    
    return 1;
}
