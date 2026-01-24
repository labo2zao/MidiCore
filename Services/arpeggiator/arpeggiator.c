/**
 * @file arpeggiator.c
 * @brief Arpeggiator implementation stub - Phase B
 */

#include "arpeggiator.h"
#include <string.h>

static uint8_t arp_enabled = 0;
static arp_pattern_t arp_pattern = ARP_PATTERN_UP;

void arp_init(void) {
  arp_enabled = 0;
  arp_pattern = ARP_PATTERN_UP;
}

void arp_set_enabled(uint8_t enabled) {
  arp_enabled = enabled ? 1 : 0;
}

uint8_t arp_get_enabled(void) {
  return arp_enabled;
}

void arp_set_pattern(arp_pattern_t pattern) {
  if (pattern < ARP_PATTERN_COUNT) {
    arp_pattern = pattern;
  }
}

arp_pattern_t arp_get_pattern(void) {
  return arp_pattern;
}
