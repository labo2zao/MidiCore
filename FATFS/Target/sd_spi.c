/**
 * @file sd_spi.c
 * @brief SD Card SPI driver implementation for FatFs
 * 
 * Low-level SD card driver using SPI interface via spi_bus abstraction.
 * Implements SD card initialization, block read/write operations.
 * 
 * Based on ChaN's FatFs sample SD card driver.
 * 
 * IMPORTANT: SD card initialization requires slow SPI speed (~400 kHz).
 * The spibus layer starts at 656 kHz (prescaler 256) for initialization,
 * then switches to 42 MHz (prescaler 4) after successful init.
 */

#include "sd_spi.h"
#include "Hal/spi_bus.h"
#include "Config/sd_pins.h"
#include "cmsis_os.h"
#include <string.h>

// Timeout values
#define SD_TIMEOUT_MS       500
#define SD_INIT_TIMEOUT_MS  1000

// SD Card status
static volatile DSTATUS sd_status = STA_NOINIT;
static uint8_t sd_card_type = SD_TYPE_UNKNOWN;

// Forward declarations
static uint8_t sd_send_cmd(uint8_t cmd, uint32_t arg);
static uint8_t sd_wait_ready(uint32_t timeout_ms);
static int sd_read_datablock(uint8_t *buff, uint32_t btr);
static int sd_write_datablock(const uint8_t *buff, uint8_t token);

/**
 * @brief Transfer a single byte over SPI
 * @param data Data byte to send
 * @retval Received byte
 */
static uint8_t spi_transfer_byte(uint8_t data)
{
  uint8_t rx;
  spibus_txrx(SPIBUS_DEV_SD, &data, &rx, 1, 100);
  return rx;
}

/**
 * @brief Wait for SD card to be ready
 * @param timeout_ms Timeout in milliseconds
 * @retval 0xFF if ready, 0 if timeout
 */
static uint8_t sd_wait_ready(uint32_t timeout_ms)
{
  uint8_t res;
  uint32_t start_tick = HAL_GetTick();
  
  do {
    res = spi_transfer_byte(0xFF);
    if (res == 0xFF) return res;
    // Use short busy wait instead of osDelay for better responsiveness
    for (volatile uint32_t i = 0; i < 1000; i++);
  } while ((HAL_GetTick() - start_tick) < timeout_ms);
  
  return 0;  // Timeout
}

/**
 * @brief Send a command to SD card
 * @param cmd Command index
 * @param arg Command argument
 * @retval Response R1
 */
static uint8_t sd_send_cmd(uint8_t cmd, uint32_t arg)
{
  uint8_t res, n;
  
  // ACMD<n> is the command sequence of CMD55-CMD<n>
  if (cmd & 0x80) {
    cmd &= 0x7F;
    res = sd_send_cmd(SD_CMD55, 0);
    if (res > 1) return res;
  }
  
  // Wait for card ready (except CMD0)
  if (cmd != SD_CMD0) {
    if (sd_wait_ready(SD_TIMEOUT_MS) != 0xFF) {
      return 0xFF;
    }
  }
  
  // Send command packet
  spi_transfer_byte( 0x40 | cmd);         // Start + command
  spi_transfer_byte( (uint8_t)(arg >> 24)); // Argument[31..24]
  spi_transfer_byte( (uint8_t)(arg >> 16)); // Argument[23..16]
  spi_transfer_byte( (uint8_t)(arg >> 8));  // Argument[15..8]
  spi_transfer_byte( (uint8_t)arg);         // Argument[7..0]
  
  // CRC (only needed for CMD0 and CMD8)
  n = 0x01;  // Dummy CRC + Stop
  if (cmd == SD_CMD0) n = 0x95;  // Valid CRC for CMD0(0)
  if (cmd == SD_CMD8) n = 0x87;  // Valid CRC for CMD8(0x1AA)
  spi_transfer_byte( n);
  
  // Receive command response
  if (cmd == SD_CMD12) {
    spi_transfer_byte( 0xFF);  // Skip stuff byte on stop read
  }
  
  // Wait for valid response (timeout: 10 attempts)
  n = 10;
  do {
    res = spi_transfer_byte( 0xFF);
  } while ((res & 0x80) && --n);
  
  // For most commands, send a dummy byte after response for timing (MIOS32 pattern)
  // Skip dummy byte for:
  //  - CMD0 (initial reset)
  //  - CMD12 (stop during multi-block)
  //  - CMD13 (SEND_STATUS - need immediate status read)
  // NOTE: CMD17/CMD18 get one dummy byte after response for NCR timing
  if (cmd != SD_CMD0 && cmd != SD_CMD12 && cmd != SD_CMD13) {
    spi_transfer_byte(0xFF);
  }
  
  return res;
}

