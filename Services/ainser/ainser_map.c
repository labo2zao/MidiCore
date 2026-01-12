#include "Services/ainser/ainser_map.h"

#include <string.h> // memset
#include <ctype.h>
#include <stdlib.h>

#if __has_include("ff.h")
  #include "ff.h"
  #define AM_HAS_FATFS 1
#else
  #define AM_HAS_FATFS 0
#endif


#ifndef AINSER_MAP_DEFAULT_THRESHOLD
#define AINSER_MAP_DEFAULT_THRESHOLD 8u
#endif

#ifndef AINSER_MAP_DEFAULT_SMOOTHING
#define AINSER_MAP_DEFAULT_SMOOTHING 6u
#endif

static AINSER_MapEntry s_map[AINSER_NUM_CHANNELS];
static uint16_t s_prev_raw[AINSER_NUM_CHANNELS];
static uint16_t s_filtered[AINSER_NUM_CHANNELS];
static uint8_t  s_last_cc[AINSER_NUM_CHANNELS];
static AINSER_MapOutputFn s_output_cb = 0;

// Simple integer square root for 16-bit domain (0..127*127 ~= 16129).
// Enough for our cheap "log-ish" curve.
static uint16_t ainser_isqrt16(uint16_t v)
{
    uint16_t res = 0;
    uint16_t bit = 1u << 14; // The second-to-top bit is set

    // "bit" starts at the highest power of four <= the argument.
    while (bit > v)
        bit >>= 2;

    while (bit != 0u) {
        if (v >= res + bit) {
            v -= (uint16_t)(res + bit);
            res = (uint16_t)((res >> 1) + bit);
        } else {
            res >>= 1;
        }
        bit >>= 2;
    }
    return res;
}

static uint8_t ainser_apply_curve(uint8_t in, uint8_t curve)
{
    switch (curve) {
    case AINSER_CURVE_EXPO: {
        // Exponential-ish: square then renormalise.
        uint16_t sq = (uint16_t)in * (uint16_t)in; // 0..16129
        uint16_t v = (uint16_t)((sq + 127u) / 127u);
        if (v > 127u)
            v = 127u;
        return (uint8_t)v;
    }
    case AINSER_CURVE_LOG: {
        // Log-ish: take sqrt in 0..127 space.
        uint16_t scaled = (uint16_t)in * 127u / 127u;
        uint16_t v = ainser_isqrt16(scaled * 127u);
        if (v > 127u)
            v = 127u;
        return (uint8_t)v;
    }
    case AINSER_CURVE_LINEAR:
    default:
        return in;
    }
}

AINSER_MapEntry *ainser_map_get_table(void)
{
    return s_map;
}

void ainser_map_set_output_cb(AINSER_MapOutputFn cb)
{
    s_output_cb = cb;
}

void ainser_map_init_defaults(void)
{
    // Basic linear defaults:
    //  - all channels enabled
    //  - CC numbers starting at 16 (16..79)
    //  - channel 0 (MIDI ch.1)
    //  - full ADC range
    //  - linear curve, non-inverted
    //  - default threshold
    for (uint8_t i = 0; i < AINSER_NUM_CHANNELS; ++i) {
        s_map[i].cc        = (uint8_t)(16u + i);
        s_map[i].channel   = 0u;
        s_map[i].curve     = (uint8_t)AINSER_CURVE_LINEAR;
        s_map[i].invert    = 0u;
        s_map[i].enabled   = 1u;
        s_map[i].reserved  = 0u;
        s_map[i].min       = 0u;
        s_map[i].max       = AINSER_ADC_MAX;
        s_map[i].threshold = (uint16_t)AINSER_MAP_DEFAULT_THRESHOLD;
    }

    for (uint8_t i = 0; i < AINSER_NUM_CHANNELS; ++i) {
        s_prev_raw[i] = 0xFFFFu;
        s_filtered[i] = 0xFFFFu;
        s_last_cc[i]  = 0xFFu;
    }
}

