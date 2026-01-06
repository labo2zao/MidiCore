#pragma once
#include <stdint.h>
#include "Services/instrument/instrument_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t velocity_apply_curve(uint8_t in_vel, const instrument_cfg_t* cfg);

#ifdef __cplusplus
}
#endif
