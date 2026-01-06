#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Atomic-ish write on FATFS:
 *  - write to <path>.tmp
 *  - f_sync + close
 *  - rename existing <path> -> <path>.bak (best-effort)
 *  - rename tmp -> path
 * Returns 0 on success.
 */
int fs_atomic_write_text(const char* path, const char* data, size_t len);

#ifdef __cplusplus
}
#endif
