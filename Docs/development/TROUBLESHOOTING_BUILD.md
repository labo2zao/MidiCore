# Troubleshooting Build Issues

## Problem: Clean Build Doesn't Recompile Modified Files

### Symptoms

After modifying source files and doing `Project → Clean → Build All`, the .map file shows old memory layout:
- `.ccmram` section smaller than expected
- Variables still in `.bss` (RAM) instead of CCMRAM
- RAM overflow persists despite code changes

### Root Cause

STM32CubeIDE sometimes caches build artifacts and doesn't properly detect source file changes, especially when:
- Files are modified outside the IDE (git pull, git checkout, etc.)
- Multiple branches are being worked on
- The project has been opened/closed multiple times

### Solution

**Method 1: Force Complete Rebuild**

1. **Close STM32CubeIDE completely**
   - File → Exit (or Alt+F4)
   - Make sure no STM32CubeIDE processes are running

2. **Manually delete build artifacts**
   ```
   Delete the entire Debug/ folder
   Delete the entire Release/ folder (if using)
   ```

3. **Reopen STM32CubeIDE**
   - Open workspace
   - Open project

4. **Build from scratch**
   ```
   Project → Build All (Ctrl+B)
   ```

5. **Verify the new .map file**
   - Check Debug/MidiCore.map
   - Verify `.ccmram` section size
   - Verify variables are in correct memory regions

**Method 2: Use Command Line Build (Alternative)**

If STM32CubeIDE continues to have issues:

1. Navigate to project directory
2. Delete Debug/ folder
3. Use command-line build (if available in your setup)

### Verification Checklist

After rebuild, verify in `Debug/MidiCore.map`:

```
✅ .ccmram section should show ~57 KB (0xE000):
   .ccmram         0x10000000     0xE000 load address 0x080xxxxx
   
✅ Variables should be in CCMRAM, not .bss:
   - g_tr (17 KB) should be in .ccmram section from looper.o
   - g_automation (8 KB) should be in .ccmram section from looper.o
   - fb (8 KB) should be in .ccmram section from oled_ssd1322.o
   - active (24 KB) should be in .ccmram section from ui_page_looper_pianoroll.o
   
✅ .bss section should be ~119 KB (0x1D800), not 127 KB:
   .bss            0x20000428    0x1D800
   
✅ No linker errors about RAM overflow
```

### Expected Memory Layout

**After correct rebuild**:

**CCMRAM (64 KB)**:
```
.ccmram section = 0xE000 (57 KB)
  g_tr[4]:              ~0x4400 (17 KB)
  g_automation[4]:      ~0x2000 (8 KB)
  fb (OLED):            ~0x2000 (8 KB)
  active[16][128]:      ~0x6000 (24 KB)
Free: 7 KB
```

**RAM (128 KB)**:
```
.bss section = 0x1D800 (119 KB)
  undo_stacks[4]:       ~0x18000 (99 KB)
  g_routes:             ~0x1400 (5 KB)
  snap (timeline):      ~0x1800 (6 KB)
  Other:                ~0x2000 (8 KB)
Free: 9 KB (for bootloader)
```

## Problem: Wrong Git Branch

### Symptoms

After clean build, .map still shows old layout but source files look correct.

### Solution

1. **Verify current branch**:
   ```bash
   git status
   git log --oneline -5
   ```

2. **Ensure on correct branch**:
   ```bash
   git checkout copilot/finalize-production-mode
   git pull origin copilot/finalize-production-mode
   ```

3. **Verify source files have CCMRAM attributes**:
   ```bash
   grep "__attribute__((section" Services/ui/ui_page_looper_pianoroll.c
   grep "__attribute__((section" Hal/oled_ssd1322/oled_ssd1322.c
   grep "__attribute__((section" Services/looper/looper.c
   ```

   Should show multiple lines with `__attribute__((section(".ccmram")))`

4. **Rebuild after confirming correct branch**

## Problem: Build Artifacts from Different Configuration

### Symptoms

Mixing Debug and Release builds, or switching between configurations.

### Solution

1. **Select correct configuration**:
   - Right-click project → Build Configurations → Set Active → Debug (or Release)

2. **Clean ALL configurations**:
   - Project → Clean...
   - Select "Clean all projects"
   - Check "Start a build immediately"

3. **Verify you're looking at correct .map file**:
   - Debug build → Debug/MidiCore.map
   - Release build → Release/MidiCore.map

## Quick Verification Script

You can verify your build with this simple check:

```bash
# Check CCMRAM section size
grep "^\.ccmram.*0x" Debug/MidiCore.map

# Should show approximately:
# .ccmram         0x10000000     0xE000 load address 0x080xxxxx
#                                 ^^^^^ This should be around 0xE000 (57 KB)

# Check BSS section size
grep "^\.bss.*0x" Debug/MidiCore.map

# Should show approximately:
# .bss            0x20000428    0x1D800
#                               ^^^^^^^ This should be around 0x1D800 (119 KB)
```

## Common Pitfalls

1. **Not closing IDE before deleting Debug/** - IDE locks files
2. **Looking at wrong .map file** - Check configuration (Debug vs Release)
3. **Not on correct git branch** - Changes not pulled from repository
4. **Incremental build instead of clean build** - Use Clean first
5. **Multiple STM32CubeIDE instances** - Close all instances

---

**Last Updated**: 2026-01-25
**Related Docs**: 
- BUILD_AFTER_MEMORY_FIXES.md
- Production mode RAM overflow fixes (PR)
