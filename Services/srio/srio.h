#pragma once
/////////////////////////////////////////////////////////////////////////////
//! \defgroup SRIO
//!
//! SRIO Driver for MidiCore - Based on MidiCore SRIO Driver
//!
//! Copyright (C) 2008 Thorsten Klose (tk@midibox.org)
//! Adapted for MidiCore by labodezao
//!
//! This module provides Shift Register Input/Output functionality:
//! - 74HC165 shift registers for Digital Inputs (DIN) - buttons, switches
//! - 74HC595 shift registers for Digital Outputs (DOUT) - LEDs, relays
//!
//! Features:
//! - Full MidiCore hardware compatibility
//! - Bulk SPI transfer for efficiency
//! - Change detection with debouncing
//! - Independent DIN/DOUT operation
//!
//! Hardware connections (MidiCore mbhp_dinx4):
//!   74HC165 (DIN):
//!     Pin 1  (/PL)  → STM32 PD10 (RC2)
//!     Pin 2  (CLK)  → STM32 PB13 (SPI2 SCK)
//!     Pin 9  (QH)   → STM32 PB14 (SPI2 MISO)
//!
//!   74HC595 (DOUT):
//!     Pin 11 (SRCLK) → STM32 PB13 (SPI2 SCK)
//!     Pin 12 (RCLK)  → STM32 PB12 (RC1)
//!     Pin 14 (SER)   → STM32 PB15 (SPI2 MOSI)
//!
//! \{
/////////////////////////////////////////////////////////////////////////////

// Include main.h for portable STM32 HAL (F4/F7/H7 compatibility)
#include "main.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __SPI_HandleTypeDef SPI_HandleTypeDef;

//! Configuration structure for SRIO driver
typedef struct {
  SPI_HandleTypeDef* hspi;  //!< SPI peripheral handle (e.g., &hspi2)

  // 74HC165 control pins (DIN)
  GPIO_TypeDef* din_pl_port;  //!< /PL (Parallel Load) GPIO port
  uint16_t      din_pl_pin;   //!< /PL pin number (active low pulse to latch inputs)

  // 74HC595 control pins (DOUT)
  GPIO_TypeDef* dout_rclk_port; //!< RCLK (Register Clock) GPIO port
  uint16_t      dout_rclk_pin;  //!< RCLK pin number (rising edge latches outputs)

  // Optional /OE (Output Enable) for 74HC595
  GPIO_TypeDef* dout_oe_port;       //!< /OE GPIO port (set to NULL if not used)
  uint16_t      dout_oe_pin;        //!< /OE pin number
  uint8_t       dout_oe_active_low; //!< 1 if /OE is active low (typical), 0 if active high

  uint16_t din_bytes;   //!< Number of 74HC165 shift registers (bytes) in DIN chain
  uint16_t dout_bytes;  //!< Number of 74HC595 shift registers (bytes) in DOUT chain
} srio_config_t;

/////////////////////////////////////////////////////////////////////////////
// Exported Functions
/////////////////////////////////////////////////////////////////////////////

//! Initializes SPI pins and peripheral
//! \param[in] cfg pointer to configuration structure
//! \note Based on MIOS32_SRIO_Init()
void srio_init(const srio_config_t* cfg);

//! Scans the SRIO chain (reads DIN values)
//! \param[out] out buffer to store DIN values (must be at least din_bytes size)
//! \return 0 on success, < 0 on errors
//! \note Based on MIOS32_SRIO_ScanStart() + MIOS32_SRIO_DMA_Callback()
int srio_read_din(uint8_t* out);

//! Writes DOUT values to shift register chain
//! \param[in] in buffer containing DOUT values to write
//! \return 0 on success, < 0 on errors
int srio_write_dout(const uint8_t* in);

//! Enables or disables DOUT output (/OE control)
//! \param[in] enable 1 to enable outputs, 0 to disable (high-Z)
void srio_set_dout_enable(uint8_t enable);

//! Returns the number of DIN bytes configured
//! \return number of 74HC165 shift registers
uint16_t srio_din_bytes(void);

//! Returns the number of DOUT bytes configured
//! \return number of 74HC595 shift registers
uint16_t srio_dout_bytes(void);

/////////////////////////////////////////////////////////////////////////////
// DIN State Access Functions
/////////////////////////////////////////////////////////////////////////////

//! Returns current DIN value for a specific shift register
//! \param[in] sr shift register number (0..din_bytes-1)
//! \return DIN byte value (8 bits)
uint8_t srio_din_get(uint16_t sr);

//! Returns and clears changed flags for a specific DIN shift register
//! \param[in] sr shift register number (0..din_bytes-1)
//! \param[in] mask bit mask for pins to check/clear
//! \return changed flags (bits set where pins changed state)
//! \note Used by encoder/button handlers to process changes once
uint8_t srio_din_changed_get_and_clear(uint16_t sr, uint8_t mask);

/////////////////////////////////////////////////////////////////////////////
// Debounce Functions
/////////////////////////////////////////////////////////////////////////////

//! Returns the debounce counter reload value
//! \return debounce time in milliseconds (0 if disabled)
//! \note Based on MIOS32_SRIO_DebounceGet()
uint16_t srio_debounce_get(void);

//! Sets the debounce counter reload value for DIN registers
//!
//! Debouncing is realized in the following way: on every button movement 
//! the debounce preload value will be loaded into the debounce counter 
//! register. The counter will be decremented on every SRIO update cycle (usually 1 mS)
//! As long as this counter isn't zero, button changes will still be recorded, 
//! but they won't trigger immediate notification.
//!
//! No (intended) button movement will get lost, but the latency will be 
//! increased. Example: if the update frequency is set to 1 mS, and the 
//! debounce value to 32, the first button movement will be recognized 
//! with a worst-case latency of 1 mS. Every additional button movement 
//! which happens within 32 mS will be recognized within a worst-case 
//! latency of 32 mS. After the debounce time has passed, the worst-case 
//! latency is 1 mS again.
//!
//! This function affects all DIN registers. If the application should 
//! record pin changes from digital sensors which are switching very fast, 
//! then debouncing should be omitted.
//!
//! \param[in] debounce_ms delay in milliseconds (1..65535), 0 disables debouncing
//! \note Based on MIOS32_SRIO_DebounceSet()
void srio_debounce_set(uint16_t debounce_ms);

//! Internally used function to start the debounce delay after a button has been moved
//! \note Based on MIOS32_SRIO_DebounceStart()
void srio_debounce_start(void);

#ifdef __cplusplus
}
#endif

//! \}
