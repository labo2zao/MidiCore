# FINAL DEBUG CHECKLIST - Systematic Verification

## Critical Verification Steps

### Step 1: Build Verification
```bash
# Clean build
Project → Clean → Clean all projects
Project → Build Project

# Expected result: 0 errors, 0 warnings (warnings OK)
# Check console for "Build Finished"
```

**If build fails**: Share exact error messages

### Step 2: Configuration File Verification

**Check Config/module_config.h line 76-77**:
```c
#define MODULE_ENABLE_USB_MIDI   1  // Must be 1
#define MODULE_ENABLE_USBH_MIDI  0  // Must be 0 (Device only)
```

**Check Core/Src/main.c line 20-21** (includes):
```c
#include "Config/module_config.h"  // Must be FIRST include
#include "main.h"
```

**Check Core/Src/main.c line 156-160** (USER CODE BEGIN 2):
```c
/* USER CODE BEGIN 2 */
#if MODULE_ENABLE_USB_MIDI
  MX_USB_DEVICE_Init();
  usb_midi_init();
#endif
/* USER CODE END 2 */
```

### Step 3: Clock Configuration Verification

**Check MidiCore.ioc → RCC Configuration**:
- HSE: 8 MHz (external crystal)
- PLL_M: 8
- PLL_N: 336
- PLL_P: 2 (for system clock)
- **PLL_Q: 7** ← CRITICAL for USB (must be 7)
- System Clock: 168 MHz
- **USB Clock: 48 MHz** ← MUST be exactly 48 MHz

**Formula**: `(8 MHz * 336) / (8 * 7) = 48 MHz` ✓

### Step 4: USB Hardware Verification

**Physical Connections**:
- [ ] PA11 (D-) connected to USB connector pin 2
- [ ] PA12 (D+) connected to USB connector pin 3
- [ ] USB GND connected to board GND
- [ ] USB shield connected to chassis ground
- [ ] 3.3V stable on STM32 (measure with multimeter)
- [ ] 5V VBUS present on USB cable (measure at connector)

**Cable Test**:
- [ ] USB cable is data cable (not charge-only)
- [ ] Test with different USB cable
- [ ] Test with different USB port on PC

### Step 5: GPIO Configuration Verification

**Check .ioc file → Pinout view**:
- PA11: USB_OTG_FS_DM (Alternate Function AF10)
- PA12: USB_OTG_FS_DP (Alternate Function AF10)
- Both pins: Speed = VERY HIGH

**Check Core/Src/stm32f4xx_hal_msp.c** (HAL_PCD_MspInit):
```c
GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
```

### Step 6: Debugger Register Verification

**With ST-Link debugger connected**:

1. **Set breakpoint** in main.c after `MX_USB_DEVICE_Init()`
2. **Run** and wait for breakpoint
3. **Check registers** (Expressions window):

```c
// Check USB clock enabled
RCC->AHB2ENR & RCC_AHB2ENR_OTGFSEN  // Should be non-zero

// Check USB configured
USB_OTG_FS->GCCFG    // Bit 16 (PWRDWN) = 1, Bit 21 (NOVBUSSENS) = 1
USB_OTG_FS->GOTGCTL  // Bit 6 (BVALOEN) = 1, Bit 7 (BVALOVAL) = 1
USB_OTG_FS->DCTL     // Bit 1 (SDIS) = 0 (device NOT soft-disconnected)

// Check PLL configured
RCC->PLLCFGR         // Should show PLLQ = 7
```

**Expected values**:
- `USB_OTG_FS->GCCFG = 0x00210000` (or similar with bits 16 and 21 set)
- `USB_OTG_FS->GOTGCTL` bit 6 and 7 = 1
- `USB_OTG_FS->DCTL` bit 1 = 0

### Step 7: Interrupt Verification

**Check Core/Src/stm32f4xx_it.c**:
```c
void OTG_FS_IRQHandler(void)
{
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);  // NOT HAL_HCD_IRQHandler!
}
```

**Check Core/Src/stm32f4xx_hal_msp.c** (HAL_PCD_MspInit):
```c
HAL_NVIC_SetPriority(OTG_FS_IRQn, 5, 0);
HAL_NVIC_EnableIRQ(OTG_FS_IRQn);  // Must be called!
```

### Step 8: Windows Device Manager Check

**What to look for**:
1. Open Device Manager (devmgmt.msc)
2. **Expand ALL categories** (not just MIDI)
3. Look in:
   - Sound, video and game controllers
   - Universal Serial Bus controllers
   - Other devices
   - Unknown devices

