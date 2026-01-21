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

static void write_cmd(uint8_t c) {
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET);
  spibus_tx(SPIBUS_DEV_OLED, &c, 1, 100);
}

static void write_data(uint8_t d) {
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET);
  spibus_tx(SPIBUS_DEV_OLED, &d, 1, 100);
}

static void reset_pulse(void) {
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET);
  delay_us(10000); // 10ms
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET);
  delay_us(10000); // 10ms
}

void oled_init(void) {
  // Set initial pin states
  HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET);

  // Wait for power stabilization (300ms like MIOS32)
  delay_us(300000);
  
  // Hardware reset
  reset_pulse();
  
  // Begin SPI transaction
  spibus_begin(SPIBUS_DEV_OLED);

  // SSD1322 initialization - MIOS32 LoopA sequence
  write_cmd(0xFD); write_data(0x12); // Unlock driver IC
  
  write_cmd(0xAE); // Display OFF
  
  write_cmd(0x15); // Set Column Address
  write_data(0x1C); // Start column 28
  write_data(0x5B); // End column 91 (64 columns for 256 pixels)
  
  write_cmd(0x75); // Set Row Address
  write_data(0x00); // Start row 0
  write_data(0x3F); // End row 63
  
  write_cmd(0xCA); write_data(0x3F); // Set MUX Ratio (64)
  
  write_cmd(0xA0); // Set Remap Format
  write_data(0x14); // Horizontal increment, column remap
  write_data(0x11); // Dual COM mode
  
  write_cmd(0xB3); // Set Display Clock
  write_data(0x00); // Divide ratio
  write_data(0x0C); // Oscillator frequency (12)
  
  write_cmd(0xC1); write_data(0xFF); // Set Contrast Current (MAX)
  
  write_cmd(0xC7); write_data(0x0F); // Master Contrast (MAX)
  
  write_cmd(0xB9); // Set Linear Gray Scale Table
  
  write_cmd(0x00); // Disable gray scale table (use linear)
  
  write_cmd(0xB1); write_data(0x56); // Set Phase Length
  
  write_cmd(0xBB); write_data(0x00); // Set Precharge Voltage
  
  write_cmd(0xB6); write_data(0x08); // Set Precharge Period
  
  write_cmd(0xBE); write_data(0x00); // Set VCOMH
  
  write_cmd(0xA6); // Normal Display (A4 | 0x02 in MIOS32)
  
  spibus_end(SPIBUS_DEV_OLED);
  
  // Clear screen
  oled_clear();
  oled_flush();
  
  // Turn display ON
  spibus_begin(SPIBUS_DEV_OLED);
  write_cmd(0xAF); // Display ON
  spibus_end(SPIBUS_DEV_OLED);
  
  delay_us(100000); // Wait 100ms
}

uint8_t* oled_framebuffer(void) { return fb; }
void oled_clear(void) { memset(fb, 0, sizeof(fb)); }

void oled_flush(void) {
  spibus_begin(SPIBUS_DEV_OLED);

  // Send framebuffer data row by row (MIOS32 style)
  for (uint8_t row = 0; row < 64; row++) {
    // Set column address
    write_cmd(0x15);
    write_data(0x1C); // Start column 28
    
    // Set row address
    write_cmd(0x75);
    write_data(row); // Current row
    
    // Write to RAM
    write_cmd(0x5C);
    
    // Send one row of data (128 bytes = 256 pixels at 4bpp)
    HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET);
    spibus_tx(SPIBUS_DEV_OLED, &fb[row * 128], 128, 200);
  }

  spibus_end(SPIBUS_DEV_OLED);
}


