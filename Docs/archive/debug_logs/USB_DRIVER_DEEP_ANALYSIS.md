# USB Driver Not Responding - Deep Code Analysis

## Problem Statement
User reports USB driver is not responding. Need deep investigation of USB implementation.

## Complete USB Stack Review

### 1. Main Initialization (Core/Src/main.c)

**Initialization Order** ✅ CORRECT
```c
Line 141: MX_GPIO_Init();           // GPIO first
Line 163: __HAL_RCC_CCMDATARAMEN_CLK_ENABLE();  // CCMRAM enabled
Line 167: MX_USB_DEVICE_Init();     // USB Device initialization
Line 172: usb_midi_init();          // MIDI interface registration
Line 181: usb_cdc_init();           // CDC interface callbacks
Line 187: osKernelInitialize();     // FreeRTOS after USB
```

### 2. USB Device Init (USB_DEVICE/App/usb_device.c)

```c
void MX_USB_DEVICE_Init(void)
{
  USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS);  // Init device library
  USBD_RegisterClass(&hUsbDeviceFS, &USBD_COMPOSITE);  // Register composite class
  USBD_Start(&hUsbDeviceFS);  // Start USB device
}
```

**Status**: ✅ CORRECT - All error handlers present

### 3. USB Low-Level Init (USB_DEVICE/Target/usbd_conf.c)

**Critical Configuration Checks**:

```c
Line 193: dev_endpoints = 4  ✅ (EP0 + EP1_MIDI + EP2_CDC + EP3_CDC)
Line 194: speed = PCD_SPEED_FULL  ✅
Line 200: vbus_sensing_enable = DISABLE  ✅ (STM32F407 has no VBUS)
```

**VBUS Workaround** ✅ CORRECT (Lines 212-220):
```c
USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_NOVBUSSENS;  // Disable VBUS sensing
USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_PWRDWN;      // Power up PHY
USB_OTG_FS->GOTGCTL |= USB_OTG_GOTGCTL_BVALOEN; // B-device override enable
USB_OTG_FS->GOTGCTL |= USB_OTG_GOTGCTL_BVALOVAL;// Force B-session valid
```

**FIFO Allocation** ✅ CORRECT (Lines 226-233):
```
Composite Device (MIDI + CDC):
- RX FIFO: 96 words (384 bytes)
- EP0 TX: 48 words (192 bytes) - Control
- EP1 TX: 64 words (256 bytes) - MIDI IN
- EP2 TX: 96 words (384 bytes) - CDC Data IN
- EP3 TX: 16 words (64 bytes) - CDC Command IN
Total: 320 words = 1280 bytes ✅ (STM32F407 has 320 words FIFO)
```

### 4. USB Clock (Core/Src/main.c - SystemClock_Config)

**PLL Configuration**:
```c
HSE: 8 MHz
PLLM = 8, PLLN = 336, PLLP = 2, PLLQ = 7

System Clock = 8MHz * 336 / 2 = 168 MHz ✅
USB Clock = 8MHz * 336 / 7 = 48 MHz ✅
```

**Status**: ✅ USB Clock is EXACTLY 48MHz (required)

### 5. USB GPIO (Core/Src/stm32f4xx_hal_msp.c)

```c
PA11 -> USB_OTG_FS_DM  (AF10)  ✅
PA12 -> USB_OTG_FS_DP  (AF10)  ✅
Speed: GPIO_SPEED_FREQ_VERY_HIGH  ✅
Pull: GPIO_NOPULL  ✅
```

### 6. USB Interrupt (Core/Src/stm32f4xx_it.c)

```c
Line 234: void OTG_FS_IRQHandler(void)  ✅
Line 239: HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);  ✅
Priority: 4 (< 5, not masked by FreeRTOS)  ✅
```

### 7. Composite Class Implementation (usbd_composite.c)

**Init Function** (Lines 100-139) ✅ CORRECT:
```c
- Initializes composite_class_data
- Sets pdev->pClassData = &composite_class_data
- Calls USBD_MIDI.Init() and saves midi_class_data
- Restores pdev->pClassData before CDC init
- Calls USBD_CDC.Init() and saves cdc_class_data
- Restores pdev->pClassData after both inits
```