void ainser_map_process_channel(uint8_t index, uint16_t raw12)
{
    if (index >= AINSER_NUM_CHANNELS)
        return;

    AINSER_MapEntry *e = &s_map[index];
    if (!e->enabled)
        return;

    uint16_t old = s_prev_raw[index];
    if (old == 0xFFFFu) {
        // First measurement: initialise caches, do not emit CC yet.
        s_prev_raw[index] = raw12;
        s_filtered[index] = raw12;
        return;
    }

    uint16_t diff = (raw12 > old) ? (uint16_t)(raw12 - old) : (uint16_t)(old - raw12);
    uint16_t th   = e->threshold ? e->threshold : (uint16_t)AINSER_MAP_DEFAULT_THRESHOLD;
    if (diff < th) {
        // Update the previous raw to slowly follow slow drifts,
        // but do not bother computing/scaling.
        s_prev_raw[index] = raw12;
        return;
    }
    s_prev_raw[index] = raw12;

    // Smoothing: simple one-pole low-pass in integer domain.
    uint16_t filtered = s_filtered[index];
    if (filtered == 0xFFFFu) {
        filtered = raw12;
    } else {
        uint16_t alpha = (uint16_t)AINSER_MAP_DEFAULT_SMOOTHING;
        uint32_t acc = (uint32_t)filtered * alpha + (uint32_t)raw12;
        filtered = (uint16_t)(acc / (uint32_t)(alpha + 1u));
    }
    s_filtered[index] = filtered;

    // Clamp to min/max and optionally invert.
    uint16_t minv = e->min;
    uint16_t maxv = e->max;
    if (maxv <= minv) {
        minv = 0u;
        maxv = AINSER_ADC_MAX;
    }
    uint16_t val = filtered;
    if (val <= minv) {
        val = minv;
    } else if (val >= maxv) {
        val = maxv;
    }

    uint16_t span = (uint16_t)(maxv - minv);
    uint16_t rel;
    if (span == 0u) {
        rel = 0u;
    } else {
        if (e->invert) {
            rel = (uint16_t)(maxv - val);
        } else {
            rel = (uint16_t)(val - minv);
        }
    }

    uint8_t out7 = 0u;
    if (span != 0u) {
        uint32_t scaled = (uint32_t)rel * 127u + (span / 2u); // for rounding
        scaled /= (uint32_t)span;
        if (scaled > 127u)
            scaled = 127u;
        out7 = (uint8_t)scaled;
    }

    // Apply curve in 0..127 domain.
    out7 = ainser_apply_curve(out7, e->curve);

    // Only emit if changed.
    if (out7 == s_last_cc[index])
        return;
    s_last_cc[index] = out7;

    if (s_output_cb) {
        s_output_cb(e->channel, e->cc, out7);
    }
}

// --- SD card config loading -------------------------------------------------

#if AM_HAS_FATFS

// Local helpers (trim, key compare, integer parsers) adapted from other *_cfg modules.

static void am_trim(char* s) {
  if (!s) return;
  while (*s && isspace((unsigned char)*s)) memmove(s, s+1, strlen(s));
  size_t n = strlen(s);
  while (n && isspace((unsigned char)s[n-1])) s[--n] = 0;
}

static int am_keyeq(const char* a, const char* b) {
  if (!a || !b) return 0;
  while (*a && *b) {
    char ca = (char)tolower((unsigned char)*a);
    char cb = (char)tolower((unsigned char)*b);
    if (ca != cb) return 0;
    ++a; ++b;
  }
  return *a == 0 && *b == 0;
}

static uint8_t am_u8(const char* s) {
  if (!s) return 0;
  return (uint8_t)strtoul(s, 0, 0);
}

static uint16_t am_u16(const char* s) {
  if (!s) return 0;
  return (uint16_t)strtoul(s, 0, 0);
}