/**
 * @brief Read a data block from SD card
 * @param buff Data buffer
 * @param btr Bytes to read (must be 512 for SD cards)
 * @retval 1 if success, 0 if failed
 */
static int sd_read_datablock(uint8_t *buff, uint32_t btr)
{
  uint8_t token;
  uint32_t i;
  uint8_t temp_byte;
  
  // Safety check - validate buffer pointer and size
  if (!buff || btr != 512) return 0;
  
  // Additional safety: check if buffer is in valid RAM range
  // STM32F407: RAM is 0x20000000-0x2001FFFF (128KB)
  //            CCMRAM is 0x10000000-0x1000FFFF (64KB)
  uint32_t buff_addr = (uint32_t)buff;
  if (!((buff_addr >= 0x20000000 && buff_addr < 0x20020000) ||
        (buff_addr >= 0x10000000 && buff_addr < 0x10010000))) {
    return 0;  // Buffer not in valid RAM
  }
  
  // Wait for data packet (start token 0xFE)
  // MIOS32 pattern: poll continuously for up to 65536 iterations
  // SD spec allows up to 100ms for card to respond with data token
  for (i = 0; i < 65536; ++i) {
    token = spi_transfer_byte(0xFF);
    if (token != 0xFF)
      break;
  }
  
  if (token != 0xFE) return 0;  // Invalid token or timeout
  
  // Read data block byte-by-byte to avoid potential DMA/buffer issues
  // CRITICAL: Use temp variable first to ensure safe write
  for (i = 0; i < btr; i++) {
    temp_byte = spi_transfer_byte(0xFF);
    buff[i] = temp_byte;  // Write through temp to avoid optimizer issues
  }
  
  // Read (and ignore) CRC
  spi_transfer_byte(0xFF);
  spi_transfer_byte(0xFF);
  
  // Required for clocking (MIOS32 pattern - see spec)
  spi_transfer_byte(0xFF);
  
  return 1;
}

/**
 * @brief Write a data block to SD card
 * @param buff Data buffer (512 bytes)
 * @param token Start token
 * @retval 1 if success, 0 if failed
 */
static int sd_write_datablock(const BYTE *buff, BYTE token)
{
  uint8_t resp;
  
  // Wait for card to be ready before sending token
  if (sd_wait_ready(SD_TIMEOUT_MS) != 0xFF) return 0;
  
  // Send token
  spi_transfer_byte(token);
  
  if (token != 0xFD) {  // Not stop token
    // Send data block (512 bytes)
    spibus_tx(SPIBUS_DEV_SD, buff, 512, 500);
    
    // Send dummy CRC (2 bytes)
    spi_transfer_byte(0xFF);
    spi_transfer_byte(0xFF);
    
    // Receive data response (wait for non-0xFF byte)
    resp = spi_transfer_byte(0xFF);
    if ((resp & 0x1F) != 0x05) return 0;  // Data rejected
    
    // CRITICAL: Wait for card to finish writing (becomes ready again)
    // Card will be busy (0x00) during flash write, then return 0xFF when done
    // This matches MIOS32 pattern and is required for reliable SD card writes
    if (sd_wait_ready(SD_TIMEOUT_MS) != 0xFF) return 0;
  }
  
  return 1;
}

/**
 * @brief Initialize SD card
 * @retval Disk status
 */
