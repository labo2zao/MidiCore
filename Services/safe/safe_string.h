/**
 * @file safe_string.h
 * @brief Shared string utility functions for MidiCore
 * 
 * This module provides common string operations used across multiple modules
 * to eliminate code duplication and ensure consistent behavior.
 * 
 * Functions consolidated from:
 * - ainser_map.c, midi_router.c, din_map.c (am_keyeq variants)
 * - patch_router.c, zones_cfg.c, instrument_cfg.c (ieq variants)
 * - pressure_i2c.c, expression_cfg.c (keyeq variants)
 * - zones_cfg.c, instrument_cfg.c (trim variants)
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Compare two strings for equality (case-sensitive)
 * 
 * Safely compares two strings with NULL pointer handling.
 * Used for key-value lookups in configuration parsing.
 * 
 * @param a First string (can be NULL)
 * @param b Second string (can be NULL)
 * @return 1 if strings are equal, 0 otherwise
 */
uint8_t string_equals(const char* a, const char* b);

/**
 * @brief Compare two strings for equality (case-insensitive)
 * 
 * Case-insensitive string comparison with NULL handling.
 * 
 * @param a First string (can be NULL)
 * @param b Second string (can be NULL)
 * @return 1 if strings are equal (ignoring case), 0 otherwise
 */
uint8_t string_iequals(const char* a, const char* b);

/**
 * @brief Trim leading and trailing whitespace from string
 * 
 * Modifies the string in-place by removing whitespace from both ends.
 * 
 * @param str String to trim (modified in-place, can be NULL)
 * @return Pointer to the trimmed string (same as input), or NULL if input was NULL
 */
char* string_trim(char* str);

#ifdef __cplusplus
}
#endif