**When plugging USB**:
- Does Device Manager refresh?
- Does Windows play a sound?
- Does ANYTHING appear (even "Unknown device")?

### Step 9: Alternative Testing

**Test 1: Simple USB Device Test**:
If you have another STM32 USB example (CDC, HID, etc.), flash it to verify:
- USB hardware works
- Cable works
- Windows USB stack works

**Test 2: Oscilloscope Test** (if available):
- Measure D+ line (PA12)
- Should see 3.3V (pull-up resistor) when device connects
- Should see data activity during enumeration

**Test 3: Different PC**:
- Test on another computer
- Test on another OS (Linux shows more USB debug info)

### Step 10: Last Resort Checks

**If STILL not working**:

1. **Verify .ioc USB configuration**:
   - Open MidiCore.ioc
   - Connectivity → USB_OTG_FS
   - Mode: Device_Only (NOT Host_Only, NOT OTG/Dual_Role)
   - Generate code: Project → Generate Code

2. **Check for conflicting code**:
   ```bash
   # Search for USB_DevDisconnect calls
   grep -r "USB_DevDisconnect" .
   
   # Search for SDIS bit manipulation
   grep -r "DCTL.*SDIS" .
   ```

3. **Verify USB_DEVICE files are compiled**:
   ```bash
   # Check that these .o files exist after build:
   ls -la Debug/USB_DEVICE/App/*.o
   ls -la Debug/USB_DEVICE/Target/*.o
   ```

4. **Check linker map** (MidiCore.map):
   ```bash
   # Verify USB symbols are linked
   grep "MX_USB_DEVICE_Init" Debug/MidiCore.map
   grep "USBD_MIDI_" Debug/MidiCore.map
   ```

## Common Issues and Solutions

### Issue 1: Clock Not 48 MHz
**Symptom**: Device not enumerated, or intermittent issues
**Solution**: Verify PLL_Q = 7 in .ioc file, regenerate code

### Issue 2: GPIO Not Configured
**Symptom**: No electrical activity on D+/D-
**Solution**: Check AF10 assignment, VERY_HIGH speed

### Issue 3: Interrupt Not Working
**Symptom**: Device connects but no MIDI data
**Solution**: Verify OTG_FS_IRQHandler calls HAL_PCD_IRQHandler

### Issue 4: Host Mode Interfering
**Symptom**: Device connects then immediately disconnects
**Solution**: Verify MODULE_ENABLE_USBH_MIDI = 0

### Issue 5: VBUS Sensing Issue
**Symptom**: Device never connects (STM32F407 specific)
**Solution**: Verify PWRDWN=1, NOVBUSSENS=1, BVALOEN=1, BVALOVAL=1

## Debug Information to Collect

**If problem persists, collect this information**:

1. **Build Output**:
   - Copy full console output from Clean → Build
   - Any warnings or errors

2. **Configuration**:
   - module_config.h lines 76-77
   - main.c lines 156-160
   - .ioc file RCC configuration (screenshot)

3. **Register Values** (with debugger):
   - RCC->AHB2ENR
   - RCC->PLLCFGR
   - USB_OTG_FS->GCCFG
   - USB_OTG_FS->GOTGCTL
   - USB_OTG_FS->DCTL

4. **Windows Behavior**:
   - Does Device Manager change at all?
   - Any sound when connecting?
   - What appears in Device Manager (screenshot)

5. **Hardware**:
   - Board model exact name
   - Crystal frequency (usually 8 MHz)
   - USB connector type

## MIOS32 Comparison

**What MIOS32 does differently** (for reference):
1. Manual clock configuration (no HAL)
2. Explicit delay sequences
3. Manual register writes
4. Careful interrupt timing
5. **Runtime mode switching** (Device ↔ Host)

**What we've implemented** (MIOS32-equivalent):
- ✓ Same VBUS configuration
- ✓ Same PWRDWN sequence
- ✓ Same B-session forcing
- ✓ Same 100µs delay
- ✓ Same FIFO allocation
- ⚠ Different HAL abstraction layer

**Next step if Device works**: Implement MIOS32-style ID pin detection for automatic mode switching.

## Success Criteria

**Device mode working** when:
1. Windows plays USB connect sound
2. "MidiCore 4x4" appears in Device Manager
3. 4 MIDI In/Out ports visible in DAW
4. MIDI data can be sent/received

**Then we can add Host mode** with MIOS32-style switching!
