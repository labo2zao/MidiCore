#include "Services/fs/fs_atomic.h"
#include "Services/fs/sd_guard.h"

#if __has_include("ff.h")
  #include "ff.h"
  #include <string.h>
  #define FS_ATOMIC_HAS_FATFS 1
#else
  #define FS_ATOMIC_HAS_FATFS 0
#endif

int fs_atomic_write_text(const char* path, const char* data, size_t len) {
#if FS_ATOMIC_HAS_FATFS
  if (sd_guard_is_readonly()) return -20;
#endif
#if !FS_ATOMIC_HAS_FATFS
  (void)path; (void)data; (void)len;
  return -10;
#else
  if (!path || !data) return -1;

  char tmp[128];
  char bak[128];
  // build tmp/bak names
  strncpy(tmp, path, sizeof(tmp)-1); tmp[sizeof(tmp)-1]=0;
  strncpy(bak, path, sizeof(bak)-1); bak[sizeof(bak)-1]=0;

  // append suffixes (truncate safely)
  strncat(tmp, ".tmp", sizeof(tmp)-strlen(tmp)-1);
  strncat(bak, ".bak", sizeof(bak)-strlen(bak)-1);

  FIL fp;
  if (f_open(&fp, tmp, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) { sd_guard_note_write_error(); return -2; }

  UINT bw=0;
  FRESULT fr = f_write(&fp, data, (UINT)len, &bw);
  if (fr != FR_OK || bw != (UINT)len) { f_close(&fp); (void)f_unlink(tmp); sd_guard_note_write_error(); return -3; }

  (void)f_sync(&fp);
  f_close(&fp);

  // rotate: existing path -> bak (best-effort)
  (void)f_unlink(bak);
  (void)f_rename(path, bak);

  // move tmp -> path
  fr = f_rename(tmp, path);
  if (fr == FR_EXIST) {
    (void)f_unlink(path);
    fr = f_rename(tmp, path);
  }
  if (fr != FR_OK) {
    sd_guard_note_write_error();
    // last resort: leave tmp
    return -4;
  }
  return 0;
#endif
}