**Setup Handler** (Lines 192-225) ✅ CORRECT:
```c
- Routes by interface number (0-1: MIDI, 2-3: CDC)
- Routes by endpoint (0x01: MIDI, 0x02-0x03: CDC)
- Switches class data before calling class handlers
- Restores composite data after handling
```

**Descriptor Building** (Lines 343-550) ✅ CORRECT:
```c
- Configuration header with 4 interfaces
- MIDI IAD + interfaces (0-1)
- CDC IAD (interfaces 2-3)
- CDC Control Interface (2)
- CDC Data Interface (3)
- All descriptors statically built (no pattern matching)
```

## Verification Status

| Component | Status | Notes |
|-----------|--------|-------|
| System Clock | ✅ | 168MHz system, 48MHz USB |
| USB GPIO | ✅ | PA11/PA12 configured |
| USB Interrupt | ✅ | Handler present, priority OK |
| USB Init Order | ✅ | Correct sequence |
| VBUS Workaround | ✅ | B-device session forced |
| FIFO Allocation | ✅ | 320 words allocated |
| Composite Init | ✅ | Class data managed correctly |
| Descriptor | ✅ | Static build, no corruption |
| Interface Registration | ✅ | usb_midi_init(), usb_cdc_init() called |

## Potential Issues

Based on code review, **ALL CONFIGURATION IS CORRECT**. If USB is not responding, check:

### Runtime Issues (Not Code Issues)

1. **Physical Connection**
   - USB cable quality
   - USB port on PC
   - Try different cable/port

2. **Windows Driver**
   - Device Manager errors
   - Driver cache issues
   - Need to uninstall/reinstall

3. **Build Issues**
   - Stale build artifacts
   - Wrong branch compiled
   - Optimization issues

4. **Hardware Issues**
   - STM32 PA11/PA12 pins damaged
   - USB transceiver issue
   - Power supply issue

### Diagnostic Steps

1. **Check USB Enumeration**:
   ```
   Windows: Device Manager
   Linux: lsusb, dmesg
   ```

2. **Check Firmware Version**:
   - Verify correct firmware flashed
   - Check build date/time

3. **Enable USB Debug**:
   ```c
   // In Config/module_config.h
   #define MODULE_ENABLE_USB_MIDI_DEBUG 1
   ```

4. **Check USB Descriptor with Tools**:
   - Windows: USBTreeView
   - Linux: lsusb -v

5. **Monitor USB Signals**:
   - Use USB analyzer (if available)
   - Check D+/D- signal quality

## Recommendations

### If Device Not Enumerating

**Check HAL_PCD_Init() return value**:
Add debug output in `USBD_LL_Init()` (usbd_conf.c:204):
```c
if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK)
{
  // Add UART debug here
  Error_Handler();
}
```

**Verify USB PHY Power**:
Check GCCFG register:
```c
// In USBD_LL_Init(), after line 217
if (!(USB_OTG_FS->GCCFG & USB_OTG_GCCFG_PWRDWN)) {
  // PHY not powered! Add debug
}
```

**Check Connection Callback**:
Verify `HAL_PCD_ConnectCallback()` is called (usbd_conf.c:162).

### If Device Enumerates But Doesn't Respond

**Check Setup Requests**:
Add debug in `USBD_COMPOSITE_Setup()` (usbd_composite.c:192):
```c
// Log interface/endpoint being accessed
// Check if target_class is NULL
```

**Verify Class Data Pointers**:
Check that `midi_class_data` and `cdc_class_data` are not NULL after init.

**Check FIFO Status**:
Verify no FIFO overflow/underflow errors.

## Conclusion

**Code is CORRECT**. All USB configuration is proper. Issue is likely:
1. Build/deployment problem
2. Hardware connection issue  
3. Windows driver cache
4. Need runtime debugging to identify actual failure point

**Next Step**: Add runtime diagnostics to identify WHERE the failure occurs.

