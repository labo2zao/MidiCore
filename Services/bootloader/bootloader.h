/**
 * @file bootloader.h
 * @brief USB MIDI Bootloader for firmware updates
 * 
 * MIOS32-compatible bootloader that allows firmware updates via USB MIDI
 * using SysEx protocol. Supports fast USB transfer (50-100x faster than DIN MIDI).
 * 
 * Features:
 * - Firmware update via USB MIDI SysEx protocol
 * - CRC verification of received blocks
 * - Safe flash operations with verification
 * - Jump to application with vector table relocation
 * - MIOS32-compatible protocol
 * 
 * Memory Layout:
 * - Bootloader: 0x08000000 - 0x08007FFF (32KB)
 * - Application: 0x08008000 - 0x080FFFFF (992KB)
 * 
 * @note This bootloader is designed for STM32F407VG with 1MB flash
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Bootloader version */
#define BOOTLOADER_VERSION_MAJOR 1
#define BOOTLOADER_VERSION_MINOR 0
#define BOOTLOADER_VERSION_PATCH 0

/** @brief Memory addresses */
#define BOOTLOADER_START_ADDRESS    0x08000000
#define BOOTLOADER_SIZE             0x00008000  // 32KB
#define APPLICATION_START_ADDRESS   0x08008000
#define APPLICATION_MAX_SIZE        0x000F8000  // 992KB
#define FLASH_END_ADDRESS           0x08100000

/** @brief Magic value for bootloader entry request */
#define BOOTLOADER_MAGIC_KEY        0xB007C0DE

/** @brief Bootloader entry reasons */
typedef enum {
  BOOTLOADER_ENTRY_NONE = 0,
  BOOTLOADER_ENTRY_REQUEST = 1,      /**< Requested via magic key */
  BOOTLOADER_ENTRY_NO_APP = 2,       /**< No valid application found */
  BOOTLOADER_ENTRY_BUTTON = 3,       /**< Button pressed during reset */
  BOOTLOADER_ENTRY_SYSEX = 4         /**< SysEx command from application */
} bootloader_entry_reason_t;

/**
 * @brief Initialize bootloader
 * @return true if bootloader mode should be entered, false to jump to app
 */
bool bootloader_init(void);

/**
 * @brief Check if a valid application exists in flash
 * @return true if application appears valid, false otherwise
 */
bool bootloader_check_application(void);

/**
 * @brief Jump to application code
 * @note This function does not return if successful
 */
void bootloader_jump_to_application(void);

/**
 * @brief Request entry into bootloader mode
 * @note Call this from application, then reset the MCU
 */
void bootloader_request_entry(void);

/**
 * @brief Get the reason for bootloader entry
 * @return Entry reason enum
 */
bootloader_entry_reason_t bootloader_get_entry_reason(void);

/**
 * @brief Process received SysEx data for firmware update
 * @param data SysEx data buffer (including F0 and F7)
 * @param len Length of data
 * @return true if processed successfully, false on error
 */
bool bootloader_process_sysex(const uint8_t* data, uint32_t len);

/**
 * @brief Erase application flash area
 * @return true if successful, false on error
 */
bool bootloader_erase_application(void);

/**
 * @brief Write data to application flash
 * @param offset Offset from APPLICATION_START_ADDRESS
 * @param data Data to write
 * @param len Length of data (automatically padded to 4-byte alignment)
 * @return true if successful, false on error
 * 
 * @note Data is automatically padded with 0xFF if not 4-byte aligned
 */
bool bootloader_write_flash(uint32_t offset, const uint8_t* data, uint32_t len);

/**
 * @brief Verify flash content
 * @param offset Offset from APPLICATION_START_ADDRESS
 * @param data Expected data
 * @param len Length of data
 * @return true if content matches, false otherwise
 */
bool bootloader_verify_flash(uint32_t offset, const uint8_t* data, uint32_t len);

/**
 * @brief Calculate CRC32 of data
 * @param data Data buffer
 * @param len Length of data
 * @return CRC32 value
 */
uint32_t bootloader_crc32(const uint8_t* data, uint32_t len);

#ifdef __cplusplus
}
#endif
