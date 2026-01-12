#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  MIDI_ROUTER_SRC_INTERNAL = 0,
  MIDI_ROUTER_SRC_DIN      = 1,
  MIDI_ROUTER_SRC_AINSER   = 2,
  MIDI_ROUTER_SRC_UART     = 3,
  MIDI_ROUTER_SRC_USBH     = 4,
  MIDI_ROUTER_SRC_USBD     = 5,
  MIDI_ROUTER_SRC_DREAM    = 6
} midi_router_src_t;

typedef enum {
  MIDI_ROUTER_DST_NONE  = 0x00,
  MIDI_ROUTER_DST_UART  = 0x01,
  MIDI_ROUTER_DST_USBH  = 0x02,
  MIDI_ROUTER_DST_USBD  = 0x04,
  MIDI_ROUTER_DST_DREAM = 0x08
} midi_router_dst_t;

void midi_router_init(void);
void midi_router_send3(midi_router_src_t src, uint8_t status, uint8_t d1, uint8_t d2);
void midi_router_note_on (midi_router_src_t src, uint8_t ch, uint8_t note, uint8_t vel);
void midi_router_note_off(midi_router_src_t src, uint8_t ch, uint8_t note, uint8_t vel);
void midi_router_cc      (midi_router_src_t src, uint8_t ch, uint8_t cc,   uint8_t val);


// Configure routing mask for a given source (bitwise OR of MIDI_ROUTER_DST_* flags).
void midi_router_set_route(midi_router_src_t src, uint8_t dst_mask);
uint8_t midi_router_get_route(midi_router_src_t src);

// Load routing configuration from SD card (e.g. 0:/cfg/router_map.ngc).
// Returns 0 on success, negative on error.
int midi_router_load_sd(const char* path);

#ifdef __cplusplus
}
#endif