#pragma once

// ============================================================================
// DEBUG/TEST ONLY - Creates the MIDI DIN debug monitoring task
// Only compiled when DEBUG_MIDI_DIN_MONITOR=1 in Config/project_config.h
// NOT NEEDED FOR PRODUCTION
// ============================================================================
void midi_din_debug_task_create(void);
