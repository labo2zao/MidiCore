#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  BOOT_REASON_UNKNOWN = 0,
  BOOT_REASON_POWERON = 1,
  BOOT_REASON_SOFTWARE = 2,
  BOOT_REASON_WATCHDOG = 3,
  BOOT_REASON_BROWNOUT = 4
} boot_reason_t;

void boot_reason_init(void);
boot_reason_t boot_reason_get(void);

#ifdef __cplusplus
}
#endif
