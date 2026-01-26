/**
 * @file sd_spi.h
 * @brief SD Card SPI driver for FatFs
 * 
 * Low-level SD card driver using SPI interface.
 * Implements SD card initialization, block read/write operations.
 */

#ifndef SD_SPI_H
#define SD_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "ff.h"
#include "diskio.h"

// SD Card Commands
#define SD_CMD0    0   // GO_IDLE_STATE
#define SD_CMD1    1   // SEND_OP_COND (MMC)
#define SD_CMD8    8   // SEND_IF_COND
#define SD_CMD9    9   // SEND_CSD
#define SD_CMD10   10  // SEND_CID
#define SD_CMD12   12  // STOP_TRANSMISSION
#define SD_CMD13   13  // SEND_STATUS
#define SD_CMD16   16  // SET_BLOCKLEN
#define SD_CMD17   17  // READ_SINGLE_BLOCK
#define SD_CMD18   18  // READ_MULTIPLE_BLOCK
#define SD_CMD24   24  // WRITE_SINGLE_BLOCK
#define SD_CMD25   25  // WRITE_MULTIPLE_BLOCK
#define SD_CMD41   41  // SEND_OP_COND (ACMD)
#define SD_CMD55   55  // APP_CMD
#define SD_CMD58   58  // READ_OCR

// SD Card Types
#define SD_TYPE_UNKNOWN  0
#define SD_TYPE_MMC      1  // MMC
#define SD_TYPE_SDV1     2  // SD Ver 1.x  
#define SD_TYPE_SDV2     3  // SD Ver 2.0+ (SDSC)
#define SD_TYPE_SDHC     4  // SD High Capacity
#define SD_TYPE_SD       SD_TYPE_SDV1  // Alias for SDv1

// Function prototypes
DSTATUS sd_spi_initialize(void);
DSTATUS sd_spi_status(void);
DRESULT sd_spi_read(BYTE *buff, DWORD sector, UINT count);
DRESULT sd_spi_write(const BYTE *buff, DWORD sector, UINT count);
DRESULT sd_spi_ioctl(BYTE cmd, void *buff);

#ifdef __cplusplus
}
#endif

#endif /* SD_SPI_H */
