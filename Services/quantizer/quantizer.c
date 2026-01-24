/**
 * @file quantizer.c
 * @brief Timing Quantizer implementation
 */

#include "Services/quantizer/quantizer.h"
#include <string.h>
#include <stdlib.h>

#define DEFAULT_TEMPO 120
#define DEFAULT_PPQN 96
#define MIN_TEMPO 20
#define MAX_TEMPO 300
#define DEFAULT_STRENGTH 100
#define DEFAULT_LOOKAHEAD_MS 50

// Resolution name strings
static const char* resolution_names[] = {
    "1/4",
    "1/8",
    "1/8T",
    "1/16",
    "1/16T",
    "1/32",
    "1/32T",
    "1/64"
};

// Late mode name strings
static const char* late_mode_names[] = {
    "Nearest",
    "Forward",
    "Backward",
    "Off"
};

// Per-track quantizer configuration
typedef struct {
    uint8_t enabled;
    quantizer_resolution_t resolution;
    uint8_t strength;                    // 0-100%
    uint16_t lookahead_ms;               // Look-ahead window
    quantizer_late_mode_t late_mode;
    uint8_t swing;                       // 0-100 (50=straight)
    
    // Buffered notes
    quantizer_note_t notes[QUANTIZER_MAX_NOTES_PER_TRACK];
    uint8_t note_count;
    
    // Statistics
    uint32_t total_notes_quantized;
    int64_t total_offset_ms;             // For averaging
} quantizer_config_t;

static quantizer_config_t g_quantizer_config[QUANTIZER_MAX_TRACKS];
static uint16_t g_tempo = DEFAULT_TEMPO;
static uint16_t g_ppqn = DEFAULT_PPQN;

/**
 * @brief Calculate milliseconds per beat
 */
static uint32_t get_ms_per_beat(void) {
    // ms per beat = 60000 / BPM
    if (g_tempo == 0) return 500;  // Fallback to 120 BPM equivalent
    return (60000UL / g_tempo);
}

/**
 * @brief Calculate milliseconds per quarter note
 */
static uint32_t get_ms_per_quarter(void) {
    return get_ms_per_beat();
}

/**
 * @brief Calculate ticks per beat based on resolution
 */
static uint32_t get_ticks_per_grid(quantizer_resolution_t resolution) {
    switch (resolution) {
        case QUANTIZER_RES_QUARTER:
            return g_ppqn;              // 1 quarter note
        case QUANTIZER_RES_8TH:
            return g_ppqn / 2;          // 1/8 note
        case QUANTIZER_RES_8TH_TRIPLET:
            return g_ppqn / 3;          // 1/8 triplet
        case QUANTIZER_RES_16TH:
            return g_ppqn / 4;          // 1/16 note
        case QUANTIZER_RES_16TH_TRIPLET:
            return g_ppqn / 6;          // 1/16 triplet
        case QUANTIZER_RES_32ND:
            return g_ppqn / 8;          // 1/32 note
        case QUANTIZER_RES_32ND_TRIPLET:
            return g_ppqn / 12;         // 1/32 triplet
        case QUANTIZER_RES_64TH:
            return g_ppqn / 16;         // 1/64 note
        default:
            return g_ppqn / 4;          // Default to 16th
    }
}

/**
 * @brief Calculate milliseconds per grid interval
 */
static uint32_t get_ms_per_grid(quantizer_resolution_t resolution) {
    uint32_t ms_per_quarter = get_ms_per_quarter();
    
    switch (resolution) {
        case QUANTIZER_RES_QUARTER:
            return ms_per_quarter;
        case QUANTIZER_RES_8TH:
            return ms_per_quarter / 2;
        case QUANTIZER_RES_8TH_TRIPLET:
            return ms_per_quarter / 3;
        case QUANTIZER_RES_16TH:
            return ms_per_quarter / 4;
        case QUANTIZER_RES_16TH_TRIPLET:
            return ms_per_quarter / 6;
        case QUANTIZER_RES_32ND:
            return ms_per_quarter / 8;
        case QUANTIZER_RES_32ND_TRIPLET:
            return ms_per_quarter / 12;
        case QUANTIZER_RES_64TH:
            return ms_per_quarter / 16;
        default:
            return ms_per_quarter / 4;
    }
}

