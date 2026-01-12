/**
 * @file bootloader.c
 * @brief USB MIDI Bootloader implementation
 */

#include "Services/bootloader/bootloader.h"
#include "Services/bootloader/bootloader_protocol.h"

#if __has_include("main.h")
#include "main.h"
#endif

#include <string.h>

// Magic key location in RAM (preserved across soft reset)
// Place at end of RAM to avoid initialization by startup code
// For STM32F407: RAM ends at 0x20020000, use last 16 bytes for bootloader data
#if !defined(BOOTLOADER_MAGIC_RAM_ADDR)
  #define BOOTLOADER_MAGIC_RAM_END 0x20020000  // End of 128KB RAM
  #define BOOTLOADER_MAGIC_RAM_ADDR  ((volatile uint32_t*)(BOOTLOADER_MAGIC_RAM_END - 16))
#endif

static bootloader_entry_reason_t g_entry_reason = BOOTLOADER_ENTRY_NONE;

/**
 * @brief Check bootloader entry conditions
 */
static bootloader_entry_reason_t check_entry_conditions(void) {
  // Check for magic key in RAM (set by application requesting bootloader)
  if (*BOOTLOADER_MAGIC_RAM_ADDR == BOOTLOADER_MAGIC_KEY) {
    *BOOTLOADER_MAGIC_RAM_ADDR = 0; // Clear magic key
    return BOOTLOADER_ENTRY_REQUEST;
  }
  
  // Check if valid application exists
  if (!bootloader_check_application()) {
    return BOOTLOADER_ENTRY_NO_APP;
  }
  
  // TODO: Check for button press (implement based on hardware config)
  // This would require GPIO initialization before main app
  
  return BOOTLOADER_ENTRY_NONE;
}

bool bootloader_init(void) {
  g_entry_reason = check_entry_conditions();
  return (g_entry_reason != BOOTLOADER_ENTRY_NONE);
}

bool bootloader_check_application(void) {
  // Read the initial stack pointer from application vector table
  uint32_t sp = *((uint32_t*)APPLICATION_START_ADDRESS);
  
  // Check if SP is within valid RAM range for STM32F407
  // RAM: 0x20000000 to 0x20020000 (128KB) + CCM RAM
  if (sp < 0x20000000 || sp > 0x20020000) {
    return false;
  }
  
  // Read the reset vector (PC) from application vector table
  uint32_t pc = *((uint32_t*)(APPLICATION_START_ADDRESS + 4));
  
  // Check if PC points to application flash area (must have Thumb bit set)
  // Valid range: 0x08008001 to 0x080FFFFF (odd addresses for Thumb)
  if ((pc < APPLICATION_START_ADDRESS) || (pc >= FLASH_END_ADDRESS)) {
    return false;
  }
  
  // Check Thumb bit (LSB must be 1 for Cortex-M)
  if ((pc & 0x1) == 0) {
    return false;
  }
  
  return true;
}

void bootloader_jump_to_application(void) {
  // Check if application is valid
  if (!bootloader_check_application()) {
    return; // Stay in bootloader
  }
  
  // Get the application stack pointer and reset vector
  uint32_t app_stack = *((uint32_t*)APPLICATION_START_ADDRESS);
  uint32_t app_reset = *((uint32_t*)(APPLICATION_START_ADDRESS + 4));
  
  // Disable all interrupts
  __disable_irq();
  
  // Relocate vector table to application
  SCB->VTOR = APPLICATION_START_ADDRESS;
  
  // Set main stack pointer
  __set_MSP(app_stack);
  
  // Jump to application reset handler
  void (*app_reset_handler)(void) = (void(*)(void))(app_reset);
  app_reset_handler();
  
  // Should never reach here
  while(1);
}

void bootloader_request_entry(void) {
  // Set magic key in RAM
  *BOOTLOADER_MAGIC_RAM_ADDR = BOOTLOADER_MAGIC_KEY;
  
  // Request system reset
  NVIC_SystemReset();
}

bootloader_entry_reason_t bootloader_get_entry_reason(void) {
  return g_entry_reason;
}

