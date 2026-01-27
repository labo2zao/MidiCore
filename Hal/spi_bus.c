#include "Hal/spi_bus.h"
#include "Config/sd_pins.h"
#include "Config/ainser64_pins.h"
// Note: OLED no longer uses hardware SPI (uses software SPI instead)

static osMutexId_t g_spi1_mutex;
static osMutexId_t g_spi3_mutex;

// Safe defaults (tuned to match MIOS32)
// SD Card: Use slow speed for initialization (400 kHz), then can switch to faster
// For STM32F407 @ 168 MHz APB2:
//   - Prescaler 256 = 168/256 = 656 kHz (safe for init)
//   - Prescaler 4 = 168/4 = 42 MHz (fast mode after init)
// AINSER: MIOS32 uses prescaler 64 @ 120 MHz = 1.875 MHz (max 2 MHz per MCP3208 datasheet)
// For STM32F407 @ 168 MHz: prescaler 64 gives 168/64 = 2.625 MHz (still within MCP3208 spec)
static uint32_t presc_sd   = SPI_BAUDRATEPRESCALER_256;  // Start slow for SD init
static uint32_t presc_ain  = SPI_BAUDRATEPRESCALER_64;

// Function to change SD card speed after initialization
void spibus_set_sd_speed_fast(void) {
  presc_sd = SPI_BAUDRATEPRESCALER_4;  // 42 MHz for fast operations
}

static void cs_high(spibus_dev_t dev) {
  if (dev == SPIBUS_DEV_SD)
    HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);
  else if (dev == SPIBUS_DEV_AIN)
    HAL_GPIO_WritePin(AIN_CS_PORT, AIN_CS_PIN, GPIO_PIN_SET);
}

static void cs_low(spibus_dev_t dev) {
  if (dev == SPIBUS_DEV_SD)
    HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET);
  else if (dev == SPIBUS_DEV_AIN)
    HAL_GPIO_WritePin(AIN_CS_PORT, AIN_CS_PIN, GPIO_PIN_RESET);
}

static SPI_HandleTypeDef* dev_spi(spibus_dev_t dev) {
  switch (dev) {
    case SPIBUS_DEV_SD:   return &hspi1;
    case SPIBUS_DEV_AIN:  return &hspi3;
    default:              return &hspi1;
  }
}

static osMutexId_t dev_mutex(spibus_dev_t dev) {
  switch (dev) {
    case SPIBUS_DEV_AIN:  return g_spi3_mutex;
    default:              return g_spi1_mutex;
  }
}

static uint32_t dev_presc(spibus_dev_t dev) {
  if (dev == SPIBUS_DEV_SD) return presc_sd;
  return presc_ain;
}

static void spi_set_prescaler(SPI_HandleTypeDef* hspi, uint32_t prescaler) {
  __HAL_SPI_DISABLE(hspi);
  MODIFY_REG(hspi->Instance->CR1, SPI_CR1_BR, prescaler);
  __HAL_SPI_ENABLE(hspi);
}

void spibus_init(void) {
  const osMutexAttr_t attr = { .name = "spibus" };
  g_spi1_mutex = osMutexNew(&attr);
  g_spi3_mutex = osMutexNew(&attr);

  cs_high(SPIBUS_DEV_SD);
  cs_high(SPIBUS_DEV_AIN);
  // Note: OLED uses software SPI (bit-bang), not managed by spibus
}

HAL_StatusTypeDef spibus_begin(spibus_dev_t dev) {
  osMutexId_t m = dev_mutex(dev);
  if (!m) return HAL_ERROR;
  if (osMutexAcquire(m, osWaitForever) != osOK) return HAL_ERROR;

  SPI_HandleTypeDef* h = dev_spi(dev);
  spi_set_prescaler(h, dev_presc(dev));
  cs_low(dev);
  return HAL_OK;
}

void spibus_end(spibus_dev_t dev) {
  cs_high(dev);
  osMutexId_t m = dev_mutex(dev);
  if (m) osMutexRelease(m);
}

HAL_StatusTypeDef spibus_tx(spibus_dev_t dev, const uint8_t* tx, uint16_t len, uint32_t timeout) {
  return HAL_SPI_Transmit(dev_spi(dev), (uint8_t*)tx, len, timeout);
}
HAL_StatusTypeDef spibus_rx(spibus_dev_t dev, uint8_t* rx, uint16_t len, uint32_t timeout) {
  return HAL_SPI_Receive(dev_spi(dev), rx, len, timeout);
}
HAL_StatusTypeDef spibus_txrx(spibus_dev_t dev, const uint8_t* tx, uint8_t* rx, uint16_t len, uint32_t timeout) {
  return HAL_SPI_TransmitReceive(dev_spi(dev), (uint8_t*)tx, rx, len, timeout);
}
