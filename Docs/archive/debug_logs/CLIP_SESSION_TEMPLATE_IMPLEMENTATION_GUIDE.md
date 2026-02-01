# Clip Library, Session Management, and Template System - Implementation Guide

**Date**: 2026-01-27  
**Status**: API Headers Complete, Implementation Pending  
**Purpose**: Guide for implementing the three requested features

---

## Overview

This document provides the implementation plan for three new looper features requested based on MIOS32 LoopA analysis:

1. **Clip Library** - Save/load reusable loops
2. **Session Management** - Save/load complete looper states
3. **Template System** - Predefined configurations

---

## API Headers Created

Three new header files define the complete API:

1. `Services/looper/looper_clip_library.h` - Clip library management
2. `Services/looper/looper_session.h` - Session save/load
3. `Services/looper/looper_templates.h` - Template system

These headers provide:
- ✅ Complete API definitions
- ✅ Comprehensive documentation
- ✅ Type definitions and enums
- ✅ Configuration macros
- ✅ Function prototypes

---

## Implementation Priority

### Phase 1: Clip Library (Highest Value)

**Why First**: Most useful standalone feature, minimal dependencies

**Implementation Order**:
1. Create `Services/looper/looper_clip_library.c`
2. Implement index management (in-RAM clip list)
3. Implement save/load using existing `looper_save_track()` / `looper_load_track()`
4. Add directory management (`/clips/`)
5. Implement metadata caching
6. Add CLI commands for testing

**Estimated RAM Impact**: ~2.5 KB (64 clips × 40 bytes per clip_info_t)

**Key Functions to Implement**:
```c
// Priority 1 - Core functionality
looper_clip_library_init()     // Load index from SD
looper_clip_save()              // Save track as clip
looper_clip_load()              // Load clip into track
looper_clip_list()              // List all clips

// Priority 2 - Management
looper_clip_delete()            // Delete clip
looper_clip_get_info()          // Get clip metadata
looper_clip_refresh()           // Rebuild index

// Priority 3 - Advanced
looper_clip_rename()            // Rename clip
looper_clip_list_by_category() // Filter by category
```

**Implementation Tips**:
- Reuse existing `looper_save_track()` infrastructure
- Store index in `/clips/.index` file
- Lazy-load index on first use
- Use FATFS directory scanning for refresh

---

### Phase 2: Session Management (Medium Value)

**Why Second**: Builds on clip library concepts, high user value

**Implementation Order**:
1. Create `Services/looper/looper_session.c`
2. Define session file format (binary or JSON)
3. Implement session save (all tracks + settings)
4. Implement session load
5. Add quick-save slots
6. Implement auto-save (optional)
7. Add CLI commands

**Estimated RAM Impact**: ~1.5 KB (32 sessions × 48 bytes per session_info_t)

**Key Functions to Implement**:
```c
// Priority 1 - Core
looper_session_init()           // Initialize system
looper_session_save()           // Save current state
looper_session_load()           // Load session
looper_session_list()           // List sessions

// Priority 2 - Quick save
looper_session_quick_save()     // Fast save to slot
looper_session_quick_load()     // Fast load from slot

// Priority 3 - Auto-save
looper_session_autosave()       // Automatic backup
looper_session_set_autosave()   // Configure auto-save
```

**Session File Structure**:
```c
typedef struct {
  uint32_t magic;                    // 'SESS'
  uint32_t version;                  // Format version
  looper_transport_t transport;      // BPM, time signature
  uint8_t current_scene;             // Active scene
  uint8_t track_mutes[4];            // Mute states
  uint8_t track_solos[4];            // Solo states
  char track_clip_paths[4][64];      // Paths to track data files
  // ... additional settings
} session_file_header_t;
```

**Implementation Tips**:
- Save track data as separate `.clip` files
- Session file contains metadata + references
- Atomic writes using temp files
- Validate on load (magic number, version check)

---

### Phase 3: Template System (Lower Value)

**Why Last**: Nice-to-have, can be implemented as special sessions

**Implementation Order**:
1. Create `Services/looper/looper_templates.c`
2. Define built-in templates (empty 4/8/16 bar, etc.)
3. Implement template application
4. Add user template creation
5. Implement template listing
6. Add CLI commands

**Estimated RAM Impact**: ~1.5 KB (32 templates × 48 bytes per template_info_t)

**Key Functions to Implement**:
```c
// Priority 1 - Core
looper_template_init()          // Register built-ins
looper_template_apply()         // Apply template
looper_template_apply_builtin() // Apply by ID
looper_template_list()          // List all templates

// Priority 2 - Custom templates
looper_template_save()          // Save as template
looper_template_delete()        // Delete user template
looper_template_get_info()      // Get metadata
```

