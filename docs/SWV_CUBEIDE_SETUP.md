# SWV Configuration in STM32CubeIDE - Step by Step Guide

This guide shows you exactly how to configure SWV (Serial Wire Viewer) in STM32CubeIDE to view debug output from your STM32F407 MidiCore firmware.

---

## Prerequisites

- ✅ STM32CubeIDE installed
- ✅ ST-Link debugger connected to STM32F407
- ✅ MidiCore project opened in CubeIDE
- ✅ `MODULE_DEBUG_OUTPUT` set to `DEBUG_OUTPUT_SWV` in `Config/module_config.h`

---

## Part 1: Enable SWV in Debug Configuration

### Step 1: Open Debug Configurations

**Method A: From toolbar**
1. Click the **Debug** button dropdown (bug icon) in toolbar
2. Select **Debug Configurations...**

**Method B: From menu**
1. Go to menu: **Run** → **Debug Configurations...**

**Method C: Right-click project**
1. Right-click your project in Project Explorer
2. Select **Debug As** → **Debug Configurations...**

### Step 2: Select Your Debug Configuration

In the Debug Configurations window:

1. In the left panel, expand **STM32 Cortex-M C/C++ Application**
2. Find your configuration (usually named like `MidiCore Debug`)
3. If no configuration exists:
   - Right-click **STM32 Cortex-M C/C++ Application**
   - Select **New Configuration**
   - Give it a name (e.g., "MidiCore Debug")

### Step 3: Configure the Debugger Tab

1. Click the **Debugger** tab at the top
2. You'll see several sections - scroll down to find **Serial Wire Viewer (SWV)**

### Step 4: Enable SWV

In the **Serial Wire Viewer (SWV)** section:

```
☑ Enable                            ← CHECK THIS BOX!
```

### Step 5: Set Clock Speeds

Still in the SWV section, configure the clocks:

```
Core Clock (Hz):    168000000       ← STM32F407 runs at 168 MHz
SWO Clock (Hz):     2000000         ← 2 MHz (recommended)
```

**Important Notes:**
- **Core Clock MUST match your actual CPU clock**
  - STM32F407VGT6: 168000000 (168 MHz)
  - If you changed clock in CubeMX, update this!
- **SWO Clock should be:**
  - Maximum: Core Clock ÷ 4 = 42 MHz for STM32F407
  - Recommended: 2000000 (2 MHz) - most reliable
  - Alternative: 500000 (500 kHz) if you get errors

### Step 6: Configure ITM Stimulus Ports

In the same SWV section, find **ITM Stimulus Ports**:

```
Port 0:  ☑ Enabled                 ← CHECK THIS - used for debug output
Port 1:  ☐ Disabled                ← Leave unchecked
Port 2:  ☐ Disabled                ← Leave unchecked
...
Port 31: ☐ Disabled                ← Leave unchecked
```

**Only Port 0 needs to be enabled** - this is where `dbg_printf()` sends data.

### Step 7: Apply and Close

1. Click **Apply** button at the bottom
2. Click **Close** button

Your debug configuration is now saved!

---

## Part 2: Start a Debug Session

### Step 1: Start Debugging

1. Click the **Debug** button (bug icon) in toolbar, OR
2. Press **F11**, OR
3. Menu: **Run** → **Debug**

The debugger will:
- Build your project (if needed)
- Flash to STM32
- Start debug session
- Stop at `main()` (usually)

### Step 2: Open SWV Console

Once debug session is running:

1. Go to menu: **Window** → **Show View** → **SWV** → **SWV ITM Data Console**

A new panel will appear at the bottom called **SWV ITM Data Console**.

**If you don't see the SWV menu:**
- You're not in debug mode yet (start debug first)
- Or SWV wasn't enabled in debug config (go back to Part 1)

---

## Part 3: Configure and Start SWV Trace

### Step 1: Configure SWV Console

In the **SWV ITM Data Console** panel:

1. Click the **Configure Trace** button (gear/settings icon ⚙️) on the toolbar
2. A configuration window will open

