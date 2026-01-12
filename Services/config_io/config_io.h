#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file config_io.h
 * @brief Configuration File I/O Service
 * 
 * Handles reading and writing .NGC configuration files from/to SD card.
 * Compatible with MIDIbox NG .NGC format for module configuration.
 */

// Configuration file format (.NGC)
#define CONFIG_FILE_PATH "0:/config.ngc"
#define CONFIG_LINE_MAX 128

// Configuration structure for DIN module
typedef struct {
  uint8_t srio_din_enable;
  uint8_t srio_din_bytes;
  uint8_t din_invert_default;
  uint8_t din_debounce_ms;
} config_din_t;

// Configuration structure for AINSER module
typedef struct {
  uint8_t ainser_enable;
  uint8_t ainser_scan_ms;
  uint8_t ainser_deadband;
} config_ainser_t;

// Configuration structure for AIN module
typedef struct {
  uint8_t ain_enable;
  uint8_t ain_velocity_enable;
  uint8_t ain_calibrate_auto;
  uint8_t ain_scan_ms;
  uint8_t ain_deadband;
} config_ain_t;

// Configuration structure for MIDI settings
typedef struct {
  uint8_t midi_default_channel;
  uint8_t midi_velocity_curve;
} config_midi_t;

// Configuration structure for Pressure module (breath controller)
typedef struct {
  uint8_t enable;
  uint8_t i2c_bus;
  uint8_t addr;
  uint8_t type;
  uint8_t map_mode;
  uint8_t interval_ms;
  int32_t pmin_pa;
  int32_t pmax_pa;
  int32_t atm0_pa;
} config_pressure_t;

// Configuration structure for Expression module
typedef struct {
  uint8_t enable;
  uint8_t midi_ch;
  uint8_t bidir;
  uint8_t cc;
  uint8_t cc_push;
  uint8_t cc_pull;
  uint8_t out_min;
  uint8_t out_max;
  uint8_t rate_ms;
  uint16_t smooth;
  uint8_t deadband_cc;
  uint8_t hyst_cc;
  uint8_t curve;
  uint16_t curve_param;
  uint16_t zero_deadband_pa;
} config_expression_t;

// Configuration structure for Calibration
typedef struct {
  uint8_t enable;
  uint16_t atm_ms;
  uint16_t ext_ms;
  uint8_t margin_raw;
  uint8_t cal_keep_files;
} config_calibration_t;

// Combined configuration structure
typedef struct {
  config_din_t din;
  config_ainser_t ainser;
  config_ain_t ain;
  config_midi_t midi;
  config_pressure_t pressure;
  config_expression_t expression;
  config_calibration_t calibration;
} config_data_t;

/**
 * @brief Initialize config I/O service
 */
void config_io_init(void);

/**
 * @brief Load configuration from SD card
 * @param cfg Pointer to config structure to fill
 * @return 0 on success, -1 on error
 */
int config_io_load(config_data_t* cfg);

/**
 * @brief Save configuration to SD card
 * @param cfg Pointer to config structure to save
 * @return 0 on success, -1 on error
 */
int config_io_save(const config_data_t* cfg);

/**
 * @brief Get default configuration
 * @param cfg Pointer to config structure to fill with defaults
 */
void config_io_get_defaults(config_data_t* cfg);

/**
 * @brief Check if SD card is available
 * @return 1 if available, 0 if not
 */
uint8_t config_io_sd_available(void);

/**
 * @brief Get last error message
 * @return Pointer to error string (read-only)
 */
const char* config_io_get_error(void);

#ifdef __cplusplus
}
#endif