DSTATUS sd_spi_initialize(void)
{
  uint8_t cmd, n, ocr[4], ty;
  uint16_t tmr;
  
  // Initialize SPI bus for SD card (if not already done - safe to call multiple times)
  spibus_init();
  
  // SD card initialization requires 74+ clock cycles with CS HIGH before first command
  // This allows the card to complete its power-up sequence
  
  // Acquire SPI bus, but keep CS high
  spibus_begin(SPIBUS_DEV_SD);
  spibus_end(SPIBUS_DEV_SD);  // Release immediately - CS is now HIGH
  
  // Wait for card power-up (busy wait to avoid FreeRTOS dependency)
  HAL_Delay(10);
  
  // Send 80 dummy clocks (10 bytes of 0xFF) with CS HIGH
  // Need to acquire bus but NOT assert CS
  spibus_begin(SPIBUS_DEV_SD);  // Acquire mutex and set prescaler
  // Manually set CS high (override spibus_begin which sets it low)
  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);
  
  for (n = 0; n < 10; n++) {
    uint8_t dummy = 0xFF;
    spibus_tx(SPIBUS_DEV_SD, &dummy, 1, 100);
  }
  
  spibus_end(SPIBUS_DEV_SD);  // Release mutex
  // Now select card and start initialization  
  spibus_begin(SPIBUS_DEV_SD);
  
  // Enter idle state (CMD0) - must return 0x01
  if (sd_send_cmd(SD_CMD0, 0) == 1) {
    // Check SD card version with CMD8
    if (sd_send_cmd(SD_CMD8, 0x1AA) == 1) {
      // SDv2 or later - read R7 response (4 bytes)
      for (n = 0; n < 4; n++) {
        ocr[n] = spi_transfer_byte(0xFF);
      }
      
      // Check voltage range and check pattern
      // R7 response format: [0]=reserved, [1]=voltage, [2]=pattern, [3]=reserved
      // But actual bytes received: [0][1][2][3] where pattern=0xAA, voltage accepted=0x01
      if (ocr[1] == 0x01 && ocr[2] == 0xAA) {
        // Voltage compatible - initialize with ACMD41
        for (tmr = 1000; tmr; tmr--) {
          // ACMD41 with HCS bit set for SDHC support
          if (sd_send_cmd(SD_CMD55, 0) <= 1 && 
              sd_send_cmd(SD_CMD41, 1UL << 30) == 0) {
            break;  // Card exited idle state
          }
          HAL_Delay(1);  // Use HAL_Delay instead of osDelay for initialization
        }
        
        // Check CCS bit in OCR to determine SDHC/SDXC
        if (tmr && sd_send_cmd(SD_CMD58, 0) == 0) {
          for (n = 0; n < 4; n++) {
            ocr[n] = spi_transfer_byte(0xFF);
          }
          // CCS bit (bit 30) indicates High Capacity card
          ty = (ocr[0] & 0x40) ? SD_TYPE_SDHC : SD_TYPE_SDV2;
        }
      }
    } else {
      // SDv1 or MMC - CMD8 not supported
      // Try ACMD41 to determine if it's SD or MMC
      if (sd_send_cmd(SD_CMD55, 0) <= 1 && 
          sd_send_cmd(SD_CMD41, 0) <= 1) {
        // SD card version 1
        ty = SD_TYPE_SDV1;
        cmd = SD_CMD41;
      } else {
        // MMC card
        ty = SD_TYPE_MMC;
        cmd = SD_CMD1;
      }
      
      // Wait for card to exit idle state
      for (tmr = 1000; tmr && cmd; tmr--) {
        if (ty == SD_TYPE_SDV1) {
          // SDv1: need CMD55 before CMD41
          if (sd_send_cmd(SD_CMD55, 0) <= 1 && 
              sd_send_cmd(cmd, 0) == 0) break;
        } else {
          // MMC: just CMD1
          if (sd_send_cmd(cmd, 0) == 0) break;
        }
        HAL_Delay(1);  // Use HAL_Delay instead of osDelay for initialization
      }
      
      // Set block length to 512 bytes for SDv1/MMC (SDHC already fixed at 512)
      // IMPORTANT: SDHC cards don't need/support CMD16 - they're always 512-byte blocks
      if (!tmr) {
        ty = SD_TYPE_UNKNOWN;
      } else if (ty != SD_TYPE_SDHC) {
        // Only send CMD16 for SDv1/MMC cards
        if (sd_send_cmd(SD_CMD16, 512) != 0) {
          ty = SD_TYPE_UNKNOWN;
        }
      }
    }
  }
  
  // Release SPI bus
  spibus_end(SPIBUS_DEV_SD);
  
  // Store card type
  sd_card_type = ty;
  
  if (sd_card_type != SD_TYPE_UNKNOWN) {
    // Switch to fast SPI speed for data operations (42 MHz)
    // IMPORTANT: Must acquire bus BEFORE changing speed to ensure safe reconfiguration
    spibus_begin(SPIBUS_DEV_SD);
    spibus_set_sd_speed_fast();
    
    // CRITICAL: Send dummy clocks after speed switch
    // Card needs time to adjust to new clock rate (80 clock cycles per SD spec)
    for (n = 0; n < 10; n++) {
      spi_transfer_byte(0xFF);
    }
    
    // Release bus briefly to deselect card (CS HIGH)
    spibus_end(SPIBUS_DEV_SD);
    
    // Small delay to let card stabilize after speed switch
    HAL_Delay(10);  // Use HAL_Delay to avoid FreeRTOS dependency
    
    // Reselect card and verify it's still responding
    spibus_begin(SPIBUS_DEV_SD);
    
    // Send CMD13 (SEND_STATUS) to check if card is ready
    if (sd_send_cmd(SD_CMD13, 0) == 0) {
      sd_status = 0;  // Clear STA_NOINIT flag - card is ready
    } else {
      sd_status = STA_NOINIT;  // Card not responding after speed switch
      sd_card_type = SD_TYPE_UNKNOWN;
    }
    spibus_end(SPIBUS_DEV_SD);
  } else {
    sd_status = STA_NOINIT;
  }
  
  return sd_status;
}

