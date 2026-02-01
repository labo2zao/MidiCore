/**
 * @file module_registry.c
 * @brief Module Registry implementation
 * 
 * MIOS32-STYLE: NO printf / snprintf / vsnprintf
 * Uses only fixed-string output: cli_puts, cli_putc, cli_print_u32, cli_newline
 */

#include "module_registry.h"
#include "Services/cli/cli.h"
#include <string.h>

// =============================================================================
// PRIVATE STATE
// =============================================================================

// Pointer-based storage: saves ~38 KB RAM by storing pointers instead of full descriptors
static const module_descriptor_t* s_modules[MODULE_REGISTRY_MAX_MODULES];
static uint32_t s_module_count = 0;
static uint8_t s_initialized = 0;

// =============================================================================
// INITIALIZATION
// =============================================================================

int module_registry_init(void)
{
  if (s_initialized) {
    return 0;
  }

  memset(s_modules, 0, sizeof(s_modules));
  s_module_count = 0;
  s_initialized = 1;

  return 0;
}

// =============================================================================
// MODULE REGISTRATION
// =============================================================================

int module_registry_register(const module_descriptor_t* descriptor)
{
  if (!descriptor || s_module_count >= MODULE_REGISTRY_MAX_MODULES) {
    return -1;
  }

  // Check for duplicates
  for (uint32_t i = 0; i < s_module_count; i++) {
    if (strcasecmp(s_modules[i]->name, descriptor->name) == 0) {
      return -1; // Already registered
    }
  }

  // Store pointer (no copy needed - saves RAM)
  s_modules[s_module_count] = descriptor;
  s_module_count++;

  return 0;
}

int module_registry_unregister(const char* name)
{
  if (!name) {
    return -1;
  }

  for (uint32_t i = 0; i < s_module_count; i++) {
    if (strcasecmp(s_modules[i]->name, name) == 0) {
      // Shift remaining module pointers
      for (uint32_t j = i; j < s_module_count - 1; j++) {
        s_modules[j] = s_modules[j + 1];
      }
      s_module_count--;
      return 0;
    }
  }

  return -1; // Not found
}

// =============================================================================
// MODULE DISCOVERY
// =============================================================================

uint32_t module_registry_get_count(void)
{
  return s_module_count;
}

const module_descriptor_t* module_registry_get_by_index(uint32_t index)
{
  if (index >= s_module_count) {
    return NULL;
  }
  return s_modules[index];
}

const module_descriptor_t* module_registry_get_by_name(const char* name)
{
  if (!name) {
    return NULL;
  }

  for (uint32_t i = 0; i < s_module_count; i++) {
    if (strcasecmp(s_modules[i]->name, name) == 0) {
      return s_modules[i];
    }
  }

  return NULL;
}

uint32_t module_registry_list_by_category(module_category_t category,
                                           const module_descriptor_t** out_modules,
                                           uint32_t max_count)
{
  if (!out_modules || max_count == 0) {
    return 0;
  }

  uint32_t count = 0;
  for (uint32_t i = 0; i < s_module_count && count < max_count; i++) {
    if (s_modules[i]->category == category) {
      out_modules[count++] = s_modules[i];
    }
  }

  return count;
}

// =============================================================================
// MODULE CONTROL
// =============================================================================

int module_registry_enable(const char* name, uint8_t track)
{
  const module_descriptor_t* module = module_registry_get_by_name(name);
  if (!module) {
    return -1;
  }

  if (module->enable) {
    return module->enable(track);
  }

  return -1; // No enable function
}

int module_registry_disable(const char* name, uint8_t track)
{
  const module_descriptor_t* module = module_registry_get_by_name(name);
  if (!module) {
    return -1;
  }

  if (module->disable) {
    return module->disable(track);
  }

  return -1; // No disable function
}

module_status_t module_registry_get_status(const char* name, uint8_t track)
{
  const module_descriptor_t* module = module_registry_get_by_name(name);
  if (!module) {
    return MODULE_STATUS_ERROR;
  }

  if (module->get_status) {
    return (module_status_t)module->get_status(track);
  }

  return MODULE_STATUS_ENABLED; // Assume enabled if no status function
}

// =============================================================================
// PARAMETER ACCESS
// =============================================================================

int module_registry_get_param(const char* module_name,
                               const char* param_name,
                               uint8_t track,
                               param_value_t* out)
{
  if (!module_name || !param_name || !out) {
    return -1;
  }

  const module_descriptor_t* module = module_registry_get_by_name(module_name);
  if (!module) {
    return -1;
  }

  // Find parameter
  for (uint8_t i = 0; i < module->param_count; i++) {
    if (strcasecmp(module->params[i].name, param_name) == 0) {
      if (module->params[i].get_value) {
        return module->params[i].get_value(track, out);
      }
      return -1; // No getter
    }
  }

  return -1; // Parameter not found
}

