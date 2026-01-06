#pragma once
#include <stdint.h>
#include "Services/instrument/instrument_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

void humanize_init(uint32_t seed);
int8_t humanize_time_ms(const instrument_cfg_t* cfg, uint8_t apply_flag);
int8_t humanize_vel_delta(const instrument_cfg_t* cfg, uint8_t apply_flag);

#ifdef __cplusplus
}
#endif