/**
 * @brief Get SD card status
 * @retval Disk status
 */
DSTATUS sd_spi_status(void)
{
  return sd_status;
}

/**
 * @brief Read sector(s) from SD card
 * @param buff Data buffer
 * @param sector Start sector number (LBA)
 * @param count Number of sectors (1..128)
 * @retval RES_OK if success
 */
DRESULT sd_spi_read(BYTE *buff, DWORD sector, UINT count)
{
  uint8_t cmd_res;
  
  // Safety checks
  if (!buff) return RES_PARERR;  // NULL pointer check
  if (sd_status & STA_NOINIT) return RES_NOTRDY;
  if (!count) return RES_PARERR;
  
  // Convert LBA to byte address for non-SDHC cards
  if (sd_card_type != SD_TYPE_SDHC) {
    sector *= 512;
  }
  
  spibus_begin(SPIBUS_DEV_SD);
  
  if (count == 1) {
    // Single block read
    cmd_res = sd_send_cmd(SD_CMD17, sector);
    if (cmd_res == 0) {
      // Card expects us to immediately poll for data token (0xFE)
      // CS must stay LOW continuously from CMD17 through data read
      if (sd_read_datablock(buff, 512)) {
        count = 0;
      }
    }
  } else {
    // Multiple block read
    // CRITICAL: Limit to reasonable size to prevent buffer overrun
    if (count > 128) {
      spibus_end(SPIBUS_DEV_SD);
      return RES_PARERR;
    }
    
    cmd_res = sd_send_cmd(SD_CMD18, sector);
    if (cmd_res == 0) {
      uint32_t blocks_read = 0;
      do {
        if (!sd_read_datablock(buff, 512)) break;
        buff += 512;
        blocks_read++;
      } while (--count);
      
      // Stop transmission
      sd_send_cmd(SD_CMD12, 0);
      
      // Required for clocking after stop (MIOS32 pattern)
      spi_transfer_byte(0xFF);
      
      // Update count to reflect actual blocks read
      count = blocks_read < count ? count - blocks_read : 0;
    }
  }
  
  spibus_end(SPIBUS_DEV_SD);
  
  return count ? RES_ERROR : RES_OK;
}

