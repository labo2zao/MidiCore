// SRIO Driver for MidiCore - Based on MIOS32 SRIO Driver
// Copyright (C) 2008 Thorsten Klose (tk@midibox.org)
// Adapted for MidiCore by labodezao
//
// This module provides Shift Register Input/Output functionality
// for 74HC165 (DIN) and 74HC595 (DOUT) shift register chains

#include "Services/srio/srio.h"
#include "Services/srio/srio_user_config.h"
#include "Config/module_config.h"

// Include project HAL umbrella via main.h for portability (F4/F7/H7).
#include "main.h"
#include "cmsis_os2.h"
#include <string.h>

/////////////////////////////////////////////////////////////////////////////
// Local variables
/////////////////////////////////////////////////////////////////////////////

static srio_config_t g;
static uint8_t g_inited = 0;

// actual scanned SRs (can be changed during runtime)
static uint8_t g_num_sr = 0;

// for debouncing
static uint16_t g_debounce_time = 0;
static uint16_t g_debounce_ctr = 0;

// DIN values of last scan
static uint8_t* g_din = NULL;

// DIN values of ongoing scan
// Note: during SRIO scan it is required to copy new DIN values into a temporary buffer
// to avoid that a task already takes a new DIN value before the whole chain has been scanned
// (e.g. relevant for encoder handler: it has to clear the changed flags, so that the DIN handler doesn't take the value)
static uint8_t* g_din_buffer = NULL;

// change notification flags
static uint8_t* g_din_changed = NULL;

#if MODULE_ENABLE_AINSER64
extern SPI_HandleTypeDef hspi3;
#endif

static inline void gpio_write(GPIO_TypeDef* port, uint16_t pin, GPIO_PinState st) {
  if (!port) return;
  HAL_GPIO_WritePin(port, pin, st);
}

static void srio_set_spi_prescaler(SPI_HandleTypeDef* hspi, uint32_t prescaler)
{
  if (!hspi) return;
  __HAL_SPI_DISABLE(hspi);
  MODIFY_REG(hspi->Instance->CR1, SPI_CR1_BR, prescaler);
  hspi->Init.BaudRatePrescaler = prescaler;
  __HAL_SPI_ENABLE(hspi);
}

static void srio_set_spi_mode(SPI_HandleTypeDef* hspi, uint32_t cpol, uint32_t cpha)
{
  if (!hspi) return;
  __HAL_SPI_DISABLE(hspi);
  MODIFY_REG(hspi->Instance->CR1, SPI_CR1_CPOL | SPI_CR1_CPHA, cpol | cpha);
  hspi->Init.CLKPolarity = cpol;
  hspi->Init.CLKPhase = cpha;
  __HAL_SPI_ENABLE(hspi);
}

/////////////////////////////////////////////////////////////////////////////
// Initializes SPI pins and peripheral
// \param[in] cfg configuration structure
// Based on MIOS32_SRIO_Init()
/////////////////////////////////////////////////////////////////////////////
void srio_init(const srio_config_t* cfg) {
  if (cfg) g = *cfg;
  g_inited = (g.hspi && g.din_pl_port && g.din_bytes) ? 1u : 0u;

#if SRIO_APPLY_SPI_CONFIG
#if MODULE_ENABLE_AINSER64
  if (g.hspi != &hspi3) {
    srio_set_spi_mode(g.hspi, SRIO_SPI_CPOL, SRIO_SPI_CPHA);
    srio_set_spi_prescaler(g.hspi, SRIO_SPI_PRESCALER);
  }
#else
  // init SPI port for baudrate of ca. 2 uS period @ 72 MHz (MIOS32)
  // using 2 MHz instead of 50 MHz to avoid fast transients which can cause flickering!
  srio_set_spi_mode(g.hspi, SRIO_SPI_CPOL, SRIO_SPI_CPHA);
  srio_set_spi_prescaler(g.hspi, SRIO_SPI_PRESCALER);
#endif
#endif

  // initial state of RCLK pins (idle HIGH for MIOS32 compatibility)
  if (g.din_pl_port) {
#if SRIO_DIN_PL_ACTIVE_LOW
    HAL_GPIO_WritePin(g.din_pl_port, g.din_pl_pin, GPIO_PIN_SET);  // RC2 idle HIGH
#else
    HAL_GPIO_WritePin(g.din_pl_port, g.din_pl_pin, GPIO_PIN_RESET); // RC2 idle LOW
#endif
  }
  if (g.dout_rclk_port) {
    HAL_GPIO_WritePin(g.dout_rclk_port, g.dout_rclk_pin, GPIO_PIN_SET); // RC1 idle HIGH
  }
  
  srio_set_dout_enable(1);

  g_num_sr = (uint8_t)g.din_bytes;
  
  // initial debounce time (debouncing disabled by default)
  g_debounce_time = 0;
  g_debounce_ctr = 0;

  // clear chains
  static uint8_t din[SRIO_DIN_BYTES];
  static uint8_t din_buffer[SRIO_DIN_BYTES];
  static uint8_t din_changed[SRIO_DIN_BYTES];
  g_din = din;
  g_din_buffer = din_buffer;
  g_din_changed = din_changed;
  for (uint8_t i = 0; i < g_num_sr; ++i) {
    g_din[i] = 0xFFu;          // passive state (Buttons depressed)
    g_din_buffer[i] = 0xFFu;   // passive state (Buttons depressed)
    g_din_changed[i] = 0u;     // no change
  }
}

