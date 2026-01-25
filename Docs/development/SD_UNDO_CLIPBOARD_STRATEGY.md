# Strategy: SD-Based Undo & Clipboard System

**Date**: 2026-01-25  
**Goal**: Use SD card for undo history and clipboards to free RAM/CCMRAM  
**Potential Savings**: ~119 KB RAM in production mode

---

## Current Memory Usage

### Production Mode (MODULE_ENABLE_UI_PAGE_PIANOROLL=0)

```
RAM (128 KB):
  ✅ undo_stacks[4]:      99 KB  (depth=5, 4 tracks)
  ✅ g_automation[4]:      8 KB  (CC automation per track)
  - Other systems:        21 KB
  --------------------------------
  Total:                 ~128 KB (FULL!)

CCMRAM (64 KB):
  - g_tr[4]:              17 KB  (track event buffers)
  - OLED framebuffer:      8 KB
  - Free:                 39 KB
  --------------------------------
  Total:                  25 KB / 64 KB ✅
```

### The Problem

With pianoroll disabled, we have ~28 KB free RAM. But:
- Undo stacks consume 99 KB (77% of RAM!)
- Cannot increase undo depth beyond 5
- Cannot enable track/scene clipboards
- Limited room for future features

---

## Solution: SD-Backed Undo & Clipboard System

### Strategy Overview

Store undo history and clipboards on SD card instead of RAM:

```
Operation Flow:
  User Action → Push to Undo
                   ↓
            Save state to SD: 0:/looper/undo_track0_level3.bin
                   ↓
            Keep only index in RAM (minimal)
                   ↓
  User presses Undo
                   ↓
            Load state from SD
                   ↓
            Restore track state
```

### Benefits

1. **RAM Savings**: Free up 99 KB for undo history
2. **Increased Capacity**: Support deeper undo (10-20 levels vs 5)
3. **Enable Clipboards**: Track/scene copy-paste with no RAM cost
4. **Persistent Undo**: Survives power cycles
5. **Future-Proof**: Easy to extend

### Trade-offs

1. **Latency**: SD read/write ~10-50ms (acceptable for undo/redo)
2. **SD Wear**: ~1000-10000 write cycles per sector (not a problem)
3. **Code Complexity**: More error handling needed
4. **SD Required**: Undo disabled if SD fails (graceful degradation)

---

## Implementation Plan

### Phase 1: SD-Backed Undo System

**Goal**: Move undo_stacks from RAM to SD

**Current System**:
```c
// In RAM: 99 KB
static undo_stack_t undo_stacks[LOOPER_TRACKS];  // 4 tracks × ~25 KB
```

**New System**:
```c
// In RAM: ~1 KB (just metadata)
typedef struct {
  uint8_t count;        // Number of undo levels available
  uint8_t write_idx;    // Next write position
  uint8_t has_data[UNDO_STACK_DEPTH];  // Which levels have data
  char filenames[UNDO_STACK_DEPTH][32];  // SD file paths
} undo_metadata_t;

static undo_metadata_t undo_meta[LOOPER_TRACKS];  // ~2 KB total
```

**File Structure**:
```
SD Card:
  0:/looper/undo/
    track0_level0.bin  (~6 KB per level)
    track0_level1.bin
    track0_level2.bin
    ...
    track3_level9.bin
```

**API Changes**: None! Keep same external API:
```c
void looper_undo_push(uint8_t track);       // Save to SD
int looper_undo(uint8_t track);             // Load from SD  
int looper_redo(uint8_t track);             // Load from SD
uint8_t looper_undo_available(uint8_t track);
uint8_t looper_redo_available(uint8_t track);
```

---

### Phase 2: SD-Backed Clipboard System

**Goal**: Enable copy/paste with zero RAM cost

**New System**:
```c
// In RAM: ~100 bytes (just flags)
typedef struct {
  uint8_t has_track_data;
  uint8_t has_scene_data;
} clipboard_metadata_t;

static clipboard_metadata_t clipboard_meta;
```

**File Structure**:
```
SD Card:
  0:/looper/clipboard/
    track.bin        (~4 KB - single track)
    scene.bin        (~16 KB - 4 tracks)
```

**API**: Unchanged
```c
int looper_copy_track(uint8_t track);
int looper_paste_track(uint8_t track);
int looper_copy_scene(uint8_t scene);
int looper_paste_scene(uint8_t scene);
```

---

## Detailed Implementation

### Step 1: SD Undo - Core Functions

**File**: `Services/looper/looper_undo_sd.c` (new)

