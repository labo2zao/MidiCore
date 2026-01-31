/**
 * @file usb_midi_mtc.c
 * @brief MIDI Time Code (MTC) implementation
 * 
 * Based on MidiCore timing patterns with precise quarter-frame generation.
 * Follows MTC specification for SMPTE timecode synchronization.
 */

#include "Services/usb_midi/usb_midi_mtc.h"
#include "Services/usb_midi/usb_midi.h"
#include "Services/usb_midi/usb_midi_sysex.h"
#include <string.h>

// Frame rates in frames per second (fixed-point: multiply by 1000)
static const uint32_t frame_rates_x1000[4] = {
  24000,   // 24 fps (film)
  25000,   // 25 fps (PAL)
  29970,   // 29.97 fps (NTSC drop-frame)
  30000    // 30 fps (NTSC non-drop)
};

// Maximum frames per rate
static const uint8_t max_frames[4] = {
  23,  // 24 fps: 0-23
  24,  // 25 fps: 0-24
  29,  // 29.97 fps: 0-29
  29   // 30 fps: 0-29
};

// Module state
static struct {
  mtc_config_t config;
  mtc_timecode_t current_tc;
  mtc_stats_t stats;
  
  // Internal generation state
  uint32_t us_accumulator;      // Microsecond accumulator for frame timing
  uint8_t qf_index;             // Quarter frame index (0-7)
  uint8_t running;              // 1 = generating, 0 = stopped
  uint8_t padding[2];
  
  // External sync state (for reception)
  uint8_t rx_qf_buffer[8];      // Received quarter frame pieces
  uint8_t rx_qf_valid_mask;     // Bitmask of received pieces
  uint32_t last_rx_us;          // Last receive timestamp (for timeout detection)
} mtc_state __attribute__((aligned(4)));

/**
 * @brief Calculate microseconds per quarter frame for current frame rate
 * @return Microseconds between quarter frames
 */
static uint32_t mtc_get_us_per_qf(void) {
  // Quarter frame rate = frame rate * 4 (4 QF per frame, 8 QF per 2 frames)
  // us_per_qf = 1000000 / (frame_rate * 4)
  uint32_t frame_rate_x1000 = frame_rates_x1000[mtc_state.config.frame_rate];
  return (1000000000UL) / (frame_rate_x1000 * 4); // Returns microseconds
}

/**
 * @brief Increment timecode by one frame (MidiCore style)
 */
static void mtc_increment_frame(void) {
  mtc_timecode_t* tc = &mtc_state.current_tc;
  uint8_t max_frame = max_frames[tc->frame_rate];
  
  tc->frames++;
  if (tc->frames > max_frame) {
    tc->frames = 0;
    tc->seconds++;
    if (tc->seconds >= 60) {
      tc->seconds = 0;
      tc->minutes++;
      if (tc->minutes >= 60) {
        tc->minutes = 0;
        tc->hours++;
        if (tc->hours >= 24) {
          tc->hours = 0;
        }
      }
    }
  }
}

void mtc_init(void) {
  memset(&mtc_state, 0, sizeof(mtc_state));
  
  // Default configuration
  mtc_state.config.source = MTC_SOURCE_OFF;
  mtc_state.config.send_quarter_frames = 0;
  mtc_state.config.send_full_frames = 0;
  mtc_state.config.cable = 0;
  mtc_state.config.frame_rate = MTC_FRAME_RATE_30;
  
  mtc_state.current_tc.frame_rate = MTC_FRAME_RATE_30;
}

void mtc_set_config(const mtc_config_t* config) {
  if (!config) return;
  memcpy(&mtc_state.config, config, sizeof(mtc_config_t));
  
  // Update current timecode frame rate
  if (config->frame_rate < 4) {
    mtc_state.current_tc.frame_rate = config->frame_rate;
  }
}

void mtc_get_config(mtc_config_t* out) {
  if (!out) return;
  memcpy(out, &mtc_state.config, sizeof(mtc_config_t));
}

