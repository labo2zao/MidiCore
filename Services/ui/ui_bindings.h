#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef UI_BIND_DISABLED
#define UI_BIND_DISABLED 0xFFFFu
#endif

typedef struct {
  uint16_t din_patch_prev;
  uint16_t din_patch_next;
  uint16_t din_load_apply;
  uint16_t din_bank_prev;
  uint16_t din_bank_next;
} ui_bindings_t;

void ui_bindings_defaults(ui_bindings_t* b);
int ui_bindings_load(ui_bindings_t* b, const char* path);

#ifdef __cplusplus
}
#endif
