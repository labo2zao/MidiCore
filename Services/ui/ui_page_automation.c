/**
 * @file ui_page_automation.c
 * @brief Automation System UI Page - Scene chaining and workflow automation
 * 
 * Provides UI for configuring and controlling automation features:
 * - Scene chaining (automatic scene transitions)
 * - Auto-trigger modes
 * - Workflow presets (record, perform, practice modes)
 * - Performance automation controls
 */

#include "Services/ui/ui_page_automation.h"
#include "Services/ui/ui_gfx.h"
#include "Services/looper/looper.h"
#include <stdio.h>
#include <string.h>

// Automation modes
typedef enum {
  AUTO_MODE_OFF = 0,
  AUTO_MODE_SCENE_CHAIN,
  AUTO_MODE_WORKFLOW,
  AUTO_MODE_CUSTOM,
  AUTO_MODE_COUNT
} automation_mode_t;

// Workflow presets
typedef enum {
  WORKFLOW_RECORD = 0,
  WORKFLOW_PERFORM,
  WORKFLOW_PRACTICE,
  WORKFLOW_JAM,
  WORKFLOW_COUNT
} workflow_preset_t;

// Current state
static automation_mode_t current_mode = AUTO_MODE_OFF;
static workflow_preset_t current_workflow = WORKFLOW_RECORD;
static uint8_t scene_chain_enabled = 0;
static uint8_t chain_from_scene = 0;  // Scene A-H (0-7)
static uint8_t chain_to_scene = 1;    // Scene A-H (0-7)
static uint8_t auto_trigger_enabled = 0;
static uint8_t cursor_pos = 0;  // 0-6: mode, workflow, chain toggle, from scene, to scene, trigger, apply

/**
 * @brief Render the automation page
 * 
 * Layout:
 * - Header: "AUTOMATION SYSTEM"
 * - Mode selection: OFF / SCENE_CHAIN / WORKFLOW / CUSTOM
 * - Scene Chain settings: Enable, From→To
 * - Workflow presets: RECORD / PERFORM / PRACTICE / JAM
 * - Auto-trigger settings
 * - Footer: Button hints
 */
void ui_page_automation_render(uint32_t now_ms) {
  (void)now_ms;
  
  looper_transport_t tp;
  looper_get_transport(&tp);
  
  ui_gfx_clear(0);
  
  // Header
  char header[64];
  snprintf(header, sizeof(header), "AUTOMATION  BPM:%3u  [%s]", 
           tp.bpm, tp.playing ? "PLAY" : "STOP");
  ui_gfx_text(0, 0, header, 15);
  ui_gfx_rect(0, 9, 256, 1, 4);
  
  // Mode selection
  ui_gfx_text(0, 14, "Mode:", cursor_pos == 0 ? 15 : 10);
  const char* mode_names[] = {"OFF", "SCENE_CHAIN", "WORKFLOW", "CUSTOM"};
  ui_gfx_text(40, 14, mode_names[current_mode], cursor_pos == 0 ? 15 : 12);
  
  // Scene Chain Configuration
  ui_gfx_text(0, 24, "Scene Chain:", 10);
  
  // Chain enable toggle
  ui_gfx_text(4, 32, "Enable:", cursor_pos == 2 ? 15 : 8);
  ui_gfx_text(50, 32, scene_chain_enabled ? "ON " : "OFF", 
              scene_chain_enabled ? 15 : 8);
  
  // From → To scene
  if (scene_chain_enabled || current_mode == AUTO_MODE_SCENE_CHAIN) {
    char chain_text[32];
    snprintf(chain_text, sizeof(chain_text), "From: %c", 'A' + chain_from_scene);
    ui_gfx_text(4, 40, chain_text, cursor_pos == 3 ? 15 : 10);
    
    snprintf(chain_text, sizeof(chain_text), "To: %c", 'A' + chain_to_scene);
    ui_gfx_text(60, 40, chain_text, cursor_pos == 4 ? 15 : 10);
    
    // Visual chain indicator
    ui_gfx_text(100, 40, "->", 12);
  }
  
  // Workflow Presets
  ui_gfx_text(140, 24, "Workflow:", 10);
  const char* workflow_names[] = {"RECORD", "PERFORM", "PRACTICE", "JAM"};
  ui_gfx_text(140, 32, workflow_names[current_workflow], 
              cursor_pos == 1 ? 15 : 10);
  
  // Auto-trigger settings
  ui_gfx_rect(0, 49, 256, 1, 3);
  ui_gfx_text(0, 51, "Auto-Trigger:", cursor_pos == 5 ? 15 : 8);
  ui_gfx_text(80, 51, auto_trigger_enabled ? "ENABLED " : "DISABLED", 
              auto_trigger_enabled ? 15 : 6);
  
  // Status indicator
  if (current_mode != AUTO_MODE_OFF) {
    ui_gfx_text(160, 51, "[ACTIVE]", 13);
  }
  
  // Footer with button hints
  ui_gfx_rect(0, 62, 256, 1, 4);
  ui_gfx_text(0, 54, "B1 APPLY  B2 RESET  B3 TEST  B4 SAVE  ENC nav", 6);
}

