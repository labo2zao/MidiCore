#include "midi_router.h"

#include "Hal/uart_midi/hal_uart_midi.h"
#include "Services/usb_host_midi/usb_host_midi.h"
#if __has_include("ff.h")
  #include "ff.h"
  #define MR_HAS_FATFS 1
#else
  #define MR_HAS_FATFS 0
#endif
#include <ctype.h>
#include <stdlib.h>

static uint8_t s_route_mask[MIDI_ROUTER_SRC_DREAM + 1];


void midi_router_init(void) {
  // Default routing:
  //  - DIN and AINSER sources -> UART + USB Host
  //  - other sources disabled (can be enabled via router_map.ngc)
  for (unsigned i = 0; i < (unsigned)(MIDI_ROUTER_SRC_DREAM + 1); ++i) {
    s_route_mask[i] = MIDI_ROUTER_DST_NONE;
  }
  s_route_mask[(unsigned)MIDI_ROUTER_SRC_DIN]    = (uint8_t)(MIDI_ROUTER_DST_UART | MIDI_ROUTER_DST_USBH);
  s_route_mask[(unsigned)MIDI_ROUTER_SRC_AINSER] = (uint8_t)(MIDI_ROUTER_DST_UART | MIDI_ROUTER_DST_USBH);
}

static void midi_router_backend_send3(uint8_t mask, uint8_t status, uint8_t d1, uint8_t d2) {
  if (mask & MIDI_ROUTER_DST_UART) {
    hal_uart_midi_send_byte(1u, status);
    hal_uart_midi_send_byte(1u, d1);
    hal_uart_midi_send_byte(1u, d2);
  }
  if (mask & MIDI_ROUTER_DST_USBH) {
    (void)usb_host_midi_send3(status, d1, d2);
  }
  // TODO: add USB device / Dream backends when available.
}

void midi_router_send3(midi_router_src_t src, uint8_t status, uint8_t d1, uint8_t d2) {
  uint8_t mask = 0;
  if ((unsigned)src <= (unsigned)MIDI_ROUTER_SRC_DREAM) {
    mask = s_route_mask[(unsigned)src];
  }
  if (!mask) {
    // nothing to send; routing disabled for this source
    return;
  }
  midi_router_backend_send3(mask, status, d1, d2);
}


void midi_router_note_on(midi_router_src_t src, uint8_t ch, uint8_t note, uint8_t vel) {
  if (vel == 0u) {
    midi_router_note_off(src, ch, note, 0u);
    return;
  }
  uint8_t status = (uint8_t)(0x90u | (ch & 0x0Fu));
  midi_router_send3(src, status, note, vel);
}

void midi_router_note_off(midi_router_src_t src, uint8_t ch, uint8_t note, uint8_t vel) {
  uint8_t status = (uint8_t)(0x80u | (ch & 0x0Fu));
  midi_router_send3(src, status, note, vel);
  (void)vel;
}

void midi_router_cc(midi_router_src_t src, uint8_t ch, uint8_t cc, uint8_t val) {
  uint8_t status = (uint8_t)(0xB0u | (ch & 0x0Fu));
  midi_router_send3(src, status, cc, val);
}

void midi_router_set_route(midi_router_src_t src, uint8_t dst_mask) {
  if ((unsigned)src > (unsigned)MIDI_ROUTER_SRC_DREAM) return;
  s_route_mask[(unsigned)src] = dst_mask;
}

uint8_t midi_router_get_route(midi_router_src_t src) {
  if ((unsigned)src > (unsigned)MIDI_ROUTER_SRC_DREAM) return 0u;
  return s_route_mask[(unsigned)src];
}

#if MR_HAS_FATFS

static void mr_trim(char* s) {
  if (!s) return;
  while (*s && isspace((unsigned char)*s)) memmove(s, s+1, strlen(s));
  size_t n = strlen(s);
  while (n && isspace((unsigned char)s[n-1])) s[--n] = 0;
}

static int mr_keyeq(const char* a, const char* b) {
  if (!a || !b) return 0;
  while (*a && *b) {
    char ca = (char)tolower((unsigned char)*a);
    char cb = (char)tolower((unsigned char)*b);
    if (ca != cb) return 0;
    ++a; ++b;
  }
  return *a == 0 && *b == 0;
}

static uint8_t mr_u8(const char* s) {
  if (!s) return 0;
  return (uint8_t)strtoul(s, 0, 0);
}

#endif // MR_HAS_FATFS

int midi_router_load_sd(const char* path) {
#if !MR_HAS_FATFS
  (void)path;
  return -10;
#else
  if (!path) return -1;

  FIL f;
  FRESULT fr = f_open(&f, path, FA_READ);
  if (fr != FR_OK) {
    return -2;
  }

  char line[128];
  int cur_src = -1;

  while (f_gets(line, sizeof(line), &f)) {
    mr_trim(line);
    if (!line[0] || line[0] == '#' || line[0] == ';')
      continue;

    if (line[0] == '[') {
      char* end = strchr(line, ']');
      if (!end) continue;
      *end = 0;
      char* tag = line + 1;
      mr_trim(tag);
      cur_src = -1;
      if ((tag[0] == 'S' || tag[0] == 's') && (tag[1] == 'R' || tag[1] == 'r') &&
          (tag[2] == 'C' || tag[2] == 'c')) {
        int idx = (int)strtol(tag + 3, 0, 10);
        if (idx >= 0 && idx <= (int)MIDI_ROUTER_SRC_DREAM) {
          cur_src = idx;
        }
      }
      continue;
    }

    if (cur_src < 0) continue;

    char* eq = strchr(line, '=');
    if (!eq) continue;
    *eq = 0;
    char* k = line;
    char* v = eq + 1;
    mr_trim(k);
    mr_trim(v);
    if (!k[0]) continue;

    uint8_t mask = s_route_mask[cur_src];

    if (mr_keyeq(k, "DST")) {
      // DST is a bitmask numeric expression, e.g. 3 => UART|USBH
      mask = mr_u8(v);
    } else if (mr_keyeq(k, "UART")) {
      if (mr_u8(v)) mask |= MIDI_ROUTER_DST_UART; else mask &= (uint8_t)~MIDI_ROUTER_DST_UART;
    } else if (mr_keyeq(k, "USBH")) {
      if (mr_u8(v)) mask |= MIDI_ROUTER_DST_USBH; else mask &= (uint8_t)~MIDI_ROUTER_DST_USBH;
    } else if (mr_keyeq(k, "USBD")) {
      if (mr_u8(v)) mask |= MIDI_ROUTER_DST_USBD; else mask &= (uint8_t)~MIDI_ROUTER_DST_USBD;
    } else if (mr_keyeq(k, "DREAM")) {
      if (mr_u8(v)) mask |= MIDI_ROUTER_DST_DREAM; else mask &= (uint8_t)~MIDI_ROUTER_DST_DREAM;
    }

    s_route_mask[cur_src] = mask;
  }

  f_close(&f);
  return 0;
#endif
}
