/**
 * @file arpeggiator.h
 * @brief Simple arpeggiator for MIDI notes
 * 
 * Provides multi-pattern arpeggiator with configurable rate,
 * gate length, and MIDI clock synchronization.
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

// Basic control
void arp_init(void);
void arp_set_enabled(uint8_t enabled);
uint8_t arp_get_enabled(void);
void arp_set_pattern(arp_pattern_t pattern);
arp_pattern_t arp_get_pattern(void);

// Note buffer management
uint8_t arp_add_note(uint8_t note, uint8_t velocity);
uint8_t arp_remove_note(uint8_t note);
void arp_clear_notes(void);

// Timing control
void arp_on_clock_tick(void);
void arp_set_rate(uint8_t division);
uint8_t arp_get_rate(void);
void arp_set_gate_length(uint8_t length);
uint8_t arp_get_gate_length(void);
void arp_reset(void);

#ifdef __cplusplus
}
#endif
