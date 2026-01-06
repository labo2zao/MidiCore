#pragma once
#include <stdint.h>

// Panic codes (extend as needed)
#define PANIC_HARDFAULT   (0x00000001u)
#define PANIC_MEMMANAGE   (0x00000002u)
#define PANIC_BUSFAULT    (0x00000003u)
#define PANIC_USAGEFAULT  (0x00000004u)
#ifdef __cplusplus
extern "C" {
#endif
void panic_set(uint32_t code);
#ifdef __cplusplus
}
#endif
