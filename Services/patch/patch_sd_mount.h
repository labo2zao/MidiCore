#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <stdlib.h>

int patch_sd_mount_init(void);
int patch_sd_mount_retry(uint8_t attempts);

#ifdef __cplusplus
}
#endif