#endif // AM_HAS_FATFS

int ainser_map_load_sd(const char* path) {
#if !AM_HAS_FATFS
  (void)path;
  return -10;
#else
  if (!path) return -1;

  FIL f;
  FRESULT fr = f_open(&f, path, FA_READ);
  if (fr != FR_OK) {
    return -2;
  }

  char line[160];
  int cur = -1;

  while (f_gets(line, sizeof(line), &f)) {
    am_trim(line);
    if (!line[0] || line[0] == '#' || line[0] == ';')
      continue;

    // Section header: [CHn]
    if (line[0] == '[') {
      char* end = strchr(line, ']');
      if (!end) continue;
      *end = 0;
      char* tag = line + 1;
      am_trim(tag);
      cur = -1;
      if ((tag[0] == 'C' || tag[0] == 'c') && (tag[1] == 'H' || tag[1] == 'h')) {
        int idx = (int)strtol(tag + 2, 0, 10);
        if (idx >= 0 && idx < (int)AINSER_NUM_CHANNELS) {
          cur = idx;
        }
      }
      continue;
    }

    if (cur < 0) continue;

    char* eq = strchr(line, '=');
    if (!eq) continue;
    *eq = 0;
    char* k = line;
    char* v = eq + 1;
    am_trim(k);
    am_trim(v);
    if (!k[0]) continue;

    AINSER_MapEntry* e = &s_map[cur];

    if (am_keyeq(k, "CC")) {
      uint8_t t = am_u8(v);
      if (t > 127u) t = 127u;
      e->cc = t;
    } else if (am_keyeq(k, "CHAN") || am_keyeq(k, "CHANNEL")) {
      e->channel = (uint8_t)(am_u8(v) & 0x0Fu);
    } else if (am_keyeq(k, "CURVE")) {
      // Allow numeric (0..2) or text (LIN/EXPO/LOG)
      if (v[0] >= '0' && v[0] <= '9') {
        uint8_t t = am_u8(v);
        if (t > (uint8_t)AINSER_CURVE_LOG) t = (uint8_t)AINSER_CURVE_LINEAR;
        e->curve = t;
      } else {
        char c0 = (char)tolower((unsigned char)v[0]);
        if (c0 == 'l') {
          e->curve = (uint8_t)AINSER_CURVE_LINEAR;
        } else if (c0 == 'e') {
          e->curve = (uint8_t)AINSER_CURVE_EXPO;
        } else if (c0 == 'o' || c0 == 'g') {
          e->curve = (uint8_t)AINSER_CURVE_LOG;
        }
      }
    } else if (am_keyeq(k, "INVERT")) {
      e->invert = am_u8(v) ? 1u : 0u;
    } else if (am_keyeq(k, "MIN")) {
      e->min = am_u16(v);
    } else if (am_keyeq(k, "MAX")) {
      e->max = am_u16(v);
    } else if (am_keyeq(k, "THRESHOLD") || am_keyeq(k, "THR")) {
      e->threshold = am_u16(v);
    } else if (am_keyeq(k, "ENABLED") || am_keyeq(k, "ENABLE")) {
      e->enabled = am_u8(v) ? 1u : 0u;
    }
  }

  f_close(&f);

  // Sanity/post-process
  for (uint8_t i = 0; i < AINSER_NUM_CHANNELS; ++i) {
    if (s_map[i].min > s_map[i].max) {
      uint16_t t = s_map[i].min;
      s_map[i].min = s_map[i].max;
      s_map[i].max = t;
    }
    if (s_map[i].threshold == 0u) {
      s_map[i].threshold = (uint16_t)AINSER_MAP_DEFAULT_THRESHOLD;
    }
    if (s_map[i].curve > (uint8_t)AINSER_CURVE_LOG) {
      s_map[i].curve = (uint8_t)AINSER_CURVE_LINEAR;
    }
  }

  return 0;
#endif
}
