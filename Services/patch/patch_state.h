#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  char bank_path[96];
  uint16_t patch_index;
} patch_state_t;

void patch_state_set_defaults(patch_state_t* st);
int patch_state_load(patch_state_t* st, const char* path);
int patch_state_save(const patch_state_t* st, const char* path);

#ifdef __cplusplus
}
#endif