uint16_t srio_din_bytes(void) { return g.din_bytes; }
uint16_t srio_dout_bytes(void) { return g.dout_bytes; }

void srio_set_dout_enable(uint8_t enable) {
  if (!g.dout_oe_port) return;
  if (g.dout_oe_active_low) {
    gpio_write(g.dout_oe_port, g.dout_oe_pin, enable ? GPIO_PIN_RESET : GPIO_PIN_SET);
  } else {
    gpio_write(g.dout_oe_port, g.dout_oe_pin, enable ? GPIO_PIN_SET : GPIO_PIN_RESET);
  }
}

/////////////////////////////////////////////////////////////////////////////
// Scans the SRIO chain (reads DIN, writes DOUT)
// Based on MIOS32_SRIO_ScanStart() and MIOS32_SRIO_DMA_Callback()
//
// \param[out] out buffer to store DIN values (must be at least g.din_bytes size)
// \return 0 on success, < 0 on errors
//
// This function implements the complete MIOS32 scan sequence:
// 1. Pulse RC pins (both RC1 and RC2) to latch inputs
// 2. Perform bulk SPI transfer (DOUT out, DIN in)
// 3. Pulse RC pins again to latch outputs
// 4. Process DIN changes with debouncing
/////////////////////////////////////////////////////////////////////////////
int srio_read_din(uint8_t* out) {
  if (!g_inited || !out) return -1;

  // MIOS32 Scan Sequence - matches MIOS32_SRIO_ScanStart() exactly
  
  // before first byte will be sent:
  // latch DIN registers by pulsing RCLK: 1->0->1
  // MIOS32 pulses BOTH RC_PIN (RC1/RCLK) and RC_PIN2 (RC2//PL) together
  if (g.dout_rclk_port) {
    HAL_GPIO_WritePin(g.dout_rclk_port, g.dout_rclk_pin, GPIO_PIN_RESET);  // RC1 LOW
  }
#if SRIO_DIN_PL_ACTIVE_LOW
  HAL_GPIO_WritePin(g.din_pl_port, g.din_pl_pin, GPIO_PIN_RESET);  // RC2 active (LOW)
#else
  HAL_GPIO_WritePin(g.din_pl_port, g.din_pl_pin, GPIO_PIN_SET);    // RC2 active (HIGH)
#endif
  
  // delay disabled - the delay caused by HAL_GPIO_WritePin function calls is sufficient
  // (MIOS32 comment: "delay disabled - the delay caused by MIOS32_SPI_RC_PinSet function calls is sufficient")
  // We add explicit NOPs for safety on faster MCUs
  for (volatile uint16_t i = 0; i < 10; ++i) { __NOP(); }
  
  // Release BOTH RC pins back to idle HIGH
  if (g.dout_rclk_port) {
    HAL_GPIO_WritePin(g.dout_rclk_port, g.dout_rclk_pin, GPIO_PIN_SET);    // RC1 HIGH
  }
#if SRIO_DIN_PL_ACTIVE_LOW
  HAL_GPIO_WritePin(g.din_pl_port, g.din_pl_pin, GPIO_PIN_SET);     // RC2 idle (HIGH)
#else
  HAL_GPIO_WritePin(g.din_pl_port, g.din_pl_pin, GPIO_PIN_RESET);   // RC2 idle (LOW)
#endif
  
  // start bulk SPI transfer (matches MIOS32_SPI_TransferBlock behavior)
  // MIOS32 uses DMA, we use blocking HAL - functionally equivalent for sync operation
  static uint8_t dout_dummy[32] = {0};  // Dummy DOUT data for full-duplex SPI
  
  if (HAL_SPI_TransmitReceive(g.hspi, dout_dummy, out, g.din_bytes, 100) != HAL_OK) {
    return -2;
  }
  
  // DMA callback equivalent - matches MIOS32_SRIO_DMA_Callback()
  // latch DOUT registers by pulsing RCLK: 1->0->1
  if (g.dout_rclk_port) {
    HAL_GPIO_WritePin(g.dout_rclk_port, g.dout_rclk_pin, GPIO_PIN_RESET);  // RC1 LOW
  }
#if SRIO_DIN_PL_ACTIVE_LOW
  HAL_GPIO_WritePin(g.din_pl_port, g.din_pl_pin, GPIO_PIN_RESET);  // RC2 active (LOW)
#else
  HAL_GPIO_WritePin(g.din_pl_port, g.din_pl_pin, GPIO_PIN_SET);    // RC2 active (HIGH)
#endif
  
  for (volatile uint16_t i = 0; i < 10; ++i) { __NOP(); }
  
  // Release BOTH RC pins back to idle HIGH
  if (g.dout_rclk_port) {
    HAL_GPIO_WritePin(g.dout_rclk_port, g.dout_rclk_pin, GPIO_PIN_SET);    // RC1 HIGH
  }
#if SRIO_DIN_PL_ACTIVE_LOW
  HAL_GPIO_WritePin(g.din_pl_port, g.din_pl_pin, GPIO_PIN_SET);     // RC2 idle (HIGH)
#else
  HAL_GPIO_WritePin(g.din_pl_port, g.din_pl_pin, GPIO_PIN_RESET);   // RC2 idle (LOW)
#endif
  
  // copy/or buffered DIN values/changed flags
  // Update internal DIN buffers with change detection (matches MIOS32 exactly)
  if (g_din && g_din_buffer && g_din_changed) {
    // STEP 1: ALWAYS copy buffered DIN values and detect changes (matches MIOS32)
    // This must happen BEFORE debounce logic is applied
    uint8_t any_change = 0;
    for (uint8_t i = 0; i < g_num_sr; ++i) {
      g_din_buffer[i] = out[i];
      uint8_t change_mask = g_din[i] ^ g_din_buffer[i]; // these are the changed pins
      g_din_changed[i] |= change_mask;
      g_din[i] = g_din_buffer[i];
      if (change_mask) {
        any_change = 1;
      }
    }
    
    // STEP 2: Start debounce counter if any change detected (matches MIOS32_SRIO_DebounceStart)
    if (any_change && g_debounce_time) {
      g_debounce_ctr = g_debounce_time;
    }
    
    // STEP 3: Apply debounce XOR trick if counter is active (matches MIOS32)
    // As long as debounce counter is != 0, clear all "changed" flags to ignore button movements 
    // at this time. In order to ensure, that a new final state of a button won't get lost, 
    // the DIN values are XORed with the "changed" flags (yes, this idea is ill, but it works! :)
    // Even the encoder handler (or others which are notified by the scan_finished_hook) still
    // work properly, because they are clearing the appr. "changed" flags, so that the DIN
    // values won't be touched by the XOR operation.
    if (g_debounce_time && g_debounce_ctr) {
      --g_debounce_ctr;
      for (uint8_t i = 0; i < g_num_sr; ++i) {
        g_din[i] ^= g_din_changed[i];
        g_din_changed[i] = 0;
      }
    }
  }

  return 0;
}

