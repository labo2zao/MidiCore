/**
 * @file config_io.c
 * @brief Configuration File I/O Service Implementation
 * 
 * Implements SD card I/O for .NGC configuration files (MIDIbox NG format).
 */

#include "Services/config_io/config_io.h"
#include "Services/fs/sd_guard.h"
#include "FATFS/App/fatfs.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Last error message
static char g_last_error[128] = {0};

// SD card initialization flag
static uint8_t g_sd_initialized = 0;

/**
 * @brief Initialize config I/O service
 */
void config_io_init(void) {
  // Initialize FATFS (already done in main, but safe to call again)
  MX_FATFS_Init();
  g_sd_initialized = 1;
  g_last_error[0] = '\0';
}

/**
 * @brief Get default configuration
 */
void config_io_get_defaults(config_data_t* cfg) {
  if (!cfg) return;
  
  // DIN module defaults
  cfg->din.srio_din_enable = 1;
  cfg->din.srio_din_bytes = 8;
  cfg->din.din_invert_default = 0;
  
  // AINSER module defaults
  cfg->ainser.ainser_enable = 1;
  cfg->ainser.ainser_scan_ms = 5;
  
  // AIN module defaults
  cfg->ain.ain_velocity_enable = 1;
  cfg->ain.ain_calibrate_auto = 1;
}

/**
 * @brief Parse a configuration line
 * @param line Line to parse
 * @param cfg Configuration structure to update
 * @return 0 on success, -1 on error
 */
static int parse_config_line(const char* line, config_data_t* cfg) {
  if (!line || !cfg) return -1;
  
  // Skip empty lines and comments
  if (line[0] == '\0' || line[0] == '#' || line[0] == '\n') {
    return 0;
  }
  
  char key[64] = {0};
  int value = 0;
  
  // Parse "KEY = VALUE" format
  if (sscanf(line, " %63[^=] = %d", key, &value) != 2) {
    return 0;  // Skip malformed lines
  }
  
  // Trim whitespace from key
  char* p = key + strlen(key) - 1;
  while (p >= key && (*p == ' ' || *p == '\t')) {
    *p = '\0';
    p--;
  }
  
  // Match key and update config
  if (strcmp(key, "SRIO_DIN_ENABLE") == 0) {
    cfg->din.srio_din_enable = (uint8_t)value;
  } else if (strcmp(key, "SRIO_DIN_BYTES") == 0) {
    cfg->din.srio_din_bytes = (uint8_t)value;
  } else if (strcmp(key, "DIN_INVERT_DEFAULT") == 0) {
    cfg->din.din_invert_default = (uint8_t)value;
  } else if (strcmp(key, "AINSER_ENABLE") == 0) {
    cfg->ainser.ainser_enable = (uint8_t)value;
  } else if (strcmp(key, "AINSER_SCAN_MS") == 0) {
    cfg->ainser.ainser_scan_ms = (uint8_t)value;
  } else if (strcmp(key, "AIN_VELOCITY_ENABLE") == 0) {
    cfg->ain.ain_velocity_enable = (uint8_t)value;
  } else if (strcmp(key, "AIN_CALIBRATE_AUTO") == 0) {
    cfg->ain.ain_calibrate_auto = (uint8_t)value;
  }
  
  return 0;
}

/**
 * @brief Load configuration from SD card
 */
int config_io_load(config_data_t* cfg) {
  if (!cfg) {
    strncpy(g_last_error, "NULL config pointer", sizeof(g_last_error) - 1);
    return -1;
  }
  
  if (!g_sd_initialized) {
    strncpy(g_last_error, "Config I/O not initialized", sizeof(g_last_error) - 1);
    return -1;
  }
  
  // Start with defaults
  config_io_get_defaults(cfg);
  
  // Open file for reading
  FIL file;
  FRESULT res = f_open(&file, CONFIG_FILE_PATH, FA_READ);
  if (res != FR_OK) {
    snprintf(g_last_error, sizeof(g_last_error), "Failed to open config file (error %d)", res);
    return -1;
  }
  
  // Read line by line
  char line[CONFIG_LINE_MAX];
  while (f_gets(line, sizeof(line), &file)) {
    parse_config_line(line, cfg);
  }
  
  // Close file
  f_close(&file);
  
  g_last_error[0] = '\0';
  return 0;
}

/**
 * @brief Save configuration to SD card
 */
int config_io_save(const config_data_t* cfg) {
  if (!cfg) {
    strncpy(g_last_error, "NULL config pointer", sizeof(g_last_error) - 1);
    return -1;
  }
  
  if (!g_sd_initialized) {
    strncpy(g_last_error, "Config I/O not initialized", sizeof(g_last_error) - 1);
    return -1;
  }
  
  // Check if SD card is read-only
  if (sd_guard_is_readonly()) {
    strncpy(g_last_error, "SD card is read-only", sizeof(g_last_error) - 1);
    return -1;
  }
  
  // Open file for writing (create if doesn't exist)
  FIL file;
  FRESULT res = f_open(&file, CONFIG_FILE_PATH, FA_WRITE | FA_CREATE_ALWAYS);
  if (res != FR_OK) {
    snprintf(g_last_error, sizeof(g_last_error), "Failed to create config file (error %d)", res);
    sd_guard_note_write_error();
    return -1;
  }
  
  // Write header
  f_puts("# MidiCore Configuration File\n", &file);
  f_puts("# Compatible with MIDIbox NG .NGC format\n\n", &file);
  
  // Write DIN module config
  f_puts("# DIN Module Configuration\n", &file);
  f_printf(&file, "SRIO_DIN_ENABLE = %u\n", cfg->din.srio_din_enable);
  f_printf(&file, "SRIO_DIN_BYTES = %u\n", cfg->din.srio_din_bytes);
  f_printf(&file, "DIN_INVERT_DEFAULT = %u\n\n", cfg->din.din_invert_default);
  
  // Write AINSER module config
  f_puts("# AINSER Module Configuration\n", &file);
  f_printf(&file, "AINSER_ENABLE = %u\n", cfg->ainser.ainser_enable);
  f_printf(&file, "AINSER_SCAN_MS = %u\n\n", cfg->ainser.ainser_scan_ms);
  
  // Write AIN module config
  f_puts("# AIN Module Configuration\n", &file);
  f_printf(&file, "AIN_VELOCITY_ENABLE = %u\n", cfg->ain.ain_velocity_enable);
  f_printf(&file, "AIN_CALIBRATE_AUTO = %u\n", cfg->ain.ain_calibrate_auto);
  
  // Close file and sync
  f_close(&file);
  
  g_last_error[0] = '\0';
  return 0;
}

/**
 * @brief Check if SD card is available
 */
uint8_t config_io_sd_available(void) {
  if (!g_sd_initialized) return 0;
  
  // Try to get volume information
  FATFS* fs;
  DWORD fre_clust;
  FRESULT res = f_getfree("0:", &fre_clust, &fs);
  
  return (res == FR_OK) ? 1 : 0;
}

/**
 * @brief Get last error message
 */
const char* config_io_get_error(void) {
  if (g_last_error[0] == '\0') {
    return "No error";
  }
  return g_last_error;
}
