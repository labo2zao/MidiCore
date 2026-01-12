/**
 * @file ui_page_config.c
 * @brief Config Editor UI Page - SD card configuration file editor
 * 
 * Lightweight SCS-style configuration editor inspired by MIDIbox NG.
 * Allows viewing and editing module configuration parameters.
 */

#include "Services/ui/ui_page_config.h"
#include "Services/ui/ui_gfx.h"
#include <stdio.h>
#include <string.h>

// Configuration categories
typedef enum {
  CONFIG_CAT_DIN = 0,
  CONFIG_CAT_AINSER,
  CONFIG_CAT_AIN,
  CONFIG_CAT_SYSTEM,
  CONFIG_CAT_COUNT
} config_category_t;

// Mock configuration values (in real implementation, load from SD)
typedef struct {
  uint8_t srio_din_enable;
  uint8_t srio_din_bytes;
  uint8_t din_invert_default;
} din_config_t;

typedef struct {
  uint8_t ainser_enable;
  uint8_t ainser_i2c_addr;
  uint8_t ainser_scan_ms;
} ainser_config_t;

typedef struct {
  uint8_t ain_velocity_enable;
  uint8_t ain_calibrate_auto;
} ain_config_t;

// Current state
static config_category_t current_category = CONFIG_CAT_DIN;
static uint8_t current_param = 0;
static uint8_t edit_mode = 0;

// Mock config data (would be loaded from SD in real implementation)
static din_config_t din_cfg = {1, 8, 0};
static ainser_config_t ainser_cfg = {1, 0x48, 5};
static ain_config_t ain_cfg = {1, 1};

/**
 * @brief Get category name
 */
static const char* get_category_name(config_category_t cat) {
  switch (cat) {
    case CONFIG_CAT_DIN: return "DIN Module";
    case CONFIG_CAT_AINSER: return "AINSER Module";
    case CONFIG_CAT_AIN: return "AIN Module";
    case CONFIG_CAT_SYSTEM: return "System";
    default: return "Unknown";
  }
}

/**
 * @brief Get number of parameters in current category
 */
static uint8_t get_param_count(config_category_t cat) {
  switch (cat) {
    case CONFIG_CAT_DIN: return 3;
    case CONFIG_CAT_AINSER: return 3;
    case CONFIG_CAT_AIN: return 2;
    case CONFIG_CAT_SYSTEM: return 0;
    default: return 0;
  }
}

/**
 * @brief Render parameter line
 */
static void render_param_line(int y, const char* name, uint8_t value, 
                               uint8_t is_selected, uint8_t is_hex) {
  char line[64];
  if (is_hex) {
    snprintf(line, sizeof(line), "%-20s = 0x%02X", name, value);
  } else {
    snprintf(line, sizeof(line), "%-20s = %u", name, value);
  }
  
  uint8_t gray = is_selected ? 15 : 10;
  if (is_selected && edit_mode) {
    ui_gfx_text(0, y, ">", 15);
  }
  ui_gfx_text(12, y, line, gray);
}

/**
 * @brief Render the config editor page
 */
