/**
 * @file ui_page_sysex.c
 * @brief SysEx UI Page - System Exclusive message capture and display
 * 
 * Captures and displays SysEx messages with hex viewer. Allows basic
 * inspection of manufacturer ID, device ID, and data payload.
 */

#include "Services/ui/ui_page_sysex.h"
#include "Services/ui/ui_gfx.h"
#include "Services/router/router.h"
#include "Services/fs/sd_guard.h"
#include "FATFS/App/fatfs.h"
#include <stdio.h>
#include <string.h>

#define SYSEX_MAX_SIZE 128
#define STATUS_MSG_DURATION_MS 2000
#define STATUS_MSG_SHORT_MS 1500
#define STATUS_MSG_BRIEF_MS 1000

// SysEx capture buffer
static uint8_t sysex_buffer[SYSEX_MAX_SIZE];
static uint16_t sysex_length = 0;
static uint8_t sysex_captured = 0;
static uint16_t scroll_offset = 0;
static char status_message[32] = {0};
static uint32_t status_start_time = 0;  // When status message was set

/**
 * @brief Capture a SysEx message (to be called by MIDI router)
 * 
 * Note: This is a placeholder. In a full implementation, this would be
 * called from the MIDI router when SysEx messages are received.
 */
void ui_sysex_capture(const uint8_t* data, uint16_t len) {
  uint8_t truncated = 0;
  if (len > SYSEX_MAX_SIZE) {
    len = SYSEX_MAX_SIZE;
    truncated = 1;
  }
  
  memcpy(sysex_buffer, data, len);
  sysex_length = len;
  sysex_captured = truncated ? 2 : 1;  // 2 = truncated, 1 = complete
  scroll_offset = 0;
}

/**
 * @brief Clear the captured SysEx data
 */
static void clear_sysex(void) {
  memset(sysex_buffer, 0, sizeof(sysex_buffer));
  sysex_length = 0;
  sysex_captured = 0;
  scroll_offset = 0;
}

/**
 * @brief Render the SysEx page
 */
void ui_page_sysex_render(uint32_t now_ms) {
  (void)now_ms;
  
  ui_gfx_clear(0);
  
  // Header
  char header[64];
  
  ui_gfx_set_font(UI_FONT_8X8);
  if (sysex_captured) {
    if (sysex_captured == 2) {
      snprintf(header, sizeof(header), "SYSEX VIEW %ub [TRUNC]", sysex_length);
    } else {
      snprintf(header, sizeof(header), "SYSEX VIEW %u bytes", sysex_length);
    }
  } else {
    snprintf(header, sizeof(header), "SYSEX VIEWER Ready");
  }
  ui_gfx_text(0, 0, header, 15);
  ui_gfx_hline(0, 11, 256, 8);
  
  if (sysex_captured && sysex_length > 0) {
    // Decode manufacturer ID (if present)
    if (sysex_buffer[0] == 0xF0 && sysex_length >= 3) {
      char mfr_line[64];
      uint8_t mfr_id = sysex_buffer[1];
      
      if (mfr_id == 0x00 && sysex_length >= 4) {
        // 3-byte manufacturer ID
        snprintf(mfr_line, sizeof(mfr_line), "Mfr: 0x%02X%02X%02X", 
                 sysex_buffer[1], sysex_buffer[2], sysex_buffer[3]);
      } else {
        // 1-byte manufacturer ID
        snprintf(mfr_line, sizeof(mfr_line), "Mfr: 0x%02X", mfr_id);
      }
      ui_gfx_text(0, 15, mfr_line, 13);
    }
    
    // Hex view (display rows of 16 bytes) - better spacing with 8x8
    ui_gfx_text(0, 26, "Hex View:", 11);
    
    uint16_t display_rows = 3;  // Show 3 rows of 16 bytes each
    uint16_t start_offset = scroll_offset * 16;
    
    for (uint16_t row = 0; row < display_rows; row++) {
      uint16_t offset = start_offset + row * 16;
      if (offset >= sysex_length) break;
      
      char line[64];
      char hex_part[48] = {0};
      
      // Format: "00: F0 7E 7F 09 01 ..."
      int pos = 0;
      for (uint16_t i = 0; i < 16 && (offset + i) < sysex_length; i++) {
        pos += snprintf(hex_part + pos, sizeof(hex_part) - pos, "%02X ", 
                       sysex_buffer[offset + i]);
      }
      
      snprintf(line, sizeof(line), "%02X: %s", offset, hex_part);
      ui_gfx_text(0, 36 + row * 9, line, 11);
    }
  } else {
    // No SysEx captured yet
    ui_gfx_text(0, 26, "No SysEx message", 10);
    ui_gfx_text(0, 36, "Send a SysEx to view", 10);
  }
  
  // Status message - fix timing logic
  if (status_start_time > 0 && status_message[0] != '\0') {
    // Initialize start time on first display
    if (status_start_time == 1) {
      status_start_time = now_ms;
    }
    
    // Check if message should still be displayed
    if (now_ms - status_start_time < STATUS_MSG_DURATION_MS) {
      ui_gfx_text(0, 47, status_message, 13);
    } else {
      // Message expired
      status_start_time = 0;
      status_message[0] = '\0';
    }
  }
  
  // Footer with smaller font
  ui_gfx_hline(0, 54, 256, 6);
  ui_gfx_set_font(UI_FONT_5X7);
  ui_gfx_text(0, 56, "B1:SEND B2:RCV B3:CLR B4:SAVE ENC:scroll", 10);
}

