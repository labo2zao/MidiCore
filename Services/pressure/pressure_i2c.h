#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  PRESS_TYPE_GENERIC_U16BE = 0,
  PRESS_TYPE_GENERIC_S16BE = 1,
  PRESS_TYPE_XGZP6847D_24B = 2
} press_type_t;

typedef enum {
  PRESS_MAP_CLAMP_0_4095 = 0,
  PRESS_MAP_CENTER_0PA   = 1   // map PMIN..PMAX to 0..4095, where 0 Pa maps to mid
} press_map_t;

typedef struct {
  uint8_t enable;
  uint8_t i2c_bus;
  uint8_t addr7;
  uint8_t reg;          // generic 16-bit sensors; XGZP uses fixed regs
  uint8_t type;         // press_type_t
  uint8_t map_mode;     // press_map_t
  uint8_t interval_ms;

  // Generic scaling:
  int32_t offset;
  float   scale;
  uint16_t clamp_min;
  uint16_t clamp_max;

  // XGZP6847D physical range (Pa) - signed range relative to atmospheric zero
  int32_t pmin_pa;      // e.g. -40000
  int32_t pmax_pa;      // e.g. +40000

  // Atmospheric zero calibration (Pa)
  int32_t atm0_pa;      // baseline to subtract from absolute sensor reading
} pressure_cfg_t;

void pressure_defaults(pressure_cfg_t* c);
int  pressure_load_sd(pressure_cfg_t* c, const char* path);

void pressure_set_cfg(const pressure_cfg_t* c);
const pressure_cfg_t* pressure_get_cfg(void);

// Reads sensor and returns SIGNED pressure (Pa) after subtracting atm0_pa (for XGZP)
// For generic sensors, returns raw as configured.
int  pressure_read_once(int32_t* out_value);
int  pressure_read_pa(int32_t* out_pa_signed);

// Reads absolute sensor pressure (Pa) before subtracting atm0_pa (XGZP only)
int  pressure_read_pa_abs(int32_t* out_pa_abs);

uint16_t pressure_to_12b(int32_t value); // value is Pa for XGZP, else raw
uint16_t pressure_mid_raw(void);         // raw position corresponding to 0 Pa (for center mapping)

#ifdef __cplusplus
}
#endif