void ui_page_config_render(uint32_t now_ms) {
  (void)now_ms;
  
  ui_gfx_clear(0);
  
  // Header
  char header[64];
  snprintf(header, sizeof(header), "CONFIG: %s", get_category_name(current_category));
  ui_gfx_text(0, 0, header, 15);
  ui_gfx_rect(0, 9, 256, 1, 4);
  
  // Category info
  char cat_info[64];
  snprintf(cat_info, sizeof(cat_info), "[Category %u/%u]  %s mode", 
           current_category + 1, CONFIG_CAT_COUNT,
           edit_mode ? "EDIT" : "VIEW");
  ui_gfx_text(0, 12, cat_info, 8);
  
  // Parameters
  int y = 24;
  uint8_t param_count = get_param_count(current_category);
  
  switch (current_category) {
    case CONFIG_CAT_DIN:
      render_param_line(y, "SRIO_DIN_ENABLE", din_cfg.srio_din_enable, 
                       current_param == 0, 0);
      render_param_line(y + 8, "SRIO_DIN_BYTES", din_cfg.srio_din_bytes, 
                       current_param == 1, 0);
      render_param_line(y + 16, "DIN_INVERT_DEFAULT", din_cfg.din_invert_default, 
                       current_param == 2, 0);
      break;
      
    case CONFIG_CAT_AINSER:
      render_param_line(y, "AINSER_ENABLE", ainser_cfg.ainser_enable, 
                       current_param == 0, 0);
      render_param_line(y + 8, "AINSER_I2C_ADDR", ainser_cfg.ainser_i2c_addr, 
                       current_param == 1, 1);
      render_param_line(y + 16, "AINSER_SCAN_MS", ainser_cfg.ainser_scan_ms, 
                       current_param == 2, 0);
      break;
      
    case CONFIG_CAT_AIN:
      render_param_line(y, "AIN_VELOCITY_ENABLE", ain_cfg.ain_velocity_enable, 
                       current_param == 0, 0);
      render_param_line(y + 8, "AIN_CALIBRATE_AUTO", ain_cfg.ain_calibrate_auto, 
                       current_param == 1, 0);
      break;
      
    case CONFIG_CAT_SYSTEM:
      ui_gfx_text(0, y, "System config (not implemented)", 8);
      break;
      
    default:
      break;
  }
  
  if (param_count > 0) {
    // Current parameter indicator
    if (current_param < param_count) {
      ui_gfx_rect(0, 24 + current_param * 8, 256, 8, 2);
    }
  }
  
  // Footer
  ui_gfx_rect(0, 62, 256, 1, 4);
  ui_gfx_text(0, 54, "B1 SAVE  B2 LOAD  B3 EDIT  B4 CAT  ENC nav", 8);
}

/**
 * @brief Handle button press in config editor
 */
void ui_page_config_on_button(uint8_t id, uint8_t pressed) {
  if (!pressed) return;
  
  switch (id) {
    case 1:  // SAVE (placeholder - not implemented yet)
      // Would save config to SD card
      break;
      
    case 2:  // LOAD (placeholder - not implemented yet)
      // Would reload config from SD card
      break;
      
    case 3:  // EDIT - toggle edit mode
      edit_mode = !edit_mode;
      break;
      
    case 4:  // CAT - cycle through categories
      current_category = (config_category_t)((current_category + 1) % CONFIG_CAT_COUNT);
      current_param = 0;
      edit_mode = 0;
      break;
      
    default:
      break;
  }
}

/**
 * @brief Handle encoder rotation in config editor
 */
void ui_page_config_on_encoder(int8_t delta) {
  uint8_t param_count = get_param_count(current_category);
  
  if (param_count == 0) return;
  
  if (edit_mode) {
    // Edit parameter value
    switch (current_category) {
      case CONFIG_CAT_DIN:
        switch (current_param) {
          case 0:
            din_cfg.srio_din_enable = (din_cfg.srio_din_enable + delta) & 1;
            break;
          case 1:
            if (delta > 0 && din_cfg.srio_din_bytes < 32) din_cfg.srio_din_bytes++;
            else if (delta < 0 && din_cfg.srio_din_bytes > 1) din_cfg.srio_din_bytes--;
            break;
          case 2:
            din_cfg.din_invert_default = (din_cfg.din_invert_default + delta) & 1;
            break;
        }
        break;
        
      case CONFIG_CAT_AINSER:
        switch (current_param) {
          case 0:
            ainser_cfg.ainser_enable = (ainser_cfg.ainser_enable + delta) & 1;
            break;
          case 1:
            if (delta > 0 && ainser_cfg.ainser_i2c_addr < 0x7F) ainser_cfg.ainser_i2c_addr++;
            else if (delta < 0 && ainser_cfg.ainser_i2c_addr > 0x08) ainser_cfg.ainser_i2c_addr--;
            break;
          case 2:
            if (delta > 0 && ainser_cfg.ainser_scan_ms < 100) ainser_cfg.ainser_scan_ms++;
            else if (delta < 0 && ainser_cfg.ainser_scan_ms > 1) ainser_cfg.ainser_scan_ms--;
            break;
        }
        break;
        
      case CONFIG_CAT_AIN:
        switch (current_param) {
          case 0:
            ain_cfg.ain_velocity_enable = (ain_cfg.ain_velocity_enable + delta) & 1;
            break;
          case 1:
            ain_cfg.ain_calibrate_auto = (ain_cfg.ain_calibrate_auto + delta) & 1;
            break;
        }
        break;
        
      default:
        break;
    }
  } else {
    // Navigate parameters
    if (delta > 0) {
      current_param = (current_param + 1) % param_count;
    } else if (delta < 0) {
      current_param = (current_param + param_count - 1) % param_count;
    }
  }
}
