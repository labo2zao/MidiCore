#pragma once
#include <stdint.h>
#include "Services/router/router.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LOOPER_TRACKS 4

typedef enum {
  LOOPER_STATE_STOP = 0,
  LOOPER_STATE_REC,
  LOOPER_STATE_PLAY,
  LOOPER_STATE_OVERDUB
} looper_state_t;

typedef enum {
  LOOPER_QUANT_OFF = 0,
  LOOPER_QUANT_1_16,
  LOOPER_QUANT_1_8,
  LOOPER_QUANT_1_4
} looper_quant_t;

typedef struct {
  uint16_t bpm;          // 20..300
  uint8_t  ts_num;       // default 4
  uint8_t  ts_den;       // default 4
  uint8_t  auto_loop;    // 1: stop REC at loop_len if known
  uint8_t  reserved;
} looper_transport_t;

void looper_init(void);

void looper_set_transport(const looper_transport_t* t);
void looper_get_transport(looper_transport_t* out);

void looper_set_tempo(uint16_t bpm);
uint16_t looper_get_tempo(void);

void looper_set_state(uint8_t track, looper_state_t st);
looper_state_t looper_get_state(uint8_t track);

void looper_clear(uint8_t track);

void looper_set_loop_beats(uint8_t track, uint16_t beats);
uint16_t looper_get_loop_beats(uint8_t track);

void looper_set_quant(uint8_t track, looper_quant_t q);
looper_quant_t looper_get_quant(uint8_t track);

void looper_set_mute(uint8_t track, uint8_t mute);
uint8_t looper_get_mute(uint8_t track);

void looper_tick_1ms(void);
void looper_on_router_msg(uint8_t in_node, const router_msg_t* msg);

int looper_save_track(uint8_t track, const char* filename);
int looper_load_track(uint8_t track, const char* filename);

// ---- UI/Debug helpers (read/edit) ----
typedef struct {
  uint32_t idx;     // stable index in internal event array (until resort/clear/load)
  uint32_t tick;    // tick position
  uint8_t  len;     // 2 or 3
  uint8_t  b0, b1, b2;
} looper_event_view_t;

uint32_t looper_get_loop_len_ticks(uint8_t track);

/** Copy events snapshot into out[]. Returns number copied. */
uint32_t looper_export_events(uint8_t track, looper_event_view_t* out, uint32_t max);

/** Edit an event (tick + bytes). Returns 0 on success. */
int looper_edit_event(uint8_t track, uint32_t idx, uint32_t new_tick,
                      uint8_t len, uint8_t b0, uint8_t b1, uint8_t b2);

// ---- Song Mode / Scene Management ----
#define LOOPER_SCENES 8

typedef struct {
  uint8_t has_clip;       // 1 if this scene has a clip recorded
  uint16_t loop_beats;    // Length in beats
} looper_scene_clip_t;

/**
 * @brief Get clip info for a specific scene and track
 * @param scene Scene index (0-7)
 * @param track Track index (0-3)
 * @return Clip info (has_clip=1 if recorded)
 */
looper_scene_clip_t looper_get_scene_clip(uint8_t scene, uint8_t track);

/**
 * @brief Set current scene
 * @param scene Scene index (0-7)
 */
void looper_set_current_scene(uint8_t scene);

/**
 * @brief Get current scene
 * @return Current scene index
 */
uint8_t looper_get_current_scene(void);

/**
 * @brief Copy current track state to a scene slot
 * @param scene Scene index (0-7)
 * @param track Track index (0-3)
 */
void looper_save_to_scene(uint8_t scene, uint8_t track);

/**
 * @brief Load a scene's track state to current
 * @param scene Scene index (0-7)
 * @param track Track index (0-3)
 */
void looper_load_from_scene(uint8_t scene, uint8_t track);

/**
 * @brief Trigger scene playback (load all tracks from scene)
 * @param scene Scene index (0-7)
 */
void looper_trigger_scene(uint8_t scene);

// ---- Step Playback ----

/**
 * @brief Enable/disable step playback mode for a track
 * @param track Track index (0-3)
 * @param enable 1 to enable step mode, 0 to disable (normal playback)
 * 
 * In step playback mode, the track cursor position is manually controlled
 * rather than automatically advancing with tempo. Use looper_step_forward()
 * and looper_step_backward() to navigate.
 */
