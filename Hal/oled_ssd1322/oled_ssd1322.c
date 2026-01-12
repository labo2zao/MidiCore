#include "Hal/oled_ssd1322/oled_ssd1322.h"
#include "Hal/spi_bus.h"
#include "Config/oled_pins.h"
#include "Hal/delay_us.h"
#include "stm32f4xx_hal.h"
#include <string.h>

// Framebuffer can live in CCMRAM to reduce pressure on main SRAM.
// (CCMRAM is fine: this buffer is only accessed by CPU, not DMA.)
static uint8_t fb[OLED_W * OLED_H / 2] __attribute__((section(".ccmram")));

static void dc_cmd(void) { HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET); }
static void dc_data(void){ HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET); }

static void cmd(uint8_t c) {
  dc_cmd();
  spibus_tx(SPIBUS_DEV_OLED, &c, 1, 20);
}

static void reset_pulse(void) {
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET);
  delay_us(2000);
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET);
  delay_us(5000);
}

void oled_init(void) {
  HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET);

  reset_pulse();
  spibus_begin(SPIBUS_DEV_OLED);

  cmd(0xAE); // display off
  cmd(0xB9); // linear gray scale table
  cmd(0xA6); // normal
  cmd(0xAF); // display on

  spibus_end(SPIBUS_DEV_OLED);

  oled_clear();
  oled_flush();
}

uint8_t* oled_framebuffer(void) { return fb; }
void oled_clear(void) { memset(fb, 0, sizeof(fb)); }

void oled_flush(void) {
  spibus_begin(SPIBUS_DEV_OLED);

  dc_cmd();
  uint8_t setcol[] = { 0x15, 0x00, 0x7F };
  spibus_tx(SPIBUS_DEV_OLED, setcol, sizeof(setcol), 20);
  uint8_t setrow[] = { 0x75, 0x00, 0x3F };
  spibus_tx(SPIBUS_DEV_OLED, setrow, sizeof(setrow), 20);

  dc_data();
  spibus_tx(SPIBUS_DEV_OLED, fb, sizeof(fb), 200);

  spibus_end(SPIBUS_DEV_OLED);
}
