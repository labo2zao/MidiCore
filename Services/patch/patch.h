#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void patch_init(void);
int  patch_load(const char* filename);
int  patch_get(const char* key, char* out, uint32_t maxlen);
int  patch_set(const char* key, const char* value);
int  patch_save(const char* filename);

#ifdef __cplusplus
}
#endif
