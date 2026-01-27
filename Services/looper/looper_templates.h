/**
 * @file looper_templates.h
 * @brief Template System for MidiCore Looper
 * 
 * Provides predefined session templates and the ability to create custom templates.
 * Templates define initial configurations for different musical styles or workflows.
 * 
 * Features:
 * - Built-in templates (4-bar loop, 8-bar loop, etc.)
 * - Custom user templates
 * - Template categories (live, studio, practice)
 * - Quick template selection
 * 
 * Storage Structure:
 *   /templates/
 *     builtin/
 *       empty_4bar.tpl
 *       empty_8bar.tpl
 *       bass_drums_melody.tpl
 *     user/
 *       my_accordion_setup.tpl
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// CONFIGURATION
// =============================================================================

#ifndef LOOPER_TEMPLATE_NAME_MAX
#define LOOPER_TEMPLATE_NAME_MAX 32
#endif

#ifndef LOOPER_TEMPLATE_DESC_MAX
#define LOOPER_TEMPLATE_DESC_MAX 64
#endif

#ifndef LOOPER_TEMPLATE_MAX_TEMPLATES
#define LOOPER_TEMPLATE_MAX_TEMPLATES 32
#endif

// =============================================================================
// TYPES
// =============================================================================

/**
 * @brief Template category
 */
typedef enum {
  LOOPER_TEMPLATE_CAT_BUILTIN = 0,  // Built-in templates
  LOOPER_TEMPLATE_CAT_USER = 1,     // User-created templates
  LOOPER_TEMPLATE_CAT_LIVE = 2,     // Live performance templates
  LOOPER_TEMPLATE_CAT_STUDIO = 3,   // Studio recording templates
  LOOPER_TEMPLATE_CAT_PRACTICE = 4  // Practice templates
} looper_template_category_t;

/**
 * @brief Template metadata
 */
typedef struct {
  char name[LOOPER_TEMPLATE_NAME_MAX];
  char description[LOOPER_TEMPLATE_DESC_MAX];
  looper_template_category_t category;
  uint16_t default_bpm;
  uint8_t default_scene;
  uint8_t num_tracks;
  uint16_t loop_beats;
  uint8_t builtin;  // 1 if built-in, 0 if user-created
  uint8_t used;
} looper_template_info_t;

/**
 * @brief Template result codes
 */
typedef enum {
  LOOPER_TEMPLATE_OK = 0,
  LOOPER_TEMPLATE_ERROR = -1,
  LOOPER_TEMPLATE_NOT_FOUND = -2,
  LOOPER_TEMPLATE_EXISTS = -3,
  LOOPER_TEMPLATE_INVALID = -4
} looper_template_result_t;

// =============================================================================
// BUILT-IN TEMPLATES
// =============================================================================

/**
 * @brief Built-in template IDs
 */
typedef enum {
  LOOPER_TEMPLATE_EMPTY_4BAR = 0,     // Empty 4-bar loop
  LOOPER_TEMPLATE_EMPTY_8BAR = 1,     // Empty 8-bar loop
  LOOPER_TEMPLATE_EMPTY_16BAR = 2,    // Empty 16-bar loop
  LOOPER_TEMPLATE_BASS_DRUMS = 3,     // Bass + Drums setup
  LOOPER_TEMPLATE_ACCORDION_FULL = 4, // Full accordion setup (bass, chords, melody, percussion)
  LOOPER_TEMPLATE_SIMPLE_SONG = 5,    // Simple song structure (verse, chorus)
  LOOPER_TEMPLATE_LIVE_LOOPING = 6    // Live performance setup
} looper_template_builtin_id_t;

// =============================================================================
// API FUNCTIONS
// =============================================================================

/**
 * @brief Initialize template system
 * 
 * Registers built-in templates and loads user templates from SD card.
 * 
 * @return 0 on success, -1 on error
 */
int looper_template_init(void);

/**
 * @brief Apply a template to current session
 * 
 * Resets looper and applies template configuration.
 * 
 * @param name Template name
 * @return looper_template_result_t result code
 */
looper_template_result_t looper_template_apply(const char* name);

/**
 * @brief Apply built-in template by ID
 * 
 * @param template_id Built-in template ID
 * @return looper_template_result_t result code
 */
looper_template_result_t looper_template_apply_builtin(looper_template_builtin_id_t template_id);

/**
 * @brief Save current configuration as custom template
 * 
 * @param name Template name
 * @param description Optional description (can be NULL)
 * @param category Template category
 * @return looper_template_result_t result code
 */
looper_template_result_t looper_template_save(const char* name, const char* description, 
                                               looper_template_category_t category);

/**
 * @brief Delete a user template
 * 
 * Built-in templates cannot be deleted.
 * 
 * @param name Template name
 * @return looper_template_result_t result code
 */
looper_template_result_t looper_template_delete(const char* name);

/**
 * @brief Get list of all templates
 * 
 * @param out_templates Output array
 * @param max_templates Maximum to return
 * @param out_count Actual count returned
 * @return 0 on success, -1 on error
 */
int looper_template_list(looper_template_info_t* out_templates, uint32_t max_templates, uint32_t* out_count);

/**
 * @brief Get templates filtered by category
 * 
 * @param category Category to filter
 * @param out_templates Output array
 * @param max_templates Maximum to return
 * @param out_count Actual count returned
 * @return 0 on success, -1 on error
 */
int looper_template_list_by_category(looper_template_category_t category, 
                                      looper_template_info_t* out_templates,
                                      uint32_t max_templates, uint32_t* out_count);

/**
 * @brief Get metadata for specific template
 * 
 * @param name Template name
 * @param out_info Output info structure
 * @return 0 on success, -1 if not found
 */
int looper_template_get_info(const char* name, looper_template_info_t* out_info);

/**
 * @brief Reset to default empty template
 * 
 * Equivalent to applying LOOPER_TEMPLATE_EMPTY_4BAR.
 * 
 * @return 0 on success, -1 on error
 */
int looper_template_reset_default(void);

#ifdef __cplusplus
}
#endif