**Built-in Templates to Implement**:
```c
// LOOPER_TEMPLATE_EMPTY_4BAR
- 4 tracks, empty
- 4 beats per track
- 120 BPM
- Scene 0

// LOOPER_TEMPLATE_ACCORDION_FULL
- Track 0: Bass (16 beats, channel 1)
- Track 1: Chords (16 beats, channel 2)
- Track 2: Melody (16 beats, channel 3)
- Track 3: Percussion (16 beats, channel 10)
- 120 BPM, 4/4 time
```

**Implementation Tips**:
- Templates are just pre-configured sessions
- Store as session files in `/templates/builtin/`
- User templates in `/templates/user/`
- Templates can reference clip library

---

## File Structure on SD Card

```
/clips/
  .index                  # Clip metadata cache (2-3 KB)
  bass_line_001.clip      # Individual clip files
  melody_a.clip
  drums_4bar.clip
  
/sessions/
  .autosave               # Auto-save backup
  live_session_001.ses    # Session files
  studio_recording.ses
  quicksave_0.ses         # Quick-save slots
  quicksave_1.ses
  
/templates/
  builtin/
    empty_4bar.tpl
    empty_8bar.tpl
    accordion_full.tpl
  user/
    my_accordion_setup.tpl
    bass_practice.tpl
```

---

## Integration with Existing Code

### Clip Library Integration

**Add to `looper_init()`**:
```c
void looper_init(void) {
  // ... existing code ...
  
  #if LOOPER_CLIP_LIBRARY_ENABLE
  looper_clip_library_init();
  #endif
}
```

**Add CLI Commands**:
```c
// In Services/cli/ or Services/looper/looper_cli.c
CLI_COMMAND(clip_save) {
  if (argc < 3) return CLI_INVALID_ARGS;
  uint8_t track = atoi(argv[1]);
  const char* name = argv[2];
  const char* category = (argc > 3) ? argv[3] : NULL;
  
  looper_clip_result_t result = looper_clip_save(track, name, category);
  printf("Clip save: %d\n", result);
  return CLI_OK;
}

CLI_COMMAND(clip_load) {
  if (argc < 3) return CLI_INVALID_ARGS;
  uint8_t track = atoi(argv[1]);
  const char* name = argv[2];
  
  looper_clip_result_t result = looper_clip_load(track, name);
  printf("Clip load: %d\n", result);
  return CLI_OK;
}

CLI_COMMAND(clip_list) {
  looper_clip_info_t clips[16];
  uint32_t count = 0;
  
  if (looper_clip_list(clips, 16, &count) == 0) {
    printf("Clips (%lu):\n", count);
    for (uint32_t i = 0; i < count; i++) {
      printf("  %s [%s] - %lu events, %u beats\n",
             clips[i].name, clips[i].category, 
             clips[i].event_count, clips[i].loop_beats);
    }
  }
  return CLI_OK;
}
```

### Session Management Integration

**Add CLI Commands**:
```c
CLI_COMMAND(session_save) {
  if (argc < 2) return CLI_INVALID_ARGS;
  looper_session_result_t result = looper_session_save(argv[1]);
  printf("Session save: %d\n", result);
  return CLI_OK;
}

CLI_COMMAND(session_load) {
  if (argc < 2) return CLI_INVALID_ARGS;
  looper_session_result_t result = looper_session_load(argv[1]);
  printf("Session load: %d\n", result);
  return CLI_OK;
}

CLI_COMMAND(session_quick_save) {
  if (argc < 2) return CLI_INVALID_ARGS;
  uint8_t slot = atoi(argv[1]);
  int result = looper_session_quick_save(slot);
  printf("Quick save slot %u: %d\n", slot, result);
  return CLI_OK;
}
```

### Template System Integration

**Add CLI Commands**:
```c
CLI_COMMAND(template_apply) {
  if (argc < 2) return CLI_INVALID_ARGS;
  looper_template_result_t result = looper_template_apply(argv[1]);
  printf("Template apply: %d\n", result);
  return CLI_OK;
}

CLI_COMMAND(template_list) {
  looper_template_info_t templates[16];
  uint32_t count = 0;
  
  if (looper_template_list(templates, 16, &count) == 0) {
    printf("Templates (%lu):\n", count);
    for (uint32_t i = 0; i < count; i++) {
      printf("  %s - %s (%s)\n",
             templates[i].name, templates[i].description,
             templates[i].builtin ? "built-in" : "user");
    }
  }
  return CLI_OK;
}
```

---

## UI Integration (Future)

### OLED UI Pages

**New UI Pages to Create**:
1. `ui_page_clip_library.c` - Browse and load clips
2. `ui_page_session_manager.c` - Session save/load
3. `ui_page_template_browser.c` - Template selection

