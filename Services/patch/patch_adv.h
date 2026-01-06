#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  char section[24];
  char key[24];
  char value[64];
  char cond[48];
} patch_entry_t;

void patch_adv_init(void);
int  patch_adv_load(const char* filename);
int  patch_adv_save(const char* filename);
int  patch_adv_get(const char* section, const char* key, char* out, uint32_t out_max);
int  patch_adv_set(const char* section, const char* key, const char* value, const char* cond);
uint32_t patch_adv_count(void);
const patch_entry_t* patch_adv_at(uint32_t idx);

int patch_adv_load_bank(uint8_t bank);

typedef struct {
  uint8_t midi_ch;   // 1..16
  uint8_t in_node;   // 0..15
} patch_ctx_t;

uint8_t patch_adv_cond_eval(const char* cond, const patch_ctx_t* ctx);

#ifdef __cplusplus
}
#endif
