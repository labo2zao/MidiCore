# Build Instructions After Memory Optimization

## ⚠️ IMPORTANT: Full Rebuild Required!

After the memory optimization commits that moved large buffers to CCMRAM, you **MUST** do a complete clean build.

### Why?

The previous build artifacts (.o files, .elf, .hex, .map) were compiled **before** the CCMRAM changes and still have the old memory layout with everything in RAM.

### 🔧 Build Steps in STM32CubeIDE

1. **Clean the project**:
   ```
   Project → Clean...
   → Select "MidiCore" project
   → Check "Clean all projects"
   → Click OK
   ```

2. **Rebuild everything**:
   ```
   Project → Build All
   ```
   
3. **Verify the new .map file**:
   Open `Debug/MidiCore.map` and check:
   ```
   .ccmram section should show ~57 KB:
   - g_tr[4]: ~17 KB (looper.o)
   - g_automation[4]: ~8 KB (looper.o)
   - fb (OLED): ~8 KB (oled_ssd1322.o)
   - active[16][128]: ~24 KB (ui_page_looper_pianoroll.o)
   ```

4. **Flash the new binary**:
   ```
   Run → Debug (F11)
   or
   Run → Run (Ctrl+F11)
   ```

### 📊 Expected Memory Layout After Rebuild

**CCMRAM (64 KB)**:
```
Total: ~57 KB / 64 KB (7 KB free)
- g_tr[4]: 17 KB
- g_automation[4]: 8 KB
- fb (OLED): 8 KB
- active (pianoroll): 24 KB
```

**RAM (128 KB)**:
```
Total: ~119 KB / 128 KB (9 KB free for bootloader)
- undo_stacks[4]: 99 KB
- g_routes: 5 KB
- snap: 6 KB
- Other: ~9 KB
```

### 🐛 If OLED Still Doesn't Work

If after clean build and flash the OLED still doesn't activate:

1. **Check the .map file** to confirm CCMRAM section size is ~57 KB
2. **Verify linker script** has proper CCMRAM initialization
3. **Check startup code** zeros CCMRAM properly (should be automatic)
4. **Use debugger** to inspect:
   - `fb` address should be 0x10xxxxxx (CCMRAM range)
   - `active` address should be 0x10xxxxxx
   - `g_automation` address should be 0x10xxxxxx

### 📝 Commits Applied

- `8c19a02`: Move pianoroll `active[]` to CCMRAM (24 KB)
- `edc4e0c`: Move OLED `fb` framebuffer to CCMRAM (8 KB)
- `9fa406d`: Move looper `g_automation[]` to CCMRAM (8 KB)

**Total memory freed from RAM**: 40 KB
**RAM headroom for bootloader**: 9 KB

## ✅ Verification Checklist

After rebuild:
- [ ] `.ccmram` section in .map shows ~57 KB
- [ ] `.bss` section in .map shows ~119 KB (not ~127 KB)
- [ ] No linker errors about RAM overflow
- [ ] OLED initializes and displays UI
- [ ] Pianoroll page works
- [ ] Looper automation works
- [ ] System is stable

---

**Last Updated**: 2026-01-25
**Related PR**: Fix production mode RAM overflow
