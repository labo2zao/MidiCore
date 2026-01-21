#include "Hal/oled_ssd1306/oled_ssd1306.h"
#include "Hal/spi_bus.h"
#include "Config/oled_pins.h"
#include "Hal/delay_us.h"
// Include main.h for portable STM32 HAL (F4/F7/H7 compatibility)
#include "main.h"
#include <string.h>

// SSD1306 framebuffer: 128x64 pixels, 1 bit per pixel = 1024 bytes
// Can live in CCMRAM to reduce pressure on main SRAM.
static uint8_t fb[OLED_W * OLED_H / 8] __attribute__((section(".ccmram")));

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

  // SSD1306 initialization sequence for 128x64 display
  cmd(0xAE); // Display OFF
  cmd(0xD5); // Set display clock divide ratio/oscillator frequency
  cmd(0x80); // Default setting
  cmd(0xA8); // Set multiplex ratio
  cmd(0x3F); // 1/64 duty (64 rows)
  cmd(0xD3); // Set display offset
  cmd(0x00); // No offset
  cmd(0x40); // Set display start line to 0
  cmd(0x8D); // Charge pump setting
  cmd(0x14); // Enable charge pump
  cmd(0x20); // Set memory addressing mode
  cmd(0x00); // Horizontal addressing mode
  cmd(0xA1); // Set segment re-map (column 127 mapped to SEG0)
  cmd(0xC8); // Set COM output scan direction (remapped mode)
  cmd(0xDA); // Set COM pins hardware configuration
  cmd(0x12); // Alternative COM pin config, disable COM left/right remap
  cmd(0x81); // Set contrast control
  cmd(0xCF); // Mid-high contrast
  cmd(0xD9); // Set pre-charge period
  cmd(0xF1); // Phase 1: 15 DCLKs, Phase 2: 1 DCLK
  cmd(0xDB); // Set VCOMH deselect level
  cmd(0x40); // ~0.77 x VCC
  cmd(0xA4); // Resume to RAM content display
  cmd(0xA6); // Set normal display (not inverted)
  cmd(0xAF); // Display ON

  spibus_end(SPIBUS_DEV_OLED);

  oled_clear();
  oled_flush();
}

uint8_t* oled_framebuffer(void) { return fb; }
void oled_clear(void) { memset(fb, 0, sizeof(fb)); }

void oled_flush(void) {
  spibus_begin(SPIBUS_DEV_OLED);

  // Set column address range (0-127)
  dc_cmd();
  uint8_t setcol[] = { 0x21, 0x00, 0x7F };
  spibus_tx(SPIBUS_DEV_OLED, setcol, sizeof(setcol), 20);
  
  // Set page address range (0-7 for 64 rows / 8 pages)
  uint8_t setpage[] = { 0x22, 0x00, 0x07 };
  spibus_tx(SPIBUS_DEV_OLED, setpage, sizeof(setpage), 20);

  // Send framebuffer data
  dc_data();
  spibus_tx(SPIBUS_DEV_OLED, fb, sizeof(fb), 200);

  spibus_end(SPIBUS_DEV_OLED);
}
