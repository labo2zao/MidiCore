#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void ui_state_mark_dirty(void);

/** Called periodically (20ms). Handles one-time load and deferred saves. */
void ui_state_tick_20ms(void);

#ifdef __cplusplus
}
#endif
