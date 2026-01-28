# CLI Fixes Verification Checklist

## Quick Verification Steps

### 1. Verify All Files Were Modified
```bash
git diff HEAD~1 --name-only | grep cli
```
Expected: 15 files (14 CLI files + 1 summary doc)

### 2. Check for Common Issues

#### A. Check for remaining DEFINE_PARAM_BOOL (without _TRACK) in per-track modules
```bash
git diff HEAD~1 | grep -E "^\+.*DEFINE_PARAM_BOOL[^_]" | grep -v "// "
```
Expected: Should only show globals (config, bootloader, etc.)

#### B. Check for remaining void init assignments
```bash
git diff HEAD~1 | grep -E "^\+.*\.init = [a-z_]+_init," | grep -v "_cli_init"
```
Expected: Empty (all should use wrapper functions)

#### C. Verify no .max_tracks fields remain
```bash
git diff HEAD~1 | grep -E "^\-.*max_tracks"
```
Expected: Shows removed lines from ainser_map_cli.c and din_map_cli.c

### 3. Module-Specific Checks

#### bellows_shake_cli.c
```bash
grep "bellows_shake_is_enabled(track)" Services/cli/bellows_shake_cli.c
```
✓ Should exist (per-track function)

#### assist_hold_cli.c
```bash
grep "HOLD_MODE_COUNT\|enum_count = 5" Services/cli/assist_hold_cli.c
```
✓ Should show 5 modes

#### bass_chord_system_cli.c
```bash
grep "bass_chord_get_layout\|bass_chord_init" Services/cli/bass_chord_system_cli.c
```
✓ Should use bass_chord_ prefix (not bass_chord_system_)

#### bellows_expression_cli.c
```bash
grep "bellows_get_curve\|bellows_set_curve" Services/cli/bellows_expression_cli.c
```
✓ Should use bellows_ prefix (not bellows_expression_)

#### ain_cli.c
```bash
grep "param_count = 0" Services/cli/ain_cli.c
```
✓ Should have no parameters

#### ainser_map_cli.c
```bash
grep "\.threshold" Services/cli/ainser_map_cli.c | wc -l
```
✓ Should show multiple uses (2+)

#### din_map_cli.c
```bash
grep "\.number\|\.type\|\.vel_on" Services/cli/din_map_cli.c | wc -l
```
✓ Should show multiple uses (8+)

#### bootloader_cli.c
```bash
grep "\.string_val" Services/cli/bootloader_cli.c
```
✓ Should exist (not .str_val)

### 4. Compilation Test

If STM32CubeIDE or arm-none-eabi-gcc is available:

```bash
# Method 1: Full build in STM32CubeIDE
# Open project and click Build

# Method 2: Command line (if Makefile exists)
make clean && make 2>&1 | tee build.log
grep -i "error:" build.log

# Method 3: Syntax check only
for f in Services/cli/*_cli.c; do
    echo "Checking $f..."
    arm-none-eabi-gcc -fsyntax-only \
        -I. -ICore/Inc -IDrivers/STM32F4xx_HAL_Driver/Inc \
        -IDrivers/CMSIS/Device/ST/STM32F4xx/Include \
        -IDrivers/CMSIS/Include -IServices \
        -DSTM32F407xx $f 2>&1 | grep -i "error:"
done
```

### 5. Runtime Verification (After Flashing Firmware)

#### Test Module Listing
```
> help
> module.list
```
Expected: All 14+ modules should be listed

#### Test Per-Track Module (bellows_shake)
```
> bellows_shake.enable 0
> bellows_shake.enabled 0
Expected: 1 (enabled)

> bellows_shake.sensitivity 0 75
> bellows_shake.sensitivity 0
Expected: 75
```

#### Test Global Module (ain)
```
> ain.status
Expected: enabled (or similar)
```

#### Test Structure Fields (dinmap)
```
> din_map.number 0 60    # Set note/CC number
> din_map.number 0       # Read back
Expected: 60
```

### 6. Git Status Check

```bash
git log -1 --stat
```
Expected output should show:
- 14 files changed
- Roughly +164 / -110 lines
- CLI_COMPILATION_FIXES_SUMMARY.md created

### 7. Verify Commit Message

```bash
git log -1 --pretty=format:"%s%n%b"
```
Should mention:
- Fixed 14 CLI files
- Root causes
- Key changes
- Reference to summary doc

## Expected Results Summary

✓ All 14 CLI files modified
✓ All per-track modules use _TRACK macros
✓ All init wrappers return int
✓ All structure field names match headers
✓ No compilation errors
✓ No .max_tracks fields
✓ Correct per-track vs global settings

## If Issues Found

1. **Compilation errors remain:**
   - Check include paths
   - Verify header files match assumptions
   - Check for typos in function names

2. **Runtime crashes:**
   - Verify init function wrappers return 0
   - Check NULL pointer dereferences
   - Verify module_registry has correct pointers

3. **Parameter access fails:**
   - Verify getter/setter function names
   - Check track parameter is passed correctly
   - Verify structure field names

## Success Criteria

- [x] All 14 files committed
- [x] Comprehensive summary created
- [x] All obvious mismatches fixed
- [ ] Firmware compiles without errors (requires STM32 toolchain)
- [ ] All CLI commands work at runtime (requires hardware/simulator)

## Notes

The fixes address all **structural and API signature issues** identified in the original problem statement. Full verification requires:

1. Access to STM32 development environment
2. Compilation of complete firmware
3. Runtime testing on target hardware or simulator

The changes are **conservative and correct** based on the actual module header files and structure definitions.
