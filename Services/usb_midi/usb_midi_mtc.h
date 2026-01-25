#pragma once
#include <stdint.h>

/**
 * @file usb_midi_mtc.h
 * @brief MIDI Time Code (MTC) support
 * 
 * Provides MTC generation and reception for SMPTE timecode synchronization.
 * Supports Quarter Frame (0xF1) and Full Frame (0xF0 7F 7F 01) messages.
 * 
 * SMPTE Frame Rates:
 * - 24 fps (Film)
 * - 25 fps (PAL video)
 * - 29.97 fps (NTSC drop-frame)
 * - 30 fps (NTSC non-drop)
 */

#ifdef __cplusplus
extern "C" {
#endif

// SMPTE frame rate
typedef enum {
  MTC_FRAME_RATE_24 = 0,      // 24 fps (film)
  MTC_FRAME_RATE_25 = 1,      // 25 fps (PAL)
  MTC_FRAME_RATE_29_97 = 2,   // 29.97 fps (NTSC drop-frame)
  MTC_FRAME_RATE_30 = 3       // 30 fps (NTSC non-drop)
} mtc_frame_rate_t;

// MTC source configuration
typedef enum {
  MTC_SOURCE_INTERNAL = 0,    // Generate MTC internally
  MTC_SOURCE_EXTERNAL = 1,    // Sync to external MTC
  MTC_SOURCE_OFF = 2          // MTC disabled
} mtc_source_t;

// SMPTE timecode
typedef struct {
  uint8_t hours;              // 0-23
  uint8_t minutes;            // 0-59
  uint8_t seconds;            // 0-59
  uint8_t frames;             // 0-29 (depends on frame rate)
  uint8_t frame_rate;         // mtc_frame_rate_t
  uint8_t reserved[3];
} mtc_timecode_t;

// MTC configuration
typedef struct {
  uint8_t source;             // mtc_source_t
  uint8_t send_quarter_frames;// 1 = send quarter frames, 0 = don't
  uint8_t send_full_frames;   // 1 = send full frame messages, 0 = don't
  uint8_t cable;              // USB MIDI cable (0-3) for output
  uint8_t frame_rate;         // mtc_frame_rate_t
  uint8_t reserved[3];
} mtc_config_t;

// MTC statistics
typedef struct {
  mtc_timecode_t current_time;  // Current timecode position
  uint32_t quarter_frame_count; // Total quarter frames sent/received
  uint32_t full_frame_count;    // Total full frames sent/received
  uint8_t is_synced;            // 1 = locked to external MTC, 0 = not synced
  uint8_t qf_index;             // Current quarter frame piece (0-7)
  uint8_t reserved[2];
} mtc_stats_t;

/**
 * @brief Initialize MTC module
 */
void mtc_init(void);

/**
 * @brief Set MTC configuration
 * @param config Pointer to configuration structure
 */
void mtc_set_config(const mtc_config_t* config);

/**
 * @brief Get current MTC configuration
 * @param out Pointer to output configuration structure
 */
void mtc_get_config(mtc_config_t* out);

/**
 * @brief Get MTC statistics
 * @param out Pointer to output statistics structure
 */
void mtc_get_stats(mtc_stats_t* out);

/**
 * @brief Set current timecode position
 * @param tc Pointer to timecode structure
 * 
 * Sets the internal timecode for generation.
 * Only applies when source is MTC_SOURCE_INTERNAL.
 */
void mtc_set_timecode(const mtc_timecode_t* tc);

/**
 * @brief Get current timecode position
 * @param out Pointer to output timecode structure
 */
void mtc_get_timecode(mtc_timecode_t* out);

/**
 * @brief Send MTC Quarter Frame message (0xF1)
 * 
 * Sends one of 8 quarter frame pieces.
 * Complete timecode takes 8 quarter frames = 2 frames duration.
 * Called automatically by internal generator.
 */
void mtc_send_quarter_frame(void);

/**
 * @brief Send MTC Full Frame message (SysEx)
 * 
 * Sends complete timecode in single SysEx message:
 * F0 7F 7F 01 01 hr mn sc fr F7
 * 
 * Used for immediate synchronization (e.g., after stop/start).
 */
void mtc_send_full_frame(void);

/**
 * @brief Process received MTC Quarter Frame (0xF1)
 * @param data Quarter frame data byte
 * 
 * Reassembles 8 quarter frames into complete timecode.
 * Called from USB MIDI RX path.
 */
void mtc_on_rx_quarter_frame(uint8_t data);

/**
 * @brief Process received MTC Full Frame (SysEx)
 * @param data Pointer to SysEx data (after F0 7F 7F 01 01)
 * @param len Length of data
 * 
 * Parses full frame message for immediate sync.
 */
void mtc_on_rx_full_frame(const uint8_t* data, uint16_t len);

/**
 * @brief Update MTC (call from high-precision timer)
 * 
 * Generates MTC quarter frames at correct intervals.
 * Must be called at sub-millisecond precision for accurate timing.
 * 
 * @param delta_us Time elapsed since last call in microseconds
 */
void mtc_tick_us(uint32_t delta_us);

/**
 * @brief Reset timecode to 00:00:00:00
 */
void mtc_reset_timecode(void);

/**
 * @brief Start MTC generation
 * 
 * Begins sending quarter frames if configured.
 */
void mtc_start(void);

/**
 * @brief Stop MTC generation
 * 
 * Stops sending quarter frames.
 * Sends full frame message if configured.
 */
void mtc_stop(void);

#ifdef __cplusplus
}
#endif