/**
 * @brief Apply swing to a grid position
 */
static uint32_t apply_swing(uint8_t track, uint32_t grid_time_ms, uint32_t grid_number) {
    quantizer_config_t* cfg = &g_quantizer_config[track];
    
    if (cfg->swing == 50) {
        return grid_time_ms;  // No swing
    }
    
    // Apply swing to every other grid position (off-beats)
    if (grid_number % 2 == 1) {
        uint32_t grid_interval = get_ms_per_grid(cfg->resolution);
        // Swing amount: 50 = no swing, 100 = max late, 0 = max early
        int32_t swing_offset = ((int32_t)cfg->swing - 50) * grid_interval / 100;
        return grid_time_ms + swing_offset;
    }
    
    return grid_time_ms;
}

/**
 * @brief Find nearest grid point
 */
static uint32_t find_nearest_grid(uint8_t track, uint32_t time_ms) {
    quantizer_config_t* cfg = &g_quantizer_config[track];
    uint32_t grid_interval = get_ms_per_grid(cfg->resolution);
    
    if (grid_interval == 0) return time_ms;
    
    uint32_t grid_number = time_ms / grid_interval;
    uint32_t prev_grid = grid_number * grid_interval;
    uint32_t next_grid = prev_grid + grid_interval;
    
    uint32_t dist_to_prev = time_ms - prev_grid;
    uint32_t dist_to_next = next_grid - time_ms;
    
    uint32_t nearest = (dist_to_prev < dist_to_next) ? prev_grid : next_grid;
    uint32_t nearest_num = nearest / grid_interval;
    
    return apply_swing(track, nearest, nearest_num);
}

/**
 * @brief Find next grid point
 */
static uint32_t find_next_grid(uint8_t track, uint32_t time_ms) {
    quantizer_config_t* cfg = &g_quantizer_config[track];
    uint32_t grid_interval = get_ms_per_grid(cfg->resolution);
    
    if (grid_interval == 0) return time_ms;
    
    uint32_t grid_number = (time_ms / grid_interval) + 1;
    uint32_t next_grid = grid_number * grid_interval;
    
    return apply_swing(track, next_grid, grid_number);
}

/**
 * @brief Find previous grid point
 */
static uint32_t find_prev_grid(uint8_t track, uint32_t time_ms) {
    quantizer_config_t* cfg = &g_quantizer_config[track];
    uint32_t grid_interval = get_ms_per_grid(cfg->resolution);
    
    if (grid_interval == 0) return time_ms;
    
    uint32_t grid_number = time_ms / grid_interval;
    uint32_t prev_grid = grid_number * grid_interval;
    
    return apply_swing(track, prev_grid, grid_number);
}

/**
 * @brief Quantize a time value based on configuration
 */
static uint32_t quantize_time_internal(uint8_t track, uint32_t time_ms) {
    quantizer_config_t* cfg = &g_quantizer_config[track];
    
    if (!cfg->enabled || cfg->strength == 0) {
        return time_ms;
    }
    
    uint32_t quantized_time;
    
    // Determine quantized position based on late mode
    switch (cfg->late_mode) {
        case QUANTIZER_LATE_SNAP_FORWARD:
            quantized_time = find_next_grid(track, time_ms);
            break;
            
        case QUANTIZER_LATE_SNAP_BACKWARD:
            quantized_time = find_prev_grid(track, time_ms);
            break;
            
        case QUANTIZER_LATE_SNAP_NEAREST:
        default:
            quantized_time = find_nearest_grid(track, time_ms);
            break;
    }
    
    // Apply strength (0-100%)
    if (cfg->strength < 100) {
        int32_t offset = (int32_t)quantized_time - (int32_t)time_ms;
        offset = (offset * cfg->strength) / 100;
        quantized_time = time_ms + offset;
    }
    
    return quantized_time;
}

/**
 * @brief Add a note to the buffer
 */
