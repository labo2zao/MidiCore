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
  cfg->din.din_debounce_ms = 20;
  
  // AINSER module defaults (SPI-based)
  cfg->ainser.ainser_enable = 1;
  cfg->ainser.ainser_scan_ms = 5;
  cfg->ainser.ainser_deadband = 2;
  
  // AIN module defaults
  cfg->ain.ain_enable = 1;
  cfg->ain.ain_velocity_enable = 1;
  cfg->ain.ain_calibrate_auto = 1;
  cfg->ain.ain_scan_ms = 10;
  cfg->ain.ain_deadband = 2;
  
  // MIDI defaults
  cfg->midi.midi_default_channel = 0;
  cfg->midi.midi_velocity_curve = 0;
  
  // Pressure module defaults
  cfg->pressure.enable = 0;
  cfg->pressure.i2c_bus = 2;
  cfg->pressure.addr = 0x58;
  cfg->pressure.type = 2;
  cfg->pressure.map_mode = 1;
  cfg->pressure.interval_ms = 5;
  cfg->pressure.pmin_pa = -40000;
  cfg->pressure.pmax_pa = 40000;
  cfg->pressure.atm0_pa = 0;
  
  // Expression module defaults
  cfg->expression.enable = 0;
  cfg->expression.midi_ch = 0;
  cfg->expression.bidir = 0;
  cfg->expression.cc = 11;
  cfg->expression.cc_push = 11;
  cfg->expression.cc_pull = 2;
  cfg->expression.out_min = 0;
  cfg->expression.out_max = 127;
  cfg->expression.rate_ms = 20;
  cfg->expression.smooth = 200;
  cfg->expression.deadband_cc = 2;
  cfg->expression.hyst_cc = 1;
  cfg->expression.curve = 1;
  cfg->expression.curve_param = 180;
  cfg->expression.zero_deadband_pa = 500;
  
  // Calibration defaults
  cfg->calibration.enable = 0;
  cfg->calibration.atm_ms = 600;
  cfg->calibration.ext_ms = 5000;
  cfg->calibration.margin_raw = 60;
  cfg->calibration.cal_keep_files = 1;
}

/**
 * @brief Parse a configuration line
 * @param line Line to parse
 * @param cfg Configuration structure to update
 * @return 0 on success, -1 on error
 */
