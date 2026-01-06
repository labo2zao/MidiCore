#include "Services/patch/patch.h"
#include "Services/patch/patch_adv.h"

void patch_init(void) { patch_adv_init(); }
int  patch_load(const char* filename) { return patch_adv_load(filename); }
int  patch_get(const char* key, char* out, uint32_t maxlen) { return patch_adv_get("global", key, out, maxlen); }
int  patch_set(const char* key, const char* value) { return patch_adv_set("global", key, value, ""); }
int  patch_save(const char* filename) { return patch_adv_save(filename); }
