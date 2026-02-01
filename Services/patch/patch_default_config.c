/**
 * @file patch_default_config.c
 * @brief Default configuration embedded in firmware
 * 
 * This configuration is compiled into the firmware and loaded into RAM
 * when no config file exists on SD card. This provides a fully functional
 * system without requiring SD card access.
 */

#include "Services/patch/patch_default_config.h"
#include "Services/patch/patch.h"
#include "App/tests/test_debug.h"

/**
 * @brief Default configuration lines (compiled into firmware)
 * 
 * These values match the sdcard/config.ngc file and provide
 * a working default configuration for the MidiCore system.
 */
static const char* default_config_lines[] = {
  "# MidiCore Default Configuration (Compiled In)",
  "# This configuration is loaded from firmware when no SD config exists",
  "",
  "# DIN Module Configuration (Digital Inputs via SRIO)",
  "SRIO_DIN_ENABLE = 1",
  "SRIO_DIN_BYTES = 8",
  "DIN_INVERT_DEFAULT = 0",
  "DIN_DEBOUNCE_MS = 20",
  "",
  "# AINSER Module Configuration (Analog Inputs via SPI)",
  "AINSER_ENABLE = 1",
  "AINSER_SCAN_MS = 5",
  "AINSER_DEADBAND = 2",
  "",
  "# AIN Module Configuration (Built-in ADC Analog Inputs)",
  "AIN_ENABLE = 1",
  "AIN_VELOCITY_ENABLE = 1",
  "AIN_CALIBRATE_AUTO = 1",
  "AIN_SCAN_MS = 10",
  "AIN_DEADBAND = 2",
  "",
  "# MIDI Settings",
  "MIDI_DEFAULT_CHANNEL = 0",
  "MIDI_VELOCITY_CURVE = 0",
  "",
  "# Pressure Module Configuration (Breath Controller)",
  "# XGZP6847D I2C pressure sensor (0x58 address)",
  "PRESSURE_ENABLE = 0",
  "PRESSURE_I2C_BUS = 2",
  "PRESSURE_ADDR = 0x58",
  "PRESSURE_TYPE = 2",
  "PRESSURE_MAP_MODE = 1",
  "PRESSURE_INTERVAL_MS = 5",
  "PMIN_PA = -40000",
  "PMAX_PA = 40000",
  "ATM0_PA = 0",
  "",
  "# Expression Module Configuration",
  "# Maps pressure to MIDI CC with curve and smoothing",
  "EXPRESSION_ENABLE = 0",
  "EXPRESSION_MIDI_CH = 0",
  "BIDIR = 0",
  "CC = 11",
  "CC_PUSH = 11",
  "CC_PULL = 2",
  "OUT_MIN = 0",
  "OUT_MAX = 127",
  "RATE_MS = 20",
  "SMOOTH = 200",
  "",
  "# Looper Module Configuration",
  "LOOPER_ENABLE = 1",
  "LOOPER_TRACKS = 8",
  "LOOPER_QUANTIZE = 1",
  "",
  NULL  // Terminator
};

const char** patch_get_default_config_lines(void) {
  return default_config_lines;
}

int patch_get_default_config_line_count(void) {
  int count = 0;
  while (default_config_lines[count] != NULL) {
    count++;
  }
  return count;
}

/**
 * @brief Parse a config line and set the parameter
 * @param line Config line (e.g., "KEY = VALUE")
 * @return 0 on success, negative on error
 */
static int parse_and_set_line(const char* line) {
  // Skip empty lines and comments
  if (!line || line[0] == '\0' || line[0] == '#') {
    return 0;
  }
  
  // Find the '=' character
  const char* eq = line;
  while (*eq && *eq != '=') eq++;
  if (*eq != '=') {
    return 0;  // Not a key=value line
  }
  
  // Extract key (trim whitespace)
  const char* key_start = line;
  while (*key_start == ' ' || *key_start == '\t') key_start++;
  
  const char* key_end = eq - 1;
  while (key_end > key_start && (*key_end == ' ' || *key_end == '\t')) key_end--;
  
  int key_len = key_end - key_start + 1;
  if (key_len <= 0 || key_len > 64) return -1;
  
  char key[65];
  for (int i = 0; i < key_len; i++) {
    key[i] = key_start[i];
  }
  key[key_len] = '\0';
  
  // Extract value (trim whitespace)
  const char* val_start = eq + 1;
  while (*val_start == ' ' || *val_start == '\t') val_start++;
  
  const char* val_end = val_start;
  while (*val_end && *val_end != '\r' && *val_end != '\n') val_end++;
  val_end--;
  while (val_end > val_start && (*val_end == ' ' || *val_end == '\t')) val_end--;
  
  int val_len = val_end - val_start + 1;
  if (val_len < 0 || val_len > 64) return -1;
  
  char value[65];
  for (int i = 0; i < val_len; i++) {
    value[i] = val_start[i];
  }
  value[val_len] = '\0';
  
  // Set the parameter
  return patch_set(key, value);
}

int patch_load_default_config(void) {
  dbg_print("Loading default config from firmware...\r\n");
  
  // Initialize patch system
  patch_init();
  
  // Parse and load each line
  int loaded = 0;
  int line_num = 0;
  
  while (default_config_lines[line_num] != NULL) {
    int result = parse_and_set_line(default_config_lines[line_num]);
    if (result == 0 && default_config_lines[line_num][0] != '#' && 
        default_config_lines[line_num][0] != '\0') {
      loaded++;
    }
    line_num++;
  }
  
  /* MIOS32-STYLE: Fixed strings + dbg_print_u32 */
  dbg_print("Loaded ");
  dbg_print_u32(loaded);
  dbg_print(" default parameters from firmware\r\n");
  return 0;
}
