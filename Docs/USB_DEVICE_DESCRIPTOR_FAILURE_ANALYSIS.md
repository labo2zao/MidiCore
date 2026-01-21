# USB DEVICE_DESCRIPTOR_FAILURE Analysis

## Critical Finding from USB Tree Viewer

**Date:** 2026-01-21
**Error:** Windows reports `USB\DEVICE_DESCRIPTOR_FAILURE`

### USB Tree Viewer Output Analysis

```
Connection Status        : 0x02 (Device failed enumeration)
Device Description       : Unknown USB Device (Device Descriptor Request Failed)
Hardware IDs             : USB\DEVICE_DESCRIPTOR_FAILURE
Device ID                : USB\VID_0000&PID_0002\...
Problem Code             : 43 (CM_PROB_FAILED_POST_START)
```

### What This Means

**Windows cannot even read the basic DEVICE descriptor** (18 bytes). This is the FIRST descriptor Windows requests during enumeration, before any Configuration descriptor.

The sequence is:
1. **Device Descriptor** ← **FAILING HERE**
2. Configuration Descriptor  
3. String Descriptors

### Why This Changes Everything

All previous fixes focused on the Configuration descriptor (207 bytes, IAD removal, Bulk endpoints, etc.), but **Windows never gets that far**. The failure happens at the very first descriptor request.

The `VID_0000&PID_0002` shown by Windows are **default/invalid values**, not the actual VID=0x16C0, PID=0x0489 from our descriptor. This confirms Windows couldn't read our descriptor.

## Root Cause Analysis

### Possible Causes (in order of likelihood):

#### 1. USB Core Not Initializing Properly
**Most Likely**

The USB peripheral itself may not be initializing before Windows tries to enumerate:
- USB clock not running (needs 48MHz for Full Speed)
- USB_OTG_FS peripheral not enabled
- GPIO pins (PA11/PA12) not configured for USB
- VBUS sensing issues
- USB DP pull-up not enabled

**Check:**
```c
// In main.c or usb init
MX_USB_DEVICE_Init();  // Is this being called?

// Clock configuration
__HAL_RCC_USB_OTG_FS_CLK_ENABLE();  // Is USB clock enabled?

// GPIO configuration
GPIO PA11 = USB_DM
GPIO PA12 = USB_DP
Both should be AF10 (USB_OTG_FS)
```

#### 2. Descriptor Callback Returning NULL
**Less Likely (but check)**

The `USBD_FS_DeviceDescriptor()` function might be:
- Returning NULL pointer
- Setting length to 0
- Not being called at all

**Current implementation looks OK:**
```c
uint8_t *USBD_FS_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  UNUSED(speed);
  *length = sizeof(USBD_FS_DeviceDesc);  // Should be 18
  return USBD_FS_DeviceDesc;  // Should return pointer
}
```

But verify with UART debug that this function is actually called.

#### 3. USB Interrupts Not Working
The USB interrupt handler might not be firing:
- IRQ not enabled in NVIC
- IRQ priority too low (blocked by other interrupts)
- ISR not properly linked

**Check:**
```c
// In stm32f4xx_it.c
void OTG_FS_IRQHandler(void)
{
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);  // Is this being called?
}
```

