#include "Services/system/safe_mode.h"
static volatile uint8_t g_forced = 0;
void safe_mode_set_forced(uint8_t on){ g_forced = on ? 1u : 0u; }
uint8_t safe_mode_is_forced(void){ return g_forced; }
