/**
 * @file ui_page_sysex.c
 * @brief SysEx UI Page - System Exclusive message capture and display
 * 
 * Captures and displays SysEx messages with hex viewer. Allows basic
 * inspection of manufacturer ID, device ID, and data payload.
 */

#include "Services/ui/ui_page_sysex.h"
#include "Services/ui/ui_gfx.h"
#include <stdio.h>
#include <string.h>

#define SYSEX_MAX_SIZE 128

// SysEx capture buffer
static uint8_t sysex_buffer[SYSEX_MAX_SIZE];
static uint16_t sysex_length = 0;
static uint8_t sysex_captured = 0;
static uint16_t scroll_offset = 0;

/**
 * @brief Capture a SysEx message (to be called by MIDI router)
 * 
 * Note: This is a placeholder. In a full implementation, this would be
 * called from the MIDI router when SysEx messages are received.
 */
void ui_sysex_capture(const uint8_t* data, uint16_t len) {
  if (len > SYSEX_MAX_SIZE) {
    len = SYSEX_MAX_SIZE;
  }
  
  memcpy(sysex_buffer, data, len);
  sysex_length = len;
  sysex_captured = 1;
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
  if (sysex_captured) {
    snprintf(header, sizeof(header), "SYSEX VIEWER  Captured: %u bytes", sysex_length);
  } else {
    snprintf(header, sizeof(header), "SYSEX VIEWER  Waiting for SysEx...");
  }
  ui_gfx_text(0, 0, header, 15);
  ui_gfx_rect(0, 9, 256, 1, 4);
  
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
      ui_gfx_text(0, 14, mfr_line, 12);
    }
    
    // Hex view (display rows of 16 bytes)
    ui_gfx_text(0, 24, "Hex View:", 10);
    
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
      ui_gfx_text(0, 34 + row * 8, line, 10);
    }
  } else {
    // No SysEx captured yet
    ui_gfx_text(0, 24, "No SysEx message captured.", 8);
    ui_gfx_text(0, 34, "Send a SysEx to this device to", 8);
    ui_gfx_text(0, 42, "view its contents here.", 8);
  }
  
  // Footer
  ui_gfx_rect(0, 62, 256, 1, 4);
  ui_gfx_text(0, 54, "B1 SEND  B2 RCV  B3 CLR  B4 SAVE  ENC scroll", 8);
}

/**
 * @brief Handle button press in SysEx viewer
 */
void ui_page_sysex_on_button(uint8_t id, uint8_t pressed) {
  if (!pressed) return;
  
  switch (id) {
    case 1:  // SEND (placeholder - not implemented yet)
      // Would send the captured SysEx message
      break;
      
    case 2:  // RCV (placeholder - already receiving)
      // Could toggle receive mode or reset capture
      break;
      
    case 3:  // CLEAR
      clear_sysex();
      break;
      
    case 4:  // SAVE (placeholder - not implemented yet)
      // Would save SysEx to SD card
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
