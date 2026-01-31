# DefaultTask Diagnostic Guide

## Issue Summary

**User Report:** "System is not stable, stack overflow everywhere, DefaultTask ending after app_init, loop does nothing"

## Current Configuration

### Memory Allocation
- **DefaultTask Stack:** 2KB (2048 bytes)
- **Total Heap:** 48KB (49,152 bytes)
- **Expected Free Heap After Init:** ~26KB

### Code Architecture

```
StartDefaultTask (main.c)
  └─> app_entry_start() (app_entry.c)
       ├─> app_init_and_start() (app_init.c)
       │    ├─> Initialize all subsystems
       │    ├─> Create all application tasks
       │    └─> RETURNS normally
       └─> for(;;) { osDelay(1000); }  ← INFINITE LOOP (never returns)
```

## Diagnostics Deployed (Commit c77ac76)

### What's Being Tracked

1. **Entry Point Logging**
   - When app_entry_start() is called
   - If it's re-entered (shouldn't happen)

2. **Initialization Tracking**
   - Before app_init_and_start()
   - After app_init_and_start() returns
   - Stack usage at this point

3. **Infinite Loop Monitoring**
   - Entry to infinite loop
   - Periodic heartbeat every 60 seconds
   - Stack usage in steady state

4. **Failure Detection**
   - If app_entry_start() returns (BUG!)
   - If infinite loop exits (CATASTROPHIC!)
   - Fallback loop activation

## Expected Terminal Output

### Normal Operation

```
[APP_ENTRY] ===========================================
[APP_ENTRY] app_entry_start() ENTRY - DefaultTask init
[APP_ENTRY] ===========================================
[APP_ENTRY] First entry - calling app_init_and_start()

========================================
   EARLY HEAP DIAGNOSTICS
========================================
Heap total:       49152 bytes (48KB)
Heap free now:    XXXXX bytes
...

[INIT] System initialization complete
[INIT] About to start MIDI IO task...
[HEAP] Before MIDI IO task: XXXXX bytes free
[HEAP] After MIDI IO task: XXXXX bytes free

========================================
   FINAL HEAP STATUS
========================================
Heap free now:    ~26000 bytes
...

[APP_ENTRY] app_init_and_start() returned successfully
[APP_ENTRY] Entering infinite idle loop - DefaultTask will sleep here
[APP_ENTRY] All application work happens in dedicated tasks
[APP_ENTRY] DefaultTask stack: XXXX bytes free (high water mark)
[APP_ENTRY] ===========================================
[APP_ENTRY] ENTERING INFINITE LOOP - should never exit!
[APP_ENTRY] ===========================================

... (60 seconds pass) ...

[APP_ENTRY] Still alive! Loop count: 60, Stack free: XXXX bytes

... (60 seconds pass) ...

[APP_ENTRY] Still alive! Loop count: 120, Stack free: XXXX bytes
```

### If app_entry_start() Returns (BUG)

```
[APP_ENTRY] app_init_and_start() returned successfully
[APP_ENTRY] ENTERING INFINITE LOOP
[DEFAULT_TASK] FATAL: app_entry_start() returned! This is impossible!
[DEFAULT_TASK] Entering fallback infinite loop
[DEFAULT_TASK] FATAL: Still in fallback loop! Count: 10000
[DEFAULT_TASK] FATAL: Still in fallback loop! Count: 20000
```

### If Infinite Loop Exits (CATASTROPHIC)

```
[FATAL] DefaultTask infinite loop exited!
[FATAL] This should be impossible - check stack/heap!
[FATAL] DefaultTask stack: 2KB allocated
```

## Root Cause Analysis

### Possible Causes

#### 1. Stack Overflow
**Symptoms:**
- Doesn't reach "[APP_ENTRY] ENTERING INFINITE LOOP"
- Crashes during app_init_and_start()
- Corrupted task names

**Evidence Needed:**
- Stack high water mark < 200 bytes
- Crash during specific init step

**Solution:**
- Increase DefaultTask stack (2KB → 4KB)

#### 2. Heap Exhaustion
**Symptoms:**
- Task creation fails
- "[HEAP] After XXX task" shows low memory
- Crashes during task cascade

**Evidence Needed:**
- Free heap < 5KB during init
- Task creation failure messages

**Solution:**
- Increase heap (48KB → 56KB)
- Reduce task stack sizes

#### 3. Hard Fault During Init
**Symptoms:**
- Stops mid-initialization
- No "[APP_ENTRY] app_init_and_start() returned"
- System hangs

**Evidence Needed:**
- Last message before hang
- Which subsystem init failed

**Solution:**
- Debug specific failing subsystem
- Check peripheral conflicts

#### 4. app_entry_start() Actually Returns
**Symptoms:**
- "[DEFAULT_TASK] FATAL: app_entry_start() returned!"
- Enters fallback loop

**Evidence Needed:**
- This specific message appearing

**Solution:**
- Bug in infinite loop (compiler optimization?)
- Memory corruption jumping over loop

#### 5. Loop Exits
**Symptoms:**
- "[FATAL] DefaultTask infinite loop exited!"

**Evidence Needed:**
- This catastrophic message

**Solution:**
- Severe memory corruption
- Hard fault recovery issue

## Diagnostic Checklist

When reporting issues, provide:

- [ ] Complete terminal output (2+ minutes minimum)
- [ ] All [APP_ENTRY] messages
- [ ] All [HEAP] messages
- [ ] Any [FATAL] messages
- [ ] Where output stops/hangs
- [ ] Periodic heartbeat messages (60s intervals)
- [ ] Stack high water mark values

## Quick Diagnosis

### Output Stops At:

1. **Before "[APP_ENTRY] First entry"**
   - StartDefaultTask not starting
   - FreeRTOS scheduler issue

2. **During "EARLY HEAP DIAGNOSTICS"**
   - Debug output system failure
   - UART misconfiguration

3. **During [INIT] messages**
   - Specific subsystem init failure
   - Check which init step hangs

4. **After "[HEAP] After MIDI IO task"**
   - MIDI IO task creation failure
   - Likely heap exhaustion

5. **Never reaches "[APP_ENTRY] ENTERING INFINITE LOOP"**
   - Stack overflow during init
   - app_init_and_start() crashes

6. **Shows "[DEFAULT_TASK] FATAL: app_entry_start() returned!"**
   - Infinite loop not working
   - Memory corruption or compiler bug

7. **Shows "[FATAL] DefaultTask infinite loop exited!"**
   - Catastrophic corruption
   - Need full memory dump

## Next Steps

### For User

1. **Flash firmware with diagnostics (commit c77ac76)**
2. **Capture complete terminal output:**
   - From power-on
   - For at least 2 minutes
   - Include all messages
3. **Report:**
   - Where output stops
   - Any repeated messages
   - Any FATAL messages
4. **Provide output** via issue comment or file attachment

### For Developer

Once terminal output received:
1. Identify which scenario matches
2. Check stack/heap values
3. Find exact failure point
4. Apply appropriate fix
5. Verify with user

## Files Modified

- **App/app_entry.c** - Added comprehensive logging
- **Core/Src/main.c** - Added fallback detection
- **This document** - Diagnostic guide

## Commit Reference

- **c77ac76** - Comprehensive diagnostics
- **d600d7c** - DefaultTask 2KB stack
- **5ca286b** - Heap 48KB

---

**Status:** Diagnostic system deployed, waiting for user terminal output
