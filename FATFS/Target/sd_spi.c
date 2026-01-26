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
  uint32_t start_tick = osKernelGetTickCount();
  
  do {
    res = spi_transfer_byte(0xFF);
    if (res == 0xFF) return res;
    osDelay(1);
  } while ((osKernelGetTickCount() - start_tick) < timeout_ms);
  
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
  
  // For most commands, send a dummy byte after response
  // This ensures proper timing and card readiness (MIOS32 pattern)
  // Skip for CMD12 (stop transmission) and CMD0 (initial reset)
  if (cmd != SD_CMD0 && cmd != SD_CMD12) {
    spi_transfer_byte(0xFF);
  }
  
  return res;
}

/**
 * @brief Read a data block from SD card
 * @param buff Data buffer
 * @param btr Bytes to read (must be multiple of 4)
 * @retval 1 if success, 0 if failed
 */
static int sd_read_datablock(uint8_t *buff, uint32_t btr)
{
  uint8_t token;
  uint32_t timeout = 50000;  // High timeout count for slow cards (up to 100ms per SD spec)
  
  // Wait for data packet (start token 0xFE)
  // SD spec allows up to 100ms for card to respond with data token
  // Don't use osDelay here - just poll continuously for better timing
  do {
    token = spi_transfer_byte(0xFF);
  } while ((token == 0xFF) && --timeout);
  
  if (token != 0xFE) return 0;  // Invalid token or timeout
  
  // Read data block
  spibus_rx(SPIBUS_DEV_SD, buff, btr, 500);
  
  // Discard CRC (2 bytes)
  spi_transfer_byte(0xFF);
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
  
  if (sd_wait_ready(SD_TIMEOUT_MS) != 0xFF) return 0;
  
  // Send token
  spi_transfer_byte( token);
  
  if (token != 0xFD) {  // Not stop token
    // Send data block
    spibus_tx(SPIBUS_DEV_SD, buff, 512, 500);
    
    // Send dummy CRC
    spi_transfer_byte( 0xFF);
    spi_transfer_byte( 0xFF);
    
    // Receive data response
    resp = spi_transfer_byte( 0xFF);
    if ((resp & 0x1F) != 0x05) return 0;  // Data rejected
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
  
  osDelay(10);  // Wait for card power-up
  
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
      if (ocr[2] == 0x01 && ocr[3] == 0xAA) {
        // Voltage compatible - initialize with ACMD41
        for (tmr = 1000; tmr; tmr--) {
          // ACMD41 with HCS bit set for SDHC support
          if (sd_send_cmd(SD_CMD55, 0) <= 1 && 
              sd_send_cmd(SD_CMD41, 1UL << 30) == 0) {
            break;  // Card exited idle state
          }
          osDelay(1);
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
        osDelay(1);
      }
      
      // Set block length to 512 bytes for SDv1/MMC
      if (!tmr || sd_send_cmd(SD_CMD16, 512) != 0) {
        ty = SD_TYPE_UNKNOWN;
      }
    }
  }
  
  // Release SPI bus
  spibus_end(SPIBUS_DEV_SD);
  
  // Store card type
  sd_card_type = ty;
  
  if (sd_card_type != SD_TYPE_UNKNOWN) {
    sd_status = 0;  // Clear STA_NOINIT flag
    // Switch to fast SPI speed for data operations (42 MHz)
    spibus_set_sd_speed_fast();
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
  
  if (sd_status & STA_NOINIT) return RES_NOTRDY;
  if (!count) return RES_PARERR;
  
  // Convert LBA to byte address for non-SDHC cards
  if (sd_card_type != SD_TYPE_SDHC) {
    sector *= 512;
  }
  
  spibus_begin(SPIBUS_DEV_SD);
  
  // Send dummy byte to ensure card is ready (MIOS32 pattern)
  spi_transfer_byte(0xFF);
  
  if (count == 1) {
    // Single block read
    cmd_res = sd_send_cmd(SD_CMD17, sector);
    if (cmd_res == 0) {
      if (sd_read_datablock(buff, 512)) {
        count = 0;
      }
    }
  } else {
    // Multiple block read
    cmd_res = sd_send_cmd(SD_CMD18, sector);
    if (cmd_res == 0) {
      do {
        if (!sd_read_datablock(buff, 512)) break;
        buff += 512;
      } while (--count);
      
      // Stop transmission
      sd_send_cmd(SD_CMD12, 0);
    }
  }
  
  // Deselect with dummy byte (MIOS32 pattern)
  spi_transfer_byte(0xFF);
  
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
  if (sd_status & STA_NOINIT) return RES_NOTRDY;
  if (!count) return RES_PARERR;
  if (sd_status & STA_PROTECT) return RES_WRPRT;
  
  // Convert LBA to byte address for non-SDHC cards
  if (sd_card_type != SD_TYPE_SDHC) {
    sector *= 512;
  }
  
  spibus_begin(SPIBUS_DEV_SD);
  
  if (count == 1) {
    // Single block write
    if (sd_send_cmd(SD_CMD24, sector) == 0) {
      if (sd_write_datablock(buff, 0xFE)) {
        count = 0;
      }
    }
  } else {
    // Multiple block write
    if (sd_card_type != SD_TYPE_UNKNOWN && sd_send_cmd(SD_CMD55 | 0x80, 0) <= 1 &&
        sd_send_cmd(23, count) == 0) {  // ACMD23 - pre-erase
      // Send write multiple command
      if (sd_send_cmd(SD_CMD25, sector) == 0) {
        do {
          if (!sd_write_datablock(buff, 0xFC)) break;
          buff += 512;
        } while (--count);
        
        // Send stop token
        if (!sd_write_datablock(0, 0xFD)) {
          count = 1;
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
