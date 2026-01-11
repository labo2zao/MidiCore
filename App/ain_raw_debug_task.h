#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Creates the AIN raw debug task if enabled in Config/project_config.h.
void ain_raw_debug_task_create(void);

#ifdef __cplusplus
}
#endif