/**
 * @brief Write sector(s) to SD card
 * @param buff Data buffer
 * @param sector Start sector number (LBA)
 * @param count Number of sectors (1..128)
 * @retval RES_OK if success
 */
DRESULT sd_spi_write(const BYTE *buff, DWORD sector, UINT count)
{
  uint8_t cmd_res;
  
  if (sd_status & STA_NOINIT) return RES_NOTRDY;
  if (!count) return RES_PARERR;
  if (sd_status & STA_PROTECT) return RES_WRPRT;
  
  // Convert LBA to byte address for non-SDHC cards
  if (sd_card_type != SD_TYPE_SDHC) {
    sector *= 512;
  }
  
  spibus_begin(SPIBUS_DEV_SD);
  
  if (count == 1) {
    // Single block write - MIOS32 pattern
    // CMD24: WRITE_BLOCK
    cmd_res = sd_send_cmd(SD_CMD24, sector);
    if (cmd_res == 0) {
      // CMD24 accepted, now send data block
      if (sd_write_datablock(buff, 0xFE)) {
        count = 0;  // Success
      }
    }
  } else {
    // Multiple block write - MIOS32 pattern
    // First send ACMD23 to set number of blocks to pre-erase
    if (sd_card_type != SD_TYPE_UNKNOWN) {
      // Send CMD55 to indicate next command is application-specific
      if (sd_send_cmd(SD_CMD55, 0) <= 1) {
        // Send ACMD23 (SET_WR_BLK_ERASE_COUNT) - pre-erase blocks
        if (sd_send_cmd(SD_CMD23, count) == 0) {
          // Now send CMD25 (WRITE_MULTIPLE_BLOCK)
          if (sd_send_cmd(SD_CMD25, sector) == 0) {
            do {
              if (!sd_write_datablock(buff, 0xFC)) break;  // Multi-block token
              buff += 512;
            } while (--count);
            
            // Send stop transmission token
            if (!sd_write_datablock(0, 0xFD)) {
              count = 1;  // Error
            }
          }
        }
      }
    }
  }
  
  spibus_end(SPIBUS_DEV_SD);
  
  return count ? RES_ERROR : RES_OK;
}

/**
 * @brief I/O control
 * @param cmd Control command
 * @param buff Control data buffer
 * @retval RES_OK if success
 */
DRESULT sd_spi_ioctl(BYTE cmd, void *buff)
{
  DRESULT res = RES_ERROR;
  uint8_t n, csd[16];
  DWORD cs;
  
  if (sd_status & STA_NOINIT) return RES_NOTRDY;
  
  spibus_begin(SPIBUS_DEV_SD);
  
  switch (cmd) {
    case CTRL_SYNC:
      // Wait for write completion
      if (sd_wait_ready(SD_TIMEOUT_MS) == 0xFF) {
        res = RES_OK;
      }
      break;
      
    case GET_SECTOR_COUNT:
      // Get number of sectors
      if (sd_send_cmd(SD_CMD9, 0) == 0 && sd_read_datablock(csd, 16)) {
        if ((csd[0] >> 6) == 1) {
          // CSD v2.0 (SDHC)
          cs = ((DWORD)(csd[9] & 0x3F) << 16) | ((DWORD)csd[10] << 8) | (csd[11] + 1);
          *(DWORD*)buff = cs << 10;
        } else {
          // CSD v1.0 (SD)
          n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
          cs = ((DWORD)(csd[8] >> 6) | ((DWORD)csd[7] << 2) | ((DWORD)(csd[6] & 3) << 10)) + 1;
          *(DWORD*)buff = cs << (n - 9);
        }
        res = RES_OK;
      }
      break;
      
    case GET_SECTOR_SIZE:
      // Return sector size (always 512 for SD cards)
      *(WORD*)buff = 512;
      res = RES_OK;
      break;
      
    case GET_BLOCK_SIZE:
      // Get erase block size (in unit of sector)
      *(DWORD*)buff = 128;
      res = RES_OK;
      break;
      
    default:
      res = RES_PARERR;
  }
  
  spibus_end(SPIBUS_DEV_SD);
  
  return res;
}
