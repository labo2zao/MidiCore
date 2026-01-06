#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void sd_guard_reset(void);
void sd_guard_note_write_error(void);
uint8_t sd_guard_is_readonly(void);
uint8_t sd_guard_error_count(void);

#ifdef __cplusplus
}
#endif
