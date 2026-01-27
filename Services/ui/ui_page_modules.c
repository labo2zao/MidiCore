/**
 * @file ui_page_modules.c
 * @brief Module Control UI Page Implementation
 */

#include "ui_page_modules.h"
#include "ui_gfx.h"
#include "Services/module_registry/module_registry.h"
#include "Services/config/runtime_config.h"
#include <stdio.h>
#include <string.h>

// =============================================================================
// UI STATE MACHINE
// =============================================================================

typedef enum {
  UI_STATE_CATEGORY_LIST,    // Browsing categories
  UI_STATE_MODULE_LIST,       // Browsing modules in category
  UI_STATE_MODULE_INFO,       // Viewing module details
  UI_STATE_PARAM_LIST,        // Browsing module parameters
  UI_STATE_PARAM_EDIT         // Editing a parameter value
} ui_state_t;

// =============================================================================
// UI STATE
// =============================================================================

static ui_state_t s_state = UI_STATE_CATEGORY_LIST;
static module_category_t s_current_category = MODULE_CATEGORY_EFFECT;
static uint8_t s_category_index = 0;
static uint8_t s_module_index = 0;
static uint8_t s_param_index = 0;
static uint8_t s_track = 0;  // Current track for per-track modules
static uint8_t s_initialized = 0;

// Current module being viewed/edited
static const module_descriptor_t* s_current_module = NULL;

// Module list for current category
static const module_descriptor_t* s_module_list[MODULE_REGISTRY_MAX_MODULES];
static uint8_t s_module_count = 0;

// Scroll position for lists
static uint8_t s_scroll_offset = 0;

// Status message
static char s_status_msg[64] = {0};
static uint32_t s_status_msg_time = 0;

// =============================================================================
// CATEGORY HELPERS
// =============================================================================

static const module_category_t s_categories[] = {
  MODULE_CATEGORY_SYSTEM,
  MODULE_CATEGORY_MIDI,
  MODULE_CATEGORY_INPUT,
  MODULE_CATEGORY_OUTPUT,
  MODULE_CATEGORY_EFFECT,
  MODULE_CATEGORY_GENERATOR,
  MODULE_CATEGORY_LOOPER,
  MODULE_CATEGORY_UI,
  MODULE_CATEGORY_ACCORDION,
  MODULE_CATEGORY_OTHER
};

#define NUM_CATEGORIES (sizeof(s_categories) / sizeof(s_categories[0]))

static void update_module_list(void) {
  s_module_count = module_registry_list_by_category(
    s_current_category,
    s_module_list,
    MODULE_REGISTRY_MAX_MODULES
  );
  s_module_index = 0;
  s_scroll_offset = 0;
}

// =============================================================================
// NAVIGATION HELPERS
// =============================================================================

static void navigate_back(void) {
  switch (s_state) {
    case UI_STATE_CATEGORY_LIST:
      // Already at top level
      break;
      
    case UI_STATE_MODULE_LIST:
      s_state = UI_STATE_CATEGORY_LIST;
      s_scroll_offset = 0;
      break;
      
    case UI_STATE_MODULE_INFO:
      s_state = UI_STATE_MODULE_LIST;
      break;
      
    case UI_STATE_PARAM_LIST:
      s_state = UI_STATE_MODULE_INFO;
      s_scroll_offset = 0;
      break;
      
    case UI_STATE_PARAM_EDIT:
      s_state = UI_STATE_PARAM_LIST;
      break;
  }
}

static void navigate_forward(void) {
  switch (s_state) {
    case UI_STATE_CATEGORY_LIST:
      // Enter category
      s_current_category = s_categories[s_category_index];
      update_module_list();
      s_state = UI_STATE_MODULE_LIST;
      break;
      
    case UI_STATE_MODULE_LIST:
      // Enter module
      if (s_module_count > 0 && s_module_index < s_module_count) {
        s_current_module = s_module_list[s_module_index];
        s_state = UI_STATE_MODULE_INFO;
        s_scroll_offset = 0;
      }
      break;
      
    case UI_STATE_MODULE_INFO:
      // Enter parameters
      if (s_current_module && s_current_module->param_count > 0) {
        s_state = UI_STATE_PARAM_LIST;
        s_param_index = 0;
        s_scroll_offset = 0;
      }
      break;
      
    case UI_STATE_PARAM_LIST:
      // Enter parameter edit
      if (s_current_module && s_param_index < s_current_module->param_count) {
        const module_param_t* param = &s_current_module->params[s_param_index];
        if (!param->read_only) {
          s_state = UI_STATE_PARAM_EDIT;
        }
      }
      break;
      
    case UI_STATE_PARAM_EDIT:
      // Exit edit mode
      s_state = UI_STATE_PARAM_LIST;
      break;
  }
}

