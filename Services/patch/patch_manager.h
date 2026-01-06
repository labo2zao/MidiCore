#pragma once
#include <stdint.h>
#include "Services/patch/patch_bank.h"
#include "Services/patch/patch_state.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  patch_state_t state;
  patch_bank_t  bank;
  char current_patch_path[96];
} patch_manager_t;

void patch_manager_init(patch_manager_t* pm);
int patch_manager_boot(patch_manager_t* pm);
int patch_manager_select_patch(patch_manager_t* pm, uint16_t patch_index);
int patch_manager_apply(patch_manager_t* pm);

#ifdef __cplusplus
}
#endif
