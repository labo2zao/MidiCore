#pragma once
#include <stdint.h>
#include "Services/router/router.h"

#ifdef __cplusplus
extern "C" {
#endif

// Number of looper tracks (configurable for memory optimization)
// Each track consumes ~16KB (g_tr + undo_stacks + g_automation)
// Options: 2 tracks (saves 32KB) or 4 tracks (default)
#ifndef LOOPER_TRACKS
  #define LOOPER_TRACKS 4  // Default: 4 tracks for full polyphony
#endif

// Undo/Redo configuration
// Memory usage per undo_stack_t: varies by depth
// 
// CCMRAM Optimization Strategy (restored original PR #54 approach):
// - Test mode: depth=2, clipboards in CCMRAM (user is in test mode)
// - Production mode: depth=5, clipboards disabled
// - Moved g_automation to RAM to free 8KB CCMRAM space
// - Moved clipboards to RAM to free 20KB CCMRAM space (when enabled)
//
// Memory allocation:
//   Test mode (MODULE_TEST_LOOPER):
//     CCMRAM: g_tr (25KB) + undo (33KB depth=2) = 58KB / 64KB ✅
//     RAM: g_automation (8KB) + clipboards (20KB) + pianoroll (53KB) + other (20KB) = 101KB / 128KB ✅
//   
//   Production mode:
//     CCMRAM: g_tr (25KB) + undo (33KB depth=1 OR move to RAM for depth=5) = 58KB / 64KB ✅
//     RAM: g_automation (8KB) + undo (99KB if depth=5) + other (20KB) = 127KB / 128KB ✅
//
#ifndef LOOPER_UNDO_STACK_DEPTH
#ifdef MODULE_TEST_LOOPER
  // Test mode: Depth 1 to fit with test clipboards (20KB) + pianoroll UI (57KB)
  #define LOOPER_UNDO_STACK_DEPTH 1
#else
  // Production mode: Depth 1 to fit with pianoroll UI (57KB, required for main page)
  // Pianoroll UI is NOT test-only - it's the main production page
  #define LOOPER_UNDO_STACK_DEPTH 1
#endif
#endif

// Clipboard feature configuration (only available in test mode)
// Allows independent control of track vs scene clipboard features
// - LOOPER_ENABLE_TRACK_CLIPBOARD: Track copy/paste (~4KB)
// - LOOPER_ENABLE_SCENE_CLIPBOARD: Scene copy/paste (~16KB)
// If MODULE_TEST_LOOPER is not defined, clipboards are always disabled
#ifdef MODULE_TEST_LOOPER
  #ifndef LOOPER_ENABLE_TRACK_CLIPBOARD
    #define LOOPER_ENABLE_TRACK_CLIPBOARD 1  // Default: enabled in test mode
  #endif
  #ifndef LOOPER_ENABLE_SCENE_CLIPBOARD
    #define LOOPER_ENABLE_SCENE_CLIPBOARD 1  // Default: enabled in test mode
  #endif
#else
  // Production mode: clipboards always disabled
  #define LOOPER_ENABLE_TRACK_CLIPBOARD 0
  #define LOOPER_ENABLE_SCENE_CLIPBOARD 0
#endif

// Quick-Save Compression (optional, reduces storage by ~40-60%)
// Enable to compress session files (adds ~10-100ms to save/load times)
// Requires ZLIB library support in build system
// #define LOOPER_QUICKSAVE_COMPRESS

/**
 * @note Return Value Conventions:
 * - Functions returning 'int': 0 = success, -1 = error
 * - Functions returning 'uint8_t': Boolean or status (0/1) or value
 * - Functions returning specific types: Valid value or 0 on error
 * 
 * @note Boundary Validation:
 * All public APIs validate input parameters:
 * - Track indices must be 0-3 (LOOPER_TRACKS)
 * - Scene indices must be 0-7 (LOOPER_SCENES)
 * - Invalid parameters return default/error values
 */

/**
 * @brief Looper states for each track
 * 
 * - LOOPER_STATE_STOP: Track stopped, no playback
 * - LOOPER_STATE_REC: Initial recording of notes and CC
 * - LOOPER_STATE_PLAY: Playback only
 * - LOOPER_STATE_OVERDUB: Add more notes/CC to existing loop
 * - LOOPER_STATE_OVERDUB_CC_ONLY: Re-record CC automation only while notes keep looping
 *   (enables re-recording CC automation layer without affecting MIDI notes)
 * - LOOPER_STATE_OVERDUB_NOTES_ONLY: Re-record MIDI notes only while CC automation keeps looping
 *   (enables re-recording melody without affecting CC automation)
 */

typedef enum {
  LOOPER_STATE_STOP = 0,
  LOOPER_STATE_REC,
  LOOPER_STATE_PLAY,
  LOOPER_STATE_OVERDUB,
  LOOPER_STATE_OVERDUB_CC_ONLY,    // Re-record only CC automation while notes keep looping
  LOOPER_STATE_OVERDUB_NOTES_ONLY  // Re-record only notes while CC automation keeps looping
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

// Track Mute/Solo Controls
void looper_set_track_muted(uint8_t track, uint8_t muted);
uint8_t looper_is_track_muted(uint8_t track);
void looper_set_track_solo(uint8_t track, uint8_t solo);
uint8_t looper_is_track_soloed(uint8_t track);
void looper_clear_all_solo(void);
uint8_t looper_is_track_audible(uint8_t track);

void looper_tick_1ms(void);
void looper_on_router_msg(uint8_t in_node, const router_msg_t* msg);

/**
 * @brief Save track to file
 * @param track Track index (0-3)
 * @param filename File path for save
 * @return 0 on success, -1 on error (invalid track, file error)
 */
int looper_save_track(uint8_t track, const char* filename);

/**
 * @brief Load track from file
 * @param track Track index (0-3)
 * @param filename File path to load
 * @return 0 on success, -1 on error (invalid track, file error, corrupt data)
 */
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

/** 
 * @brief Edit an event (tick + bytes)
 * @param track Track index (0-3)
 * @param idx Event index from looper_export_events()
 * @param new_tick New tick position
 * @param len Event length (2 or 3 bytes)
 * @param b0 First MIDI byte
 * @param b1 Second MIDI byte
 * @param b2 Third MIDI byte
 * @return 0 on success, -1 on error (invalid track, invalid index)
 */
int looper_edit_event(uint8_t track, uint32_t idx, uint32_t new_tick,
                      uint8_t len, uint8_t b0, uint8_t b1, uint8_t b2);

/**
 * @brief Add a new event to track
 * @param track Track index (0-3)
 * @param tick Tick position for the new event
 * @param len Event length (2 or 3 bytes)
 * @param b0 First MIDI byte
 * @param b1 Second MIDI byte
 * @param b2 Third MIDI byte
 * @return 0 on success, negative on error (invalid track, invalid len, buffer full)
 */
int looper_add_event(uint8_t track, uint32_t tick, uint8_t len, uint8_t b0, uint8_t b1, uint8_t b2);

// ---- Song Mode / Scene Management ----
// Number of scene slots (configurable for memory optimization)
// Each scene uses minimal memory (~32 bytes per track)
// Options: 4 scenes (minimal) or 6 scenes (default) or 8 scenes (full)
#ifndef LOOPER_SCENES
  #define LOOPER_SCENES 6  // Default: 6 scenes, adequate for live performance
#endif

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

// ---- Randomizer Feature ----

/**
 * @brief Apply randomization to a track
 * @param track Track index (0-3)
 * @param velocity_range Velocity randomization range (0-64, ±range from original)
 * @param timing_range Timing randomization range in ticks (0-12, ±range from original)
 * @param note_skip_prob Probability of skipping notes (0-100, percentage)
 * 
 * Adds controlled variation to a track:
 * - Velocity: Randomly adjusts note velocities within specified range
 * - Timing: Adds micro-timing shifts (shuffle/swing feel)
 * - Note Skip: Randomly skips some notes for sparse variations
 * 
 * All values are clamped to safe ranges. Recommend pushing undo before applying.
 */
void looper_randomize_track(uint8_t track, uint8_t velocity_range, 
                            uint8_t timing_range, uint8_t note_skip_prob);

/**
 * @brief Set randomization parameters for a track
 * @param track Track index (0-3)
 * @param velocity_range Velocity randomization (0-64)
 * @param timing_range Timing randomization in ticks (0-12)
 * @param note_skip_prob Note skip probability (0-100%)
 */
void looper_set_randomize_params(uint8_t track, uint8_t velocity_range,
                                 uint8_t timing_range, uint8_t note_skip_prob);

/**
 * @brief Get randomization parameters for a track
 * @param track Track index (0-3)
 * @param out_velocity_range Velocity randomization output
 * @param out_timing_range Timing randomization output
 * @param out_note_skip_prob Note skip probability output
 */
void looper_get_randomize_params(uint8_t track, uint8_t* out_velocity_range,
                                 uint8_t* out_timing_range, uint8_t* out_note_skip_prob);

// ---- Humanizer Feature ----

/**
 * @brief Apply humanization to a track
 * @param track Track index (0-3)
 * @param velocity_amount Velocity humanization amount (0-32, subtle variations)
 * @param timing_amount Timing humanization in ticks (0-6, micro-timing shifts)
 * @param intensity Overall humanization intensity (0-100%)
 * 
 * Adds musical, subtle variations for natural-feeling loops:
 * - Velocity: Smooth velocity curves following natural performance
 * - Timing: Groove-aware micro-timing shifts (on-beat less affected)
 * - Intensity: Controls overall humanization strength
 * 
 * Differs from Randomizer: Musical and groove-preserving vs. chaotic.
 * Recommend pushing undo before applying.
 */
void looper_humanize_track(uint8_t track, uint8_t velocity_amount,
                           uint8_t timing_amount, uint8_t intensity);

/**
 * @brief Set humanization parameters for a track
 * @param track Track index (0-3)
 * @param velocity_amount Velocity humanization (0-32)
 * @param timing_amount Timing humanization in ticks (0-6)
 * @param intensity Humanization intensity (0-100%)
 */
void looper_set_humanize_params(uint8_t track, uint8_t velocity_amount,
                                uint8_t timing_amount, uint8_t intensity);

/**
 * @brief Get humanization parameters for a track
 * @param track Track index (0-3)
 * @param out_velocity_amount Velocity humanization output
 * @param out_timing_amount Timing humanization output
 * @param out_intensity Humanization intensity output
 */
void looper_get_humanize_params(uint8_t track, uint8_t* out_velocity_amount,
                                uint8_t* out_timing_amount, uint8_t* out_intensity);

// ---- LFO (Low Frequency Oscillator) Feature ----

// Forward declare LFO types from lfo.h
typedef enum {
    LOOPER_LFO_WAVEFORM_SINE = 0,
    LOOPER_LFO_WAVEFORM_TRIANGLE,
    LOOPER_LFO_WAVEFORM_SAW,
    LOOPER_LFO_WAVEFORM_SQUARE,
    LOOPER_LFO_WAVEFORM_RANDOM,
    LOOPER_LFO_WAVEFORM_SAMPLE_HOLD
} looper_lfo_waveform_t;

typedef enum {
    LOOPER_LFO_TARGET_VELOCITY = 0,
    LOOPER_LFO_TARGET_TIMING,
    LOOPER_LFO_TARGET_PITCH
} looper_lfo_target_t;

/**
 * @brief Enable/disable LFO for a track
 * @param track Track index (0-3)
 * @param enabled 1 to enable, 0 to disable
 * 
 * LFOs provide cyclic modulation for creating evolving, dreamlike effects.
 * Can be synced to BPM or run freely for ambient modulation.
 */
void looper_set_lfo_enabled(uint8_t track, uint8_t enabled);

/**
 * @brief Check if LFO is enabled for a track
 * @param track Track index (0-3)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t looper_is_lfo_enabled(uint8_t track);

/**
 * @brief Set LFO waveform
 * @param track Track index (0-3)
 * @param waveform Waveform type (sine, triangle, saw, square, random, S&H)
 * 
 * Different waveforms create different modulation characteristics:
 * - SINE: Smooth, organic modulation
 * - TRIANGLE: Linear ramp modulation
 * - SAW: Rising modulation
 * - SQUARE: Stepped on/off modulation
 * - RANDOM: Smooth random modulation (interpolated)
 * - SAMPLE_HOLD: Stepped random modulation
 */
void looper_set_lfo_waveform(uint8_t track, looper_lfo_waveform_t waveform);

/**
 * @brief Get current LFO waveform
 * @param track Track index (0-3)
 * @return Current waveform type
 */
looper_lfo_waveform_t looper_get_lfo_waveform(uint8_t track);

/**
 * @brief Set LFO rate in Hz (0.01 - 10.0 Hz)
 * @param track Track index (0-3)
 * @param rate_hundredths Rate in 0.01Hz units (1 = 0.01Hz, 1000 = 10Hz)
 * 
 * Very slow rates (0.01-0.1 Hz) create "dream" effects.
 * Faster rates (1-10 Hz) create rhythmic modulation.
 */
void looper_set_lfo_rate(uint8_t track, uint16_t rate_hundredths);

/**
 * @brief Get current LFO rate
 * @param track Track index (0-3)
 * @return Rate in 0.01Hz units
 */
uint16_t looper_get_lfo_rate(uint8_t track);

/**
 * @brief Set LFO modulation depth (0-100%)
 * @param track Track index (0-3)
 * @param depth Modulation depth percentage (0-100)
 */
void looper_set_lfo_depth(uint8_t track, uint8_t depth);

/**
 * @brief Get current LFO depth
 * @param track Track index (0-3)
 * @return Depth percentage (0-100)
 */
uint8_t looper_get_lfo_depth(uint8_t track);

/**
 * @brief Set LFO target parameter
 * @param track Track index (0-3)
 * @param target Parameter to modulate (velocity, timing, or pitch)
 */
void looper_set_lfo_target(uint8_t track, looper_lfo_target_t target);

/**
 * @brief Get current LFO target
 * @param track Track index (0-3)
 * @return Target parameter
 */
looper_lfo_target_t looper_get_lfo_target(uint8_t track);

/**
 * @brief Enable/disable BPM sync for LFO
 * @param track Track index (0-3)
 * @param bpm_sync 1 for BPM sync, 0 for free-running
 * 
 * When BPM synced, LFO cycles are locked to musical time (bars/beats).
 * When free-running, LFO uses the rate_hz setting independently.
 */
void looper_set_lfo_bpm_sync(uint8_t track, uint8_t bpm_sync);

/**
 * @brief Check if LFO is BPM synced
 * @param track Track index (0-3)
 * @return 1 if BPM synced, 0 if free-running
 */
uint8_t looper_is_lfo_bpm_synced(uint8_t track);

/**
 * @brief Set BPM sync divisor (1, 2, 4, 8, 16, 32 bars)
 * @param track Track index (0-3)
 * @param divisor Number of bars for one LFO cycle
 */
void looper_set_lfo_bpm_divisor(uint8_t track, uint8_t divisor);

/**
 * @brief Get current BPM divisor
 * @param track Track index (0-3)
 * @return Bars divisor
 */
uint8_t looper_get_lfo_bpm_divisor(uint8_t track);

/**
 * @brief Reset LFO phase to zero
 * @param track Track index (0-3)
 * 
 * Useful for synchronizing LFO start with loop playback.
 */
void looper_reset_lfo_phase(uint8_t track);

// ---- Humanizer Feature ----

/**
 * @brief Enable/disable humanizer for a track
 * @param track Track index (0-3)
 * @param enabled 1 to enable, 0 to disable
 * 
 * The humanizer adds groove-aware micro-variations to velocity and timing,
 * creating more natural, less mechanical performances.
 */
void looper_set_humanizer_enabled(uint8_t track, uint8_t enabled);

/**
 * @brief Check if humanizer is enabled for a track
 * @param track Track index (0-3)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t looper_is_humanizer_enabled(uint8_t track);

/**
 * @brief Set velocity humanization amount
 * @param track Track index (0-3)
 * @param amount Velocity variation amount (0-32)
 * 
 * Controls how much velocity variation is applied. Higher values create
 * more dynamic variation. On-beat notes get 20% variation, off-beat notes
 * get 100% for groove preservation.
 */
void looper_set_humanizer_velocity(uint8_t track, uint8_t amount);

/**
 * @brief Get velocity humanization amount
 * @param track Track index (0-3)
 * @return Velocity variation amount (0-32)
 */
uint8_t looper_get_humanizer_velocity(uint8_t track);

/**
 * @brief Set timing humanization amount
 * @param track Track index (0-3)
 * @param amount Timing variation in ticks (0-6)
 * 
 * Controls how much timing variation is applied. Adds subtle micro-shifts
 * to note timing for a more human feel.
 */
void looper_set_humanizer_timing(uint8_t track, uint8_t amount);

/**
 * @brief Get timing humanization amount
 * @param track Track index (0-3)
 * @return Timing variation in ticks (0-6)
 */
uint8_t looper_get_humanizer_timing(uint8_t track);

/**
 * @brief Set humanizer intensity
 * @param track Track index (0-3)
 * @param intensity Overall effect intensity (0-100%)
 * 
 * Master control for humanizer strength. 0% disables humanization,
 * 100% applies full variation amounts.
 */
void looper_set_humanizer_intensity(uint8_t track, uint8_t intensity);

/**
 * @brief Get humanizer intensity
 * @param track Track index (0-3)
 * @return Overall effect intensity (0-100%)
 */
uint8_t looper_get_humanizer_intensity(uint8_t track);

// ---- Tempo Tap Feature ----

/**
 * @brief Register a tempo tap event
 * 
 * Call this function when the user taps a button/pad in rhythm.
 * After 2+ taps, the system calculates and applies the average BPM.
 * Auto-resets after 2 seconds of no taps.
 */
void looper_tempo_tap(void);

/**
 * @brief Get current tap count
 * @return Number of taps registered (0-8)
 * 
 * Useful for UI feedback to show tap progress.
 * Returns 0 if sequence has timed out.
 */
uint8_t looper_tempo_get_tap_count(void);

/**
 * @brief Reset tempo tap sequence
 * 
 * Clears all tap data. Call this to cancel a tap sequence.
 */
void looper_tempo_tap_reset(void);

// ---- Undo/Redo System ----

/**
 * @brief Save current track state to undo history
 * @param track Track index (0-3)
 * 
 * Call before operations that modify track data (record, overdub, clear, etc.)
 * to enable undo functionality.
 */
void looper_undo_push(uint8_t track);

/**
 * @brief Undo last operation on track
 * @param track Track index (0-3)
 * @return 0 on success, -1 if no undo available
 * 
 * Restores the previous track state from undo history.
 */
int looper_undo(uint8_t track);

/**
 * @brief Redo previously undone operation
 * @param track Track index (0-3)
 * @return 0 on success, -1 if no redo available
 * 
 * Restores the next track state from redo history.
 */
int looper_redo(uint8_t track);

/**
 * @brief Clear undo/redo history for track
 * @param track Track index (0-3)
 * 
 * Frees all undo/redo states for the specified track.
 */
void looper_undo_clear(uint8_t track);

/**
 * @brief Check if undo is available
 * @param track Track index (0-3)
 * @return 1 if undo available, 0 otherwise
 */
uint8_t looper_can_undo(uint8_t track);

/**
 * @brief Check if redo is available
 * @param track Track index (0-3)
 * @return 1 if redo available, 0 otherwise
 */
uint8_t looper_can_redo(uint8_t track);

// ---- Loop Quantization ----

/**
 * @brief Quantize all events in a track to nearest grid position
 * @param track Track index (0-3)
 * @param resolution Quantization resolution (0=1/4, 1=1/8, 2=1/16, 3=1/32, 4=1/64)
 * 
 * Snaps all MIDI events to the nearest grid position based on the specified resolution.
 * Recommended to call looper_undo_push() before quantizing to enable undo.
 * 
 * Resolutions at 96 PPQN:
 * - 0: Quarter note (96 ticks)
 * - 1: Eighth note (48 ticks)
 * - 2: Sixteenth note (24 ticks) - default
 * - 3: Thirty-second note (12 ticks)
 * - 4: Sixty-fourth note (6 ticks)
 */
void looper_quantize_track(uint8_t track, uint8_t resolution);

/**
 * @brief Enable/disable auto-quantization for track
 * @param track Track index (0-3)
 * @param enabled 1 to enable, 0 to disable
 * 
 * When enabled, events are quantized during recording (future enhancement).
 * Currently manual quantization via looper_quantize_track() is supported.
 */
void looper_set_quantize_enabled(uint8_t track, uint8_t enabled);

/**
 * @brief Get quantization enabled state
 * @param track Track index (0-3)
 * @return 1 if enabled, 0 otherwise
 */
uint8_t looper_get_quantize_enabled(uint8_t track);

/**
 * @brief Set quantization resolution for track
 * @param track Track index (0-3)
 * @param resolution Resolution index (0-4)
 */
void looper_set_quantize_resolution(uint8_t track, uint8_t resolution);

/**
 * @brief Get quantization resolution
 * @param track Track index (0-3)
 * @return Resolution index (0-4)
 */
uint8_t looper_get_quantize_resolution(uint8_t track);

// ---- MIDI Clock Sync ----

/**
 * @brief Enable/disable external MIDI clock synchronization
 * @param enabled 1 to enable external clock sync, 0 to use internal clock
 * 
 * When enabled, the looper will synchronize its tempo to incoming MIDI clock
 * messages (0xF8) from an external source. The BPM is calculated from the
 * average interval of 24 clock pulses (1 quarter note).
 */
void looper_set_clock_sync_enabled(uint8_t enabled);

/**
 * @brief Get external clock sync status
 * @return 1 if external clock sync enabled, 0 otherwise
 */
uint8_t looper_get_clock_sync_enabled(void);

/**
 * @brief Process incoming MIDI clock message (0xF8)
 * 
 * Call this function when a MIDI clock message (0xF8) is received.
 * The system will measure intervals and calculate external BPM.
 * After 24 pulses (1 quarter note), the tempo is updated if sync is enabled.
 */
void looper_process_midi_clock(void);

/**
 * @brief Process MIDI Start message (0xFA)
 * 
 * Call when receiving MIDI Start. If external clock sync is enabled,
 * this will start playback from the beginning.
 */
void looper_process_midi_start(void);

/**
 * @brief Process MIDI Stop message (0xFC)
 * 
 * Call when receiving MIDI Stop. If external clock sync is enabled,
 * this will stop playback.
 */
void looper_process_midi_stop(void);

/**
 * @brief Process MIDI Continue message (0xFB)
 * 
 * Call when receiving MIDI Continue. If external clock sync is enabled,
 * this will resume playback from current position.
 */
void looper_process_midi_continue(void);

/**
 * @brief Get detected external BPM
 * @return BPM detected from external MIDI clock (20-300), or 0 if no clock detected
 */
uint16_t looper_get_external_bpm(void);

/**
 * @brief Check if external clock is actively being received
 * @return 1 if clock messages received within last 2 seconds, 0 otherwise
 */
uint8_t looper_is_external_clock_active(void);

// ---- Copy/Paste Scenes and Tracks ----

/**
 * @brief Copy track data to clipboard
 * @param track Track index (0-3)
 * @return 0 on success, -1 on error
 * 
 * Copies the track's complete state (events, loop length, settings) to an
 * internal clipboard for later pasting.
 */
int looper_copy_track(uint8_t track);

/**
 * @brief Paste clipboard data to track
 * @param track Track index (0-3)
 * @return 0 on success, -1 if clipboard empty or error
 * 
 * Pastes previously copied track data. Recommended to call looper_undo_push()
 * before pasting to enable undo.
 */
int looper_paste_track(uint8_t track);

/**
 * @brief Copy entire scene to clipboard
 * @param scene Scene index (0-7)
 * @return 0 on success, -1 on error
 * 
 * Copies all 4 tracks from the specified scene to clipboard.
 */
int looper_copy_scene(uint8_t scene);

/**
 * @brief Paste clipboard scene data to target scene
 * @param scene Target scene index (0-7)
 * @return 0 on success, -1 if clipboard empty or error
 * 
 * Pastes previously copied scene data (all tracks) to the specified scene.
 */
int looper_paste_scene(uint8_t scene);

/**
 * @brief Check if track clipboard has data
 * @return 1 if track clipboard contains data, 0 otherwise
 */
uint8_t looper_has_track_clipboard(void);

/**
 * @brief Check if scene clipboard has data
 * @return 1 if scene clipboard contains data, 0 otherwise
 */
uint8_t looper_has_scene_clipboard(void);

/**
 * @brief Clear track clipboard
 */
void looper_clear_track_clipboard(void);

/**
 * @brief Clear scene clipboard
 */
void looper_clear_scene_clipboard(void);

// ---- Global Transpose ----

/**
 * @brief Set global transpose for all tracks
 * @param semitones Transpose amount in semitones (-24 to +24, 0 = no transpose)
 * 
 * Applies transpose offset to all MIDI note events across all tracks.
 * Useful for quick key changes during live performance or practice.
 * Notes are clamped to valid MIDI range (0-127).
 */
void looper_set_global_transpose(int8_t semitones);

/**
 * @brief Get current global transpose value
 * @return Current transpose in semitones (-24 to +24)
 */
int8_t looper_get_global_transpose(void);

/**
 * @brief Transpose all events in all tracks by specified semitones
 * @param semitones Transpose amount in semitones (-24 to +24)
 * 
 * Permanently modifies all note events in all tracks by the specified interval.
 * Recommended to call looper_undo_push() for each track before transposing.
 * Notes are clamped to valid MIDI range (0-127).
 */
void looper_transpose_all_tracks(int8_t semitones);

// ---- Arpeggiator Feature ----

/**
 * Arpeggiator pattern types
 */
typedef enum {
  ARP_PATTERN_UP = 0,      // Ascending notes
  ARP_PATTERN_DOWN,        // Descending notes
  ARP_PATTERN_UPDOWN,      // Up then down (no repeat at top/bottom)
  ARP_PATTERN_RANDOM,      // Random note order
  ARP_PATTERN_CHORD        // All notes together (no arpeggiation)
} arp_pattern_t;

/**
 * @brief Enable/disable arpeggiator for a track
 * @param track Track index (0-3)
 * @param enabled 1 to enable, 0 to disable
 * 
 * When enabled, held notes are arpeggiated according to the configured pattern.
 * The arpeggiator operates in real-time during playback.
 */
void looper_set_arp_enabled(uint8_t track, uint8_t enabled);

/**
 * @brief Get arpeggiator enabled state
 * @param track Track index (0-3)
 * @return 1 if enabled, 0 otherwise
 */
uint8_t looper_get_arp_enabled(uint8_t track);

/**
 * @brief Set arpeggiator pattern
 * @param track Track index (0-3)
 * @param pattern Pattern type (see arp_pattern_t enum)
 */
void looper_set_arp_pattern(uint8_t track, arp_pattern_t pattern);

/**
 * @brief Get arpeggiator pattern
 * @param track Track index (0-3)
 * @return Current pattern type
 */
arp_pattern_t looper_get_arp_pattern(uint8_t track);

/**
 * @brief Set arpeggiator gate length
 * @param track Track index (0-3)
 * @param gate_percent Gate length as percentage of step (10-95%)
 * 
 * Controls how long each arpeggiated note is held.
 * - 50% = staccato (notes are half the step duration)
 * - 90% = legato (notes are nearly the full step duration)
 */
void looper_set_arp_gate(uint8_t track, uint8_t gate_percent);

/**
 * @brief Get arpeggiator gate length
 * @param track Track index (0-3)
 * @return Gate length percentage (10-95%)
 */
uint8_t looper_get_arp_gate(uint8_t track);

/**
 * @brief Set arpeggiator octave range
 * @param track Track index (0-3)
 * @param octaves Number of octaves (1-4)
 * 
 * Extends the arpeggio across multiple octaves.
 * - 1: Single octave (original notes only)
 * - 2: Two octaves (original + 1 octave up)
 * - 3-4: Three or four octaves
 */
void looper_set_arp_octaves(uint8_t track, uint8_t octaves);

/**
 * @brief Get arpeggiator octave range
 * @param track Track index (0-3)
 * @return Number of octaves (1-4)
 */
uint8_t looper_get_arp_octaves(uint8_t track);

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

// ---- Footswitch Mapping ----

/**
 * @brief Footswitch action types
 */
typedef enum {
    FS_ACTION_NONE = 0,
    FS_ACTION_PLAY_STOP,
    FS_ACTION_RECORD,
    FS_ACTION_OVERDUB,
    FS_ACTION_UNDO,
    FS_ACTION_REDO,
    FS_ACTION_TAP_TEMPO,
    FS_ACTION_SELECT_TRACK,
    FS_ACTION_TRIGGER_SCENE,
    FS_ACTION_MUTE_TRACK,
    FS_ACTION_SOLO_TRACK,
    FS_ACTION_CLEAR_TRACK,
    FS_ACTION_QUANTIZE_TRACK
} footswitch_action_t;

/**
 * @brief Assign function to footswitch
 * @param fs_num Footswitch number (0-7)
 * @param action Action to assign
 * @param param Action parameter (track number, scene number, etc.)
 */
void looper_set_footswitch_action(uint8_t fs_num, footswitch_action_t action, uint8_t param);

/**
 * @brief Get footswitch assignment
 * @param fs_num Footswitch number (0-7)
 * @param out_param Pointer to store action parameter (can be NULL)
 * @return Assigned action
 */
footswitch_action_t looper_get_footswitch_action(uint8_t fs_num, uint8_t* out_param);

/**
 * @brief Process footswitch press event
 * @param fs_num Footswitch number (0-7)
 */
void looper_footswitch_press(uint8_t fs_num);

/**
 * @brief Process footswitch release event
 * @param fs_num Footswitch number (0-7)
 */
void looper_footswitch_release(uint8_t fs_num);

// ---- MIDI Learn System ----

/**
 * @brief Start MIDI learn mode for an action
 * @param action Action to map (e.g., FS_ACTION_PLAY_STOP, FS_ACTION_RECORD)
 * @param param Action parameter (track/scene number, 0-3 for tracks, 0-7 for scenes)
 * 
 * After calling this, the next incoming MIDI CC or Note message will be
 * mapped to the specified action. Learn mode auto-cancels after 10 seconds.
 * 
 * Example workflow:
 * 1. Call looper_midi_learn_start(FS_ACTION_PLAY_STOP, 0)
 * 2. Press a button on your MIDI controller (e.g., CC64)
 * 3. CC64 is now mapped to Play/Stop
 * 4. Future CC64 messages will trigger Play/Stop
 * 
 * Multiple mappings example:
 * - Map CC80 to Track 1 Mute: looper_midi_learn_start(FS_ACTION_MUTE_TRACK, 0)
 * - Map CC81 to Track 2 Mute: looper_midi_learn_start(FS_ACTION_MUTE_TRACK, 1)
 * - Map Note C4 to Scene A: looper_midi_learn_start(FS_ACTION_TRIGGER_SCENE, 0)
 */
void looper_midi_learn_start(footswitch_action_t action, uint8_t param);

/**
 * @brief Cancel MIDI learn mode
 */
void looper_midi_learn_cancel(void);

/**
 * @brief Process incoming MIDI message for learn mode
 * @param msg MIDI message to process
 */
void looper_midi_learn_process(const router_msg_t* msg);

/**
 * @brief Check if MIDI message triggers a learned action
 * @param msg MIDI message to check
 */
void looper_midi_learn_check(const router_msg_t* msg);

/**
 * @brief Clear all MIDI learn mappings
 */
void looper_midi_learn_clear(void);

/**
 * @brief Get number of learned MIDI mappings
 * @return Number of active mappings (0-32)
 */
uint8_t looper_midi_learn_get_count(void);

// ---- CC Automation Layer ----

/**
 * @brief CC automation event storage
 * 
 * Each track can store up to 128 CC automation events that play back
 * synchronized with the main loop. CC messages are recorded with tick
 * timing and replayed automatically during loop playback.
 */
#define LOOPER_AUTOMATION_MAX_EVENTS 128

typedef struct {
  uint32_t tick;      // Tick position within loop
  uint8_t  cc_num;    // CC number (0-127)
  uint8_t  cc_value;  // CC value (0-127)
  uint8_t  channel;   // MIDI channel (0-15)
} looper_automation_event_t;

/**
 * @brief Start recording CC automation for a track
 * @param track Track index (0-3)
 * 
 * Enables CC automation recording mode. All incoming CC messages
 * will be stored with tick timing until recording is stopped.
 * Recording automatically stops when max events is reached.
 */
void looper_automation_start_record(uint8_t track);

/**
 * @brief Stop recording CC automation for a track
 * @param track Track index (0-3)
 */
void looper_automation_stop_record(uint8_t track);

/**
 * @brief Check if automation recording is active
 * @param track Track index (0-3)
 * @return 1 if recording, 0 otherwise
 */
uint8_t looper_automation_is_recording(uint8_t track);

/**
 * @brief Enable/disable automation playback for a track
 * @param track Track index (0-3)
 * @param enable 1 to enable playback, 0 to disable
 * 
 * When enabled, recorded CC automation events will be sent
 * automatically during loop playback, synchronized with the cursor position.
 */
void looper_automation_enable_playback(uint8_t track, uint8_t enable);

/**
 * @brief Check if automation playback is enabled
 * @param track Track index (0-3)
 * @return 1 if playback enabled, 0 otherwise
 */
uint8_t looper_automation_is_playback_enabled(uint8_t track);

/**
 * @brief Clear all automation events for a track
 * @param track Track index (0-3)
 * 
 * Removes all recorded CC automation events. Does not affect
 * the main note/MIDI event loop.
 */
void looper_automation_clear(uint8_t track);

/**
 * @brief Get number of automation events for a track
 * @param track Track index (0-3)
 * @return Number of recorded CC automation events (0-128)
 */
uint32_t looper_automation_get_event_count(uint8_t track);

/**
 * @brief Export automation events for inspection/editing
 * @param track Track index (0-3)
 * @param out Output buffer for automation events
 * @param max Maximum number of events to copy
 * @return Number of events copied
 * 
 * Copies automation events to the provided buffer for inspection or editing.
 * Events are returned in chronological order (sorted by tick).
 */
uint32_t looper_automation_export_events(uint8_t track, 
                                          looper_automation_event_t* out, 
                                          uint32_t max);

/**
 * @brief Manually add a CC automation event
 * @param track Track index (0-3)
 * @param tick Tick position within loop
 * @param cc_num CC number (0-127)
 * @param cc_value CC value (0-127)
 * @param channel MIDI channel (0-15)
 * @return 0 on success, -1 on error (buffer full, invalid parameters)
 * 
 * Allows programmatic creation of automation events without recording.
 * Useful for algorithmic composition or preset automation patterns.
 */
int looper_automation_add_event(uint8_t track, uint32_t tick, 
                                  uint8_t cc_num, uint8_t cc_value, 
                                  uint8_t channel);

/**
 * @brief Set automation layer mute state
 * @param track Track index (0-3)
 * @param muted 1 to mute CC automation, 0 to unmute
 * 
 * When muted, CC automation events will not be played back during loop playback.
 * The main MIDI notes continue playing normally. Useful for A/B testing automation.
 */
void looper_automation_set_mute(uint8_t track, uint8_t muted);

/**
 * @brief Get automation layer mute state
 * @param track Track index (0-3)
 * @return 1 if muted, 0 if not muted
 */
uint8_t looper_automation_is_muted(uint8_t track);

/**
 * @brief Set automation layer solo state
 * @param track Track index (0-3)
 * @param solo 1 to solo CC automation, 0 to unsolo
 * 
 * When soloed, only the CC automation layer plays back (notes are muted).
 * Useful for isolating and testing automation without the melodic content.
 */
void looper_automation_set_solo(uint8_t track, uint8_t solo);

/**
 * @brief Get automation layer solo state
 * @param track Track index (0-3)
 * @return 1 if soloed, 0 if not soloed
 */
uint8_t looper_automation_is_soloed(uint8_t track);

// ---- Loop Length Constraints ----

/**
 * @brief Set loop length constraint for a track
 * @param track Track index (0-3)
 * @param beats Constrained loop length in beats (0 = no constraint, 1/2/4/8/16/32)
 * 
 * When set, forces the loop to be exactly the specified length. When recording
 * reaches this length, it automatically stops or loops. Prevents accidental
 * timing drift. Set to 0 to disable constraint (free-form recording).
 */
void looper_set_length_constraint(uint8_t track, uint16_t beats);

/**
 * @brief Get loop length constraint for a track
 * @param track Track index (0-3)
 * @return Constrained loop length in beats (0 = no constraint)
 */
uint16_t looper_get_length_constraint(uint8_t track);

/**
 * @brief Auto-quantize loop length to nearest musical value
 * @param track Track index (0-3)
 * 
 * Automatically adjusts the current loop length to the nearest standard musical
 * duration (1, 2, 4, 8, 16, or 32 beats). Prevents timing drift and ensures
 * loops align with musical structure.
 */
void looper_quantize_loop_length(uint8_t track);

// ---- Track Link/Group ----

/**
 * @brief Link two tracks for synchronized playback
 * @param track1 First track index (0-3)
 * @param track2 Second track index (0-3)
 * @param linked 1 to link tracks, 0 to unlink
 * 
 * When tracks are linked, they start and stop together. Useful for layered
 * parts that must stay synchronized (e.g., bass + drums, melody + harmony).
 * State changes on either track affect both.
 */
void looper_link_tracks(uint8_t track1, uint8_t track2, uint8_t linked);

/**
 * @brief Check if two tracks are linked
 * @param track1 First track index (0-3)
 * @param track2 Second track index (0-3)
 * @return 1 if tracks are linked, 0 if not linked
 */
uint8_t looper_are_tracks_linked(uint8_t track1, uint8_t track2);

/**
 * @brief Clear all track links
 * 
 * Unlinks all tracks, restoring independent operation.
 */
void looper_clear_all_track_links(void);

// ---- Scene Snapshot (Lightweight State Management) ----

/**
 * @brief Save current scene state to a snapshot slot
 * @param slot Snapshot slot number (0-15)
 * @return 0 on success, -1 on error
 * 
 * Saves only track states (play/stop/rec), mute/solo settings, and basic
 * configuration. Does NOT save full track data (unlike clipboards).
 * Uses minimal RAM (~100 bytes vs 16KB for full clipboard).
 * Useful for storing arrangement variations and performance presets.
 */
int looper_save_scene_state(uint8_t slot);

/**
 * @brief Recall scene state from a snapshot slot
 * @param slot Snapshot slot number (0-15)
 * @return 0 on success, -1 on error (slot empty)
 * 
 * Restores track states, mute/solo settings, and configuration.
 * Track audio content remains unchanged. Allows quick arrangement switching
 * during performance without loading full clips.
 */
int looper_recall_scene_state(uint8_t slot);

/**
 * @brief Check if a scene state slot contains data
 * @param slot Snapshot slot number (0-15)
 * @return 1 if slot is used, 0 if empty
 */
uint8_t looper_scene_state_is_saved(uint8_t slot);

/**
 * @brief Clear a scene state snapshot slot
 * @param slot Snapshot slot number (0-15)
 */
void looper_clear_scene_state(uint8_t slot);

// ---- Quick-Save System ----

/**
 * @brief Save current session to a quick-save slot
 * @param slot Slot number (0-7)
 * @param name Optional name (can be NULL for auto-naming)
 * @return 0 on success, negative on error
 * 
 * Saves all tracks, current scene, and transport settings to the specified slot.
 * Data is persisted to SD card for recall after power cycle.
 */
int looper_quick_save(uint8_t slot, const char* name);

/**
 * @brief Load session from a quick-save slot
 * @param slot Slot number (0-7)
 * @return 0 on success, negative on error (e.g., slot empty)
 * 
 * Restores all tracks, scene, and transport settings from the specified slot.
 */
int looper_quick_load(uint8_t slot);

/**
 * @brief Check if quick-save slot contains data
 * @param slot Slot number (0-7)
 * @return 1 if slot is used, 0 if empty
 */
uint8_t looper_quick_save_is_used(uint8_t slot);

/**
 * @brief Get quick-save slot name
 * @param slot Slot number (0-7)
 * @return Slot name, or NULL if slot is empty
 */
const char* looper_quick_save_get_name(uint8_t slot);

/**
 * @brief Clear a quick-save slot
 * @param slot Slot number (0-7)
 */
void looper_quick_save_clear(uint8_t slot);

#ifdef __cplusplus
}
#endif
