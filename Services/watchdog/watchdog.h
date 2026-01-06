#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

// Keep call-sites simple (some handlers call watchdog_panic() with no args)
void watchdog_panic_code(uint32_t code);
#define watchdog_panic() watchdog_panic_code(0u)

#ifdef __cplusplus
}
#endif