### Step 2: Enable Port 0

In the configuration window:

```
ITM Stimulus Ports:

Port 0:  ☑ Enabled                 ← CHECK THIS
Port 1:  ☐ Disabled
Port 2:  ☐ Disabled
...
Port 31: ☐ Disabled
```

Click **OK** to close the configuration.

### Step 3: Start Trace

In the **SWV ITM Data Console** toolbar:

1. Click the **Start Trace** button (red circle ⏺️)
2. You should see a message: "Trace started"

### Step 4: Resume Execution

Your program is probably paused at `main()`. To see debug output:

1. Click **Resume** button (green play ▶️), OR
2. Press **F8**

---

## Part 4: View Debug Output

### What You Should See

If everything is configured correctly, you'll see debug output in the SWV ITM Data Console:

```
=== SWV Debug Output Active ===

==============================================
Debug output: SWV (Serial Wire Viewer)
View in STM32CubeIDE: SWV ITM Data Console
==============================================

[App Init] Starting initialization...
[Module] AINSER64 initialized
[Module] SRIO initialized
[Module] Looper initialized
...
```

### If You See Nothing

Try these steps:

1. **Check if trace is running:**
   - Look for "Trace started" message
   - Red dot should be visible on Start Trace button

2. **Verify Port 0 is enabled:**
   - Click Configure (⚙️) button
   - Ensure Port 0 is checked
   - Click OK

3. **Check clock speeds:**
   - Go back to Debug Configuration
   - Verify Core Clock = 168000000
   - Verify SWO Clock = 2000000
   - Try lower SWO Clock (500000) if problems

4. **Restart debug session:**
   - Stop debug (Ctrl+F2)
   - Start again (F11)
   - Open SWV console
   - Configure Port 0
   - Start trace
   - Resume

---

## Part 5: Troubleshooting

### ⚠️ Problem: "Could not start SWV" Error (MOST COMMON!)

**This is the #1 issue users face with SWV!**

**Cause 1: JTAG Mode Instead of SWD** ⭐ **MOST LIKELY**

**Solution:**
1. Open Debug Configurations (Run → Debug Configurations)
2. Select your configuration
3. Go to **Debugger** tab
4. Find "Debug probe" section
5. **Interface: Change from JTAG to SWD** ← THIS!
6. Click Apply → Close
7. Try debug again

```
Debug Configuration → Debugger:
Debug probe: ST-LINK (ST-LINK GDB server)
Interface: SWD ← Change from JTAG to SWD!
```

**Cause 2: SWO Clock Too High**

**Solution:** Lower the SWO clock speed:
1. Debug Configuration → Debugger → Serial Wire Viewer (SWV)
2. Change SWO Clock to: **500000** (500 kHz)
3. Apply and try again

**Cause 3: ST-Link Firmware Outdated**

**Solution:**
1. Download "STM32 ST-LINK Utility" from ST website
2. Connect ST-Link
3. Run utility → ST-LINK → Firmware Update
4. Update to V2.J45 or newer
5. Reconnect and try again

**Cause 4: PB3 (SWO) Pin Not Configured**

**Solution:** In STM32CubeMX:
1. System Core → SYS
2. Debug: **Serial Wire** (not Trace Asynchronous)
3. Regenerate code
4. Rebuild and flash

**Cause 5: Target Not Running**

**Solution:**
- Resume execution (F8) BEFORE starting trace
- Or: Start trace → Immediately resume

**Quick Fix Sequence:**
1. ✅ Change Interface to **SWD** (most important!)
2. ✅ Lower SWO Clock to **500000**
3. ✅ Disable SWV → Apply → Enable SWV → Apply
4. ✅ Restart debug (Ctrl+F2, then F11)
5. ✅ Resume (F8) → Start Trace

### Problem: No SWV Menu in Window → Show View

**Cause:** Not in debug mode

**Solution:**
1. Start a debug session first (F11)
2. THEN open the view: Window → Show View → SWV

### Problem: "Trace not started" or No Output

