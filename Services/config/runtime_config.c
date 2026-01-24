/**
 * @file runtime_config.c
 * @brief Runtime configuration implementation
 */

#include "Services/config/runtime_config.h"
#include "ff.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

// =============================================================================
// PRIVATE STATE
// =============================================================================

static config_entry_t g_config[CONFIG_MAX_ENTRIES];
static uint32_t g_config_count = 0;
static uint8_t g_initialized = 0;
static config_change_callback_t g_change_callback = NULL;

// =============================================================================
// PRIVATE FUNCTIONS
// =============================================================================

static int find_entry(const char* key)
{
  for (uint32_t i = 0; i < g_config_count; i++) {
    if (strcmp(g_config[i].key, key) == 0) {
      return (int)i;
    }
  }
  return -1;
}

// =============================================================================
// INITIALIZATION
// =============================================================================

int runtime_config_init(void)
{
  memset(g_config, 0, sizeof(g_config));
  g_config_count = 0;
  g_initialized = 1;
  return 0;
}

int runtime_config_load(const char* filename)
{
  if (!g_initialized) {
    runtime_config_init();
  }
  
  FIL fp;
  if (f_open(&fp, filename, FA_READ) != FR_OK) {
    return -1;
  }
  
  char line[256];
  
  while (f_gets(line, sizeof(line), &fp)) {
    // Remove newline
    line[strcspn(line, "\r\n")] = 0;
    
    // Skip comments, empty lines, and section headers
    // Note: Section support not yet implemented - all entries are global
    if (line[0] == '#' || line[0] == ';' || line[0] == 0 || line[0] == '[') {
      continue;
    }
    
    // Parse key=value
    char* eq = strchr(line, '=');
    if (!eq) continue;
    
    *eq = 0;
    char* key = line;
    char* val = eq + 1;
    
    // Trim whitespace
    while (*key == ' ' || *key == '\t') key++;
    while (*val == ' ' || *val == '\t') val++;
    
    char* key_end = key + strlen(key) - 1;
    while (key_end > key && (*key_end == ' ' || *key_end == '\t')) {
      *key_end = 0;
      key_end--;
    }
    
    // Set value
    runtime_config_set_string(key, val);
  }
  
  f_close(&fp);
  return 0;
}

int runtime_config_save(const char* filename)
{
  FIL fp;
  
  if (f_open(&fp, filename, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) {
    return -1;
  }
  
  f_printf(&fp, "# MidiCore Runtime Configuration\r\n");
  f_printf(&fp, "# Auto-generated\r\n\r\n");
  
  for (uint32_t i = 0; i < g_config_count; i++) {
    f_printf(&fp, "%s=%s\r\n", g_config[i].key, g_config[i].value);
  }
  
  f_close(&fp);
  return 0;
}

// =============================================================================
// GET VALUES
// =============================================================================

const char* runtime_config_get_string(const char* key, const char* default_value)
{
  int idx = find_entry(key);
  if (idx < 0) return default_value;
  return g_config[idx].value;
}

int32_t runtime_config_get_int(const char* key, int32_t default_value)
{
  const char* str = runtime_config_get_string(key, NULL);
  if (!str) return default_value;
  
  char* endptr;
  long val = strtol(str, &endptr, 10);
  if (endptr == str) return default_value;
  
  return (int32_t)val;
}

uint8_t runtime_config_get_bool(const char* key, uint8_t default_value)
{
  const char* str = runtime_config_get_string(key, NULL);
  if (!str) return default_value;
  
  if (strcmp(str, "1") == 0 || strcmp(str, "true") == 0 || 
      strcmp(str, "TRUE") == 0 || strcmp(str, "yes") == 0) {
    return 1;
  }
  
  if (strcmp(str, "0") == 0 || strcmp(str, "false") == 0 || 
      strcmp(str, "FALSE") == 0 || strcmp(str, "no") == 0) {
    return 0;
  }
  
  return default_value;
}

float runtime_config_get_float(const char* key, float default_value)
{
  const char* str = runtime_config_get_string(key, NULL);
  if (!str) return default_value;
  
  char* endptr;
  float val = strtof(str, &endptr);
  if (endptr == str) return default_value;
  
  return val;
}

// =============================================================================
// SET VALUES
// =============================================================================

int runtime_config_set_string(const char* key, const char* value)
{
  if (!g_initialized) {
    runtime_config_init();
  }
  
  int idx = find_entry(key);
  const char* old_value = NULL;
  
  if (idx < 0) {
    // New entry
    if (g_config_count >= CONFIG_MAX_ENTRIES) {
      return -1;
    }
    idx = g_config_count++;
    strncpy(g_config[idx].key, key, CONFIG_MAX_KEY_LEN - 1);
    g_config[idx].key[CONFIG_MAX_KEY_LEN - 1] = 0;
  } else {
    old_value = g_config[idx].value;
  }
  
  strncpy(g_config[idx].value, value, CONFIG_MAX_VALUE_LEN - 1);
  g_config[idx].value[CONFIG_MAX_VALUE_LEN - 1] = 0;
  
  // Notify callback
  if (g_change_callback) {
    g_change_callback(key, old_value, value);
  }
  
  return 0;
}

int runtime_config_set_int(const char* key, int32_t value)
{
  char buf[32];
  snprintf(buf, sizeof(buf), "%" PRId32, value);
  return runtime_config_set_string(key, buf);
}

int runtime_config_set_bool(const char* key, uint8_t value)
{
  return runtime_config_set_string(key, value ? "1" : "0");
}

int runtime_config_set_float(const char* key, float value)
{
  char buf[32];
  snprintf(buf, sizeof(buf), "%.6f", value);
  return runtime_config_set_string(key, buf);
}

// =============================================================================
// MANAGEMENT
// =============================================================================

uint8_t runtime_config_exists(const char* key)
{
  return find_entry(key) >= 0 ? 1 : 0;
}

int runtime_config_delete(const char* key)
{
  int idx = find_entry(key);
  if (idx < 0) return -1;
  
  const char* old_value = g_config[idx].value;
  
  // Shift remaining entries
  for (uint32_t i = idx; i < g_config_count - 1; i++) {
    memcpy(&g_config[i], &g_config[i + 1], sizeof(config_entry_t));
  }
  g_config_count--;
  
  // Notify callback
  if (g_change_callback) {
    g_change_callback(key, old_value, NULL);
  }
  
  return 0;
}

void runtime_config_clear(void)
{
  g_config_count = 0;
  memset(g_config, 0, sizeof(g_config));
}

void runtime_config_set_change_callback(config_change_callback_t callback)
{
  g_change_callback = callback;
}

// =============================================================================
// DEBUGGING
// =============================================================================

void runtime_config_print(void)
{
  printf("\r\n");
  printf("==============================================\r\n");
  printf("       RUNTIME CONFIGURATION\r\n");
  printf("==============================================\r\n");
  printf("Total entries: %" PRIu32 "\r\n\r\n", g_config_count);
  
  for (uint32_t i = 0; i < g_config_count; i++) {
    printf("%-32s = %s\r\n", g_config[i].key, g_config[i].value);
  }
  
  printf("==============================================\r\n");
  printf("\r\n");
}

uint32_t runtime_config_get_count(void)
{
  return g_config_count;
}
