#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ZONES_MAX 4
#define ZONE_LAYERS_MAX 2

typedef struct {
  uint8_t enable;
  uint8_t key_min;
  uint8_t key_max;
  uint8_t ch[ZONE_LAYERS_MAX];
  uint8_t l2_enable;     // explicit enable for layer2
  uint8_t stack;         // if 1, both layers always active when zone matches
         // 0..15 (0=MIDI ch1)
  int8_t  transpose[ZONE_LAYERS_MAX];  // semitones
  uint8_t vel_mul_q7;                  // 128=1.0
  int8_t  vel_add;
  uint8_t prio;
} zone_t;

typedef struct {
  zone_t zone[ZONES_MAX];
} zones_cfg_t;

void zones_cfg_defaults(zones_cfg_t* z);
int  zones_cfg_load_sd(zones_cfg_t* z, const char* path);

const zones_cfg_t* zones_cfg_get(void);
void zones_cfg_set(const zones_cfg_t* z);

uint8_t zones_map_note(uint8_t key, uint8_t in_note, uint8_t in_vel,
                       uint8_t out_ch[ZONE_LAYERS_MAX],
                       uint8_t out_note[ZONE_LAYERS_MAX],
                       uint8_t out_vel[ZONE_LAYERS_MAX]);

#ifdef __cplusplus
}
#endif
