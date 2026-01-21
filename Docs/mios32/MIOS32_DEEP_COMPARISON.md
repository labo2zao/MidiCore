# MIOS32 Deep Comparison Analysis for USB Issue

## Problem Statement
- Hardware: STM32F407VGT6 (confirmed working with MIOS32)
- LED lights up (power OK)
- Build succeeds (0 errors)
- **Windows sees NOTHING** (not even unknown device)

## MIOS32 Reference Implementation
**Source**: github.com/midibox/mios32  
**Files**: `mios32/STM32F4xx/mios32_usb.c`, `mios32_usb_midi.c`

---

## Critical Points from MIOS32 Analysis

### 1. USB Clock Configuration

**MIOS32 Code** (mios32_usb.c):
```c
// Enable USB OTG FS clock
RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_OTG_FS, ENABLE);

// For STM32F4: USB clock comes from PLL
// Must be 48 MHz exactly
// PLL_Q divider configured in system_stm32f4xx.c
```

**Our Code** (stm32f4xx_hal_msp.c line 842):
```c
__HAL_RCC_USB_OTG_FS_CLK_ENABLE();
```

**Action**: ✅ Clock enabled - OK

**VERIFY**: Check `system_stm32f4xx.c` that PLL_Q generates exactly 48 MHz!

---

### 2. GPIO Alternate Function

**MIOS32 Code**:
```c
GPIO_InitTypeDef GPIO_InitStructure;
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
GPIO_Init(GPIOA, &GPIO_InitStructure);

GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_OTG1_FS);
GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_OTG1_FS);
```

**Our Code** (stm32f4xx_hal_msp.c lines 835-840):
```c
GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
```

**Action**: ✅ GPIO config looks correct - OK

---

### 3. USB Interrupt Priority

**MIOS32 Code**:
```c
NVIC_InitTypeDef NVIC_InitStructure;
NVIC_InitStructure.NVIC_IRQChannel = OTG_FS_IRQn;
NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = MIOS32_IRQ_USB_PRIORITY;
NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
NVIC_Init(&NVIC_InitStructure);

// MIOS32_IRQ_USB_PRIORITY is typically 5 or 6
```

**Our Code** (stm32f4xx_hal_msp.c lines 848-849):
```c
HAL_NVIC_SetPriority(OTG_FS_IRQn, 5, 0);
HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
```

**Action**: ✅ Priority 5 - matches MIOS32 - OK

---

### 4. VBUS Sensing Configuration (CRITICAL!)

**MIOS32 Code** (mios32_usb.c):
```c
// For STM32F4 without VBUS pin
USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_NOVBUSSENS;  // Disable VBUS sensing
USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_PWRDWN;      // Power up PHY

// Force B-Device session valid
USB_OTG_FS->GOTGCTL |= (1 << 6);  // BVALOEN
USB_OTG_FS->GOTGCTL |= (1 << 7);  // BVALOVAL
```

**Our Code** (usbd_conf.c lines 211-219):
```c
USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_NOVBUSSENS;
USB_OTG_FS->GCCFG &= ~USB_OTG_GCCFG_VBUSBSEN;
USB_OTG_FS->GCCFG &= ~USB_OTG_GCCFG_VBUSASEN;
USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_PWRDWN;
USB_OTG_FS->GOTGCTL |= USB_OTG_GOTGCTL_BVALOEN;
USB_OTG_FS->GOTGCTL |= USB_OTG_GOTGCTL_BVALOVAL;
```

**Action**: ✅ Matches MIOS32 - OK

**BUT**: Check if VBUSBSEN/VBUSASEN clear operations interfere!

---

### 5. FIFO Allocation

**MIOS32 Code**:
```c
// RX FIFO
USB_OTG_FS->GRXFSIZ = 128;  // 128 words (512 bytes)

// TX FIFOs
USB_OTG_FS->DIEPTXF0_HNPTXFSIZ = (64 << 16) | 128;  // EP0: 64 words at offset 128
USB_OTG_FS->DIEPTXF[0] = (128 << 16) | 192;          // EP1: 128 words at offset 192
```