/**
 * @brief Send captured SysEx message via MIDI router
 */
static void send_sysex(void) {
  if (!sysex_captured || sysex_length == 0) {
    strncpy(status_message, "No SysEx to send", sizeof(status_message) - 1);
    status_start_time = 0;  // Will use now_ms when displayed
    return;
  }
  
  // Send via MIDI router node 0 (typically USB MIDI)
  router_msg_t msg = {
    .type = ROUTER_MSG_SYSEX,
    .data = sysex_buffer,
    .len = sysex_length
  };
  
  router_process(0, &msg);
  
  strncpy(status_message, "SysEx sent", sizeof(status_message) - 1);
  status_start_time = 0;
}

/**
 * @brief Save captured SysEx to SD card
 */
static void save_sysex(void) {
  if (!sysex_captured || sysex_length == 0) {
    strncpy(status_message, "No SysEx to save", sizeof(status_message) - 1);
    status_start_time = 0;
    return;
  }
  
  if (sd_guard_is_readonly()) {
    strncpy(status_message, "SD read-only", sizeof(status_message) - 1);
    status_start_time = 0;
    return;
  }
  
  // Generate filename with timestamp-like pattern
  char filename[64];
  static uint16_t file_counter = 0;
  snprintf(filename, sizeof(filename), "/sysex/capture_%04u.syx", file_counter++);
  
  FIL file;
  FRESULT res = f_open(&file, filename, FA_WRITE | FA_CREATE_ALWAYS);
  if (res != FR_OK) {
    strncpy(status_message, "Save failed", sizeof(status_message) - 1);
    status_start_time = 0;
    sd_guard_note_write_error();
    return;
  }
  
  UINT written;
  res = f_write(&file, sysex_buffer, sysex_length, &written);
  f_close(&file);
  
  if (res != FR_OK || written != sysex_length) {
    strncpy(status_message, "Write error", sizeof(status_message) - 1);
    sd_guard_note_write_error();
  } else {
    snprintf(status_message, sizeof(status_message), "Saved %ub", sysex_length);
  }
  status_start_time = 0;
}

/**
 * @brief Handle button press in SysEx viewer
 */
void ui_page_sysex_on_button(uint8_t id, uint8_t pressed) {
  if (!pressed) return;
  
  switch (id) {
    case 1:  // SEND - send captured SysEx message
      send_sysex();
      if (status_start_time == 0) status_start_time = 1;  // Mark for display
      break;
      
    case 2:  // RCV - reset capture (already receiving)
      sysex_captured = 0;
      sysex_length = 0;
      scroll_offset = 0;
      strncpy(status_message, "Ready to receive", sizeof(status_message) - 1);
      status_start_time = 1;
      break;
      
    case 3:  // CLEAR
      clear_sysex();
      strncpy(status_message, "Cleared", sizeof(status_message) - 1);
      status_start_time = 1;
      break;
      
    case 4:  // SAVE - save SysEx to SD card
      save_sysex();
      if (status_start_time == 0) status_start_time = 1;
      break;
      
    default:
      break;
  }
}

/**
 * @brief Handle encoder rotation in SysEx viewer
 */
void ui_page_sysex_on_encoder(int8_t delta) {
  // Scroll through hex view
  if (delta > 0) {
    if ((scroll_offset + 3) * 16 < sysex_length) {
      scroll_offset++;
    }
  } else if (delta < 0 && scroll_offset > 0) {
    scroll_offset--;
  }
}
