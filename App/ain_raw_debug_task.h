#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// DEBUG/TEST ONLY - Creates the AIN raw debug task
// Only compiled when MODULE_ENABLE_AIN_RAW_DEBUG=1 in Config/project_config.h
// NOT NEEDED FOR PRODUCTION
// ============================================================================
void ain_raw_debug_task_create(void);

#ifdef __cplusplus
}
#endif
