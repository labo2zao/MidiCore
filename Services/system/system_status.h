#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void system_set_sd_required(uint8_t required);
void system_set_sd_ok(uint8_t ok);
uint8_t system_is_sd_required(void);
uint8_t system_is_sd_ok(void);
uint8_t system_is_fatal(void);

#ifdef __cplusplus
}
#endif
