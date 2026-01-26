#include "Services/patch/patch.h"
#include "Services/patch/patch_adv.h"
#include "Services/patch/patch_default_config.h"
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
 * 
 * This function loads the default config from firmware (compiled in)
 * and saves it to the SD card.
 */
int patch_create_default_config(const char* filename) {
  // Load default config from firmware (compiled in)
  int result = patch_load_default_config();
  if (result != 0) {
    return result;
  }
  
  // Save to SD card
  result = patch_save(filename);
  if (result == 0) {
    dbg_printf("Created default config: %s\r\n", filename);
  }
  
  return result;
}