**Our Code** (usbd_conf.c lines 222-224):
```c
HAL_PCDEx_SetRxFiFo(&hpcd_USB_OTG_FS, 0x80);  // 128 words
HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 0, 0x40);  // EP0: 64 words
HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 1, 0x80);  // EP1: 128 words
```

**Action**: ✅ FIFO allocation matches MIOS32 - OK

---

### 6. Device Connect Timing (CRITICAL!)

**MIOS32 Code** (mios32_usb.c):
```c
MIOS32_USB_Init(0);  // Initialize USB stack

// IMPORTANT: Small delay before connect!
MIOS32_DELAY_Wait_uS(100);  // 100 microseconds

MIOS32_USB_DevConnect();  // Connect to host
```

**Our Code Flow**:
```c
MX_USB_DEVICE_Init()
  └─ USBD_Start()
       └─ USBD_LL_Start()
            └─ HAL_PCD_Start()
                 └─ USB_DevConnect()  // IMMEDIATE!
```

**POTENTIAL PROBLEM**: No delay between init and connect!

---

### 7. USB PHY Embedded Configuration

**MIOS32 Code**:
```c
// Select Full Speed embedded PHY
USB_OTG_FS->GUSBCFG |= USB_OTG_GUSBCFG_PHYSEL;

// USB Reset
while((USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_AHBIDL) == 0);
USB_OTG_FS->GRSTCTL |= USB_OTG_GRSTCTL_CSRST;
while((USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_CSRST) != 0);

// Small delay after reset
for(volatile int i=0; i<1000; i++);

// Then configure GCCFG
```

**Our Code** (via HAL_PCD_Init → USB_CoreInit):
```c
// HAL does USB_CoreReset internally
// Then immediately proceeds
```

**POTENTIAL PROBLEM**: May need delay after reset!

---

### 8. Device Descriptor Timing

**MIOS32 Observation**:
- MIOS32 registers USB class BEFORE connecting device
- Ensures descriptors ready when host queries

**Our Code**:
```c
USBD_Init()           // Init library
USBD_RegisterClass()  // Register MIDI class
USBD_Start()          // Start + Connect
```

**Action**: ✅ Order correct - OK

---

## CRITICAL TESTS TO TRY

### Test 1: Add Delay After Init Before Connect

**File**: `USB_DEVICE/Target/usbd_conf.c`

**Modify USBD_LL_Init()** - Add delay at end:
```c
USBD_StatusTypeDef USBD_LL_Init(USBD_HandleTypeDef *pdev)
{
  // ... existing init code ...
  
  /* CRITICAL: Add delay before USB_DevConnect in USBD_LL_Start */
  /* Delay here ensures PHY is stable before connect */
  // NOTE: Actual connect happens in USBD_LL_Start
  
  return USBD_OK;
}
```

**Modify USBD_LL_Start()**:
```c
USBD_StatusTypeDef USBD_LL_Start(USBD_HandleTypeDef *pdev)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  
  /* MIOS32-style delay before connect */
  for(volatile uint32_t i=0; i<10000; i++);  // ~100us delay
  
  hal_status = HAL_PCD_Start(pdev->pData);
  // ...
}
```

### Test 2: Check PLL_Q Clock (48 MHz)

**File**: Check `Core/Src/system_stm32f4xx.c` or `.ioc` file

**Requirements**:
- USB needs EXACTLY 48 MHz clock
- Comes from PLL via PLL_Q divider
- Formula: PLL_Q = PLL_VCO / Q_DIV
- Must equal 48000000 Hz

**Verify**:
```c
// In system_stm32f4xx.c or RCC config
// PLL_Q must generate 48 MHz
```

### Test 3: Simplify GCCFG Operations

**File**: `USB_DEVICE/Target/usbd_conf.c`

**Current** (lines 211-213):
```c
USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_NOVBUSSENS;
USB_OTG_FS->GCCFG &= ~USB_OTG_GCCFG_VBUSBSEN;
USB_OTG_FS->GCCFG &= ~USB_OTG_GCCFG_VBUSASEN;
```