**Menu Structure**:
```
Main Menu
  ├─ Looper
  │   ├─ Record/Play (existing)
  │   ├─ Scenes (existing)
  │   ├─ Clip Library [NEW]
  │   │   ├─ Load Clip
  │   │   ├─ Save Clip
  │   │   └─ Browse Clips
  │   ├─ Sessions [NEW]
  │   │   ├─ Save Session
  │   │   ├─ Load Session
  │   │   ├─ Quick Save (F1-F8)
  │   │   └─ Auto-save Settings
  │   └─ Templates [NEW]
  │       ├─ Apply Template
  │       ├─ Save as Template
  │       └─ Browse Templates
```

---

## Testing Strategy

### Unit Tests

**Clip Library Tests**:
```c
void test_clip_save_load(void) {
  // Setup test track
  looper_clip_save(0, "test_clip", "test");
  looper_clip_load(1, "test_clip");
  // Verify track 1 matches original track 0
}

void test_clip_list(void) {
  looper_clip_info_t clips[64];
  uint32_t count = 0;
  assert(looper_clip_list(clips, 64, &count) == 0);
  assert(count > 0);
}
```

**Session Tests**:
```c
void test_session_save_load(void) {
  // Setup looper state
  looper_session_save("test_session");
  // Modify state
  looper_session_load("test_session");
  // Verify state restored
}

void test_quick_save_load(void) {
  looper_session_quick_save(0);
  looper_session_quick_load(0);
}
```

**Template Tests**:
```c
void test_builtin_templates(void) {
  looper_template_apply_builtin(LOOPER_TEMPLATE_EMPTY_4BAR);
  // Verify 4-bar setup
  
  looper_template_apply_builtin(LOOPER_TEMPLATE_ACCORDION_FULL);
  // Verify accordion setup
}
```

### Integration Tests

**End-to-End Workflow Tests**:
```c
void test_live_performance_workflow(void) {
  // 1. Apply template
  looper_template_apply_builtin(LOOPER_TEMPLATE_LIVE_LOOPING);
  
  // 2. Record some loops
  // ... record on tracks 0, 1, 2 ...
  
  // 3. Save interesting clips
  looper_clip_save(0, "bass_intro", "bass");
  looper_clip_save(1, "chord_prog", "chords");
  
  // 4. Save session
  looper_session_save("live_gig_2026");
  
  // 5. Reset and reload
  looper_template_reset_default();
  looper_session_load("live_gig_2026");
  
  // 6. Verify all restored
}
```

---

## RAM Budget Impact

| Feature | RAM Usage | Details |
|---------|-----------|---------|
| Clip Library Index | ~2.5 KB | 64 clips × 40 bytes |
| Session Index | ~1.5 KB | 32 sessions × 48 bytes |
| Template Index | ~1.5 KB | 32 templates × 48 bytes |
| **Total** | **~5.5 KB** | Still well under 128 KB limit |

**After All Features**:
- Current: ~85 KB
- With new features: ~90.5 KB
- Available: ~37.5 KB (29% buffer)
- Status: ✅ Still comfortable

---

## Implementation Checklist

### Clip Library
- [ ] Create `looper_clip_library.c`
- [ ] Implement core functions (init, save, load, list)
- [ ] Add index caching
- [ ] Add CLI commands
- [ ] Add unit tests
- [ ] Document usage

### Session Management
- [ ] Create `looper_session.c`
- [ ] Define session file format
- [ ] Implement core functions (init, save, load)
- [ ] Add quick-save/load
- [ ] Add auto-save feature
- [ ] Add CLI commands
- [ ] Add unit tests
- [ ] Document usage

### Template System
- [ ] Create `looper_templates.c`
- [ ] Define built-in templates
- [ ] Implement template application
- [ ] Add custom template support
- [ ] Add CLI commands
- [ ] Add unit tests
- [ ] Document usage

### UI Integration (Optional)
- [ ] Create `ui_page_clip_library.c`
- [ ] Create `ui_page_session_manager.c`
- [ ] Create `ui_page_template_browser.c`
- [ ] Add menu entries
- [ ] Test on hardware

---

## Next Steps

1. **Review API Headers**: Confirm API design meets requirements
2. **Implement Phase 1**: Start with clip library (highest value)
3. **Test Thoroughly**: CLI-based testing before UI
4. **Implement Phase 2**: Session management
5. **Implement Phase 3**: Template system
6. **UI Integration**: Add OLED pages (optional)

---

## Additional Resources

- `MIOS32_LOOPA_MEMORY_ANALYSIS.md` - Comparison with LoopA
- `SD_BASED_UNDO_IMPLEMENTATION.md` - SD card patterns
- Existing `looper_save_track()` / `looper_load_track()` - Reuse this infrastructure

---

**Status**: ✅ API Design Complete  
**Next**: Implementation of `.c` files  
**Estimated Effort**: 2-3 days per phase  
**Risk**: 🟢 Low - Additive features, no breaking changes
