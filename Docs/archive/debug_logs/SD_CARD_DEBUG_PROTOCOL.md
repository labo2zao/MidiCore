# SD Card Debug Protocol

This document provides step-by-step debugging procedures for SD card initialization issues.

## Quick Reference

**Common Issues**:
- Mount failed → Hardware/initialization issue
- Wrong pins → Check PA4=CS, PA5=SCK, PA6=MISO, PA7=MOSI
- Wrong speed → Should be 656 kHz init, 42 MHz data
- Card incompatible → Use Class 4+ SDHC/SDv2 cards

---

## Debug Levels

### Level 1: Hardware Verification

**Purpose**: Verify SD card slot and SPI connections.

**Steps**:

1. **Visual Inspection**:
   ```
   ☐ SD card fully inserted in slot
   ☐ Card write-protect switch OFF (if present)
   ☐ No physical damage to card or slot
   ☐ Card is 2GB-32GB (SDHC recommended)
   ```

2. **Pin Continuity Test** (multimeter):
   ```
   STM32F407        SD Card Pin
   PA4 (CS)    ←→   Pin 1 (DAT3/CS)
   PA7 (MOSI)  ←→   Pin 2 (CMD/DI)
   GND         ←→   Pin 3 (VSS)
   3.3V        ←→   Pin 4 (VDD)
   PA5 (SCK)   ←→   Pin 5 (CLK)
   GND         ←→   Pin 6 (VSS)
   PA6 (MISO)  ←→   Pin 7 (DAT0/DO)
   PB2 (CD)    ←→   Card Detect (optional)
   ```

3. **Power Supply Check**:
   ```
   ☐ Measure 3.3V on SD card VDD pin
   ☐ Check for voltage drop under load (should stay >3.1V)
   ☐ Verify GND connection solid
   ```

### Level 2: SPI Bus Test

**Purpose**: Verify SPI1 peripheral is working.

**Test Code** (add to module_tests.c):

```c
void sd_debug_spi_loopback_test(void)
{
  dbg_print("=== SPI1 Loopback Test ===\r\n");
  dbg_print("Connect PA7 (MOSI) to PA6 (MISO) with jumper wire\r\n");
  dbg_print("Press any key when ready...\r\n");
  // Wait for user input
  osDelay(5000);
  
  spibus_init();
  spibus_begin(SPIBUS_DEV_SD);
  
  uint8_t test_patterns[] = {0x00, 0xFF, 0xAA, 0x55, 0xF0, 0x0F};
  int pass = 0, fail = 0;
  
  for (int i = 0; i < 6; i++) {
    uint8_t tx = test_patterns[i];
    uint8_t rx = 0;
    
    spibus_txrx(SPIBUS_DEV_SD, &tx, &rx, 1, 100);
    
    if (rx == tx) {
      dbg_printf("[PASS] 0x%02X → 0x%02X\r\n", tx, rx);
      pass++;
    } else {
      dbg_printf("[FAIL] 0x%02X → 0x%02X (expected 0x%02X)\r\n", tx, rx, tx);
      fail++;
    }
  }
  
  spibus_end(SPIBUS_DEV_SD);
  
  dbg_printf("\r\nResult: %d passed, %d failed\r\n", pass, fail);
  if (fail == 0) {
    dbg_print("SPI1 peripheral is working correctly!\r\n");
  } else {
    dbg_print("SPI1 peripheral has issues - check GPIO/clock config\r\n");
  }
}
```

**Expected Result**: All 6 patterns should pass (TX == RX).

### Level 3: SD Card Command Tracing

**Purpose**: Debug SD card initialization sequence with detailed logging.

**Add Debug Version of sd_spi_initialize()**:

```c
// Add to FATFS/Target/sd_spi.c (or create sd_spi_debug.c)

#include "Services/debug/debug.h"  // For dbg_print functions

DSTATUS sd_spi_initialize_debug(void)
{
  uint8_t cmd, n, ocr[4];
  uint16_t tmr;
  
  dbg_print("\r\n=== SD Card Initialization Debug ===\r\n");
  
  // Initialize SPI bus
  dbg_print("[1] Initializing SPI bus...\r\n");
  spibus_init();
  dbg_print("    SPI bus initialized (656 kHz for init)\r\n");
  
  // Send 80 dummy clocks
  dbg_print("[2] Sending 80 dummy clocks (CS=HIGH)...\r\n");
  spibus_end(SPIBUS_DEV_SD);
  osDelay(10);
  
  for (n = 0; n < 10; n++) {
    uint8_t dummy = 0xFF;
    spibus_tx(SPIBUS_DEV_SD, &dummy, 1, 100);
  }
  dbg_print("    80 clocks sent\r\n");
  
  osDelay(1);
  spibus_begin(SPIBUS_DEV_SD);
  
  // CMD0: GO_IDLE_STATE
  dbg_print("[3] Sending CMD0 (GO_IDLE_STATE)...\r\n");
  uint8_t r1 = sd_send_cmd(SD_CMD0, 0);
  dbg_printf("    Response: 0x%02X ", r1);
  if (r1 == 0x01) {
    dbg_print("(OK - card in idle state)\r\n");
  } else {
    dbg_printf("(FAIL - expected 0x01)\r\n");
    dbg_print("    → Card not responding. Check:\r\n");
    dbg_print("       - Card fully inserted\r\n");
    dbg_print("       - SPI connections (PA4/5/6/7)\r\n");
    dbg_print("       - 3.3V power supply\r\n");
    sd_status = STA_NOINIT;
    spibus_end(SPIBUS_DEV_SD);
    return sd_status;
  }
  
  // CMD8: SEND_IF_COND
  dbg_print("[4] Sending CMD8 (SEND_IF_COND, 0x1AA)...\r\n");
  r1 = sd_send_cmd(SD_CMD8, 0x1AA);
  dbg_printf("    Response: 0x%02X ", r1);
  
  if (r1 == 0x01) {
    dbg_print("(OK - SDv2 card detected)\r\n");
    
    // Read R7 response (4 bytes)
    for (n = 0; n < 4; n++) {
      ocr[n] = spi_transfer_byte(0xFF);
    }
    dbg_printf("    R7: %02X %02X %02X %02X\r\n", ocr[0], ocr[1], ocr[2], ocr[3]);
    
    // Check voltage range
    if (ocr[2] == 0x01 && ocr[3] == 0xAA) {
      dbg_print("    Voltage range accepted (2.7-3.6V)\r\n");
      
      // ACMD41: SD_SEND_OP_COND
      dbg_print("[5] Sending ACMD41 (SD_SEND_OP_COND, HCS=1)...\r\n");
      dbg_print("    This may take up to 1 second...\r\n");
      
      for (tmr = 1000; tmr; tmr--) {
        uint8_t r55 = sd_send_cmd(SD_CMD55, 0);
        uint8_t r41 = sd_send_cmd(SD_CMD41, 1UL << 30);
        
        if (tmr % 100 == 0) {
          dbg_printf("    [%4d ms] CMD55: 0x%02X, CMD41: 0x%02X\r\n", 1000-tmr, r55, r41);
        }
        
        if (r55 <= 1 && r41 == 0) {
          dbg_printf("    Card ready after %d ms\r\n", 1000-tmr);
          break;
        }
        osDelay(1);
      }
      
      if (tmr == 0) {
        dbg_print("    [FAIL] Timeout waiting for card ready\r\n");
        dbg_print("    → Card initialization failed. Try:\r\n");
        dbg_print("       - Different SD card\r\n");
        dbg_print("       - Verify SPI speed (should be <400 kHz)\r\n");
        sd_card_type = SD_TYPE_UNKNOWN;
      } else {
        // CMD58: READ_OCR
        dbg_print("[6] Sending CMD58 (READ_OCR)...\r\n");
        if (sd_send_cmd(SD_CMD58, 0) == 0) {
          for (n = 0; n < 4; n++) {
            ocr[n] = spi_transfer_byte(0xFF);
          }
          dbg_printf("    OCR: %02X %02X %02X %02X\r\n", ocr[0], ocr[1], ocr[2], ocr[3]);
          
          if (ocr[0] & 0x40) {
            sd_card_type = SD_TYPE_SDHC;
            dbg_print("    Card type: SDHC (High Capacity)\r\n");
          } else {
            sd_card_type = SD_TYPE_SDV2;
            dbg_print("    Card type: SDv2 (Standard Capacity)\r\n");
          }
        }
      }
    } else {
      dbg_printf("    [FAIL] Voltage range mismatch: 0x%02X%02X\r\n", ocr[2], ocr[3]);
      sd_card_type = SD_TYPE_UNKNOWN;
    }
  } else if (r1 & 0x04) {
    dbg_print("(SDv1 card - illegal command response)\r\n");
    
    // Try SDv1 initialization
    dbg_print("[5] Trying SDv1 initialization (CMD55+CMD41)...\r\n");
    
    if (sd_send_cmd(SD_CMD55, 0) <= 1 && sd_send_cmd(SD_CMD41, 0) <= 1) {
      dbg_print("    SDv1 card detected\r\n");
      sd_card_type = SD_TYPE_SDV1;
      cmd = SD_CMD41;
      
      dbg_print("    Waiting for card ready...\r\n");
      for (tmr = 1000; tmr && cmd; tmr--) {
        if (sd_send_cmd(SD_CMD55, 0) <= 1 && sd_send_cmd(cmd, 0) == 0) {
          dbg_printf("    Card ready after %d ms\r\n", 1000-tmr);
          break;
        }
        osDelay(1);
      }
      
      // Set block length to 512
      if (tmr) {
        dbg_print("[6] Setting block length to 512 bytes...\r\n");
        if (sd_send_cmd(SD_CMD16, 512) == 0) {
          dbg_print("    Block length set\r\n");
        } else {
          dbg_print("    [FAIL] Could not set block length\r\n");
          sd_card_type = SD_TYPE_UNKNOWN;
        }
      } else {
        dbg_print("    [FAIL] Timeout waiting for SDv1 card\r\n");
        sd_card_type = SD_TYPE_UNKNOWN;
      }
    } else {
      dbg_print("    [FAIL] Not a valid SD card\r\n");
      sd_card_type = SD_TYPE_UNKNOWN;
    }
  } else {
    dbg_printf("(Unknown response)\r\n");
    sd_card_type = SD_TYPE_UNKNOWN;
  }
  
  spibus_end(SPIBUS_DEV_SD);
  
  // Final status
  dbg_print("\r\n=== Initialization Result ===\r\n");
  if (sd_card_type != SD_TYPE_UNKNOWN) {
    sd_status = 0;
    dbg_print("Status: SUCCESS\r\n");
    dbg_printf("Card Type: %s\r\n", 
      sd_card_type == SD_TYPE_SDHC ? "SDHC" :
      sd_card_type == SD_TYPE_SDV2 ? "SDv2" : "SDv1");
    
    // Switch to fast speed
    dbg_print("[7] Switching to fast SPI speed (42 MHz)...\r\n");
    spibus_set_sd_speed_fast();
    dbg_print("    Ready for data transfers\r\n");
  } else {
    sd_status = STA_NOINIT;
    dbg_print("Status: FAILED\r\n");
    dbg_print("Card Type: Unknown/Incompatible\r\n");
  }
  
  dbg_print("================================\r\n\r\n");
  
  return sd_status;
}
```

