#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file ui_page_automation.h
 * @brief Automation System UI Page - Scene chaining and workflow automation
 * 
 * Displays automation configuration including:
 * - Scene chaining setup (A→B→C chains)
 * - Auto-trigger settings
 * - Workflow presets
 * - Performance automation controls
 */

void ui_page_automation_render(uint32_t now_ms);
void ui_page_automation_on_button(uint8_t id, uint8_t pressed);
void ui_page_automation_on_encoder(int8_t delta);

#ifdef __cplusplus
}
#endif