static uint8_t add_note_to_buffer(uint8_t track, uint8_t note, uint8_t velocity,
                                   uint8_t channel, uint32_t time_ms) {
    quantizer_config_t* cfg = &g_quantizer_config[track];
    
    // Find empty slot
    for (uint8_t i = 0; i < QUANTIZER_MAX_NOTES_PER_TRACK; i++) {
        if (!cfg->notes[i].active) {
            cfg->notes[i].note = note;
            cfg->notes[i].velocity = velocity;
            cfg->notes[i].channel = channel;
            cfg->notes[i].original_time_ms = time_ms;
            cfg->notes[i].quantized_time_ms = quantize_time_internal(track, time_ms);
            cfg->notes[i].active = 1;
            cfg->note_count++;
            
            // Update statistics
            cfg->total_notes_quantized++;
            cfg->total_offset_ms += (int32_t)cfg->notes[i].quantized_time_ms - 
                                    (int32_t)cfg->notes[i].original_time_ms;
            
            return 1;
        }
    }
    
    return 0;  // Buffer full
}

// Public API Implementation

void quantizer_init(uint16_t tempo, uint16_t ppqn) {
    memset(g_quantizer_config, 0, sizeof(g_quantizer_config));
    
    g_tempo = (tempo >= MIN_TEMPO && tempo <= MAX_TEMPO) ? tempo : DEFAULT_TEMPO;
    g_ppqn = (ppqn > 0) ? ppqn : DEFAULT_PPQN;
    
    // Initialize all tracks with defaults
    for (uint8_t i = 0; i < QUANTIZER_MAX_TRACKS; i++) {
        g_quantizer_config[i].enabled = 0;
        g_quantizer_config[i].resolution = QUANTIZER_RES_16TH;
        g_quantizer_config[i].strength = DEFAULT_STRENGTH;
        g_quantizer_config[i].lookahead_ms = DEFAULT_LOOKAHEAD_MS;
        g_quantizer_config[i].late_mode = QUANTIZER_LATE_SNAP_NEAREST;
        g_quantizer_config[i].swing = 50;
        g_quantizer_config[i].note_count = 0;
        g_quantizer_config[i].total_notes_quantized = 0;
        g_quantizer_config[i].total_offset_ms = 0;
    }
}

void quantizer_set_tempo(uint16_t tempo) {
    if (tempo >= MIN_TEMPO && tempo <= MAX_TEMPO) {
        g_tempo = tempo;
    }
}

uint16_t quantizer_get_tempo(void) {
    return g_tempo;
}

void quantizer_set_ppqn(uint16_t ppqn) {
    if (ppqn > 0) {
        g_ppqn = ppqn;
    }
}

uint16_t quantizer_get_ppqn(void) {
    return g_ppqn;
}

void quantizer_set_enabled(uint8_t track, uint8_t enabled) {
    if (track < QUANTIZER_MAX_TRACKS) {
        g_quantizer_config[track].enabled = enabled ? 1 : 0;
    }
}

uint8_t quantizer_is_enabled(uint8_t track) {
    if (track < QUANTIZER_MAX_TRACKS) {
        return g_quantizer_config[track].enabled;
    }
    return 0;
}

void quantizer_set_resolution(uint8_t track, quantizer_resolution_t resolution) {
    if (track < QUANTIZER_MAX_TRACKS && resolution < QUANTIZER_RES_COUNT) {
        g_quantizer_config[track].resolution = resolution;
    }
}

quantizer_resolution_t quantizer_get_resolution(uint8_t track) {
    if (track < QUANTIZER_MAX_TRACKS) {
        return g_quantizer_config[track].resolution;
    }
    return QUANTIZER_RES_16TH;
}

void quantizer_set_strength(uint8_t track, uint8_t strength) {
    if (track < QUANTIZER_MAX_TRACKS) {
        g_quantizer_config[track].strength = (strength > 100) ? 100 : strength;
    }
}

uint8_t quantizer_get_strength(uint8_t track) {
    if (track < QUANTIZER_MAX_TRACKS) {
        return g_quantizer_config[track].strength;
    }
    return DEFAULT_STRENGTH;
}

void quantizer_set_lookahead(uint8_t track, uint16_t window_ms) {
    if (track < QUANTIZER_MAX_TRACKS) {
        g_quantizer_config[track].lookahead_ms = (window_ms > 500) ? 500 : window_ms;
    }
}

uint16_t quantizer_get_lookahead(uint8_t track) {
    if (track < QUANTIZER_MAX_TRACKS) {
        return g_quantizer_config[track].lookahead_ms;
    }
    return DEFAULT_LOOKAHEAD_MS;
}