```c
#include "Services/looper/looper_undo_sd.h"
#include "ff.h"  // FatFS
#include <string.h>
#include <stdio.h>

// Configuration
#define UNDO_SD_PATH "0:/looper/undo"
#define UNDO_MAX_DEPTH 10  // Increased from 5!

// Metadata (in RAM - minimal)
typedef struct {
  uint8_t count;
  uint8_t write_idx;
  uint8_t has_data[UNDO_MAX_DEPTH];
} undo_metadata_t;

static undo_metadata_t g_undo_meta[4];  // 4 tracks
static uint8_t g_sd_available = 0;

/**
 * @brief Initialize SD-backed undo system
 * @return 0 on success, -1 if SD not available
 */
int looper_undo_sd_init(void) {
  // Check if SD is mounted
  FATFS* fs;
  DWORD fre_clust;
  if (f_getfree("0:", &fre_clust, &fs) != FR_OK) {
    g_sd_available = 0;
    return -1;
  }
  
  // Create undo directory if it doesn't exist
  f_mkdir(UNDO_SD_PATH);
  
  // Clear metadata
  memset(g_undo_meta, 0, sizeof(g_undo_meta));
  
  g_sd_available = 1;
  return 0;
}

/**
 * @brief Save current track state to SD
 */
int looper_undo_sd_push(uint8_t track, const looper_track_t* tr) {
  if (!g_sd_available || track >= 4) return -1;
  
  undo_metadata_t* meta = &g_undo_meta[track];
  
  // Generate filename
  char filename[64];
  snprintf(filename, sizeof(filename), 
           "%s/track%u_lv%u.bin", 
           UNDO_SD_PATH, track, meta->write_idx);
  
  // Open file for writing
  FIL fp;
  if (f_open(&fp, filename, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) {
    return -1;
  }
  
  // Write track state (simplified version)
  UINT written;
  
  // Write header
  uint32_t magic = 0x554E444F;  // "UNDO"
  f_write(&fp, &magic, 4, &written);
  
  // Write track data
  f_write(&fp, &tr->loop_len_ticks, sizeof(tr->loop_len_ticks), &written);
  f_write(&fp, &tr->loop_beats, sizeof(tr->loop_beats), &written);
  f_write(&fp, &tr->quant, sizeof(tr->quant), &written);
  f_write(&fp, &tr->count, sizeof(tr->count), &written);
  
  // Write events (up to 256)
  uint32_t event_count = tr->count < 256 ? tr->count : 256;
  for (uint32_t i = 0; i < event_count; i++) {
    f_write(&fp, &tr->ev[i].tick, sizeof(tr->ev[i].tick), &written);
    f_write(&fp, &tr->ev[i].len, sizeof(tr->ev[i].len), &written);
    f_write(&fp, &tr->ev[i].b0, sizeof(tr->ev[i].b0), &written);
    f_write(&fp, &tr->ev[i].b1, sizeof(tr->ev[i].b1), &written);
    f_write(&fp, &tr->ev[i].b2, sizeof(tr->ev[i].b2), &written);
  }
  
  f_close(&fp);
  
  // Update metadata
  meta->has_data[meta->write_idx] = 1;
  meta->write_idx = (meta->write_idx + 1) % UNDO_MAX_DEPTH;
  if (meta->count < UNDO_MAX_DEPTH) meta->count++;
  
  return 0;
}

/**
 * @brief Restore track state from SD
 */
int looper_undo_sd_pop(uint8_t track, looper_track_t* tr) {
  if (!g_sd_available || track >= 4) return -1;
  
  undo_metadata_t* meta = &g_undo_meta[track];
  if (meta->count == 0) return -1;  // Nothing to undo
  
  // Calculate previous index
  uint8_t prev_idx = (meta->write_idx == 0) 
                     ? (UNDO_MAX_DEPTH - 1) 
                     : (meta->write_idx - 1);
  
  if (!meta->has_data[prev_idx]) return -1;
  
  // Generate filename
  char filename[64];
  snprintf(filename, sizeof(filename), 
           "%s/track%u_lv%u.bin", 
           UNDO_SD_PATH, track, prev_idx);
  
  // Open file for reading
  FIL fp;
  if (f_open(&fp, filename, FA_READ) != FR_OK) {
    return -1;
  }
  
  // Read and verify header
  uint32_t magic;
  UINT read_bytes;
  f_read(&fp, &magic, 4, &read_bytes);
  if (magic != 0x554E444F) {  // "UNDO"
    f_close(&fp);
    return -1;
  }
  
  // Read track data
  f_read(&fp, &tr->loop_len_ticks, sizeof(tr->loop_len_ticks), &read_bytes);
  f_read(&fp, &tr->loop_beats, sizeof(tr->loop_beats), &read_bytes);
  f_read(&fp, &tr->quant, sizeof(tr->quant), &read_bytes);
  f_read(&fp, &tr->count, sizeof(tr->count), &read_bytes);
  
  // Read events
  for (uint32_t i = 0; i < tr->count && i < 256; i++) {
    f_read(&fp, &tr->ev[i].tick, sizeof(tr->ev[i].tick), &read_bytes);
    f_read(&fp, &tr->ev[i].len, sizeof(tr->ev[i].len), &read_bytes);
    f_read(&fp, &tr->ev[i].b0, sizeof(tr->ev[i].b0), &read_bytes);
    f_read(&fp, &tr->ev[i].b1, sizeof(tr->ev[i].b1), &read_bytes);
    f_read(&fp, &tr->ev[i].b2, sizeof(tr->ev[i].b2), &read_bytes);
  }
  
  f_close(&fp);
  
  // Update metadata
  meta->write_idx = prev_idx;
  meta->count--;
  
  return 0;
}

/**
 * @brief Check if undo is available for a track
 */
uint8_t looper_undo_sd_available(uint8_t track) {
  if (!g_sd_available || track >= 4) return 0;
  return g_undo_meta[track].count > 0 ? 1 : 0;
}
```

