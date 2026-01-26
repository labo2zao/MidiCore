#include "Services/patch/patch.h"
#include "Services/patch/patch_adv.h"
#include "App/tests/test_debug.h"

void patch_init(void) { patch_adv_init(); }
int  patch_load(const char* filename) { return patch_adv_load(filename); }
int  patch_get(const char* key, char* out, uint32_t maxlen) { return patch_adv_get("global", key, out, maxlen); }
int  patch_set(const char* key, const char* value) { return patch_adv_set("global", key, value, ""); }
int  patch_save(const char* filename) { return patch_adv_save(filename); }

/**
 * @brief Create and save default configuration file
 * @param filename Path to save config file (e.g., "0:/config.ngc")
 * @return 0 on success, negative on error
 */
int patch_create_default_config(const char* filename) {
  // Initialize patch system first (creates in-RAM config)
  patch_init();
  
  // Set default configuration values in RAM
  patch_set("SRIO_DIN_ENABLE", "1");
  patch_set("SRIO_DIN_BYTES", "8");
  patch_set("DIN_INVERT_DEFAULT", "0");
  patch_set("DIN_DEBOUNCE_MS", "20");
  
  patch_set("AINSER_ENABLE", "1");
  patch_set("AINSER_SCAN_MS", "5");
  patch_set("AINSER_DEADBAND", "2");
  
  patch_set("AIN_ENABLE", "1");
  patch_set("AIN_VELOCITY_ENABLE", "1");
  patch_set("AIN_CALIBRATE_AUTO", "1");
  patch_set("AIN_SCAN_MS", "10");
  patch_set("AIN_DEADBAND", "2");
  
  patch_set("MIDI_DEFAULT_CHANNEL", "0");
  patch_set("MIDI_VELOCITY_CURVE", "0");
  
  patch_set("PRESSURE_ENABLE", "0");
  patch_set("PRESSURE_I2C_BUS", "2");
  patch_set("PRESSURE_ADDR", "0x58");
  patch_set("PRESSURE_TYPE", "2");
  patch_set("PRESSURE_MAP_MODE", "1");
  patch_set("PRESSURE_INTERVAL_MS", "5");
  patch_set("PMIN_PA", "-40000");
  patch_set("PMAX_PA", "40000");
  patch_set("ATM0_PA", "0");
  
  patch_set("EXPRESSION_ENABLE", "0");
  patch_set("EXPRESSION_MIDI_CH", "0");
  patch_set("BIDIR", "0");
  patch_set("CC", "11");
  patch_set("CC_PUSH", "11");
  patch_set("CC_PULL", "2");
  patch_set("OUT_MIN", "0");
  patch_set("OUT_MAX", "127");
  patch_set("RATE_MS", "20");
  patch_set("SMOOTH", "200");
  
  // Save to SD card
  int result = patch_save(filename);
  if (result == 0) {
    dbg_printf("Created default config: %s\r\n", filename);
  }
  
  return result;
}
