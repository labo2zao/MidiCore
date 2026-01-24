# RAM Overflow Fix

## Problem
The linker was reporting a RAM overflow error:
```
region `RAM' overflowed by 79712 bytes
```

## Root Cause
The looper undo system was consuming approximately 80 KB of RAM with `LOOPER_UNDO_STACK_DEPTH=10`.

### Memory Analysis
- **undo_state_t**: ~2,064 bytes per state
- **undo_stack_t**: ~20,643 bytes per track (10 states)
- **Total for 4 tracks**: ~82,572 bytes (~80 KB)

The STM32F407VGTx has only 128 KB of RAM, and the undo stacks alone were consuming over 60% of available memory.

## Solution
Reduced `LOOPER_UNDO_STACK_DEPTH` from 10 to 3 in `Services/looper/looper.h`.

### Memory Savings
| Depth | Memory Usage (4 tracks) | Savings vs Depth=10 |
|-------|------------------------|---------------------|
| 10    | ~80 KB                 | -                   |
| 3     | ~24 KB                 | ~56 KB saved        |

This reduction brings memory usage well within the 128 KB available RAM and resolves the overflow by approximately 56 KB.

## Trade-offs
- **Before**: 10 levels of undo/redo per track
- **After**: 3 levels of undo/redo per track

Users will still have 3 levels of undo/redo functionality, which is sufficient for most use cases while staying within memory constraints.

## Files Modified
- `Services/looper/looper.h`: Changed `LOOPER_UNDO_STACK_DEPTH` from 10 to 3
