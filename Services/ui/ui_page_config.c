/**
 * @file ui_page_config.c
 * @brief Config Editor UI Page - SD card configuration file editor
 * 
 * Lightweight SCS-style configuration editor inspired by MIDIbox NG.
 * Allows viewing and editing module configuration parameters.
 */

#include "Services/ui/ui_page_config.h"
#include "Services/ui/ui_gfx.h"
#include "Services/config_io/config_io.h"
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

// Current state
static config_category_t current_category = CONFIG_CAT_DIN;
static uint8_t current_param = 0;
static uint8_t edit_mode = 0;
static uint8_t initialized = 0;

// Configuration data (loaded from SD card)
static config_data_t config_data;

// Status messages
static char status_msg[64] = {0};
static uint32_t status_msg_time = 0;

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
 * @brief Initialize config page (load from SD)
 */
static void init_if_needed(void) {
  if (initialized) return;
  
  config_io_init();
  
  // Try to load from SD, use defaults if fails
  if (config_io_load(&config_data) != 0) {
    config_io_get_defaults(&config_data);
    strncpy(status_msg, "Using defaults (SD load failed)", sizeof(status_msg) - 1);
    status_msg_time = 3000;  // Show for 3 seconds
  } else {
    strncpy(status_msg, "Config loaded from SD", sizeof(status_msg) - 1);
    status_msg_time = 2000;  // Show for 2 seconds
  }
  
  initialized = 1;
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
  init_if_needed();
  
  ui_gfx_clear(0);
  
  // Header
  char header[64];
  snprintf(header, sizeof(header), "CONFIG: %s", get_category_name(current_category));
  ui_gfx_text(0, 0, header, 15);
  ui_gfx_rect(0, 9, 256, 1, 4);
  
  // Category info with SD status
  char cat_info[64];
  const char* sd_status = config_io_sd_available() ? "SD:OK" : "SD:N/A";
  snprintf(cat_info, sizeof(cat_info), "[Cat %u/%u] %s %s", 
           current_category + 1, CONFIG_CAT_COUNT,
           edit_mode ? "EDIT" : "VIEW", sd_status);
  ui_gfx_text(0, 12, cat_info, 8);
  
  // Status message (if active)
  if (status_msg_time > 0) {
    if (status_msg_time > now_ms - 10000) {  // Still valid
      ui_gfx_text(0, 20, status_msg, 12);
    } else {
      status_msg_time = 0;  // Expired
      status_msg[0] = '\0';
    }
  }
  
  // Parameters
  int y = (status_msg_time > 0) ? 32 : 24;
  uint8_t param_count = get_param_count(current_category);
  
  switch (current_category) {
    case CONFIG_CAT_DIN:
      render_param_line(y, "SRIO_DIN_ENABLE", config_data.din.srio_din_enable, 
                       current_param == 0, 0);
      render_param_line(y + 8, "SRIO_DIN_BYTES", config_data.din.srio_din_bytes, 
                       current_param == 1, 0);
      render_param_line(y + 16, "DIN_INVERT_DEFAULT", config_data.din.din_invert_default, 
                       current_param == 2, 0);
      break;
      
    case CONFIG_CAT_AINSER:
      render_param_line(y, "AINSER_ENABLE", config_data.ainser.ainser_enable, 
                       current_param == 0, 0);
      render_param_line(y + 8, "AINSER_I2C_ADDR", config_data.ainser.ainser_i2c_addr, 
                       current_param == 1, 1);
      render_param_line(y + 16, "AINSER_SCAN_MS", config_data.ainser.ainser_scan_ms, 
                       current_param == 2, 0);
      break;
      
    case CONFIG_CAT_AIN:
      render_param_line(y, "AIN_VELOCITY_ENABLE", config_data.ain.ain_velocity_enable, 
                       current_param == 0, 0);
      render_param_line(y + 8, "AIN_CALIBRATE_AUTO", config_data.ain.ain_calibrate_auto, 
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
      int indicator_y = (status_msg_time > 0) ? (32 + current_param * 8) : (24 + current_param * 8);
      ui_gfx_rect(0, indicator_y, 256, 8, 2);
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
  
  init_if_needed();
  
  switch (id) {
    case 1:  // SAVE - save config to SD card
      if (config_io_save(&config_data) == 0) {
        strncpy(status_msg, "Config saved to SD", sizeof(status_msg) - 1);
      } else {
        snprintf(status_msg, sizeof(status_msg), "Save failed: %s", config_io_get_error());
      }
      status_msg_time = 2000;
      break;
      
    case 2:  // LOAD - reload config from SD card
      if (config_io_load(&config_data) == 0) {
        strncpy(status_msg, "Config reloaded from SD", sizeof(status_msg) - 1);
      } else {
        config_io_get_defaults(&config_data);
        snprintf(status_msg, sizeof(status_msg), "Load failed, using defaults");
      }
      status_msg_time = 2000;
      edit_mode = 0;  // Exit edit mode after load
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
            config_data.din.srio_din_enable = !config_data.din.srio_din_enable;
            break;
          case 1:
            if (delta > 0 && config_data.din.srio_din_bytes < 32) config_data.din.srio_din_bytes++;
            else if (delta < 0 && config_data.din.srio_din_bytes > 1) config_data.din.srio_din_bytes--;
            break;
          case 2:
            config_data.din.din_invert_default = !config_data.din.din_invert_default;
            break;
        }
        break;
        
      case CONFIG_CAT_AINSER:
        switch (current_param) {
          case 0:
            config_data.ainser.ainser_enable = !config_data.ainser.ainser_enable;
            break;
          case 1:
            if (delta > 0 && config_data.ainser.ainser_i2c_addr < 0x7F) config_data.ainser.ainser_i2c_addr++;
            else if (delta < 0 && config_data.ainser.ainser_i2c_addr > 0x08) config_data.ainser.ainser_i2c_addr--;
            break;
          case 2:
            if (delta > 0 && config_data.ainser.ainser_scan_ms < 100) config_data.ainser.ainser_scan_ms++;
            else if (delta < 0 && config_data.ainser.ainser_scan_ms > 1) config_data.ainser.ainser_scan_ms--;
            break;
        }
        break;
        
      case CONFIG_CAT_AIN:
        switch (current_param) {
          case 0:
            config_data.ain.ain_velocity_enable = !config_data.ain.ain_velocity_enable;
            break;
          case 1:
            config_data.ain.ain_calibrate_auto = !config_data.ain.ain_calibrate_auto;
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