int module_registry_set_param(const char* module_name,
                               const char* param_name,
                               uint8_t track,
                               const param_value_t* value)
{
  if (!module_name || !param_name || !value) {
    return -1;
  }

  const module_descriptor_t* module = module_registry_get_by_name(module_name);
  if (!module) {
    return -1;
  }

  // Find parameter
  for (uint8_t i = 0; i < module->param_count; i++) {
    if (strcasecmp(module->params[i].name, param_name) == 0) {
      if (module->params[i].read_only) {
        return -1; // Read-only
      }
      if (module->params[i].set_value) {
        return module->params[i].set_value(track, value);
      }
      return -1; // No setter
    }
  }

  return -1; // Parameter not found
}

const module_param_t* module_registry_get_param_descriptor(const char* module_name,
                                                            const char* param_name)
{
  if (!module_name || !param_name) {
    return NULL;
  }

  const module_descriptor_t* module = module_registry_get_by_name(module_name);
  if (!module) {
    return NULL;
  }

  for (uint8_t i = 0; i < module->param_count; i++) {
    if (strcasecmp(module->params[i].name, param_name) == 0) {
      return &module->params[i];
    }
  }

  return NULL;
}

// =============================================================================
// UTILITIES
// =============================================================================

void module_registry_print_modules(void)
{
  cli_newline();
  cli_puts("=== Registered Modules (");
  cli_print_u32(s_module_count);
  cli_puts(") ===");
  cli_newline();
  cli_newline();

  const char* current_category = NULL;
  for (uint32_t i = 0; i < s_module_count; i++) {
    const char* cat = module_registry_category_to_string(s_modules[i]->category);
    if (current_category == NULL || strcmp(cat, current_category) != 0) {
      current_category = cat;
      cli_newline();
      cli_puts("[");
      cli_puts(current_category);
      cli_puts("]");
      cli_newline();
    }
    cli_puts("  ");
    cli_puts(s_modules[i]->name);
    cli_puts(" - ");
    cli_puts(s_modules[i]->description);
    cli_newline();
  }
  cli_newline();
}

void module_registry_print_module(const char* name)
{
  const module_descriptor_t* module = module_registry_get_by_name(name);
  if (!module) {
    cli_error("Module not found");
    return;
  }

  cli_newline();
  cli_puts("Module: ");
  cli_puts(module->name);
  cli_newline();
  cli_puts("Category: ");
  cli_puts(module_registry_category_to_string(module->category));
  cli_newline();
  cli_puts("Description: ");
  cli_puts(module->description);
  cli_newline();
  cli_puts("Global: ");
  cli_puts(module->is_global ? "yes" : "no");
  cli_newline();
  cli_puts("Per-track: ");
  cli_puts(module->has_per_track_state ? "yes" : "no");
  cli_newline();
  cli_puts("Parameters: ");
  cli_print_u32(module->param_count);
  cli_newline();
  cli_newline();
}

void module_registry_print_params(const char* name)
{
  const module_descriptor_t* module = module_registry_get_by_name(name);
  if (!module) {
    cli_error("Module not found");
    return;
  }

  cli_newline();
  cli_puts("=== ");
  cli_puts(module->name);
  cli_puts(" Parameters ===");
  cli_newline();
  cli_newline();

  if (module->param_count == 0) {
    cli_puts("  (no parameters)");
    cli_newline();
  } else {
    for (uint8_t i = 0; i < module->param_count; i++) {
      const module_param_t* param = &module->params[i];
      cli_puts("  ");
      cli_puts(param->name);
      cli_puts(" [");
      cli_puts(module_registry_param_type_to_string(param->type));
      cli_puts("] ");
      if (param->read_only) {
        cli_puts("(RO) ");
      }
      cli_puts(param->description);
      cli_newline();
    }
  }
  cli_newline();
}

const char* module_registry_category_to_string(module_category_t category)
{
  switch (category) {
    case MODULE_CATEGORY_SYSTEM: return "System";
    case MODULE_CATEGORY_MIDI: return "MIDI";
    case MODULE_CATEGORY_INPUT: return "Input";
    case MODULE_CATEGORY_OUTPUT: return "Output";
    case MODULE_CATEGORY_EFFECT: return "Effect";
    case MODULE_CATEGORY_GENERATOR: return "Generator";
    case MODULE_CATEGORY_LOOPER: return "Looper";
    case MODULE_CATEGORY_UI: return "UI";
    case MODULE_CATEGORY_ACCORDION: return "Accordion";
    case MODULE_CATEGORY_OTHER: return "Other";
    default: return "Unknown";
  }
}

const char* module_registry_param_type_to_string(param_type_t type)
{
  switch (type) {
    case PARAM_TYPE_BOOL: return "bool";
    case PARAM_TYPE_INT: return "int";
    case PARAM_TYPE_FLOAT: return "float";
    case PARAM_TYPE_ENUM: return "enum";
    case PARAM_TYPE_STRING: return "string";
    default: return "unknown";
  }
}
