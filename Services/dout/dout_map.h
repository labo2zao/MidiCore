#pragma once
#include <stdint.h>
#include "Services/config/config.h"

#ifdef __cplusplus
extern "C" {
#endif

void dout_map_init(const config_t* cfg);
void dout_map_apply(const uint8_t* logical, uint8_t* physical, uint16_t bytes);
void dout_set_rgb(uint8_t* logical, uint8_t led, uint8_t r, uint8_t g, uint8_t b);

#ifdef __cplusplus
}
#endif