// =============================================================================
// RENDERING
// =============================================================================

static void render_header(int w, int h) {
  // Draw header background
  ui_gfx_fill_rect(0, 0, w, 10, 3);
  
  // Draw title based on state
  const char* title = "Modules";
  switch (s_state) {
    case UI_STATE_CATEGORY_LIST:
      title = "Module Categories";
      break;
    case UI_STATE_MODULE_LIST:
      title = module_registry_category_to_string(s_current_category);
      break;
    case UI_STATE_MODULE_INFO:
      title = s_current_module ? s_current_module->name : "Module";
      break;
    case UI_STATE_PARAM_LIST:
      title = "Parameters";
      break;
    case UI_STATE_PARAM_EDIT:
      title = "Edit Parameter";
      break;
  }
  
  ui_gfx_text(2, 2, title, 15);
}

static void render_footer(int w, int h) {
  // Draw footer background
  ui_gfx_fill_rect(0, h - 10, w, 10, 3);
  
  // Draw navigation hints
  const char* hint = "";
  switch (s_state) {
    case UI_STATE_CATEGORY_LIST:
      hint = "[Enc:Select] [Btn:Enter]";
      break;
    case UI_STATE_MODULE_LIST:
      hint = "[Enc:Select] [Btn:Info] [Back]";
      break;
    case UI_STATE_MODULE_INFO:
      hint = "[Btn1:Enable] [Btn2:Params] [Back]";
      break;
    case UI_STATE_PARAM_LIST:
      hint = "[Enc:Select] [Btn:Edit] [Back]";
      break;
    case UI_STATE_PARAM_EDIT:
      hint = "[Enc2:Value] [Btn:Done]";
      break;
  }
  
  ui_gfx_text(2, h - 8, hint, 12);
}

static void render_category_list(int w, int h) {
  int y = 14;
  int line_height = 10;
  int visible_items = (h - 24) / line_height;
  
  // Adjust scroll offset
  if (s_category_index < s_scroll_offset) {
    s_scroll_offset = s_category_index;
  }
  if (s_category_index >= s_scroll_offset + visible_items) {
    s_scroll_offset = s_category_index - visible_items + 1;
  }
  
  // Render visible categories
  for (uint8_t i = s_scroll_offset; i < NUM_CATEGORIES && y < h - 14; i++) {
    const char* name = module_registry_category_to_string(s_categories[i]);
    uint8_t is_selected = (i == s_category_index);
    uint8_t gray = is_selected ? 15 : 10;
    
    if (is_selected) {
      ui_gfx_text(2, y, ">", 15);
    }
    
    ui_gfx_text(12, y, name, gray);
    y += line_height;
  }
  
  // Draw scrollbar if needed
  if (NUM_CATEGORIES > visible_items) {
    int sb_h = (h - 24) * visible_items / NUM_CATEGORIES;
    int sb_y = 14 + (h - 24) * s_scroll_offset / NUM_CATEGORIES;
    ui_gfx_fill_rect(w - 3, sb_y, 2, sb_h, 8);
  }
}

static void render_module_list(int w, int h) {
  int y = 14;
  int line_height = 10;
  int visible_items = (h - 24) / line_height;
  
  if (s_module_count == 0) {
    ui_gfx_text(12, y, "(no modules)", 10);
    return;
  }
  
  // Adjust scroll offset
  if (s_module_index < s_scroll_offset) {
    s_scroll_offset = s_module_index;
  }
  if (s_module_index >= s_scroll_offset + visible_items) {
    s_scroll_offset = s_module_index - visible_items + 1;
  }
  
  // Render visible modules
  for (uint8_t i = s_scroll_offset; i < s_module_count && y < h - 14; i++) {
    const module_descriptor_t* mod = s_module_list[i];
    uint8_t is_selected = (i == s_module_index);
    uint8_t gray = is_selected ? 15 : 10;
    
    if (is_selected) {
      ui_gfx_text(2, y, ">", 15);
    }
    
    // Show module name and status
    module_status_t status = module_registry_get_status(mod->name, 0xFF);
    const char* status_str = (status == MODULE_STATUS_ENABLED) ? "[ON]" : "[OFF]";
    
    char line[64];
    snprintf(line, sizeof(line), "%-18s %s", mod->name, status_str);
    ui_gfx_text(12, y, line, gray);
    
    y += line_height;
  }
  
  // Draw scrollbar if needed
  if (s_module_count > visible_items) {
    int sb_h = (h - 24) * visible_items / s_module_count;
    int sb_y = 14 + (h - 24) * s_scroll_offset / s_module_count;
    ui_gfx_fill_rect(w - 3, sb_y, 2, sb_h, 8);
  }
}