**✅ YES, Port 0 is correct!** 
- ITM Port 0 is the **standard** debug output port
- CMSIS convention for printf/debug output
- All debug frameworks use Port 0

**Possible Causes & Solutions:**

**1. Port 0 not enabled in console**
- Click Configure (⚙️)
- **Check Port 0** checkbox
- Click OK
- Click Start Trace

**2. Trace not actually started**
- Click "Start Trace" button (⏺️)
- Button should turn green when active
- If still red, check debug interface (SWD not JTAG)

**3. MCU not running (halted)**
- Press Resume (▶️) or F8
- MCU must be executing for ITM output
- Check status bar shows "Running"

**4. Wrong core clock setting**
- Check your clock configuration in CubeMX
- Update Core Clock in debug config to match
- For STM32F407: should be **168000000**

**5. SWO clock too high**
- Maximum is Core Clock ÷ 4
- Try lower value: **500000** or 1000000
- More reliable than maximum speed

**6. DEBUG_OUTPUT not set to SWV**
- Check Config/module_config.h
- Should be: `#define MODULE_DEBUG_OUTPUT DEBUG_OUTPUT_SWV`
- Rebuild firmware after changing

**7. SWV not enabled in debug config**
- Go to Debug Configurations
- Debugger tab
- Check "Enable" under SWV section

### Problem: Garbled Characters

**Cause:** Clock mismatch

**Solution:**
1. Verify actual CPU clock (check your CubeMX clock tree)
2. Update Core Clock in debug configuration
3. Reduce SWO Clock to 500000 or 1000000

### Problem: Output Stops After a While

**Cause:** ITM buffer overflow

**Solutions:**
1. Reduce debug output frequency
2. Use dbg_printf less often
3. Buffer data and print in batches
4. Increase SWO clock speed (if stable)

### Problem: "Configure trace" Button Grayed Out

**Cause:** Trace is running

**Solution:**
1. Stop trace first (red square ⏹️)
2. Configure
3. Start trace again

---

## Part 6: Advanced Configuration

### Changing SWO Clock Speed

**Lower speeds (more reliable):**
- 500000 Hz (500 kHz) - Very reliable, slower
- 1000000 Hz (1 MHz) - Good balance

**Higher speeds (less reliable):**
- 4000000 Hz (4 MHz) - Faster, may have errors
- 42000000 Hz (42 MHz) - Maximum for STM32F407, unstable

**Recommendation:** Start with 2000000 (2 MHz), decrease if problems.

### Using Multiple Ports

You can use different ITM ports for different purposes:

```c
// In your code:
ITM->PORT[0].u8 = 'A';  // General debug (Port 0)
ITM->PORT[1].u8 = 'B';  // MIDI events (Port 1)
ITM->PORT[2].u8 = 'C';  // Timing data (Port 2)
```

Then in SWV configuration, enable multiple ports:
- Port 0: ☑ General debug
- Port 1: ☑ MIDI events
- Port 2: ☑ Timing

Each port will appear as a separate tab in SWV console.

### Timestamps

In SWV ITM Data Console toolbar:

- Click **Display Timestamps** button (clock icon 🕐)
- Timestamps will appear before each line
- Useful for timing analysis

---

## Part 7: Quick Reference

### Debug Configuration (One-Time Setup)

1. **Run** → **Debug Configurations...**
2. Select your config under **STM32 Cortex-M C/C++ Application**
3. **Debugger** tab
4. Scroll to **Serial Wire Viewer (SWV)**
5. **☑ Enable**
6. **Core Clock:** 168000000
7. **SWO Clock:** 2000000
8. **Port 0:** ☑ Enabled
9. **Apply** → **Close**

### Every Debug Session

1. **F11** (Start Debug)
2. **Window** → **Show View** → **SWV** → **SWV ITM Data Console**
3. **Configure** (⚙️) → **Port 0: ☑** → **OK**
4. **Start Trace** (⏺️)
5. **Resume** (▶️ or F8)
6. View output in console!

### Common Buttons

