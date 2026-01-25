/**
 * @file legato.h
 * @brief Legato/Mono/Priority module for monophonic note handling
 * 
 * Provides monophonic behavior with configurable note priority modes,
 * legato (smooth note transitions), and retrigger control. Perfect for
 * mono synth emulation and expressive solo instruments.
 * 
 * Features:
 * - Note priority modes (last, highest, lowest)
 * - Legato mode (smooth transitions without note-offs)
 * - Retrigger control (always retrigger vs. legato glide)
 * - Note stealing with configurable priority
 * - Per-track configuration (4 tracks)
 * - Full polyphonic note tracking
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LEGATO_MAX_TRACKS 4
#define LEGATO_MAX_NOTES 16  // Maximum notes held simultaneously per track

/**
 * @brief Note priority modes
 */
typedef enum {
    LEGATO_PRIORITY_LAST = 0,    // Last note pressed has priority
    LEGATO_PRIORITY_HIGHEST,     // Highest note has priority
    LEGATO_PRIORITY_LOWEST,      // Lowest note has priority
    LEGATO_PRIORITY_FIRST,       // First note pressed has priority
    LEGATO_PRIORITY_COUNT
} legato_priority_t;

/**
 * @brief Retrigger modes
 */
typedef enum {
    LEGATO_RETRIGGER_OFF = 0,    // Never retrigger (true legato glide)
    LEGATO_RETRIGGER_ON,         // Always retrigger envelope
    LEGATO_RETRIGGER_COUNT
} legato_retrigger_t;

/**
 * @brief Legato event types for callback
 */
typedef enum {
    LEGATO_EVENT_NOTE_ON = 0,    // New note on
    LEGATO_EVENT_NOTE_OFF,       // Note off
    LEGATO_EVENT_NOTE_CHANGE,    // Active note changed (legato transition)
    LEGATO_EVENT_RETRIGGER       // Retrigger same note
} legato_event_type_t;

/**
 * @brief Legato event structure
 */
typedef struct {
    legato_event_type_t type;    // Event type
    uint8_t note;                // MIDI note number
    uint8_t velocity;            // Note velocity
    uint8_t channel;             // MIDI channel
    uint8_t prev_note;           // Previous active note (for note change events)
    uint8_t is_legato;           // 1 if this is a legato transition (no envelope retrigger)
} legato_event_t;

/**
 * @brief Initialize legato module
 */
void legato_init(void);

/**
 * @brief Enable/disable legato mode for a track
 * @param track Track index (0-3)
 * @param enabled 1 to enable, 0 to disable
 */
void legato_set_enabled(uint8_t track, uint8_t enabled);

/**
 * @brief Check if legato mode is enabled for a track
 * @param track Track index (0-3)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t legato_is_enabled(uint8_t track);

/**
 * @brief Set note priority mode
 * @param track Track index (0-3)
 * @param priority Priority mode
 */
void legato_set_priority(uint8_t track, legato_priority_t priority);

/**
 * @brief Get note priority mode
 * @param track Track index (0-3)
 * @return Current priority mode
 */
legato_priority_t legato_get_priority(uint8_t track);

/**
 * @brief Set retrigger mode
 * @param track Track index (0-3)
 * @param retrigger Retrigger mode
 */
void legato_set_retrigger(uint8_t track, legato_retrigger_t retrigger);

/**
 * @brief Get retrigger mode
 * @param track Track index (0-3)
 * @return Current retrigger mode
 */
legato_retrigger_t legato_get_retrigger(uint8_t track);

/**
 * @brief Set portamento/glide time in milliseconds
 * @param track Track index (0-3)
 * @param time_ms Glide time in ms (0-2000, 0=instant)
 */
void legato_set_glide_time(uint8_t track, uint16_t time_ms);

/**
 * @brief Get portamento/glide time
 * @param track Track index (0-3)
 * @return Glide time in milliseconds
 */
uint16_t legato_get_glide_time(uint8_t track);

/**
 * @brief Process note on event
 * @param track Track index (0-3)
 * @param note MIDI note number
 * @param velocity Note velocity
 * @param channel MIDI channel
 * @return 1 if event should be processed, 0 if suppressed by legato logic
 */
uint8_t legato_process_note_on(uint8_t track, uint8_t note, uint8_t velocity, uint8_t channel);

/**
 * @brief Process note off event
 * @param track Track index (0-3)
 * @param note MIDI note number
 * @param channel MIDI channel
 * @return 1 if event should be processed, 0 if suppressed by legato logic
 */
uint8_t legato_process_note_off(uint8_t track, uint8_t note, uint8_t channel);

/**
 * @brief Get currently active note for a track
 * @param track Track index (0-3)
 * @return Active MIDI note number, or 0xFF if no note active
 */
uint8_t legato_get_active_note(uint8_t track);

/**
 * @brief Get currently active velocity for a track
 * @param track Track index (0-3)
 * @return Active velocity, or 0 if no note active
 */
uint8_t legato_get_active_velocity(uint8_t track);

/**
 * @brief Get number of notes currently held on a track
 * @param track Track index (0-3)
 * @return Number of held notes
 */
uint8_t legato_get_held_note_count(uint8_t track);

/**
 * @brief Clear all held notes on a track (panic/reset)
 * @param track Track index (0-3)
 */
void legato_clear_all_notes(uint8_t track);

/**
 * @brief Clear all held notes on all tracks (global panic)
 */
void legato_clear_all_tracks(void);

/**
 * @brief Get priority mode name
 * @param priority Priority mode
 * @return Priority mode name string
 */
const char* legato_get_priority_name(legato_priority_t priority);

/**
 * @brief Get retrigger mode name
 * @param retrigger Retrigger mode
 * @return Retrigger mode name string
 */
const char* legato_get_retrigger_name(legato_retrigger_t retrigger);

/**
 * @brief Callback for legato events (set by user)
 * @param track Track index
 * @param event Legato event structure
 */
typedef void (*legato_event_cb_t)(uint8_t track, const legato_event_t* event);

/**
 * @brief Set event callback
 * @param callback Callback function
 */
void legato_set_event_callback(legato_event_cb_t callback);

/**
 * @brief Enable/disable mono mode (single note output)
 * @param track Track index (0-3)
 * @param enabled 1 to enable mono, 0 for poly
 * @note When disabled, module acts as note priority tracker without mono enforcement
 */
void legato_set_mono_mode(uint8_t track, uint8_t enabled);

/**
 * @brief Check if mono mode is enabled
 * @param track Track index (0-3)
 * @return 1 if mono mode enabled, 0 if poly
 */
uint8_t legato_is_mono_mode(uint8_t track);

#ifdef __cplusplus
}
#endif