static void render_module_info(int w, int h) {
  if (!s_current_module) return;
  
  int y = 14;
  int line_height = 10;
  
  // Module description
  ui_gfx_text(2, y, s_current_module->description, 12);
  y += line_height + 2;
  
  // Status
  module_status_t status = module_registry_get_status(s_current_module->name, s_track);
  const char* status_str = "";
  switch (status) {
    case MODULE_STATUS_DISABLED: status_str = "Disabled"; break;
    case MODULE_STATUS_ENABLED: status_str = "Enabled"; break;
    case MODULE_STATUS_ERROR: status_str = "Error"; break;
  }
  
  char line[64];
  snprintf(line, sizeof(line), "Status: %s", status_str);
  ui_gfx_text(2, y, line, 10);
  y += line_height;
  
  // Global/Per-track
  const char* scope = s_current_module->is_global ? "Global" : "Per-track";
  snprintf(line, sizeof(line), "Scope: %s", scope);
  ui_gfx_text(2, y, line, 10);
  y += line_height;
  
  // Parameter count
  snprintf(line, sizeof(line), "Parameters: %d", s_current_module->param_count);
  ui_gfx_text(2, y, line, 10);
  y += line_height + 2;
  
  // Actions
  ui_gfx_text(2, y, "[1] Toggle Enable", 12);
  y += line_height;
  ui_gfx_text(2, y, "[2] View Parameters", 12);
}

static void render_param_list(int w, int h) {
  if (!s_current_module) return;
  
  int y = 14;
  int line_height = 10;
  int visible_items = (h - 24) / line_height;
  
  if (s_current_module->param_count == 0) {
    ui_gfx_text(12, y, "(no parameters)", 10);
    return;
  }
  
  // Adjust scroll offset
  if (s_param_index < s_scroll_offset) {
    s_scroll_offset = s_param_index;
  }
  if (s_param_index >= s_scroll_offset + visible_items) {
    s_scroll_offset = s_param_index - visible_items + 1;
  }
  
  // Render visible parameters
  for (uint8_t i = s_scroll_offset; i < s_current_module->param_count && y < h - 14; i++) {
    const module_param_t* param = &s_current_module->params[i];
    uint8_t is_selected = (i == s_param_index);
    uint8_t gray = is_selected ? 15 : 10;
    
    if (is_selected) {
      ui_gfx_text(2, y, ">", 15);
    }
    
    // Get current value
    param_value_t value;
    char value_str[32] = "?";
    if (param->get_value && param->get_value(s_track, &value) == 0) {
      switch (param->type) {
        case PARAM_TYPE_BOOL:
          snprintf(value_str, sizeof(value_str), "%s", value.bool_val ? "ON" : "OFF");
          break;
        case PARAM_TYPE_INT:
        case PARAM_TYPE_ENUM:
          if (param->type == PARAM_TYPE_ENUM && param->enum_values && 
              value.int_val >= 0 && value.int_val < param->enum_count) {
            snprintf(value_str, sizeof(value_str), "%s", param->enum_values[value.int_val]);
          } else {
            snprintf(value_str, sizeof(value_str), "%ld", (long)value.int_val);
          }
          break;
        case PARAM_TYPE_FLOAT:
          snprintf(value_str, sizeof(value_str), "%.2f", value.float_val);
          break;
        case PARAM_TYPE_STRING:
          snprintf(value_str, sizeof(value_str), "%s", 
                  value.string_val ? value.string_val : "");
          break;
      }
    }
    
    char line[64];
    snprintf(line, sizeof(line), "%-16s: %s%s", 
             param->name, value_str, param->read_only ? " (RO)" : "");
    ui_gfx_text(12, y, line, gray);
    
    y += line_height;
  }
  
  // Draw scrollbar if needed
  if (s_current_module->param_count > visible_items) {
    int sb_h = (h - 24) * visible_items / s_current_module->param_count;
    int sb_y = 14 + (h - 24) * s_scroll_offset / s_current_module->param_count;
    ui_gfx_fill_rect(w - 3, sb_y, 2, sb_h, 8);
  }
}

