/**
 * @file arpeggiator.c
 * @brief Arpeggiator implementation
 * 
 * Provides multi-pattern arpeggiator with note buffer management,
 * timing control, and MIDI integration.
 */

#include "arpeggiator.h"
#include "Services/usb_midi/usb_midi_clock.h"
#include <string.h>

#define ARP_PPQN 24  // Pulses per quarter note (MIDI standard)

// Note buffer for arpeggiator
typedef struct {
  uint8_t note;
  uint8_t velocity;
} arp_note_t;

static uint8_t arp_enabled = 0;
static arp_pattern_t arp_pattern = ARP_PATTERN_UP;
static arp_note_t note_buffer[ARP_MAX_NOTES];
static uint8_t note_count = 0;
static uint8_t current_step = 0;
static uint8_t gate_length = 80;  // Gate length in % (0-100)
static uint8_t rate_division = 4; // Clock division: 1=whole, 2=half, 4=quarter, 8=eighth, etc.
static uint32_t last_clock_tick = 0;
static uint32_t clock_counter = 0;
static uint8_t note_on_sent = 0;
static uint8_t last_sent_note = 0;

void arp_init(void) {
  arp_enabled = 0;
  arp_pattern = ARP_PATTERN_UP;
  note_count = 0;
  current_step = 0;
  gate_length = 80;
  rate_division = 4;
  clock_counter = 0;
  note_on_sent = 0;
  memset(note_buffer, 0, sizeof(note_buffer));
}

void arp_set_enabled(uint8_t enabled) {
  arp_enabled = enabled ? 1 : 0;
  if (!arp_enabled) {
    // Clear buffer when disabled
    note_count = 0;
    current_step = 0;
    note_on_sent = 0;
  }
}

uint8_t arp_get_enabled(void) {
  return arp_enabled;
}

void arp_set_pattern(arp_pattern_t pattern) {
  if (pattern < ARP_PATTERN_COUNT) {
    arp_pattern = pattern;
    current_step = 0;  // Reset step when changing pattern
  }
}

arp_pattern_t arp_get_pattern(void) {
  return arp_pattern;
}

/**
 * @brief Add note to arpeggiator buffer
 * @param note MIDI note number (0-127)
 * @param velocity MIDI velocity (1-127)
 * @return 1 if added, 0 if buffer full
 */
uint8_t arp_add_note(uint8_t note, uint8_t velocity) {
  if (!arp_enabled) return 0;
  if (note_count >= ARP_MAX_NOTES) return 0;
  
  // Check if note already exists (avoid duplicates)
  for (uint8_t i = 0; i < note_count; i++) {
    if (note_buffer[i].note == note) {
      note_buffer[i].velocity = velocity;  // Update velocity
      return 1;
    }
  }
  
  // Add new note
  note_buffer[note_count].note = note;
  note_buffer[note_count].velocity = velocity;
  note_count++;
  
  // Sort buffer by note number (for UP/DOWN patterns)
  for (uint8_t i = 0; i < note_count - 1; i++) {
    for (uint8_t j = i + 1; j < note_count; j++) {
      if (note_buffer[i].note > note_buffer[j].note) {
        arp_note_t temp = note_buffer[i];
        note_buffer[i] = note_buffer[j];
        note_buffer[j] = temp;
      }
    }
  }
  
  return 1;
}

/**
 * @brief Remove note from arpeggiator buffer
 * @param note MIDI note number
 * @return 1 if removed, 0 if not found
 */
uint8_t arp_remove_note(uint8_t note) {
  for (uint8_t i = 0; i < note_count; i++) {
    if (note_buffer[i].note == note) {
      // Shift remaining notes down
      for (uint8_t j = i; j < note_count - 1; j++) {
        note_buffer[j] = note_buffer[j + 1];
      }
      note_count--;
      
      // Adjust current step if needed
      if (current_step >= note_count && note_count > 0) {
        current_step = 0;
      }
      
      return 1;
    }
  }
  return 0;
}

/**
 * @brief Clear all notes from buffer
 */
