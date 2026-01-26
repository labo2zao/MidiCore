#include "Services/patch/patch_sd_mount.h"
#include "Services/fs/sd_guard.h"
#include "Services/log/log.h"
#include "ff.h"
#include <stdio.h>

// Declare MX_FATFS_Init - defined in FATFS/App/fatfs.c
extern void MX_FATFS_Init(void);

static FATFS g_fs;

// Get FatFs error string
const char* get_fresult_str(FRESULT fr) {
  switch(fr) {
    case FR_OK: return "FR_OK";
    case FR_DISK_ERR: return "FR_DISK_ERR (disk error)";
    case FR_INT_ERR: return "FR_INT_ERR (internal error)";
    case FR_NOT_READY: return "FR_NOT_READY (disk not ready)";
    case FR_NO_FILE: return "FR_NO_FILE";
    case FR_NO_PATH: return "FR_NO_PATH";
    case FR_INVALID_NAME: return "FR_INVALID_NAME";
    case FR_DENIED: return "FR_DENIED";
    case FR_EXIST: return "FR_EXIST";
    case FR_INVALID_OBJECT: return "FR_INVALID_OBJECT";
    case FR_WRITE_PROTECTED: return "FR_WRITE_PROTECTED";
    case FR_INVALID_DRIVE: return "FR_INVALID_DRIVE";
    case FR_NOT_ENABLED: return "FR_NOT_ENABLED";
    case FR_NO_FILESYSTEM: return "FR_NO_FILESYSTEM (no FAT volume)";
    case FR_MKFS_ABORTED: return "FR_MKFS_ABORTED";
    case FR_TIMEOUT: return "FR_TIMEOUT";
    case FR_LOCKED: return "FR_LOCKED";
    case FR_NOT_ENOUGH_CORE: return "FR_NOT_ENOUGH_CORE";
    case FR_TOO_MANY_OPEN_FILES: return "FR_TOO_MANY_OPEN_FILES";
    case FR_INVALID_PARAMETER: return "FR_INVALID_PARAMETER";
    default: return "UNKNOWN";
  }
}

int patch_sd_mount_init(void) {
  MX_FATFS_Init();
  FRESULT fr = f_mount(&g_fs, "0:", 1);
  
  if (fr != FR_OK) {
    log_printf("SD", "f_mount failed: %s", get_fresult_str(fr));
    return -1;
  }
  
  return 0;
}


int patch_sd_mount_retry(uint8_t attempts) {
  for (uint8_t i=0;i<attempts;i++) {
    if (patch_sd_mount_init() == 0) return 0;
  }
  log_printf("SD", "mount fail after %d attempts", attempts); 
  return -1;
}