static void render_param_edit(int w, int h) {
  if (!s_current_module || s_param_index >= s_current_module->param_count) return;
  
  const module_param_t* param = &s_current_module->params[s_param_index];
  
  int y = 14;
  int line_height = 12;
  
  // Parameter name
  ui_gfx_text(2, y, param->name, 15);
  y += line_height;
  
  // Description
  ui_gfx_text(2, y, param->description, 10);
  y += line_height + 4;
  
  // Current value (large)
  param_value_t value;
  char value_str[64] = "?";
  if (param->get_value && param->get_value(s_track, &value) == 0) {
    switch (param->type) {
      case PARAM_TYPE_BOOL:
        snprintf(value_str, sizeof(value_str), "%s", value.bool_val ? "ON" : "OFF");
        break;
      case PARAM_TYPE_INT:
      case PARAM_TYPE_ENUM:
        if (param->type == PARAM_TYPE_ENUM && param->enum_values && 
            value.int_val >= 0 && value.int_val < param->enum_count) {
          snprintf(value_str, sizeof(value_str), "%s", param->enum_values[value.int_val]);
        } else {
          snprintf(value_str, sizeof(value_str), "%ld", (long)value.int_val);
        }
        break;
      case PARAM_TYPE_FLOAT:
        snprintf(value_str, sizeof(value_str), "%.2f", value.float_val);
        break;
      case PARAM_TYPE_STRING:
        snprintf(value_str, sizeof(value_str), "%s", 
                value.string_val ? value.string_val : "");
        break;
    }
  }
  
  // Draw value in center
  ui_gfx_text(w/2 - 20, h/2 - 6, value_str, 15);
  
  // Range indicator for int/float
  if (param->type == PARAM_TYPE_INT || param->type == PARAM_TYPE_FLOAT) {
    y = h - 24;
    char range[32];
    snprintf(range, sizeof(range), "Range: %ld - %ld", 
             (long)param->min, (long)param->max);
    ui_gfx_text(2, y, range, 8);
  }
}

static void render_status_message(int w, int h) {
  if (s_status_msg_time > 0) {
    int y = h / 2 - 6;
    ui_gfx_fill_rect(10, y - 2, w - 20, 14, 5);
    ui_gfx_rect(10, y - 2, w - 20, 14, 15);
    ui_gfx_text(14, y, s_status_msg, 15);
  }
}

// =============================================================================
// PUBLIC API
// =============================================================================

void ui_page_modules_init(void) {
  if (s_initialized) return;
  
  s_state = UI_STATE_CATEGORY_LIST;
  s_category_index = 0;
  s_module_index = 0;
  s_param_index = 0;
  s_track = 0;
  s_scroll_offset = 0;
  s_current_module = NULL;
  s_module_count = 0;
  
  s_initialized = 1;
}

void ui_page_modules_render(uint8_t* fb, int w, int h) {
  // Clear background
  ui_gfx_clear(0);
  
  // Render header
  render_header(w, h);
  
  // Render content based on state
  switch (s_state) {
    case UI_STATE_CATEGORY_LIST:
      render_category_list(w, h);
      break;
    case UI_STATE_MODULE_LIST:
      render_module_list(w, h);
      break;
    case UI_STATE_MODULE_INFO:
      render_module_info(w, h);
      break;
    case UI_STATE_PARAM_LIST:
      render_param_list(w, h);
      break;
    case UI_STATE_PARAM_EDIT:
      render_param_edit(w, h);
      break;
  }
  
  // Render footer
  render_footer(w, h);
  
  // Render status message overlay
  render_status_message(w, h);
}