---

### Step 2: Integration with Existing Code

**File**: `Services/looper/looper.c` - Add configuration option

```c
// At top of file:
#ifndef LOOPER_UNDO_USE_SD
  #define LOOPER_UNDO_USE_SD 1  // Default: Use SD for undo (saves 99KB RAM!)
#endif

#if LOOPER_UNDO_USE_SD
  #include "Services/looper/looper_undo_sd.h"
#else
  // Original RAM-based undo (for systems without SD)
  static undo_stack_t undo_stacks[LOOPER_TRACKS];
#endif

// In looper_init():
void looper_init(void) {
  // ... existing code ...
  
  #if LOOPER_UNDO_USE_SD
    looper_undo_sd_init();
  #endif
}

// Modify undo functions:
void looper_undo_push(uint8_t track) {
  #if LOOPER_UNDO_USE_SD
    looper_undo_sd_push(track, &g_tr[track]);
  #else
    // Original RAM-based implementation
    // ... existing code ...
  #endif
}

int looper_undo(uint8_t track) {
  #if LOOPER_UNDO_USE_SD
    return looper_undo_sd_pop(track, &g_tr[track]);
  #else
    // Original RAM-based implementation
    // ... existing code ...
  #endif
}
```

---

## Performance Analysis

### Latency Measurements

**RAM-Based Undo** (current):
- Push: ~0.1 ms (memcpy in RAM)
- Pop: ~0.1 ms (memcpy in RAM)
- **Total**: 0.2 ms

**SD-Based Undo** (proposed):
- Push: ~15-30 ms (SD write)
- Pop: ~10-20 ms (SD read)
- **Total**: 25-50 ms

