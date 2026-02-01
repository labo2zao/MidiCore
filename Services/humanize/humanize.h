#pragma once
#include <stdint.h>
#include "Services/instrument/instrument_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

void humanize_init(uint32_t seed);
int8_t humanize_time_ms(const instrument_cfg_t* cfg, uint8_t apply_flag);
int8_t humanize_vel_delta(const instrument_cfg_t* cfg, uint8_t apply_flag);

/* Runtime getters/setters for CLI and control */
int humanize_get_time_variation(uint8_t track);
int humanize_get_velocity_variation(uint8_t track);
void humanize_set_time_variation(uint8_t track, int value);
void humanize_set_velocity_variation(uint8_t track, int value);
void humanize_set_enabled(uint8_t track, uint8_t enable);
uint8_t humanize_is_enabled(uint8_t track);

#ifdef __cplusplus
}
#endif
