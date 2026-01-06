#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void safe_mode_set_forced(uint8_t on);
uint8_t safe_mode_is_forced(void);
#ifdef __cplusplus
}
#endif