**Is this acceptable?**
- ✅ YES for undo/redo (user doesn't notice 30ms)
- ✅ YES for copy/paste operations
- ❌ NO for real-time MIDI processing (but undo isn't real-time)

### SD Write Endurance

**Typical SD Card**: 10,000-100,000 write cycles per sector

**Usage Pattern**:
- User makes edit → undo push
- Typical session: 50-200 undo pushes
- Daily usage: 200-500 undo pushes
- **Lifetime**: 10,000 / 500 = 20 years ✅

**Wear Leveling**: Modern SD cards automatically distribute writes

---

## Memory Savings Summary

### Before (Current Production Mode)

```
RAM Usage:
  undo_stacks[4]:         99 KB
  g_automation[4]:         8 KB
  Other:                  21 KB
  --------------------------------
  Total:                 128 KB / 128 KB ❌ FULL

Limitations:
  - Cannot increase undo depth beyond 5
  - Cannot enable clipboards
  - No room for new features
```

### After (SD-Backed System)

```
RAM Usage:
  undo_metadata[4]:        2 KB  (was 99 KB) ✅ SAVES 97 KB!
  clipboard_metadata:      0.1 KB (was disabled)
  g_automation[4]:         8 KB
  Other:                  21 KB
  --------------------------------
  Total:                  31 KB / 128 KB ✅ 97 KB FREE!

Benefits:
  ✅ 97 KB RAM freed
  ✅ Undo depth: 5 → 10 levels (2x improvement)
  ✅ Clipboards enabled (track + scene)
  ✅ Room for future features
  ✅ Persistent undo (survives reboot)
```

---

## Configuration Options

### Option 1: Hybrid Mode (Recommended)

```c
// Best of both worlds
#define LOOPER_UNDO_USE_SD 1          // Primary: SD-backed undo
#define LOOPER_UNDO_RAM_CACHE 1       // Cache last level in RAM for instant undo
#define LOOPER_UNDO_SD_DEPTH 10       // 10 levels on SD
#define LOOPER_UNDO_RAM_CACHE_SIZE 1  // Last 1 level cached in RAM
```

**Benefits**:
- First undo: instant (from RAM cache)
- Subsequent undos: 20ms (from SD)
- Only 10 KB RAM for cache (vs 99 KB)

### Option 2: Pure SD Mode (Maximum RAM Savings)

```c
#define LOOPER_UNDO_USE_SD 1
#define LOOPER_UNDO_RAM_CACHE 0
#define LOOPER_UNDO_SD_DEPTH 10
```

**Benefits**:
- Only 2 KB RAM for metadata
- 97 KB RAM freed
- All undos: 20ms latency

### Option 3: Pure RAM Mode (No SD, Original)

```c
#define LOOPER_UNDO_USE_SD 0
#define LOOPER_UNDO_STACK_DEPTH 5
```

**Use when**:
- SD card not available
- Maximum performance required
- RAM not constrained

---

## Implementation Roadmap

### Phase 1: Foundation (1-2 days)
- [ ] Create `looper_undo_sd.c` / `.h`
- [ ] Implement SD file write/read functions
- [ ] Add configuration flags
- [ ] Basic testing with 1 track

### Phase 2: Integration (1 day)
- [ ] Integrate with existing `looper_undo_push()`
- [ ] Integrate with existing `looper_undo()` / `looper_redo()`
- [ ] Add error handling (SD failure → disable undo gracefully)
- [ ] Test all 4 tracks

### Phase 3: Clipboards (1 day)
- [ ] Create `looper_clipboard_sd.c` / `.h`
- [ ] Implement track copy/paste to SD
- [ ] Implement scene copy/paste to SD
- [ ] Enable clipboards by default

### Phase 4: Optimization (1 day)
- [ ] Add RAM cache for last undo level (instant first undo)
- [ ] Optimize file format (compression?)
- [ ] Performance testing and tuning

### Phase 5: Testing (1-2 days)
- [ ] Stress test: 1000 undo/redo cycles
- [ ] Error handling: unplug SD during undo
- [ ] Memory leak detection
- [ ] Real-world usage testing

**Total Estimated Time**: 5-7 days

---

## Risk Analysis

### Risk 1: SD Card Failure

**Probability**: Low  
**Impact**: Undo/clipboard disabled  
**Mitigation**:
- Graceful degradation (disable features, don't crash)
- UI notification: "Undo disabled (SD error)"
- Fall back to single-level undo in RAM?

### Risk 2: Performance Issues

**Probability**: Low  
**Impact**: User notices lag on undo  
**Mitigation**:
- RAM cache for last undo level (instant)
- Background writes (write to SD in separate task)
- Measure actual latency on target hardware

### Risk 3: File Corruption

**Probability**: Very Low  
**Impact**: Lost undo history  
**Mitigation**:
- Magic number validation (0x554E444F)
- CRC checksum in file
- If file corrupt, skip that level (don't crash)

### Risk 4: Increased Code Complexity

**Probability**: High  
**Impact**: More code to maintain  
**Mitigation**:
- Clean abstraction layer
- Keep old RAM-based code as fallback
- Comprehensive documentation

---

## Alternative Approaches

### Alternative 1: Compress Undo in RAM

**Idea**: Use zlib to compress undo states in RAM  
**Savings**: ~50-70% compression (99 KB → 30-50 KB)  
**Pros**: No SD dependency, faster than SD  
**Cons**: CPU overhead, still uses significant RAM  

### Alternative 2: Circular Buffer in CCMRAM

**Idea**: Move undo to CCMRAM, reduce depth to 3  
**Savings**: Frees 99 KB RAM, uses 30 KB CCMRAM  
**Pros**: Fast (no SD latency)  
**Cons**: CCMRAM nearly full (25 + 30 = 55 / 64 KB)  

### Alternative 3: Hybrid RAM + SD

**Idea**: Keep 1 level in RAM (instant), rest on SD  
**Savings**: 89 KB RAM freed (keep 10 KB for cache)  
**Pros**: Best of both worlds  
**Cons**: More complex implementation  

**Recommended**: Alternative 3 (Hybrid)

---

## Conclusion

SD-backed undo and clipboard system is **highly recommended**:

✅ **Pros**:
- Frees 97 KB RAM (75% of total RAM!)
- Doubles undo capacity (5 → 10 levels)
- Enables clipboards with zero RAM cost
- Persistent across power cycles
- Room for future features

⚠️ **Cons**:
- 20-30ms latency per undo (acceptable for UI operations)
- Requires SD card (graceful fallback if unavailable)
- Slightly more complex code

**Recommendation**: Implement Hybrid Mode (RAM cache + SD storage) for best user experience.

---

**Status**: PROPOSED  
**Next Step**: Prototype SD undo functions and measure actual latency  
**Decision Required**: Approve for implementation?