void looper_set_step_mode(uint8_t track, uint8_t enable);

/**
 * @brief Get step playback mode status
 * @param track Track index (0-3)
 * @return 1 if step mode enabled, 0 otherwise
 */
uint8_t looper_get_step_mode(uint8_t track);

/**
 * @brief Step forward to next event (or by specified ticks)
 * @param track Track index (0-3)
 * @param ticks Number of ticks to advance (0 = next event)
 * @return Current position in ticks after step
 * 
 * In step mode, advances the cursor and triggers any events at the new position.
 * If ticks is 0, steps to the next event. Otherwise steps by specified ticks.
 */
uint32_t looper_step_forward(uint8_t track, uint32_t ticks);

/**
 * @brief Step backward to previous event (or by specified ticks)
 * @param track Track index (0-3)
 * @param ticks Number of ticks to rewind (0 = previous event)
 * @return Current position in ticks after step
 * 
 * In step mode, moves the cursor backward. Notes that were on will be
 * turned off, and the cursor position updated.
 */
uint32_t looper_step_backward(uint8_t track, uint32_t ticks);

/**
 * @brief Get current cursor position in step mode
 * @param track Track index (0-3)
 * @return Current position in ticks
 */
uint32_t looper_get_cursor_position(uint8_t track);

/**
 * @brief Set cursor position in step mode
 * @param track Track index (0-3)
 * @param tick Tick position to set
 */
void looper_set_cursor_position(uint8_t track, uint32_t tick);

/**
 * @brief Configure step size for footswitch control
 * @param ticks Step size in ticks (0 = auto/event-based, or fixed tick count)
 * 
 * Sets the default step size. Common values:
 * - 0: Auto (step to next/previous event)
 * - 24: 16th note at PPQN=96
 * - 48: 8th note at PPQN=96
 * - 96: Quarter note at PPQN=96
 */
void looper_set_step_size(uint32_t ticks);

/**
 * @brief Get configured step size
 * @return Step size in ticks (0 = auto)
 */
uint32_t looper_get_step_size(void);

// ---- Scene Chaining/Automation ----

/**
 * @brief Set scene chaining configuration
 * @param scene Scene index (0-7)
 * @param next_scene Next scene to trigger (0-7), or 0xFF to disable
 * @param enabled 1 to enable auto-chain, 0 to disable
 * 
 * When enabled, the specified next scene will be automatically triggered
 * when the current scene's loop ends.
 */
void looper_set_scene_chain(uint8_t scene, uint8_t next_scene, uint8_t enabled);

/**
 * @brief Get next scene in chain
 * @param scene Scene index (0-7)
 * @return Next scene index (0-7), or 0xFF if no chain configured
 */
uint8_t looper_get_scene_chain(uint8_t scene);

/**
 * @brief Check if scene has chaining enabled
 * @param scene Scene index (0-7)
 * @return 1 if chaining enabled, 0 otherwise
 */
uint8_t looper_is_scene_chain_enabled(uint8_t scene);

// ---- MIDI File Export ----

/**
 * @brief Export all tracks to Standard MIDI File (SMF Format 1)
 * @param filename Output filename (e.g., "loop.mid")
 * @return 0 on success, negative on error
 * 
 * Exports all non-empty tracks to a multi-track MIDI file with tempo,
 * time signature, and track names. Compatible with all DAWs.
 */
int looper_export_midi(const char* filename);

/**
 * @brief Export single track to MIDI file
 * @param track Track index (0-3)
 * @param filename Output filename
 * @return 0 on success, negative on error
 */
int looper_export_track_midi(uint8_t track, const char* filename);

/**
 * @brief Export a scene to MIDI file
 * @param scene Scene index (0-7)
 * @param filename Output filename (e.g., "scene_A.mid")
 * @return 0 on success, negative on error
 * 
 * Exports all tracks from the specified scene to a multi-track MIDI file.
 */
int looper_export_scene_midi(uint8_t scene, const char* filename);

#ifdef __cplusplus
}
#endif
