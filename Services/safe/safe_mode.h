#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  SAFE_REASON_NONE = 0,
  SAFE_REASON_FORCED_SHIFT = 1,
  SAFE_REASON_CFG = 2,
  SAFE_REASON_NO_SD = 3
} safe_reason_t;

void safe_mode_set_forced(uint8_t forced);
void safe_mode_set_cfg(uint8_t enabled);
void safe_mode_set_sd_ok(uint8_t sd_ok);

uint8_t safe_mode_is_enabled(void);
safe_reason_t safe_mode_reason(void);
const char* safe_mode_reason_str(void);

#ifdef __cplusplus
}
#endif