#### 4. DMA or Cache Issues
Less likely for STM32F4, but:
- Descriptor buffer not in correct memory region
- Cache coherency issues (F4 doesn't have cache, so unlikely)

#### 5. Electrical/Hardware Issues
- USB cable bad
- DP/DM pins shorted or open
- 1.5kΩ pull-up resistor missing on DP
- VBUS not detected
- Crystal/oscillator not running

## Diagnostic Steps

### Step 1: Enable UART Debug (HIGHEST PRIORITY)

Add debug output to see what's happening:

```c
// In USB_DEVICE/App/usbd_desc.c

uint8_t *USBD_FS_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  printf("DEVICE DESCRIPTOR REQUESTED\r\n");  // ADD THIS
  UNUSED(speed);
  *length = sizeof(USBD_FS_DeviceDesc);
  printf("  Length: %u bytes\r\n", *length);   // ADD THIS
  printf("  Pointer: 0x%08lX\r\n", (uint32_t)USBD_FS_DeviceDesc);  // ADD THIS
  return USBD_FS_DeviceDesc;
}
```

**If you see "DEVICE DESCRIPTOR REQUESTED":**
- Descriptor callback is being called
- Problem is likely in the descriptor data itself or USB response

**If you DON'T see this message:**
- USB core not initializing
- Windows not even sending GET_DESCRIPTOR
- USB electrical problem

### Step 2: Add Debug to USB Init

```c
// In main.c or wherever USB is initialized

printf("Initializing USB...\r\n");
MX_USB_DEVICE_Init();
printf("USB Init complete\r\n");

// In USB_DEVICE/Target/usbd_conf.c - HAL_PCD_MspInit()
printf("USB MSP Init - Enabling clocks\r\n");
__HAL_RCC_USB_OTG_FS_CLK_ENABLE();
printf("USB MSP Init - Configuring GPIO\r\n");
// ... GPIO config
printf("USB MSP Init complete\r\n");
```

### Step 3: Check USB Clock

Verify 48MHz USB clock is actually running:

```c
// Check PLL configuration
RCC_OscInitTypeDef RCC_OscInitStruct = {0};
RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

// Should have:
// PLLM = 8 (for 8MHz HSE)
// PLLN = 336
// PLLP = 2 (for 168MHz system clock)
// PLLQ = 7 (for 48MHz USB clock)  // ← CHECK THIS

// Formula: USB_CLK = (HSE * PLLN) / (PLLM * PLLQ)
//                  = (8MHz * 336) / (8 * 7)
//                  = 2688MHz / 56
//                  = 48MHz ✓
```

### Step 4: Verify GPIO Configuration

```c
// PA11 and PA12 should be configured as:
GPIO_InitTypeDef GPIO_InitStruct = {0};

GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
```

### Step 5: Check VBUS Sensing

Some boards have issues with VBUS sensing. Try disabling it:

```c
// In USB_DEVICE/Target/usbd_conf.c - HAL_PCD_MspInit()

// Try disabling VBUS sensing
USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_NOVBUSSENS;
USB_OTG_FS->GCCFG &= ~USB_OTG_GCCFG_VBUSBSEN;
USB_OTG_FS->GCCFG &= ~USB_OTG_GCCFG_VBUSASEN;
```

Or enable forced B-device mode:
```c
USB_OTG_FS->GUSBCFG |= USB_OTG_GUSBCFG_FDMOD;  // Force device mode
```

## Comparison with MIOS32

Since MIOS32 works on the same hardware, the differences are likely:
1. **Clock configuration** - MIOS32 has correct 48MHz USB clock
2. **Init sequence** - MIOS32 initializes USB at correct time
3. **VBUS handling** - MIOS32 may have different VBUS config

## Immediate Actions

**Priority 1: UART Debug**
Enable printf via UART and add debug to:
- `USBD_FS_DeviceDescriptor()` 
- `MX_USB_DEVICE_Init()`
- `HAL_PCD_MspInit()`
- `OTG_FS_IRQHandler()`

**Priority 2: Clock Verification**
Verify 48MHz USB clock with oscilloscope or logic analyzer on MCO pin.

**Priority 3: Try VBUS Disable**
Disable VBUS sensing as temporary test.

## Expected Output with Debug

### If working:
```
USB MSP Init - Enabling clocks
USB MSP Init - Configuring GPIO  
USB MSP Init complete
Initializing USB...
USB Init complete
[plug in cable]
DEVICE DESCRIPTOR REQUESTED
  Length: 18 bytes
  Pointer: 0x20000xxx
[Windows enumerates successfully]
```

### If USB core not initializing:
```
USB MSP Init - Enabling clocks
USB MSP Init - Configuring GPIO  
USB MSP Init complete
Initializing USB...
USB Init complete
[plug in cable]
[nothing happens - no "DEVICE DESCRIPTOR REQUESTED"]
```

### If descriptor callback has issue:
```
DEVICE DESCRIPTOR REQUESTED
  Length: 0 bytes  ← PROBLEM
  Pointer: 0x00000000  ← PROBLEM (NULL)
```

## Conclusion

The USB Tree Viewer data is **critical** - it shows we're failing much earlier than Configuration descriptor. All descriptor fixes won't help until we fix the DEVICE descriptor enumeration.

The solution is likely:
1. USB clock configuration issue
2. USB peripheral init timing
3. VBUS sensing configuration
4. GPIO pin configuration

UART debug will reveal which one.

---

**Status**: DEVICE_DESCRIPTOR_FAILURE identified. Need UART debug to pinpoint exact cause.