void mtc_get_stats(mtc_stats_t* out) {
  if (!out) return;
  
  // Copy current stats
  out->current_time = mtc_state.current_tc;
  out->quarter_frame_count = mtc_state.stats.quarter_frame_count;
  out->full_frame_count = mtc_state.stats.full_frame_count;
  out->is_synced = mtc_state.stats.is_synced;
  out->qf_index = mtc_state.qf_index;
}

void mtc_set_timecode(const mtc_timecode_t* tc) {
  if (!tc) return;
  
  // Validate and copy (MIOS32-style bounds checking)
  if (tc->hours < 24 && tc->minutes < 60 && tc->seconds < 60) {
    uint8_t max_frame = max_frames[mtc_state.config.frame_rate];
    if (tc->frames <= max_frame) {
      mtc_state.current_tc = *tc;
      mtc_state.current_tc.frame_rate = mtc_state.config.frame_rate;
    }
  }
}

void mtc_get_timecode(mtc_timecode_t* out) {
  if (!out) return;
  *out = mtc_state.current_tc;
}

void mtc_send_quarter_frame(void) {
  if (!mtc_state.config.send_quarter_frames) return;
  if (mtc_state.config.source != MTC_SOURCE_INTERNAL) return;
  
  mtc_timecode_t* tc = &mtc_state.current_tc;
  uint8_t qf_index = mtc_state.qf_index;
  uint8_t data = 0;
  
  // Build quarter frame message (MTC specification)
  // Format: 0xF1 0bHHHHLLLL where HHHH = piece type, LLLL = data nibble
  switch (qf_index) {
    case 0: data = (0 << 4) | (tc->frames & 0x0F); break;           // Frame LSB
    case 1: data = (1 << 4) | ((tc->frames >> 4) & 0x01); break;    // Frame MSB
    case 2: data = (2 << 4) | (tc->seconds & 0x0F); break;          // Second LSB
    case 3: data = (3 << 4) | ((tc->seconds >> 4) & 0x03); break;   // Second MSB
    case 4: data = (4 << 4) | (tc->minutes & 0x0F); break;          // Minute LSB
    case 5: data = (5 << 4) | ((tc->minutes >> 4) & 0x03); break;   // Minute MSB
    case 6: data = (6 << 4) | (tc->hours & 0x0F); break;            // Hour LSB
    case 7: {
      // Hour MSB + frame rate code
      uint8_t hr_msb = (tc->hours >> 4) & 0x01;
      uint8_t rate_code = tc->frame_rate << 1;
      data = (7 << 4) | (rate_code | hr_msb);
      break;
    }
  }
  
  // Send via USB MIDI (System Common 0xF1, CIN 0x2)
  uint8_t cin = (mtc_state.config.cable << 4) | 0x02;  // 2-byte system common
  usb_midi_send_packet(cin, 0xF1, data, 0);
  
  // Update state
  mtc_state.qf_index = (qf_index + 1) & 0x07;
  mtc_state.stats.quarter_frame_count++;
  
  // Every 2 frames (8 quarter frames), increment timecode
  if (mtc_state.qf_index == 0) {
    mtc_increment_frame();
    mtc_increment_frame();
  }
}

void mtc_send_full_frame(void) {
  if (!mtc_state.config.send_full_frames) return;
  
  mtc_timecode_t* tc = &mtc_state.current_tc;
  
  // Build full frame SysEx: F0 7F 7F 01 01 hr mn sc fr F7
  // Rate code in bits 5-6 of hour byte
  uint8_t hr_byte = tc->hours | (tc->frame_rate << 5);
  
  uint8_t sysex_data[] = {
    0xF0, 0x7F, 0x7F,    // Universal Real-Time SysEx
    0x01, 0x01,          // MTC Full Frame
    hr_byte,             // Hours + rate
    tc->minutes,
    tc->seconds,
    tc->frames,
    0xF7
  };
  
  usb_midi_send_sysex(sysex_data, sizeof(sysex_data), mtc_state.config.cable);
  mtc_state.stats.full_frame_count++;
}