void arp_clear_notes(void) {
  note_count = 0;
  current_step = 0;
  note_on_sent = 0;
}

/**
 * @brief Get next note based on current pattern
 * @param note Output note number
 * @param velocity Output velocity
 * @return 1 if note available, 0 if buffer empty
 */
static uint8_t get_next_note(uint8_t* note, uint8_t* velocity) {
  if (note_count == 0) return 0;
  
  uint8_t index = 0;
  
  switch (arp_pattern) {
    case ARP_PATTERN_UP:
      index = current_step % note_count;
      break;
      
    case ARP_PATTERN_DOWN:
      index = (note_count - 1) - (current_step % note_count);
      break;
      
    case ARP_PATTERN_UP_DOWN:
      {
        uint8_t cycle_len = (note_count > 1) ? (note_count * 2 - 2) : 1;
        uint8_t pos = current_step % cycle_len;
        if (pos < note_count) {
          index = pos;
        } else {
          index = (note_count - 2) - (pos - note_count);
        }
      }
      break;
      
    case ARP_PATTERN_RANDOM:
      // Simple pseudo-random: use clock counter as seed
      index = (clock_counter * 13 + current_step * 7) % note_count;
      break;
      
    case ARP_PATTERN_AS_PLAYED:
    default:
      index = current_step % note_count;
      break;
  }
  
  *note = note_buffer[index].note;
  *velocity = note_buffer[index].velocity;
  return 1;
}

/**
 * @brief Process MIDI clock tick for arpeggiator timing
 * 
 * Should be called on each MIDI clock pulse (24 PPQN).
 * Handles note triggering and gate timing.
 */
void arp_on_clock_tick(void) {
  if (!arp_enabled || note_count == 0) return;
  
  clock_counter++;
  uint32_t clocks_per_step = ARP_PPQN / rate_division;
  
  // Check if it's time for a new note
  if ((clock_counter % clocks_per_step) == 0) {
    // Send note off for previous note if gate is active
    if (note_on_sent) {
      // Note off would be sent via MIDI output here
      // For now, just mark as sent off
      note_on_sent = 0;
    }
    
    // Get next note
    uint8_t note, velocity;
    if (get_next_note(&note, &velocity)) {
      // Send note on
      // This would integrate with MIDI router:
      // router_msg_t msg = { .type = ROUTER_MSG_3B, .b0 = 0x90, .b1 = note, .b2 = velocity };
      // router_process(ARP_INPUT_NODE, &msg);
      
      last_sent_note = note;
      note_on_sent = 1;
      last_clock_tick = clock_counter;
      
      current_step++;
    }
  }
  
  // Handle gate timing (note off before next note)
  if (note_on_sent) {
    uint32_t gate_ticks = (clocks_per_step * gate_length) / 100;
    uint32_t elapsed = clock_counter - last_clock_tick;
    
    if (elapsed >= gate_ticks) {
      // Send note off
      // router_msg_t msg = { .type = ROUTER_MSG_3B, .b0 = 0x80, .b1 = last_sent_note, .b2 = 0 };
      // router_process(ARP_INPUT_NODE, &msg);
      
      note_on_sent = 0;
    }
  }
}

/**
 * @brief Set arpeggiator rate division
 * @param division Clock division factor (1, 2, 4, 8, 16, etc.)
 */
void arp_set_rate(uint8_t division) {
  if (division > 0 && division <= 32) {
    rate_division = division;
  }
}

/**
 * @brief Get arpeggiator rate division
 */
uint8_t arp_get_rate(void) {
  return rate_division;
}

/**
 * @brief Set gate length percentage
 * @param length Gate length in percent (1-100)
 */
void arp_set_gate_length(uint8_t length) {
  if (length > 0 && length <= 100) {
    gate_length = length;
  }
}

/**
 * @brief Get gate length percentage
 */
uint8_t arp_get_gate_length(void) {
  return gate_length;
}

/**
 * @brief Reset arpeggiator position
 */
void arp_reset(void) {
  current_step = 0;
  clock_counter = 0;
  note_on_sent = 0;
}
