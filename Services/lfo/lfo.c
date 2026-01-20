#include "Services/lfo/lfo.h"
#include <math.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct {
    uint8_t enabled;
    lfo_waveform_t waveform;
    uint16_t rate_hundredths;  // 0.01Hz units (1 = 0.01Hz, 1000 = 10Hz)
    uint8_t depth;             // 0-100%
    lfo_target_t target;
    uint8_t bpm_sync;
    uint8_t bpm_divisor;       // 1, 2, 4, 8, 16, 32 bars
    uint32_t phase;            // 0-65535 (16-bit phase accumulator)
    uint32_t phase_increment;  // How much to add per ms
    int16_t last_random;       // For smooth random interpolation
    int16_t next_random;       // Target for smooth random
    uint16_t random_counter;   // For sample & hold timing
} lfo_state_t;

static lfo_state_t g_lfo[LFO_MAX_TRACKS];
static uint16_t g_tempo_bpm = 120;
static uint32_t g_random_seed = 0x87654321u;

// Fast sin approximation using lookup table (256 entries)
static const int16_t sine_table[256] = {
    0, 804, 1608, 2410, 3212, 4011, 4808, 5602, 6393, 7179, 7962, 8739, 9512, 10278, 11039, 11793,
    12539, 13279, 14010, 14732, 15446, 16151, 16846, 17530, 18204, 18868, 19519, 20159, 20787, 21403, 22005, 22594,
    23170, 23731, 24279, 24811, 25329, 25832, 26319, 26790, 27245, 27683, 28105, 28510, 28898, 29268, 29621, 29956,
    30273, 30571, 30852, 31113, 31356, 31580, 31785, 31971, 32137, 32285, 32412, 32521, 32609, 32678, 32728, 32757,
    32767, 32757, 32728, 32678, 32609, 32521, 32412, 32285, 32137, 31971, 31785, 31580, 31356, 31113, 30852, 30571,
    30273, 29956, 29621, 29268, 28898, 28510, 28105, 27683, 27245, 26790, 26319, 25832, 25329, 24811, 24279, 23731,
    23170, 22594, 22005, 21403, 20787, 20159, 19519, 18868, 18204, 17530, 16846, 16151, 15446, 14732, 14010, 13279,
    12539, 11793, 11039, 10278, 9512, 8739, 7962, 7179, 6393, 5602, 4808, 4011, 3212, 2410, 1608, 804,
    0, -804, -1608, -2410, -3212, -4011, -4808, -5602, -6393, -7179, -7962, -8739, -9512, -10278, -11039, -11793,
    -12539, -13279, -14010, -14732, -15446, -16151, -16846, -17530, -18204, -18868, -19519, -20159, -20787, -21403, -22005, -22594,
    -23170, -23731, -24279, -24811, -25329, -25832, -26319, -26790, -27245, -27683, -28105, -28510, -28898, -29268, -29621, -29956,
    -30273, -30571, -30852, -31113, -31356, -31580, -31785, -31971, -32137, -32285, -32412, -32521, -32609, -32678, -32728, -32757,
    -32767, -32757, -32728, -32678, -32609, -32521, -32412, -32285, -32137, -31971, -31785, -31580, -31356, -31113, -30852, -30571,
    -30273, -29956, -29621, -29268, -28898, -28510, -28105, -27683, -27245, -26790, -26319, -25832, -25329, -24811, -24279, -23731,
    -23170, -22594, -22005, -21403, -20787, -20159, -19519, -18868, -18204, -17530, -16846, -16151, -15446, -14732, -14010, -13279,
    -12539, -11793, -11039, -10278, -9512, -8739, -7962, -7179, -6393, -5602, -4808, -4011, -3212, -2410, -1608, -804
};

static inline uint32_t lcg_random(void) {
    g_random_seed = g_random_seed * 1664525u + 1013904223u;
    return g_random_seed;
}

static void calculate_phase_increment(uint8_t track) {
    lfo_state_t* lfo = &g_lfo[track];
    
    if (lfo->bpm_sync && g_tempo_bpm > 0) {
        // BPM sync mode: calculate based on tempo and divisor
        // phase_inc = (65536 * 1000) / (ms_per_cycle)
        // ms_per_cycle = (60000 * 4 * divisor) / bpm
        uint32_t ms_per_cycle = (60000UL * 4UL * lfo->bpm_divisor) / g_tempo_bpm;
        if (ms_per_cycle > 0) {
            lfo->phase_increment = (65536000UL) / ms_per_cycle;
        } else {
            lfo->phase_increment = 0;
        }
    } else {
        // Free-running mode: use rate_hundredths
        // phase_inc = (rate_hz * 65536 * 1000) / 1000000
        // rate_hz = rate_hundredths / 100
        if (lfo->rate_hundredths > 0) {
            lfo->phase_increment = (lfo->rate_hundredths * 65536UL) / 100UL;
        } else {
            lfo->phase_increment = 0;
        }
    }
}

