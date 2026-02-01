#pragma once
#include <stdint.h>

// Panic codes (extend as needed)
#define PANIC_HARDFAULT      (0x00000001u)
#define PANIC_MEMMANAGE      (0x00000002u)
#define PANIC_BUSFAULT       (0x00000003u)
#define PANIC_USAGEFAULT     (0x00000004u)
#define PANIC_STACK_OVERFLOW (0x00000005u)
#define PANIC_MALLOC_FAILED  (0x00000006u)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Record panic code and HALT - does NOT return!
 * 
 * IMPORTANT: Do all critical setup BEFORE calling this:
 *   1. safe_mode_set_forced(1) - so next boot enters safe mode
 *   2. NVIC_SystemReset() if you want auto-recovery
 * 
 * If you need auto-recovery, reset BEFORE calling panic_set().
 * panic_set() is for debug inspection when debugger is attached.
 */
void panic_set(uint32_t code);

/**
 * @brief Get the last panic code (survives soft reset if in retained RAM)
 */
uint32_t panic_get(void);

#ifdef __cplusplus
}
#endif