void quantizer_set_late_mode(uint8_t track, quantizer_late_mode_t mode) {
    if (track < QUANTIZER_MAX_TRACKS && mode < QUANTIZER_LATE_COUNT) {
        g_quantizer_config[track].late_mode = mode;
    }
}

quantizer_late_mode_t quantizer_get_late_mode(uint8_t track) {
    if (track < QUANTIZER_MAX_TRACKS) {
        return g_quantizer_config[track].late_mode;
    }
    return QUANTIZER_LATE_SNAP_NEAREST;
}

void quantizer_set_swing(uint8_t track, uint8_t swing) {
    if (track < QUANTIZER_MAX_TRACKS) {
        g_quantizer_config[track].swing = (swing > 100) ? 100 : swing;
    }
}

uint8_t quantizer_get_swing(uint8_t track) {
    if (track < QUANTIZER_MAX_TRACKS) {
        return g_quantizer_config[track].swing;
    }
    return 50;
}

uint8_t quantizer_process_note_on(uint8_t track, uint8_t note, uint8_t velocity,
                                   uint8_t channel, uint32_t time_ms) {
    if (track >= QUANTIZER_MAX_TRACKS) {
        return 0;
    }
    
    quantizer_config_t* cfg = &g_quantizer_config[track];
    
    if (!cfg->enabled) {
        return 0;  // Not enabled, note should be played immediately
    }
    
    return add_note_to_buffer(track, note, velocity, channel, time_ms);
}

uint8_t quantizer_process_note_on_ticks(uint8_t track, uint8_t note, uint8_t velocity,
                                         uint8_t channel, uint32_t tick_position) {
    if (track >= QUANTIZER_MAX_TRACKS || g_ppqn == 0) {
        return 0;
    }
    
    // Convert ticks to milliseconds
    uint32_t ms_per_tick = get_ms_per_quarter() / g_ppqn;
    uint32_t time_ms = tick_position * ms_per_tick;
    
    return quantizer_process_note_on(track, note, velocity, channel, time_ms);
}

uint32_t quantizer_calculate_time(uint8_t track, uint32_t time_ms) {
    if (track >= QUANTIZER_MAX_TRACKS) {
        return time_ms;
    }
    
    return quantize_time_internal(track, time_ms);
}

uint32_t quantizer_calculate_ticks(uint8_t track, uint32_t tick_position) {
    if (track >= QUANTIZER_MAX_TRACKS || g_ppqn == 0) {
        return tick_position;
    }
    
    quantizer_config_t* cfg = &g_quantizer_config[track];
    
    if (!cfg->enabled) {
        return tick_position;
    }
    
    uint32_t ticks_per_grid = get_ticks_per_grid(cfg->resolution);
    
    if (ticks_per_grid == 0) return tick_position;
    
    uint32_t grid_number = tick_position / ticks_per_grid;
    uint32_t prev_grid = grid_number * ticks_per_grid;
    uint32_t next_grid = prev_grid + ticks_per_grid;
    
    uint32_t quantized_ticks;
    
    switch (cfg->late_mode) {
        case QUANTIZER_LATE_SNAP_FORWARD:
            quantized_ticks = next_grid;
            break;
            
        case QUANTIZER_LATE_SNAP_BACKWARD:
            quantized_ticks = prev_grid;
            break;
            
        case QUANTIZER_LATE_SNAP_NEAREST:
        default: {
            uint32_t dist_to_prev = tick_position - prev_grid;
            uint32_t dist_to_next = next_grid - tick_position;
            quantized_ticks = (dist_to_prev < dist_to_next) ? prev_grid : next_grid;
            break;
        }
    }
    
    // Apply strength
    if (cfg->strength < 100) {
        int32_t offset = (int32_t)quantized_ticks - (int32_t)tick_position;
        offset = (offset * cfg->strength) / 100;
        quantized_ticks = tick_position + offset;
    }
    
    return quantized_ticks;
}

int32_t quantizer_get_offset(uint8_t track, uint32_t time_ms) {
    if (track >= QUANTIZER_MAX_TRACKS) {
        return 0;
    }
    
    uint32_t quantized = quantize_time_internal(track, time_ms);
    return (int32_t)quantized - (int32_t)time_ms;
}