void mtc_on_rx_quarter_frame(uint8_t data) {
  if (mtc_state.config.source != MTC_SOURCE_EXTERNAL) return;
  
  uint8_t piece = (data >> 4) & 0x07;
  uint8_t nibble = data & 0x0F;
  
  // Store received piece
  mtc_state.rx_qf_buffer[piece] = nibble;
  mtc_state.rx_qf_valid_mask |= (1 << piece);
  
  mtc_state.stats.quarter_frame_count++;
  
  // When all 8 pieces received, assemble complete timecode
  if (mtc_state.rx_qf_valid_mask == 0xFF) {
    mtc_timecode_t new_tc;
    
    new_tc.frames = mtc_state.rx_qf_buffer[0] | (mtc_state.rx_qf_buffer[1] << 4);
    new_tc.seconds = mtc_state.rx_qf_buffer[2] | (mtc_state.rx_qf_buffer[3] << 4);
    new_tc.minutes = mtc_state.rx_qf_buffer[4] | (mtc_state.rx_qf_buffer[5] << 4);
    new_tc.hours = mtc_state.rx_qf_buffer[6] | ((mtc_state.rx_qf_buffer[7] & 0x01) << 4);
    new_tc.frame_rate = (mtc_state.rx_qf_buffer[7] >> 1) & 0x03;
    
    // Validate before accepting
    if (new_tc.frame_rate < 4 && new_tc.hours < 24 && 
        new_tc.minutes < 60 && new_tc.seconds < 60 &&
        new_tc.frames <= max_frames[new_tc.frame_rate]) {
      mtc_state.current_tc = new_tc;
      mtc_state.stats.is_synced = 1;
    }
    
    // Reset for next complete timecode
    mtc_state.rx_qf_valid_mask = 0;
  }
}

void mtc_on_rx_full_frame(const uint8_t* data, uint16_t len) {
  if (mtc_state.config.source != MTC_SOURCE_EXTERNAL) return;
  if (!data || len < 9) return;
  
  // Parse: F0 7F 7F 01 01 hr mn sc fr F7
  if (data[0] == 0xF0 && data[1] == 0x7F && data[2] == 0x7F &&
      data[3] == 0x01 && data[4] == 0x01 && data[9] == 0xF7) {
    
    mtc_timecode_t new_tc;
    new_tc.hours = data[5] & 0x1F;
    new_tc.frame_rate = (data[5] >> 5) & 0x03;
    new_tc.minutes = data[6] & 0x3F;
    new_tc.seconds = data[7] & 0x3F;
    new_tc.frames = data[8] & 0x1F;
    
    // Validate before accepting
    if (new_tc.frame_rate < 4 && new_tc.hours < 24 &&
        new_tc.minutes < 60 && new_tc.seconds < 60 &&
        new_tc.frames <= max_frames[new_tc.frame_rate]) {
      mtc_state.current_tc = new_tc;
      mtc_state.stats.is_synced = 1;
      mtc_state.stats.full_frame_count++;
    }
  }
}

void mtc_tick_us(uint32_t delta_us) {
  if (!mtc_state.running) return;
  if (mtc_state.config.source != MTC_SOURCE_INTERNAL) return;
  
  mtc_state.us_accumulator += delta_us;
  
  uint32_t us_per_qf = mtc_get_us_per_qf();
  
  // Send quarter frame when enough time has elapsed
  while (mtc_state.us_accumulator >= us_per_qf) {
    mtc_send_quarter_frame();
    mtc_state.us_accumulator -= us_per_qf;
  }
}

void mtc_reset_timecode(void) {
  memset(&mtc_state.current_tc, 0, sizeof(mtc_timecode_t));
  mtc_state.current_tc.frame_rate = mtc_state.config.frame_rate;
  mtc_state.qf_index = 0;
  mtc_state.us_accumulator = 0;
}

void mtc_start(void) {
  if (mtc_state.config.source != MTC_SOURCE_INTERNAL) return;
  
  mtc_state.running = 1;
  mtc_state.us_accumulator = 0;
  
  // Send full frame for immediate sync
  if (mtc_state.config.send_full_frames) {
    mtc_send_full_frame();
  }
}

void mtc_stop(void) {
  mtc_state.running = 0;
  
  // Send full frame on stop if configured
  if (mtc_state.config.send_full_frames) {
    mtc_send_full_frame();
  }
}