int srio_write_dout(const uint8_t* in) {
  if (!g_inited || !in) return -1;
  if (!g.dout_rclk_port || !g.dout_bytes) return -1;

  // Shift out to 595 chain.
  if (HAL_SPI_Transmit(g.hspi, (uint8_t*)in, g.dout_bytes, 10) != HAL_OK) return -2;

  // Latch: RCLK rising edge (idle low).
  HAL_GPIO_WritePin(g.dout_rclk_port, g.dout_rclk_pin, GPIO_PIN_SET);
  __NOP(); __NOP(); __NOP();
  HAL_GPIO_WritePin(g.dout_rclk_port, g.dout_rclk_pin, GPIO_PIN_RESET);

  return 0;
}

uint8_t srio_din_get(uint16_t sr)
{
  if (!g_din || sr >= g_num_sr) return 0xFFu;
  return g_din[sr];
}

uint8_t srio_din_changed_get_and_clear(uint16_t sr, uint8_t mask)
{
  if (!g_din_changed || sr >= g_num_sr) return 0u;
  uint8_t changed = g_din_changed[sr] & mask;
  g_din_changed[sr] &= (uint8_t)(~mask);
  return changed;
}

uint16_t srio_debounce_get(void)
{
  return g_debounce_time;
}

void srio_debounce_set(uint16_t debounce_ms)
{
  g_debounce_time = debounce_ms;
  if (g_debounce_ctr > g_debounce_time) {
    g_debounce_ctr = g_debounce_time;
  }
}

void srio_debounce_start(void)
{
  g_debounce_ctr = g_debounce_time;
}
