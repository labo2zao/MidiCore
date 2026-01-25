#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PATCH_BANK_MAX_PATCHES
#define PATCH_BANK_MAX_PATCHES 12  // Configurable: 12 banks sufficient for accordion (was 32, saves ~2.5 KB)
#endif

typedef struct {
  char file[96];
  char label[32];
} patch_bank_item_t;

typedef struct {
  char bank_name[32];
  char bank_id[8];
  char chord_bank_path[96]; // optional: path to chord_bank.ngc
  uint16_t patch_count;
  patch_bank_item_t patches[PATCH_BANK_MAX_PATCHES];
} patch_bank_t;

int patch_bank_load(patch_bank_t* bank, const char* path);

#ifdef __cplusplus
}
#endif