**Usage**: Replace `sd_spi_initialize()` call with `sd_spi_initialize_debug()` in user_diskio.c temporarily.

### Level 4: Logic Analyzer Capture

**Purpose**: Capture actual SPI signals to verify timing and protocol.

**Setup**:
- Connect logic analyzer to PA4 (CS), PA5 (SCK), PA6 (MISO), PA7 (MOSI)
- Sample rate: 10 MHz minimum
- Protocol: SPI Mode 0 (CPOL=0, CPHA=0)

**What to Look For**:

1. **80 Dummy Clocks** (power-up):
   - CS should be HIGH
   - 10 bytes of 0xFF on MOSI
   - Clock frequency: ~656 kHz

2. **CMD0 Sequence**:
   - CS goes LOW
   - MOSI: `40 00 00 00 00 95` (CMD0 with CRC)
   - MISO: Should respond with `01` (idle state)

3. **CMD8 Sequence**:
   - MOSI: `48 00 00 01 AA 87` (CMD8 with CRC)
   - MISO: Should respond with `01` followed by 4 bytes

4. **ACMD41 Sequence**:
   - CMD55: MOSI `77 00 00 00 00 xx`
   - Then CMD41: MOSI `69 40 00 00 00 xx`
   - Repeat until MISO returns `00` (card ready)

### Level 5: Alternative Card Test

**Purpose**: Rule out card-specific issues.

**Steps**:

1. **Try Different Cards**:
   ```
   ☐ Test with known-good SD card from another device
   ☐ Try different brands (SanDisk, Kingston, Samsung)
   ☐ Test SDHC (4-32GB) vs SDv2 (<=2GB)
   ☐ Avoid MMC cards (not supported)
   ```

