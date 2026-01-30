/**
 * @file looper_clip_library.h
 * @brief Clip Library Management for MidiCore Looper
 * 
 * Provides functionality to save, load, and manage reusable loop clips.
 * Inspired by MidiCore LoopA's clip management system.
 * 
 * Features:
 * - Save track loops as named clips to SD card
 * - Load clips from library into any track
 * - List available clips with metadata
 * - Delete and rename clips
 * - Clip categories/tags for organization
 * 
 * Storage Structure:
 *   /clips/
 *     bass_line_001.clip
 *     melody_a.clip
 *     drums_4bar.clip
 *   /clips/.index (metadata cache)
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// CONFIGURATION
// =============================================================================

#ifndef LOOPER_CLIP_LIBRARY_MAX_CLIPS
#define LOOPER_CLIP_LIBRARY_MAX_CLIPS 64  // Maximum clips in library index
#endif

#ifndef LOOPER_CLIP_NAME_MAX
#define LOOPER_CLIP_NAME_MAX 32  // Maximum clip name length
#endif

#ifndef LOOPER_CLIP_CATEGORY_MAX
#define LOOPER_CLIP_CATEGORY_MAX 16  // Maximum category name length
#endif

// =============================================================================
// TYPES
// =============================================================================

/**
 * @brief Clip metadata structure
 */
typedef struct {
  char name[LOOPER_CLIP_NAME_MAX];           // Clip name
  char category[LOOPER_CLIP_CATEGORY_MAX];   // Category (e.g., "bass", "drums")
  uint32_t size_bytes;                       // File size
  uint32_t event_count;                      // Number of MIDI events
  uint16_t loop_beats;                       // Loop length in beats
  uint8_t source_track;                      // Original track number
  uint32_t timestamp;                        // Creation timestamp
  uint8_t used;                              // Slot in use
} looper_clip_info_t;

/**
 * @brief Clip library result codes
 */
typedef enum {
  LOOPER_CLIP_OK = 0,
  LOOPER_CLIP_ERROR = -1,
  LOOPER_CLIP_NOT_FOUND = -2,
  LOOPER_CLIP_EXISTS = -3,
  LOOPER_CLIP_FULL = -4,
  LOOPER_CLIP_INVALID_NAME = -5,
  LOOPER_CLIP_SD_ERROR = -6
} looper_clip_result_t;

// =============================================================================
// API FUNCTIONS
// =============================================================================

/**
 * @brief Initialize clip library system
 * 
 * Loads the clip index from SD card if available.
 * 
 * @return 0 on success, -1 on error
 */
int looper_clip_library_init(void);

/**
 * @brief Save current track state as a clip
 * 
 * @param track Track number (0-3)
 * @param name Clip name (max 32 chars, alphanumeric + underscore)
 * @param category Optional category/tag (can be NULL)
 * @return looper_clip_result_t result code
 */
looper_clip_result_t looper_clip_save(uint8_t track, const char* name, const char* category);

/**
 * @brief Load clip from library into track
 * 
 * @param track Target track number (0-3)
 * @param name Clip name to load
 * @return looper_clip_result_t result code
 */
looper_clip_result_t looper_clip_load(uint8_t track, const char* name);

/**
 * @brief Delete clip from library
 * 
 * @param name Clip name to delete
 * @return looper_clip_result_t result code
 */
looper_clip_result_t looper_clip_delete(const char* name);

/**
 * @brief Rename a clip
 * 
 * @param old_name Current clip name
 * @param new_name New clip name
 * @return looper_clip_result_t result code
 */
looper_clip_result_t looper_clip_rename(const char* old_name, const char* new_name);

/**
 * @brief Get list of all clips in library
 * 
 * @param out_clips Output array for clip info
 * @param max_clips Maximum number of clips to return
 * @param out_count Actual number of clips returned
 * @return 0 on success, -1 on error
 */
int looper_clip_list(looper_clip_info_t* out_clips, uint32_t max_clips, uint32_t* out_count);

/**
 * @brief Get clips filtered by category
 * 
 * @param category Category name to filter (e.g., "bass")
 * @param out_clips Output array for clip info
 * @param max_clips Maximum number of clips to return
 * @param out_count Actual number of clips returned
 * @return 0 on success, -1 on error
 */
int looper_clip_list_by_category(const char* category, looper_clip_info_t* out_clips, 
                                  uint32_t max_clips, uint32_t* out_count);

/**
 * @brief Get metadata for a specific clip
 * 
 * @param name Clip name
 * @param out_info Output clip info structure
 * @return 0 on success, -1 if not found
 */
int looper_clip_get_info(const char* name, looper_clip_info_t* out_info);

/**
 * @brief Get total number of clips in library
 * 
 * @return Number of clips
 */
uint32_t looper_clip_count(void);

/**
 * @brief Refresh library index from SD card
 * 
 * Scans /clips/ directory and rebuilds index.
 * 
 * @return 0 on success, -1 on error
 */
int looper_clip_refresh(void);

#ifdef __cplusplus
}
#endif
