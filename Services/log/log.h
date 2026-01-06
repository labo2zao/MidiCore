#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void log_init(void);
void log_printf(const char* tag, const char* fmt, ...);

/** Flush buffered lines to SD if allowed (not SAFE, not SD RO). */
void log_flush(void);

#ifdef __cplusplus
}
#endif
