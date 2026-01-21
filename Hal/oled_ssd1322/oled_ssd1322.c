#include "Hal/oled_ssd1322/oled_ssd1322.h"
#include "Hal/spi_bus.h"
#include "Config/oled_pins.h"
#include "Hal/delay_us.h"
// Include main.h for portable STM32 HAL (F4/F7/H7 compatibility)
#include "main.h"
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

static void cmd_data(uint8_t c, uint8_t d) {
  cmd(c);
  dc_data();
  spibus_tx(SPIBUS_DEV_OLED, &d, 1, 20);
  dc_cmd();
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

  // SSD1322 initialization sequence for 256x64 display
  cmd_data(0xFD, 0x12); // Set Command Lock - Unlock OLED driver IC
  
  cmd(0xAE); // Display OFF (sleep mode)
  
  cmd_data(0xB3, 0x91); // Set Front Clock Divider / Oscillator Frequency
  
  cmd_data(0xCA, 0x3F); // Set MUX Ratio - 1/64 duty (64 MUX)
  
  cmd_data(0xA2, 0x00); // Set Display Offset - No offset
  
  cmd_data(0xA1, 0x00); // Set Display Start Line - Start line = 0
  
  cmd(0xA0); // Set Re-map and Dual COM Line mode
  cmd(0x14); // Enable Column Address Re-map, Disable Nibble Re-map
  cmd(0x11); // Enable Dual COM mode (for 64 MUX)
  
  cmd_data(0xB5, 0x00); // Set GPIO - GPIO0, GPIO1 = HiZ (disabled)
  
  cmd_data(0xAB, 0x01); // Function Selection - Enable internal VDD regulator
  
  cmd(0xB4); // Display Enhancement A
  cmd(0xA0); // Enable external VSL
  cmd(0xFD); // Enhanced low GS display quality
  
  cmd_data(0xC1, 0x7F); // Set Contrast Current - Medium-high contrast
  
  cmd_data(0xC7, 0x0F); // Master Contrast Current Control - Maximum
  
  cmd(0xB9); // Set Linear Gray Scale Table (default)
  
  cmd_data(0xB1, 0xE2); // Set Phase Length - Phase 1 = 5 DCLKs, Phase 2 = 14 DCLKs
  
  cmd(0xD1); // Display Enhancement B
  cmd(0x82); // Normal enhancement
  cmd(0x20); // Reserved
  
  cmd_data(0xBB, 0x1F); // Set Pre-charge voltage - 0.60 x VCC
  
  cmd_data(0xB6, 0x08); // Set Second Pre-charge Period - 8 DCLKs
  
  cmd_data(0xBE, 0x07); // Set VCOMH - 0.86 x VCC
  
  cmd(0xA6); // Normal Display (not inverted)
  
  cmd(0xA9); // Exit Partial Display
  
  cmd(0xAF); // Display ON

  spibus_end(SPIBUS_DEV_OLED);

  delay_us(100000); // Wait 100ms for display to stabilize
  
  oled_clear();
  oled_flush();
}

uint8_t* oled_framebuffer(void) { return fb; }
void oled_clear(void) { memset(fb, 0, sizeof(fb)); }

void oled_flush(void) {
  spibus_begin(SPIBUS_DEV_OLED);

  // Set column address range (0-127 for 256 pixels with 4bpp = 128 bytes per row)
  dc_cmd();
  uint8_t setcol[] = { 0x15, 0x00, 0x7F };
  spibus_tx(SPIBUS_DEV_OLED, setcol, sizeof(setcol), 20);
  
  // Set row address range (0-63 for 64 rows)
  uint8_t setrow[] = { 0x75, 0x00, 0x3F };
  spibus_tx(SPIBUS_DEV_OLED, setrow, sizeof(setrow), 20);
  
  // Enable MCU to write data into RAM
  cmd(0x5C);

  // Send framebuffer data
  dc_data();
  spibus_tx(SPIBUS_DEV_OLED, fb, sizeof(fb), 200);

  spibus_end(SPIBUS_DEV_OLED);
}
