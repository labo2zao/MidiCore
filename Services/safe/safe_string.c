/**
 * @file safe_string.c
 * @brief Implementation of shared string utility functions
 */

#include "Services/safe/safe_string.h"
#include <string.h>
#include <ctype.h>

uint8_t string_equals(const char* a, const char* b) {
    // Handle NULL pointers
    if (!a && !b) return 1;      // Both NULL = equal
    if (!a || !b) return 0;      // One NULL = not equal
    
    // Compare strings
    return (strcmp(a, b) == 0) ? 1 : 0;
}

uint8_t string_iequals(const char* a, const char* b) {
    // Handle NULL pointers
    if (!a && !b) return 1;
    if (!a || !b) return 0;
    
    // Case-insensitive comparison
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
            return 0;
        }
        a++;
        b++;
    }
    
    // Both should be at end of string
    return (*a == 0 && *b == 0) ? 1 : 0;
}

char* string_trim(char* str) {
    if (!str) return NULL;
    
    // Trim leading whitespace
    char* start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    
    // If string was all whitespace
    if (*start == 0) {
        *str = 0;
        return str;
    }
    
    // Move trimmed content to beginning if needed
    if (start != str) {
        char* dst = str;
        while (*start) {
            *dst++ = *start++;
        }
        *dst = 0;
    }
    
    // Trim trailing whitespace
    size_t len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[--len] = 0;
    }
    
    return str;
}
