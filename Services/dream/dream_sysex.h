#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Apply DREAM init from patch file ([DREAM] section). Returns 0 if nothing to do. */
int dream_apply_from_patch(const char* patch_path);

#ifdef __cplusplus
}
#endif