static int parse_config_line(const char* line, config_data_t* cfg) {
  if (!line || !cfg) return -1;
  
  // Skip empty lines, comments, and section headers
  if (line[0] == '\0' || line[0] == '#' || line[0] == '\n' || line[0] == '[') {
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
  // DIN Module
  if (strcmp(key, "SRIO_DIN_ENABLE") == 0) {
    cfg->din.srio_din_enable = (uint8_t)value;
  } else if (strcmp(key, "SRIO_DIN_BYTES") == 0) {
    cfg->din.srio_din_bytes = (uint8_t)value;
  } else if (strcmp(key, "DIN_INVERT_DEFAULT") == 0) {
    cfg->din.din_invert_default = (uint8_t)value;
  } else if (strcmp(key, "DIN_DEBOUNCE_MS") == 0) {
    cfg->din.din_debounce_ms = (uint8_t)value;
  }
  // AINSER Module
  else if (strcmp(key, "AINSER_ENABLE") == 0) {
    cfg->ainser.ainser_enable = (uint8_t)value;
  } else if (strcmp(key, "AINSER_SCAN_MS") == 0) {
    cfg->ainser.ainser_scan_ms = (uint8_t)value;
  } else if (strcmp(key, "AINSER_DEADBAND") == 0) {
    cfg->ainser.ainser_deadband = (uint8_t)value;
  }
  // AIN Module
  else if (strcmp(key, "AIN_ENABLE") == 0) {
    cfg->ain.ain_enable = (uint8_t)value;
  } else if (strcmp(key, "AIN_VELOCITY_ENABLE") == 0) {
    cfg->ain.ain_velocity_enable = (uint8_t)value;
  } else if (strcmp(key, "AIN_CALIBRATE_AUTO") == 0) {
    cfg->ain.ain_calibrate_auto = (uint8_t)value;
  } else if (strcmp(key, "AIN_SCAN_MS") == 0) {
    cfg->ain.ain_scan_ms = (uint8_t)value;
  } else if (strcmp(key, "AIN_DEADBAND") == 0) {
    cfg->ain.ain_deadband = (uint8_t)value;
  }
  // MIDI Settings
  else if (strcmp(key, "MIDI_DEFAULT_CHANNEL") == 0) {
    cfg->midi.midi_default_channel = (uint8_t)value;
  } else if (strcmp(key, "MIDI_VELOCITY_CURVE") == 0) {
    cfg->midi.midi_velocity_curve = (uint8_t)value;
  }
  // Pressure Module
  else if (strcmp(key, "PRESSURE_ENABLE") == 0 || strcmp(key, "ENABLE") == 0) {
    cfg->pressure.enable = (uint8_t)value;
  } else if (strcmp(key, "PRESSURE_I2C_BUS") == 0 || strcmp(key, "I2C_BUS") == 0) {
    cfg->pressure.i2c_bus = (uint8_t)value;
  } else if (strcmp(key, "PRESSURE_ADDR") == 0 || strcmp(key, "ADDR") == 0) {
    cfg->pressure.addr = (uint8_t)value;
  } else if (strcmp(key, "PRESSURE_TYPE") == 0 || strcmp(key, "TYPE") == 0) {
    cfg->pressure.type = (uint8_t)value;
  } else if (strcmp(key, "PRESSURE_MAP_MODE") == 0 || strcmp(key, "MAP_MODE") == 0) {
    cfg->pressure.map_mode = (uint8_t)value;
  } else if (strcmp(key, "PRESSURE_INTERVAL_MS") == 0 || strcmp(key, "INTERVAL_MS") == 0) {
    cfg->pressure.interval_ms = (uint8_t)value;
  } else if (strcmp(key, "PMIN_PA") == 0) {
    cfg->pressure.pmin_pa = value;
  } else if (strcmp(key, "PMAX_PA") == 0) {
    cfg->pressure.pmax_pa = value;
  } else if (strcmp(key, "ATM0_PA") == 0) {
    cfg->pressure.atm0_pa = value;
  }
  // Expression Module
  else if (strcmp(key, "EXPRESSION_ENABLE") == 0) {
    cfg->expression.enable = (uint8_t)value;
  } else if (strcmp(key, "EXPRESSION_MIDI_CH") == 0 || strcmp(key, "MIDI_CH") == 0) {
    cfg->expression.midi_ch = (uint8_t)value;
  } else if (strcmp(key, "BIDIR") == 0) {
    cfg->expression.bidir = (uint8_t)value;
  } else if (strcmp(key, "CC") == 0) {
    cfg->expression.cc = (uint8_t)value;
  } else if (strcmp(key, "CC_PUSH") == 0) {
    cfg->expression.cc_push = (uint8_t)value;
  } else if (strcmp(key, "CC_PULL") == 0) {
    cfg->expression.cc_pull = (uint8_t)value;
  } else if (strcmp(key, "OUT_MIN") == 0) {
    cfg->expression.out_min = (uint8_t)value;
  } else if (strcmp(key, "OUT_MAX") == 0) {
    cfg->expression.out_max = (uint8_t)value;
  } else if (strcmp(key, "RATE_MS") == 0) {
    cfg->expression.rate_ms = (uint8_t)value;
  } else if (strcmp(key, "SMOOTH") == 0) {
    cfg->expression.smooth = (uint16_t)value;
  } else if (strcmp(key, "DEADBAND_CC") == 0) {
    cfg->expression.deadband_cc = (uint8_t)value;
  } else if (strcmp(key, "HYST_CC") == 0) {
    cfg->expression.hyst_cc = (uint8_t)value;
  } else if (strcmp(key, "CURVE") == 0) {
    cfg->expression.curve = (uint8_t)value;
  } else if (strcmp(key, "CURVE_PARAM") == 0) {
    cfg->expression.curve_param = (uint16_t)value;
  } else if (strcmp(key, "ZERO_DEADBAND_PA") == 0) {
    cfg->expression.zero_deadband_pa = (uint16_t)value;
  }
  // Calibration
  else if (strcmp(key, "CALIBRATION_ENABLE") == 0) {
    cfg->calibration.enable = (uint8_t)value;
  } else if (strcmp(key, "ATM_MS") == 0) {
    cfg->calibration.atm_ms = (uint16_t)value;
  } else if (strcmp(key, "EXT_MS") == 0) {
    cfg->calibration.ext_ms = (uint16_t)value;
  } else if (strcmp(key, "MARGIN_RAW") == 0) {
    cfg->calibration.margin_raw = (uint8_t)value;
  } else if (strcmp(key, "CAL_KEEP_FILES") == 0) {
    cfg->calibration.cal_keep_files = (uint8_t)value;
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
  f_puts("# DIN Module Configuration (Digital Inputs via SRIO)\n", &file);
  f_printf(&file, "SRIO_DIN_ENABLE = %u\n", cfg->din.srio_din_enable);
  f_printf(&file, "SRIO_DIN_BYTES = %u\n", cfg->din.srio_din_bytes);
  f_printf(&file, "DIN_INVERT_DEFAULT = %u\n", cfg->din.din_invert_default);
  f_printf(&file, "DIN_DEBOUNCE_MS = %u\n\n", cfg->din.din_debounce_ms);
  
  // Write AINSER module config
  f_puts("# AINSER Module Configuration (Analog Inputs via SPI)\n", &file);
  f_printf(&file, "AINSER_ENABLE = %u\n", cfg->ainser.ainser_enable);
  f_printf(&file, "AINSER_SCAN_MS = %u\n", cfg->ainser.ainser_scan_ms);
  f_printf(&file, "AINSER_DEADBAND = %u\n\n", cfg->ainser.ainser_deadband);
  
  // Write AIN module config
  f_puts("# AIN Module Configuration (Built-in ADC Analog Inputs)\n", &file);
  f_printf(&file, "AIN_ENABLE = %u\n", cfg->ain.ain_enable);
  f_printf(&file, "AIN_VELOCITY_ENABLE = %u\n", cfg->ain.ain_velocity_enable);
  f_printf(&file, "AIN_CALIBRATE_AUTO = %u\n", cfg->ain.ain_calibrate_auto);
  f_printf(&file, "AIN_SCAN_MS = %u\n", cfg->ain.ain_scan_ms);
  f_printf(&file, "AIN_DEADBAND = %u\n\n", cfg->ain.ain_deadband);
  
  // Write MIDI settings
  f_puts("# MIDI Settings\n", &file);
  f_printf(&file, "MIDI_DEFAULT_CHANNEL = %u\n", cfg->midi.midi_default_channel);
  f_printf(&file, "MIDI_VELOCITY_CURVE = %u\n\n", cfg->midi.midi_velocity_curve);
  
  // Write Pressure module config
  f_puts("# Pressure Module Configuration (Breath Controller)\n", &file);
  f_puts("# XGZP6847D I2C pressure sensor\n", &file);
  f_printf(&file, "PRESSURE_ENABLE = %u\n", cfg->pressure.enable);
  f_printf(&file, "PRESSURE_I2C_BUS = %u\n", cfg->pressure.i2c_bus);
  f_printf(&file, "PRESSURE_ADDR = 0x%02X\n", cfg->pressure.addr);
  f_printf(&file, "PRESSURE_TYPE = %u\n", cfg->pressure.type);
  f_printf(&file, "PRESSURE_MAP_MODE = %u\n", cfg->pressure.map_mode);
  f_printf(&file, "PRESSURE_INTERVAL_MS = %u\n", cfg->pressure.interval_ms);
  f_printf(&file, "PMIN_PA = %ld\n", (long)cfg->pressure.pmin_pa);
  f_printf(&file, "PMAX_PA = %ld\n", (long)cfg->pressure.pmax_pa);
  f_printf(&file, "ATM0_PA = %ld\n\n", (long)cfg->pressure.atm0_pa);
  
  // Write Expression module config
  f_puts("# Expression Module Configuration\n", &file);
  f_printf(&file, "EXPRESSION_ENABLE = %u\n", cfg->expression.enable);
  f_printf(&file, "EXPRESSION_MIDI_CH = %u\n", cfg->expression.midi_ch);
  f_printf(&file, "BIDIR = %u\n", cfg->expression.bidir);
  f_printf(&file, "CC = %u\n", cfg->expression.cc);
  f_printf(&file, "CC_PUSH = %u\n", cfg->expression.cc_push);
  f_printf(&file, "CC_PULL = %u\n", cfg->expression.cc_pull);
  f_printf(&file, "OUT_MIN = %u\n", cfg->expression.out_min);
  f_printf(&file, "OUT_MAX = %u\n", cfg->expression.out_max);
  f_printf(&file, "RATE_MS = %u\n", cfg->expression.rate_ms);
  f_printf(&file, "SMOOTH = %u\n", cfg->expression.smooth);
  f_printf(&file, "DEADBAND_CC = %u\n", cfg->expression.deadband_cc);
  f_printf(&file, "HYST_CC = %u\n", cfg->expression.hyst_cc);
  f_printf(&file, "CURVE = %u\n", cfg->expression.curve);
  f_printf(&file, "CURVE_PARAM = %u\n", cfg->expression.curve_param);
  f_printf(&file, "ZERO_DEADBAND_PA = %u\n\n", cfg->expression.zero_deadband_pa);
  
  // Write Calibration config
  f_puts("# Calibration Configuration\n", &file);
  f_printf(&file, "CALIBRATION_ENABLE = %u\n", cfg->calibration.enable);
  f_printf(&file, "ATM_MS = %u\n", cfg->calibration.atm_ms);
  f_printf(&file, "EXT_MS = %u\n", cfg->calibration.ext_ms);
  f_printf(&file, "MARGIN_RAW = %u\n", cfg->calibration.margin_raw);
  f_printf(&file, "CAL_KEEP_FILES = %u\n", cfg->calibration.cal_keep_files);
  
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