static int16_t get_waveform_value(lfo_state_t* lfo) {
    uint16_t phase_8bit = lfo->phase >> 8;  // Convert to 8-bit for lookup
    int16_t value = 0;
    
    switch (lfo->waveform) {
        case LFO_WAVEFORM_SINE:
            value = sine_table[phase_8bit];
            break;
            
        case LFO_WAVEFORM_TRIANGLE:
            if (phase_8bit < 128) {
                value = ((int32_t)phase_8bit * 512) - 32768;
            } else {
                value = 32768 - ((int32_t)(phase_8bit - 128) * 512);
            }
            break;
            
        case LFO_WAVEFORM_SAW:
            value = ((int32_t)phase_8bit * 256) - 32768;
            break;
            
        case LFO_WAVEFORM_SQUARE:
            value = (phase_8bit < 128) ? 32767 : -32768;
            break;
            
        case LFO_WAVEFORM_RANDOM:
            // Smooth random: interpolate between last and next random values
            {
                uint16_t interp = lfo->phase >> 8;
                int32_t diff = lfo->next_random - lfo->last_random;
                value = lfo->last_random + ((diff * interp) >> 8);
            }
            break;
            
        case LFO_WAVEFORM_SAMPLE_HOLD:
            value = lfo->last_random;
            break;
            
        default:
            value = 0;
            break;
    }
    
    return value;
}

void lfo_init(void) {
    for (uint8_t i = 0; i < LFO_MAX_TRACKS; i++) {
        g_lfo[i].enabled = 0;
        g_lfo[i].waveform = LFO_WAVEFORM_SINE;
        g_lfo[i].rate_hundredths = 50;  // 0.5 Hz default
        g_lfo[i].depth = 50;             // 50% default
        g_lfo[i].target = LFO_TARGET_VELOCITY;
        g_lfo[i].bpm_sync = 0;
        g_lfo[i].bpm_divisor = 4;        // 4 bars default
        g_lfo[i].phase = 0;
        g_lfo[i].phase_increment = 0;
        g_lfo[i].last_random = 0;
        g_lfo[i].next_random = (int16_t)((lcg_random() & 0xFFFF) - 32768);
        g_lfo[i].random_counter = 0;
        calculate_phase_increment(i);
    }
}

void lfo_tick_1ms(void) {
    for (uint8_t i = 0; i < LFO_MAX_TRACKS; i++) {
        if (!g_lfo[i].enabled) continue;
        
        lfo_state_t* lfo = &g_lfo[i];
        uint16_t old_phase_high = lfo->phase >> 16;
        
        lfo->phase += lfo->phase_increment;
        
        // Check for phase wrap (completed one cycle)
        uint16_t new_phase_high = lfo->phase >> 16;
        if (new_phase_high < old_phase_high) {
            // Wrapped around - generate new random values
            if (lfo->waveform == LFO_WAVEFORM_RANDOM || 
                lfo->waveform == LFO_WAVEFORM_SAMPLE_HOLD) {
                lfo->last_random = lfo->next_random;
                lfo->next_random = (int16_t)((lcg_random() & 0xFFFF) - 32768);
            }
        }
    }
}

void lfo_set_enabled(uint8_t track, uint8_t enabled) {
    if (track >= LFO_MAX_TRACKS) return;
    g_lfo[track].enabled = enabled ? 1 : 0;
}

uint8_t lfo_is_enabled(uint8_t track) {
    if (track >= LFO_MAX_TRACKS) return 0;
    return g_lfo[track].enabled;
}

void lfo_set_waveform(uint8_t track, lfo_waveform_t waveform) {
    if (track >= LFO_MAX_TRACKS) return;
    if (waveform >= LFO_WAVEFORM_COUNT) return;
    g_lfo[track].waveform = waveform;
}

lfo_waveform_t lfo_get_waveform(uint8_t track) {
    if (track >= LFO_MAX_TRACKS) return LFO_WAVEFORM_SINE;
    return g_lfo[track].waveform;
}

void lfo_set_rate(uint8_t track, uint16_t rate_hundredths) {
    if (track >= LFO_MAX_TRACKS) return;
    if (rate_hundredths < 1) rate_hundredths = 1;
    if (rate_hundredths > 1000) rate_hundredths = 1000;
    g_lfo[track].rate_hundredths = rate_hundredths;
    calculate_phase_increment(track);
}

