#include "Hal/oled_ssd1322/oled_ssd1322.h"
#include "Hal/spi_bus.h"
#include "Config/oled_pins.h"
#include "Hal/delay_us.h"
#include "main.h"
#include <string.h>

// Framebuffer: flat array compatible with MidiCore UI (256×64 pixels, 4-bit grayscale)
// Size: OLED_W * OLED_H / 2 = 256 * 64 / 2 = 8192 bytes
// Layout: 64 rows × 128 bytes per row (each byte = 2 pixels)
static uint8_t fb[OLED_W * OLED_H / 2] __attribute__((section(".ccmram")));

// Low-level command/data transmission (adapted from LoopA)
static void cmd(uint8_t c) {
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET);
  spibus_tx(SPIBUS_DEV_OLED, &c, 1, 100);
}

static void data(uint8_t d) {
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET);
  spibus_tx(SPIBUS_DEV_OLED, &d, 1, 100);
}

void oled_init(void) {
  // Initialize GPIO pins
  HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET);

  // Power stabilization delay (300ms like LoopA)
  for (uint16_t ctr = 0; ctr < 300; ++ctr)
    delay_us(1000);
  
  // Hardware reset pulse
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET);
  delay_us(10000);
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET);
  delay_us(10000);
  
  // Begin SPI transaction
  spibus_begin(SPIBUS_DEV_OLED);

  // SSD1322 initialization (LoopA sequence)
  cmd(0xFD); data(0x12); // Unlock OLED driver IC
  
  cmd(0xAE); // Display OFF during init
  
  cmd(0x15); // Set Column Address (0x1C-0x5B for 256px)
  data(0x1C); // Start column 28
  data(0x5B); // End column 91
  
  cmd(0x75); // Set Row Address
  data(0x00); // Start row 0
  data(0x3F); // End row 63
  
  cmd(0xCA); data(0x3F); // Set MUX Ratio (64 lines)
  
  cmd(0xA0); // Set Re-map / Color Depth
  data(0x14); // Enable Dual COM, nibble remap
  data(0x11); // Enable COM Split Odd Even
  
  cmd(0xB3); // Display Clock Divider
  data(0x00); // Divide ratio = 1
  data(0x0C); // Oscillator Frequency = 12
  
  cmd(0xC1); data(0xFF); // Set Contrast Current (MAX)
  
  cmd(0xC7); data(0x0F); // Master Contrast Current Control (MAX)
  
  cmd(0xB9); // Select Default Linear Gray Scale table
  
  cmd(0xB1); data(0x56); // Set Phase Length
  
  cmd(0xBB); data(0x00); // Set Pre-charge voltage
  
  cmd(0xB6); data(0x08); // Set Second Precharge Period
  
  cmd(0xBE); data(0x00); // Set VCOMH Voltage
  
  cmd(0xA6); // Set Normal Display (not inverted)
  
  cmd(0xA9); // Exit partial display
  
  // Fill display with test pattern to verify it's working
  cmd(0x15); data(0x1C); data(0x5B);
  cmd(0x75); data(0x00); data(0x3F);
  cmd(0x5C); // Write RAM
  
  // Send test pattern: white bar at top (first 4 rows)
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET);
  for (uint16_t i = 0; i < 128 * 4; i++) {
    uint8_t test_val = 0xFF; // Bright white
    spibus_tx(SPIBUS_DEV_OLED, &test_val, 1, 100);
  }
  
  // Rest of screen: medium gray
  for (uint16_t i = 0; i < 128 * 60; i++) {
    uint8_t test_val = 0x77; // Medium gray
    spibus_tx(SPIBUS_DEV_OLED, &test_val, 1, 100);
  }
  
  cmd(0xAF); // Display ON
  
  spibus_end(SPIBUS_DEV_OLED);
  
  // Wait to see test pattern
  delay_us(1000000); // 1 second
  
  // Clear framebuffer for UI use
  memset(fb, 0, sizeof(fb));
}

uint8_t* oled_framebuffer(void) { 
  return fb; 
}

void oled_clear(void) { 
  memset(fb, 0, sizeof(fb)); 
}

void oled_flush(void) {
  spibus_begin(SPIBUS_DEV_OLED);
  
  // Push framebuffer to display row by row (LoopA style)
  // Framebuffer: 64 rows × 128 bytes per row
  for (uint8_t row = 0; row < 64; row++) {
    // Set column address for this row
    cmd(0x15);
    data(0x1C); // Start column 28
    
    // Set row address
    cmd(0x75);
    data(row); // Current row
    
    // Write RAM command
    cmd(0x5C);
    
    // Send 128 bytes (256 pixels at 4bpp) for this row
    HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET);
    uint8_t* row_data = &fb[row * 128]; // Calculate row offset
    spibus_tx(SPIBUS_DEV_OLED, row_data, 128, 200);
  }
  
  spibus_end(SPIBUS_DEV_OLED);
}
