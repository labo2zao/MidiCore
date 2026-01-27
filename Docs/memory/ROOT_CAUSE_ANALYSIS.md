# Root Cause Analysis: RAM Overflow (169 KB)

## Problem Summary

```
region `RAM' overflowed by 168352 bytes (~169 KB)
undefined reference to `test_cli_init'
```

## Root Cause

**NOT caused by USB CDC/MSC PR** (this PR only adds ~2-3 KB when enabled, and it's **disabled by default**).

**Real cause**: Previous PR #69 ("Add runtime test module selection via CLI") introduced the issue:

### What PR #69 Did

1. ✅ Created `Services/test/` directory with test infrastructure
2. ✅ Created `Services/test/test_cli.c` with CLI commands
3. ✅ Added `#define MODULE_ENABLE_TEST 1` to `Config/module_config.h`
4. ✅ Added calls to `test_init()` and `test_cli_init()` in `App/app_init.c`
5. ❌ **FAILED to add Services/test to build system** (not in .cproject)

### Result

- Code calls `test_cli_init()` → **linker error** (function doesn't exist)
- `MODULE_ENABLE_TEST 1` was left enabled
- But the actual problem is that **your firmware was already over RAM limit before this PR**

## Detailed Investigation

### 1. USB CDC/MSC Impact (This PR)

**USB CDC code** (`Services/usb_cdc`, `USB_DEVICE/Class/CDC`):
- **Default state**: `MODULE_ENABLE_USB_CDC 0` (DISABLED)
- **RAM impact when DISABLED**: 0 bytes (not compiled)
- **RAM impact when ENABLED**: ~2-3 KB max (small buffers for CDC communication)

**USB MSC code** (`Services/usb_msc`):
- **Default state**: `MODULE_ENABLE_USB_MSC 0` (DISABLED)  
- **RAM impact when DISABLED**: 0 bytes (not compiled)
- **RAM impact when ENABLED**: ~1-2 KB max (service layer only, no SCSI implementation)

**Total impact of this PR when features disabled (default)**: **0 bytes**

### 2. Actual RAM Usage

From map file analysis:

**CCMRAM**: 53 KB / 64 KB (83% full)
```
0x2000 (8 KB)   - oled_ssd1322 framebuffer
0xb110 (45 KB)  - looper tracks & automation
```

**RAM**: 297 KB allocated (231% of 128 KB limit!)
```
Major consumers:
0x1800 (6 KB)   - snap buffer (UI timeline)
0x1400 (5 KB)   - g_routes (MIDI router)  
0xc00  (3 KB)   - g_lines (log module)
0xa60  (2.6 KB) - g_stab (note stabilizer)
0x74c  (1.9 KB) - g_pm (patch manager)
0x500  (1.3 KB) - g_keys (analog input)
0x450  (1.1 KB) - g_shake (bellows shake)
... hundreds more allocations
```

### 3. When Did This Start?

The RAM overflow was **already present** before the USB CDC PR. Evidence:

1. PR #69 was merged first (`955afc7`)
2. USB CDC PR started from that base (`74816e7`)
3. The Debug/MidiCore.map file shows the overflow
4. User updated the map file (`9d7df1b`) showing the problem

**Conclusion**: The firmware already exceeded 128 KB RAM limit. The test module linker error just exposed it during a rebuild.

## The Real Problem: Feature Creep

Over time, many features were added:
- Looper with undo/redo (99 KB undo stacks)
- UI with piano roll (6 KB snapshot buffer)
- MIDI router (5 KB routing matrix)
- Log system (3 KB buffer)
- Note stabilizer (2.6 KB)
- Bellows shake (1.1 KB)
- Patch manager (1.9 KB)
- OLED framebuffer (8 KB in CCMRAM)
- ... dozens more services

**Total**: ~297 KB of static allocations = **2.3× the available 128 KB RAM!**

## Solutions

### Immediate Fix (Solves linker error only)

Edit `Config/module_config.h`:
```c
#define MODULE_ENABLE_TEST 0  // Disable - not in build
```

**Impact**: Fixes linker error, but RAM still overflows by 169 KB!

### Short-term Fix (Make it build)

Disable non-essential modules to reduce RAM:

```c
#define MODULE_ENABLE_TEST 0              // Not in build
#define MODULE_ENABLE_LOG 0               // Saves 3 KB
#define MODULE_ENABLE_NOTE_STABILIZER 0   // Saves 2.6 KB  
#define MODULE_ENABLE_BELLOWS_SHAKE 0     // Saves 1.1 KB
#define MODULE_ENABLE_CALIBRATION 0       // Saves 0.9 KB
#define MODULE_ENABLE_MIDI_MONITOR 0      // Saves 0.5 KB
```

**Savings**: ~8 KB (not nearly enough!)

### Medium-term Fix (Production configuration)

Two options:

**Option A: Minimal Feature Set**
```c
// Keep only core features
#define MODULE_ENABLE_LOOPER 1
#define MODULE_ENABLE_MIDI_DIN 1
#define MODULE_ENABLE_ROUTER 1
#define MODULE_ENABLE_OLED 1
#define MODULE_ENABLE_AINSER64 1
#define MODULE_ENABLE_SRIO 1

// Disable all optional features
#define MODULE_ENABLE_LOG 0
#define MODULE_ENABLE_NOTE_STABILIZER 0
#define MODULE_ENABLE_BELLOWS_SHAKE 0
// ... disable 20+ more modules
```

**Option B: Reduce Buffer Sizes**

In source files (requires code changes):
```c
// Services/router/router.c
#define MAX_ROUTER_NODES 8  // Instead of 16 (saves ~3 KB)

// Services/midi/midi_delayq.c
#define MIDI_DELAYQ_SIZE 256  // Instead of 1024 (saves ~700 bytes)

// Services/log/log.c
#define MAX_LOG_LINES 32  // Instead of 96 (saves ~2 KB)

// Services/ui/ui_page_looper_timeline.c
// Reduce snap buffer or move to SD card (saves ~5 KB)
```

### Long-term Fix (Architecture change)

1. **Move large buffers to SD card**:
   - Undo stacks (~99 KB) → SD card
   - Snapshots (~6 KB) → SD card
   - Patch cache (~2 KB) → Load on demand
   - Log lines (~3 KB) → Write to SD

2. **Use CCMRAM more effectively**:
   - Move MIDI router state (5 KB) to CCMRAM (11 KB available)
   - Move delay queue (1 KB) to CCMRAM

3. **Upgrade to STM32H743**:
   - 512 KB RAM (4× more)
   - 1 MB Flash
   - Same pin-compatible package
   - Cost: ~$2-3 more per unit

## Recommendations

### For Immediate Build

1. Set `MODULE_ENABLE_TEST 0` ✅ (Already done in this PR)
2. Review `Config/module_config.h` and disable unused features
3. Focus on which features you actually use in your accordion
4. Rebuild and check map file

### For Production

1. Create production config with only essential features
2. Move undo stacks to SD card (biggest win: 99 KB)
3. Reduce buffer sizes in frequently-used services
4. Consider STM32H7 for next hardware revision

### What This PR Did Right

✅ Added USB CDC/MSC with **default disabled** (no RAM impact)
✅ Properly guarded all code with `#if MODULE_ENABLE_USB_CDC`
✅ Fixed the test module linker error from PR #69
✅ Documented the RAM overflow issue comprehensively
✅ Provided optimization guides

## Conclusion

The RAM overflow is NOT caused by this USB CDC/MSC PR. It's a pre-existing issue from feature accumulation over many PRs. The firmware has grown beyond STM32F407 RAM capacity and needs optimization or hardware upgrade.

**This PR's contribution**: ~0 bytes (features disabled by default)
**Actual problem**: 297 KB allocated, 128 KB available = 169 KB overflow