| Button | Name | Function |
|--------|------|----------|
| ⏺️ | Start Trace | Begin capturing SWV data |
| ⏹️ | Stop Trace | Stop capturing |
| ⚙️ | Configure | Enable/disable ports |
| 🗑️ | Clear | Clear console output |
| 🕐 | Timestamps | Show/hide timestamps |
| 💾 | Save | Save output to file |

---

## Part 8: Comparing SWV vs USB CDC

### When to Use SWV

- ✅ During active debugging sessions
- ✅ When USB is not working
- ✅ When you need real-time traces
- ✅ When USB MIDI conflicts with CDC
- ✅ For timing-critical analysis

### When to Use USB CDC

- ✅ Standalone testing (no debugger)
- ✅ With MIOS Studio
- ✅ Production/field testing
- ✅ Sharing logs with others
- ✅ When ST-Link not available

### Best Configuration: Use Both!

```c
// In Config/module_config.h:
#define MODULE_DEBUG_OUTPUT DEBUG_OUTPUT_SWV  // Debug traces via SWV
#define MODULE_ENABLE_USB_CDC 1               // MIOS terminal via USB
```

This way:
- Debug output goes to SWV (no conflicts)
- MIOS Studio terminal works via USB CDC
- CLI commands work via USB CDC
- Best of both worlds!

---

## Part 9: Example Debug Session

### Step-by-Step Example

1. **Open project** in STM32CubeIDE
2. **Verify configuration** in `Config/module_config.h`:
   ```c
   #define MODULE_DEBUG_OUTPUT DEBUG_OUTPUT_SWV
   ```
3. **Configure debug** (Run → Debug Configurations)
   - Enable SWV
   - Core Clock: 168000000
   - SWO Clock: 2000000
   - Port 0: Enabled
4. **Build project** (Ctrl+B)
5. **Start debug** (F11)
6. **Open SWV console** (Window → Show View → SWV → SWV ITM Data Console)
7. **Configure Port 0** (⚙️ → Port 0: ☑ → OK)
8. **Start trace** (⏺️)
9. **Resume execution** (▶️)
10. **See output** in console! 🎉

### Expected Output

```
=== SWV Debug Output Active ===

==============================================
Debug output: SWV (Serial Wire Viewer)
View in STM32CubeIDE: SWV ITM Data Console
==============================================

[TRACE-1] Main after HAL_Init
[TRACE-2] Main after osKernelInit
[TRACE-3] Main after spibus_init
[TRACE-4] Main before osKernelStart
[TRACE-5] StartDefaultTask entry
[TRACE-6] Before app_entry_start
[App Init] Starting initialization...
[Module] AINSER64: Initialized
[Module] SRIO: 8 DIN + 8 DOUT
[Module] Looper: Ready
[Module] Router: 16 nodes
[CLI] Task started - commands ready
> 
```

---

## Summary

### Quick Setup Checklist

- [ ] Set `MODULE_DEBUG_OUTPUT = DEBUG_OUTPUT_SWV`
- [ ] Debug Config: Enable SWV
- [ ] Debug Config: Core Clock = 168000000
- [ ] Debug Config: SWO Clock = 2000000
- [ ] Debug Config: Port 0 = Enabled
- [ ] Start debug session (F11)
- [ ] Open SWV ITM Data Console
- [ ] Configure Port 0 enabled
- [ ] Start Trace
- [ ] Resume execution
- [ ] See output! ✅

### Key Points to Remember

1. **SWV must be enabled** in debug configuration
2. **Core clock must match** your actual CPU clock (168 MHz)
3. **Port 0 must be enabled** for debug output
4. **Start trace** every debug session
5. **SWV only works** during debug (not standalone)

### Need Help?

See:
- **docs/DEBUG_OUTPUT_GUIDE.md** - Complete debug output guide
- **Config/module_config.h** - Configuration options and comments
- **App/tests/test_debug.c** - Implementation details

---

## Conclusion

SWV is a powerful debugging tool that doesn't interfere with USB. Once configured, you just need to:
1. Start debug (F11)
2. Start trace (⏺️)
3. View output!

Happy debugging! 🚀
