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
} config_din_t;

// Configuration structure for AINSER module
typedef struct {
  uint8_t ainser_enable;
  uint8_t ainser_i2c_addr;
  uint8_t ainser_scan_ms;
} config_ainser_t;

// Configuration structure for AIN module
typedef struct {
  uint8_t ain_velocity_enable;
  uint8_t ain_calibrate_auto;
} config_ain_t;

// Combined configuration structure
typedef struct {
  config_din_t din;
  config_ainser_t ainser;
  config_ain_t ain;
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
