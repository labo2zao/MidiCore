/**
 * @file arpeggiator.h
 * @brief Simple arpeggiator for MIDI notes - Phase B stub
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ARP_MAX_NOTES 16

typedef enum {
  ARP_PATTERN_UP = 0,
  ARP_PATTERN_DOWN,
  ARP_PATTERN_UP_DOWN,
  ARP_PATTERN_RANDOM,
  ARP_PATTERN_AS_PLAYED,
  ARP_PATTERN_COUNT
} arp_pattern_t;

void arp_init(void);
void arp_set_enabled(uint8_t enabled);
uint8_t arp_get_enabled(void);
void arp_set_pattern(arp_pattern_t pattern);
arp_pattern_t arp_get_pattern(void);

#ifdef __cplusplus
}
#endif