**Try**:
```c
// Single operation - set what we want, clear what we don't
uint32_t gccfg = USB_OTG_FS->GCCFG;
gccfg |= USB_OTG_GCCFG_NOVBUSSENS;  // Disable VBUS sensing
gccfg &= ~(USB_OTG_GCCFG_VBUSBSEN | USB_OTG_GCCFG_VBUSASEN);  // Clear both
gccfg |= USB_OTG_GCCFG_PWRDWN;  // Power up
USB_OTG_FS->GCCFG = gccfg;
```

### Test 4: Check USB_DevConnect Call

**File**: Verify in HAL that USB_DevConnect sets DCTL properly

**Expected**:
```c
// In USB_DevConnect function
USBx_DEVICE->DCTL &= ~USB_OTG_DCTL_SDIS;  // Clear soft disconnect
```

**Verify** this is being called and taking effect.

---

## Hardware Verification Checklist

### Physical Connections
- [ ] PA11 (D-) connected to USB connector pin 2
- [ ] PA12 (D+) connected to USB connector pin 3
- [ ] USB GND connected to board GND
- [ ] USB shield connected to chassis ground
- [ ] 1.5kΩ pull-up on D+ (USB Full Speed indicator)

### Power
- [ ] 3.3V stable on STM32
- [ ] USB cable provides 5V VBUS (even if not sensed)
- [ ] No brown-out during USB operations

### Oscilloscope Test (If Available)
- [ ] Check PA11/PA12 for any activity when USB connected
- [ ] Should see D+ pulled up to 3.3V via 1.5kΩ
- [ ] Should see USB bus activity after connect

---

## MIOS32 vs MidiCore Summary

| Feature | MIOS32 | MidiCore | Status |
|---------|--------|----------|--------|
| GPIO Config | AF10, 100MHz | AF10, VERY_HIGH | ✅ OK |
| USB Clock | 48MHz from PLL_Q | HAL auto | ⚠️ VERIFY |
| VBUS Sense | Disabled + PWRDWN | Disabled + PWRDWN | ✅ OK |
| B-Session | Forced valid | Forced valid | ✅ OK |
| FIFO Alloc | 128/64/128 | 128/64/128 | ✅ OK |
| IRQ Priority | 5 or 6 | 5 | ✅ OK |
| **Connect Delay** | **100us delay** | **NONE** | ❌ **MISSING!** |
| **Reset Delay** | **After core reset** | **HAL default** | ⚠️ **CHECK** |

---

## Recommended Actions (Priority Order)

1. **HIGHEST PRIORITY**: Add 100us delay before USB_DevConnect
2. **HIGH**: Verify PLL_Q generates exactly 48 MHz
3. **MEDIUM**: Add delay after USB_CoreReset
4. **LOW**: Simplify GCCFG operations to single write

---

## Debug Strategy

### If Still Not Working After Above:

1. **Use Debugger Breakpoints**:
   - Set breakpoint in `HAL_PCD_Start()`
   - Verify `USB_OTG_FS->GCCFG` register value
   - Verify `USB_OTG_FS->GOTGCTL` register value
   - Check `USB_OTG_FS->DCTL` soft disconnect bit

2. **Add Debug LEDs**:
```c
// At key points in init
HAL_GPIO_WritePin(DEBUG_LED_PORT, DEBUG_LED_PIN, GPIO_PIN_SET);
// Toggle at each stage
```

3. **Check USB Registers After Init**:
```c
volatile uint32_t gccfg = USB_OTG_FS->GCCFG;
volatile uint32_t gotgctl = USB_OTG_FS->GOTGCTL;
volatile uint32_t dctl = USB_OTG_FS->DCTL;
// Examine in debugger
```

Expected values:
- `GCCFG` should have `PWRDWN` (bit 16) = 1
- `GCCFG` should have `NOVBUSSENS` (bit 21) = 1
- `GOTGCTL` should have `BVALOEN` (bit 6) = 1
- `GOTGCTL` should have `BVALOVAL` (bit 7) = 1
- `DCTL` should NOT have `SDIS` (bit 1) = 1

---

## Next Steps

**IMMEDIATE**: Implement Test 1 (add delay before connect)  
**THEN**: Verify PLL_Q clock configuration  
**IF FAILS**: Implement Tests 2-4  
**IF STILL FAILS**: Hardware debug with oscilloscope

**Document Version**: 1.0  
**Analysis Date**: 2026-01-21