/**
 * @brief Handle button press in automation mode
 */
void ui_page_automation_on_button(uint8_t id, uint8_t pressed) {
  if (!pressed) return;
  
  switch (id) {
    case 1:  // APPLY - Apply current automation settings
      if (current_mode == AUTO_MODE_SCENE_CHAIN && scene_chain_enabled) {
        // Configure scene chaining in looper
        looper_set_scene_chain(chain_from_scene, chain_to_scene, auto_trigger_enabled);
      }
      break;
      
    case 2:  // RESET - Reset automation to defaults
      current_mode = AUTO_MODE_OFF;
      scene_chain_enabled = 0;
      auto_trigger_enabled = 0;
      chain_from_scene = 0;
      chain_to_scene = 1;
      current_workflow = WORKFLOW_RECORD;
      looper_set_scene_chain(0, 0, 0);  // Disable chaining
      break;
      
    case 3:  // TEST - Test current automation configuration
      if (current_mode == AUTO_MODE_SCENE_CHAIN && scene_chain_enabled) {
        // Trigger scene transition for testing
        looper_trigger_scene(chain_from_scene);
      }
      break;
      
    case 4:  // SAVE - Save automation configuration (would save to SD card)
      // TODO: Implement save to SD card config
      break;
      
    case 5:  // PAGE - Navigate to next page
      // Handled by main UI navigation
      break;
  }
}

/**
 * @brief Handle encoder rotation in automation mode
 */
void ui_page_automation_on_encoder(int8_t delta) {
  if (delta == 0) return;
  
  switch (cursor_pos) {
    case 0:  // Mode selection
      if (delta > 0 && current_mode < AUTO_MODE_COUNT - 1) {
        current_mode++;
        if (current_mode == AUTO_MODE_SCENE_CHAIN) {
          scene_chain_enabled = 1;
        }
      } else if (delta < 0 && current_mode > 0) {
        current_mode--;
      }
      break;
      
    case 1:  // Workflow preset
      if (delta > 0 && current_workflow < WORKFLOW_COUNT - 1) {
        current_workflow++;
      } else if (delta < 0 && current_workflow > 0) {
        current_workflow--;
      }
      break;
      
    case 2:  // Chain enable toggle
      scene_chain_enabled = !scene_chain_enabled;
      break;
      
    case 3:  // From scene
      if (delta > 0 && chain_from_scene < 7) {
        chain_from_scene++;
        if (chain_from_scene >= chain_to_scene) {
          chain_to_scene = chain_from_scene + 1;
          if (chain_to_scene > 7) chain_to_scene = 7;
        }
      } else if (delta < 0 && chain_from_scene > 0) {
        chain_from_scene--;
      }
      break;
      
    case 4:  // To scene
      if (delta > 0 && chain_to_scene < 7) {
        chain_to_scene++;
      } else if (delta < 0 && chain_to_scene > chain_from_scene + 1) {
        chain_to_scene--;
      }
      break;
      
    case 5:  // Auto-trigger toggle
      auto_trigger_enabled = !auto_trigger_enabled;
      break;
      
    case 6:  // Apply button (encoder can trigger)
      // Apply settings
      if (current_mode == AUTO_MODE_SCENE_CHAIN && scene_chain_enabled) {
        looper_set_scene_chain(chain_from_scene, chain_to_scene, auto_trigger_enabled);
      }
      // Move back to mode selection
      cursor_pos = 0;
      return;
  }
  
  // Cycle cursor position (0-6)
  if (delta > 0) {
    cursor_pos = (cursor_pos + 1) % 7;
  } else {
    cursor_pos = (cursor_pos == 0) ? 6 : cursor_pos - 1;
  }
}