uint32_t quantizer_get_next_grid(uint8_t track, uint32_t time_ms) {
    if (track >= QUANTIZER_MAX_TRACKS) {
        return time_ms;
    }
    
    return find_next_grid(track, time_ms);
}

uint32_t quantizer_get_prev_grid(uint8_t track, uint32_t time_ms) {
    if (track >= QUANTIZER_MAX_TRACKS) {
        return time_ms;
    }
    
    return find_prev_grid(track, time_ms);
}

uint32_t quantizer_get_grid_interval_ms(uint8_t track) {
    if (track >= QUANTIZER_MAX_TRACKS) {
        return 0;
    }
    
    return get_ms_per_grid(g_quantizer_config[track].resolution);
}

uint32_t quantizer_get_grid_interval_ticks(uint8_t track) {
    if (track >= QUANTIZER_MAX_TRACKS) {
        return 0;
    }
    
    return get_ticks_per_grid(g_quantizer_config[track].resolution);
}

uint8_t quantizer_is_on_grid(uint8_t track, uint32_t time_ms, uint16_t tolerance_ms) {
    if (track >= QUANTIZER_MAX_TRACKS) {
        return 0;
    }
    
    uint32_t nearest = find_nearest_grid(track, time_ms);
    uint32_t distance = (time_ms > nearest) ? (time_ms - nearest) : (nearest - time_ms);
    
    return (distance <= tolerance_ms) ? 1 : 0;
}

uint8_t quantizer_get_ready_notes(uint8_t track, uint32_t current_time_ms,
                                   quantizer_note_t* notes) {
    if (track >= QUANTIZER_MAX_TRACKS || !notes) {
        return 0;
    }
    
    quantizer_config_t* cfg = &g_quantizer_config[track];
    uint8_t count = 0;
    
    for (uint8_t i = 0; i < QUANTIZER_MAX_NOTES_PER_TRACK; i++) {
        if (cfg->notes[i].active && 
            cfg->notes[i].quantized_time_ms <= current_time_ms) {
            // Copy note to output
            notes[count] = cfg->notes[i];
            count++;
            
            // Remove from buffer
            cfg->notes[i].active = 0;
            cfg->note_count--;
        }
    }
    
    return count;
}

void quantizer_reset(uint8_t track) {
    if (track >= QUANTIZER_MAX_TRACKS) {
        return;
    }
    
    quantizer_config_t* cfg = &g_quantizer_config[track];
    
    // Clear all buffered notes
    for (uint8_t i = 0; i < QUANTIZER_MAX_NOTES_PER_TRACK; i++) {
        cfg->notes[i].active = 0;
    }
    cfg->note_count = 0;
    
    // Don't reset statistics or settings
}

void quantizer_reset_all(void) {
    for (uint8_t i = 0; i < QUANTIZER_MAX_TRACKS; i++) {
        quantizer_reset(i);
    }
}

const char* quantizer_get_resolution_name(quantizer_resolution_t resolution) {
    if (resolution < QUANTIZER_RES_COUNT) {
        return resolution_names[resolution];
    }
    return "Unknown";
}

const char* quantizer_get_late_mode_name(quantizer_late_mode_t mode) {
    if (mode < QUANTIZER_LATE_COUNT) {
        return late_mode_names[mode];
    }
    return "Unknown";
}

void quantizer_get_stats(uint8_t track, uint8_t* notes_buffered,
                        uint32_t* notes_quantized, int32_t* avg_offset_ms) {
    if (track >= QUANTIZER_MAX_TRACKS) {
        if (notes_buffered) *notes_buffered = 0;
        if (notes_quantized) *notes_quantized = 0;
        if (avg_offset_ms) *avg_offset_ms = 0;
        return;
    }
    
    quantizer_config_t* cfg = &g_quantizer_config[track];
    
    if (notes_buffered) {
        *notes_buffered = cfg->note_count;
    }
    
    if (notes_quantized) {
        *notes_quantized = cfg->total_notes_quantized;
    }
    
    if (avg_offset_ms) {
        if (cfg->total_notes_quantized > 0) {
            *avg_offset_ms = (int32_t)(cfg->total_offset_ms / cfg->total_notes_quantized);
        } else {
            *avg_offset_ms = 0;
        }
    }
}
