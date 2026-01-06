#pragma once
#include <stdint.h>
#include "Services/patch/patch_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

void patch_system_init(void);
int patch_system_apply(void);

int patch_system_patch_next(void);
int patch_system_patch_prev(void);

int patch_system_bank_next(void);
int patch_system_bank_prev(void);

const patch_manager_t* patch_system_get(void);

#ifdef __cplusplus
}
#endif