uint16_t lfo_get_rate(uint8_t track) {
    if (track >= LFO_MAX_TRACKS) return 0;
    return g_lfo[track].rate_hundredths;
}

void lfo_set_depth(uint8_t track, uint8_t depth) {
    if (track >= LFO_MAX_TRACKS) return;
    if (depth > 100) depth = 100;
    g_lfo[track].depth = depth;
}

uint8_t lfo_get_depth(uint8_t track) {
    if (track >= LFO_MAX_TRACKS) return 0;
    return g_lfo[track].depth;
}

void lfo_set_target(uint8_t track, lfo_target_t target) {
    if (track >= LFO_MAX_TRACKS) return;
    if (target >= LFO_TARGET_COUNT) return;
    g_lfo[track].target = target;
}

lfo_target_t lfo_get_target(uint8_t track) {
    if (track >= LFO_MAX_TRACKS) return LFO_TARGET_VELOCITY;
    return g_lfo[track].target;
}

void lfo_set_bpm_sync(uint8_t track, uint8_t bpm_sync) {
    if (track >= LFO_MAX_TRACKS) return;
    g_lfo[track].bpm_sync = bpm_sync ? 1 : 0;
    calculate_phase_increment(track);
}

uint8_t lfo_is_bpm_synced(uint8_t track) {
    if (track >= LFO_MAX_TRACKS) return 0;
    return g_lfo[track].bpm_sync;
}

void lfo_set_bpm_divisor(uint8_t track, uint8_t divisor) {
    if (track >= LFO_MAX_TRACKS) return;
    // Only allow 1, 2, 4, 8, 16, 32
    if (divisor != 1 && divisor != 2 && divisor != 4 && 
        divisor != 8 && divisor != 16 && divisor != 32) return;
    g_lfo[track].bpm_divisor = divisor;
    calculate_phase_increment(track);
}

uint8_t lfo_get_bpm_divisor(uint8_t track) {
    if (track >= LFO_MAX_TRACKS) return 0;
    return g_lfo[track].bpm_divisor;
}

void lfo_reset_phase(uint8_t track) {
    if (track >= LFO_MAX_TRACKS) return;
    g_lfo[track].phase = 0;
}

uint8_t lfo_get_velocity_value(uint8_t track, uint8_t base_velocity) {
    if (track >= LFO_MAX_TRACKS || !g_lfo[track].enabled) return base_velocity;
    if (g_lfo[track].target != LFO_TARGET_VELOCITY) return base_velocity;
    
    int16_t lfo_val = get_waveform_value(&g_lfo[track]);
    int32_t modulation = ((int32_t)lfo_val * g_lfo[track].depth) / 32767;  // -depth to +depth
    int32_t result = base_velocity + modulation;
    
    if (result < 1) result = 1;
    if (result > 127) result = 127;
    return (uint8_t)result;
}

int8_t lfo_get_timing_value(uint8_t track) {
    if (track >= LFO_MAX_TRACKS || !g_lfo[track].enabled) return 0;
    if (g_lfo[track].target != LFO_TARGET_TIMING) return 0;
    
    int16_t lfo_val = get_waveform_value(&g_lfo[track]);
    // Scale to ±12 ticks based on depth
    int32_t modulation = ((int32_t)lfo_val * g_lfo[track].depth * 12) / (32767 * 100);
    
    if (modulation < -12) modulation = -12;
    if (modulation > 12) modulation = 12;
    return (int8_t)modulation;
}

uint8_t lfo_get_pitch_value(uint8_t track, uint8_t base_note) {
    if (track >= LFO_MAX_TRACKS || !g_lfo[track].enabled) return base_note;
    if (g_lfo[track].target != LFO_TARGET_PITCH) return base_note;
    
    int16_t lfo_val = get_waveform_value(&g_lfo[track]);
    // Scale to ±12 semitones based on depth
    int32_t modulation = ((int32_t)lfo_val * g_lfo[track].depth * 12) / (32767 * 100);
    int32_t result = base_note + modulation;
    
    if (result < 0) result = 0;
    if (result > 127) result = 127;
    return (uint8_t)result;
}

void lfo_set_tempo(uint16_t bpm) {
    if (bpm < 20) bpm = 20;
    if (bpm > 300) bpm = 300;
    g_tempo_bpm = bpm;
    
    // Recalculate phase increments for all BPM-synced LFOs
    for (uint8_t i = 0; i < LFO_MAX_TRACKS; i++) {
        if (g_lfo[i].bpm_sync) {
            calculate_phase_increment(i);
        }
    }
}