void ui_page_modules_on_encoder(uint8_t enc_id, int8_t delta) {
  if (delta == 0) return;
  
  if (enc_id == 0) {
    // Encoder 1: Navigation
    switch (s_state) {
      case UI_STATE_CATEGORY_LIST:
        if (delta > 0 && s_category_index < NUM_CATEGORIES - 1) {
          s_category_index++;
        } else if (delta < 0 && s_category_index > 0) {
          s_category_index--;
        }
        break;
        
      case UI_STATE_MODULE_LIST:
        if (delta > 0 && s_module_index < s_module_count - 1) {
          s_module_index++;
        } else if (delta < 0 && s_module_index > 0) {
          s_module_index--;
        }
        break;
        
      case UI_STATE_PARAM_LIST:
        if (s_current_module) {
          if (delta > 0 && s_param_index < s_current_module->param_count - 1) {
            s_param_index++;
          } else if (delta < 0 && s_param_index > 0) {
            s_param_index--;
          }
        }
        break;
        
      case UI_STATE_MODULE_INFO:
      case UI_STATE_PARAM_EDIT:
        // Encoder 1 not used in these states
        break;
    }
  } else if (enc_id == 1) {
    // Encoder 2: Parameter editing
    if (s_state == UI_STATE_PARAM_EDIT && s_current_module) {
      if (s_param_index >= s_current_module->param_count) return;
      
      const module_param_t* param = &s_current_module->params[s_param_index];
      if (param->read_only || !param->set_value) return;
      
      // Get current value
      param_value_t value;
      if (param->get_value && param->get_value(s_track, &value) != 0) return;
      
      // Modify value based on type
      switch (param->type) {
        case PARAM_TYPE_BOOL:
          value.bool_val = !value.bool_val;
          break;
          
        case PARAM_TYPE_INT:
        case PARAM_TYPE_ENUM:
          value.int_val += delta;
          if (value.int_val < param->min) value.int_val = param->min;
          if (value.int_val > param->max) value.int_val = param->max;
          break;
          
        case PARAM_TYPE_FLOAT:
          value.float_val += delta * 0.1f;
          if (value.float_val < (float)param->min) value.float_val = (float)param->min;
          if (value.float_val > (float)param->max) value.float_val = (float)param->max;
          break;
          
        case PARAM_TYPE_STRING:
          // String editing not supported via encoder
          break;
      }
      
      // Set new value
      if (param->set_value(s_track, &value) == 0) {
        snprintf(s_status_msg, sizeof(s_status_msg), "Value updated");
        s_status_msg_time = 1000;
      }
    }
  }
}

void ui_page_modules_on_button(uint8_t btn_id, uint8_t pressed) {
  if (!pressed) return;  // Only act on button press
  
  switch (btn_id) {
    case 0:  // Button 1 (encoder 1 button or separate button)
      if (s_state == UI_STATE_PARAM_EDIT) {
        // Exit edit mode
        navigate_back();
      } else {
        // Enter/select
        navigate_forward();
      }
      break;
      
    case 1:  // Button 2
      if (s_state == UI_STATE_MODULE_INFO && s_current_module) {
        // Toggle enable/disable
        module_status_t status = module_registry_get_status(s_current_module->name, s_track);
        if (status == MODULE_STATUS_ENABLED) {
          module_registry_disable(s_current_module->name, s_track);
          snprintf(s_status_msg, sizeof(s_status_msg), "%s disabled", s_current_module->name);
        } else {
          module_registry_enable(s_current_module->name, s_track);
          snprintf(s_status_msg, sizeof(s_status_msg), "%s enabled", s_current_module->name);
        }
        s_status_msg_time = 2000;
      } else {
        // Back/cancel
        navigate_back();
      }
      break;
      
    case 2:  // Button 3
      if (s_state != UI_STATE_CATEGORY_LIST) {
        // Save configuration
        if (runtime_config_save("0:/modules.ini") == 0) {
          snprintf(s_status_msg, sizeof(s_status_msg), "Config saved");
          s_status_msg_time = 2000;
        }
      }
      break;
      
    case 3:  // Button 4
      if (s_state != UI_STATE_CATEGORY_LIST) {
        // Load configuration
        if (runtime_config_load("0:/modules.ini") == 0) {
          snprintf(s_status_msg, sizeof(s_status_msg), "Config loaded");
          s_status_msg_time = 2000;
        }
      }
      break;
  }
}

void ui_page_modules_tick(void) {
  // Decrement status message timer
  if (s_status_msg_time > 0) {
    if (s_status_msg_time > 20) {
      s_status_msg_time -= 20;
    } else {
      s_status_msg_time = 0;
    }
  }
}
