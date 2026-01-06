#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <stdlib.h>
#include <stdint.h>
int patch_sd_mount_init(void);
int patch_sd_mount_retry(uint8_t attempts);

#ifdef __cplusplus
}
#endif
