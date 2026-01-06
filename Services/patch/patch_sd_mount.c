#include "Services/patch/patch_sd_mount.h"
#include "Services/fs/sd_guard.h"
#include "Services/log/log.h"
#include "ff.h"

__attribute__((weak)) void MX_FATFS_Init(void) {}

static FATFS g_fs;

int patch_sd_mount_init(void) {
  MX_FATFS_Init();
  FRESULT fr = f_mount(&g_fs, "0:", 1);
  return (fr == FR_OK) ? 0 : -1;
}


int patch_sd_mount_retry(uint8_t attempts) {
  for (uint8_t i=0;i<attempts;i++) {
    if (patch_sd_mount_init() == 0) return 0;
  }
  log_printf("SD", "mount fail -1"); return -1;
}