2. **Card Formatting**:
   ```bash
   # Windows: Use SD Card Formatter tool (official)
   # Linux:
   sudo mkfs.vfat -F 32 -n MIDICORE /dev/sdX1
   
   # Verify:
   # - File system: FAT32
   # - Cluster size: 32KB (default)
   # - No bad sectors
   ```

3. **Test File Creation**:
   ```
   ☐ Create test file: 0:/config.ngc
   ☐ File size: <1KB
   ☐ Check file is readable on PC
   ☐ Safely eject before removing
   ```

---

## Common Error Patterns

### Error: CMD0 returns 0xFF (no response)

**Cause**: Card not detected or SPI not working.

**Solutions**:
1. Check CS pin (PA4) - should toggle LOW during command
2. Verify MOSI pin (PA7) - should show command data
3. Check 3.3V power supply
4. Increase delay after power-on (try 100ms instead of 10ms)

### Error: CMD8 illegal command (0x05)

**Interpretation**: SDv1 card (doesn't support CMD8).

**Solution**: Code should handle this by trying SDv1 initialization.

### Error: ACMD41 timeout (stays 0x01)

**Cause**: Card not completing initialization.

**Solutions**:
1. Verify HCS bit set correctly (bit 30 = 1)
2. Check SPI speed (<400 kHz during init)
3. Increase timeout from 1000ms to 2000ms
4. Try sending CMD1 instead (MMC card?)

### Error: Mount succeeded but file operations fail

**Cause**: Wrong SPI speed or timing issues.

**Solutions**:
1. Verify speed switched to 42 MHz after init
2. Check CS timing (must stay LOW during entire block transfer)
3. Verify DMA not interfering with SPI
4. Check FreeRTOS mutex timeout (should be sufficient)

---

## Configuration Checklist

### SPI Bus Configuration

**Hal/spi_bus.c**:
```c
☐ SPIBUS_DEV_SD = 0
☐ Initial prescaler = 256 (656 kHz)
☐ Fast prescaler = 4 (42 MHz)
☐ SPI Mode 0 (CPOL=0, CPHA=0)
☐ CS active LOW
```

### GPIO Configuration

**Core/Src/main.c**:
```c
☐ PA4 configured as GPIO output (SD_CS)
☐ PA5 configured as SPI1_SCK (alternate function)
☐ PA6 configured as SPI1_MISO (alternate function)
☐ PA7 configured as SPI1_MOSI (alternate function)
☐ PB2 configured as GPIO input (SD_CD, optional)
```

### FatFs Configuration

**FATFS/Target/ffconf.h**:
```c
☐ FF_FS_READONLY = 0 (read/write enabled)
☐ FF_USE_MKFS = 1 (format support)
☐ FF_USE_LFN = 1 or 2 (long filename support)
☐ FF_MAX_SS = 512 (sector size)
```

---

## Success Criteria

When everything is working, you should see:

```
=== SD Card Initialization Debug ===
[1] Initializing SPI bus...
    SPI bus initialized (656 kHz for init)
[2] Sending 80 dummy clocks (CS=HIGH)...
    80 clocks sent
[3] Sending CMD0 (GO_IDLE_STATE)...
    Response: 0x01 (OK - card in idle state)
[4] Sending CMD8 (SEND_IF_COND, 0x1AA)...
    Response: 0x01 (OK - SDv2 card detected)
    R7: 00 00 01 AA
    Voltage range accepted (2.7-3.6V)
[5] Sending ACMD41 (SD_SEND_OP_COND, HCS=1)...
    Card ready after 342 ms
[6] Sending CMD58 (READ_OCR)...
    OCR: C0 FF 80 00
    Card type: SDHC (High Capacity)

=== Initialization Result ===
Status: SUCCESS
Card Type: SDHC
[7] Switching to fast SPI speed (42 MHz)...
    Ready for data transfers
================================

TEST 1: SD Card Mount
--------------------------------------
[PASS] SD card mounted successfully
```

---

## Support Information

**Hardware**: STM32F407VGT6 + MIOS32-compatible SD card slot  
**SPI Interface**: SPI1 (APB2 @ 168 MHz)  
**SD Card Standards**: SDv1, SDv2, SDHC (up to 32GB)  
**File System**: FAT32  

**For Further Help**:
- Check SD card specification: SD Physical Layer Simplified Specification
- FatFs documentation: http://elm-chan.org/fsw/ff/
- MIOS32 SD card implementation: apps/controllers/midibox_ng_v1/
