/**
 * @file looper_session.h
 * @brief Session Management for MidiCore Looper
 * 
 * Provides functionality to save and load complete looper sessions.
 * A session includes all track states, settings, and scene configurations.
 * 
 * Features:
 * - Save complete looper state as named session
 * - Load saved sessions
 * - Auto-save on changes (optional)
 * - Session templates
 * - Quick-save slots
 * 
 * Storage Structure:
 *   /sessions/
 *     live_session_001.ses
 *     studio_recording.ses
 *   /sessions/.autosave (automatic backup)
 */

#pragma once

#include <stdint.h>
#include "Services/looper/looper.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// CONFIGURATION
// =============================================================================

#ifndef LOOPER_SESSION_NAME_MAX
#define LOOPER_SESSION_NAME_MAX 32
#endif

#ifndef LOOPER_SESSION_MAX_SESSIONS
#define LOOPER_SESSION_MAX_SESSIONS 32
#endif

#ifndef LOOPER_SESSION_QUICK_SLOTS
#define LOOPER_SESSION_QUICK_SLOTS 8  // Quick-save slots
#endif

// =============================================================================
// TYPES
// =============================================================================

/**
 * @brief Session metadata
 */
typedef struct {
  char name[LOOPER_SESSION_NAME_MAX];
  uint32_t timestamp;                    // Last modified timestamp
  uint32_t size_bytes;                   // Total session size
  uint8_t num_tracks_used;               // Number of tracks with data
  uint8_t current_scene;                 // Active scene
  uint16_t bpm;                          // Tempo
  uint8_t used;                          // Slot in use
} looper_session_info_t;

/**
 * @brief Session result codes
 */
typedef enum {
  LOOPER_SESSION_OK = 0,
  LOOPER_SESSION_ERROR = -1,
  LOOPER_SESSION_NOT_FOUND = -2,
  LOOPER_SESSION_EXISTS = -3,
  LOOPER_SESSION_FULL = -4,
  LOOPER_SESSION_INVALID_NAME = -5,
  LOOPER_SESSION_SD_ERROR = -6
} looper_session_result_t;

// =============================================================================
// API FUNCTIONS
// =============================================================================

/**
 * @brief Initialize session management system
 * 
 * @return 0 on success, -1 on error
 */
int looper_session_init(void);

/**
 * @brief Save complete current state as a session
 * 
 * Saves all tracks, scenes, automation, settings to a session file.
 * 
 * @param name Session name (max 32 chars)
 * @return looper_session_result_t result code
 */
looper_session_result_t looper_session_save(const char* name);

/**
 * @brief Load a session
 * 
 * Restores complete looper state from session file.
 * 
 * @param name Session name
 * @return looper_session_result_t result code
 */
looper_session_result_t looper_session_load(const char* name);

/**
 * @brief Delete a session
 * 
 * @param name Session name
 * @return looper_session_result_t result code
 */
looper_session_result_t looper_session_delete(const char* name);

/**
 * @brief Rename a session
 * 
 * @param old_name Current name
 * @param new_name New name
 * @return looper_session_result_t result code
 */
looper_session_result_t looper_session_rename(const char* old_name, const char* new_name);

/**
 * @brief Get list of all sessions
 * 
 * @param out_sessions Output array
 * @param max_sessions Maximum to return
 * @param out_count Actual count returned
 * @return 0 on success, -1 on error
 */
int looper_session_list(looper_session_info_t* out_sessions, uint32_t max_sessions, uint32_t* out_count);

/**
 * @brief Get metadata for specific session
 * 
 * @param name Session name
 * @param out_info Output info structure
 * @return 0 on success, -1 if not found
 */
int looper_session_get_info(const char* name, looper_session_info_t* out_info);

/**
 * @brief Quick-save to numbered slot
 * 
 * Fast save operation to predefined slot (0-7).
 * 
 * @param slot Slot number (0-7)
 * @return 0 on success, -1 on error
 */
int looper_session_quick_save(uint8_t slot);

/**
 * @brief Quick-load from numbered slot
 * 
 * @param slot Slot number (0-7)
 * @return 0 on success, -1 on error
 */
int looper_session_quick_load(uint8_t slot);

/**
 * @brief Auto-save current state
 * 
 * Saves to automatic backup slot. Called periodically if auto-save enabled.
 * 
 * @return 0 on success, -1 on error
 */
int looper_session_autosave(void);

/**
 * @brief Load most recent auto-save
 * 
 * @return 0 on success, -1 on error
 */
int looper_session_load_autosave(void);

/**
 * @brief Enable/disable auto-save
 * 
 * @param enabled 1 to enable, 0 to disable
 * @param interval_seconds Interval between auto-saves (0 = on every change)
 */
void looper_session_set_autosave(uint8_t enabled, uint32_t interval_seconds);

#ifdef __cplusplus
}
#endif