bool bootloader_erase_application(void) {
#ifdef HAL_FLASH_MODULE_ENABLED
  HAL_StatusTypeDef status;
  
  // Unlock flash
  status = HAL_FLASH_Unlock();
  if (status != HAL_OK) {
    return false;
  }
  
  // Erase application sectors (Sector 4-11 for STM32F407)
  // Sector 4: 0x08010000 (64KB) - but we start at 0x08008000
  // Actually, we need to erase from sector 2
  // Sector 0: 0x08000000 (16KB) - Bootloader
  // Sector 1: 0x08004000 (16KB) - Bootloader
  // Sector 2: 0x08008000 (16KB) - Application start
  // Sector 3: 0x0800C000 (16KB)
  // Sector 4: 0x08010000 (64KB)
  // Sectors 5-11: 128KB each
  
  FLASH_EraseInitTypeDef erase_config;
  uint32_t sector_error = 0;
  
  erase_config.TypeErase = FLASH_TYPEERASE_SECTORS;
  erase_config.VoltageRange = FLASH_VOLTAGE_RANGE_3; // 2.7-3.6V
  erase_config.Sector = FLASH_SECTOR_2; // Start from sector 2
  erase_config.NbSectors = 10; // Sectors 2-11 (application area)
  
  status = HAL_FLASHEx_Erase(&erase_config, &sector_error);
  
  // Lock flash
  HAL_FLASH_Lock();
  
  return (status == HAL_OK);
#else
  return false;
#endif
}

bool bootloader_write_flash(uint32_t offset, const uint8_t* data, uint32_t len) {
#ifdef HAL_FLASH_MODULE_ENABLED
  if (data == NULL || len == 0) {
    return false;
  }
  
  // Check bounds
  if (offset + len > APPLICATION_MAX_SIZE) {
    return false;
  }
  
  // Auto-pad to 4-byte alignment if needed
  uint32_t pad_len = len;
  if (len % 4 != 0) {
    pad_len = (len + 3) & ~3;  // Round up to multiple of 4
  }
  
  HAL_StatusTypeDef status;
  uint32_t address = APPLICATION_START_ADDRESS + offset;
  
  // Unlock flash
  status = HAL_FLASH_Unlock();
  if (status != HAL_OK) {
    return false;
  }
  
  // Write data word by word
  for (uint32_t i = 0; i < pad_len; i += 4) {
    uint32_t word;
    if (i + 4 <= len) {
      // Normal word from data
      word = *((uint32_t*)(data + i));
    } else {
      // Partial word - pad with 0xFF (erased flash value)
      word = 0xFFFFFFFF;
      for (uint32_t j = 0; i + j < len; j++) {
        ((uint8_t*)&word)[j] = data[i + j];
      }
    }
    
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address + i, word);
    if (status != HAL_OK) {
      HAL_FLASH_Lock();
      return false;
    }
  }
  
  // Lock flash
  HAL_FLASH_Lock();
  
  return true;
#else
  return false;
#endif
}

bool bootloader_verify_flash(uint32_t offset, const uint8_t* data, uint32_t len) {
  if (data == NULL || len == 0) {
    return false;
  }
  
  // Check bounds
  if (offset + len > APPLICATION_MAX_SIZE) {
    return false;
  }
  
  uint32_t address = APPLICATION_START_ADDRESS + offset;
  const uint8_t* flash_data = (const uint8_t*)address;
  
  return (memcmp(flash_data, data, len) == 0);
}

uint32_t bootloader_crc32(const uint8_t* data, uint32_t len) {
  // Simple CRC32 implementation (software-based)
  // Can be replaced with hardware CRC if available
  uint32_t crc = 0xFFFFFFFF;
  
  for (uint32_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (uint32_t j = 0; j < 8; j++) {
      if (crc & 1) {
        crc = (crc >> 1) ^ 0xEDB88320;
      } else {
        crc >>= 1;
      }
    }
  }
  
  return ~crc;
}

bool bootloader_process_sysex(const uint8_t* data, uint32_t len) {
  return bootloader_protocol_process(data, len);
}
