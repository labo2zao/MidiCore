#include "Hal/oled_ssd1322/oled_ssd1322.h"
#include "Hal/spi_bus.h"
#include "Config/oled_pins.h"
#include "Hal/delay_us.h"
#include "main.h"
#include <string.h>

// Framebuffer: screen[row][column] where each byte contains 2 pixels (4-bit grayscale)
// Matches LoopA screen buffer layout: screen[64][128]
static uint8_t fb[64][128] __attribute__((section(".ccmram")));

// Low-level command/data transmission (matching LoopA APP_LCD_Cmd/Data)
static void oled_cmd(uint8_t c) {
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET);
  spibus_tx(SPIBUS_DEV_OLED, &c, 1, 100);
}

static void oled_data(uint8_t d) {
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET);
  spibus_tx(SPIBUS_DEV_OLED, &d, 1, 100);
}

void oled_init(void) {
  // Initialize GPIO pins
  HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET);

  // Power stabilization delay
  uint16_t ctr;
  for (ctr = 0; ctr < 300; ++ctr)
    delay_us(1000);
  
  // Hardware reset pulse
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET);
  delay_us(10000);
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET);
  delay_us(10000);
  
  // Begin SPI transaction
  spibus_begin(SPIBUS_DEV_OLED);

  // SSD1322 initialization sequence (from MIOS32 LoopA app_lcd.c)
  oled_cmd(0xFD); oled_data(0x12); // Unlock driver
  
  oled_cmd(0xAE); // Display OFF
  
  oled_cmd(0x15); // Set Column Address
  oled_data(0x1C);
  oled_data(0x5B);
  
  oled_cmd(0x75); // Set Row Address
  oled_data(0x00);
  oled_data(0x3F);
  
  oled_cmd(0xCA); oled_data(0x3F); // Set Multiplex Ratio
  
  oled_cmd(0xA0); // Set Remap Format
  oled_data(0x14);
  oled_data(0x11);
  
  oled_cmd(0xB3); // Set Display Clock
  oled_data(0x00);
  oled_data(0x0C);
  
  oled_cmd(0xC1); oled_data(0xFF); // Set Contrast Current
  
  oled_cmd(0xC7); oled_data(0x0F); // Master Contrast Current
  
  oled_cmd(0xB9); // Set Linear Gray Scale Table
  
  oled_cmd(0x00); // Enable gray scale table
  
  oled_cmd(0xB1); oled_data(0x56); // Set Phase Length
  
  oled_cmd(0xBB); oled_data(0x00); // Set Precharge Voltage
  
  oled_cmd(0xB6); oled_data(0x08); // Set Precharge Period
  
  oled_cmd(0xBE); oled_data(0x00); // Set VCOMH
  
  oled_cmd(0xA6); // Normal Display
  
  // Clear the display (LoopA APP_LCD_Clear style)
  oled_clear();
  oled_flush();
  
  oled_cmd(0xAF); // Display ON
  
  spibus_end(SPIBUS_DEV_OLED);
}

uint8_t* oled_framebuffer(void) { 
  return (uint8_t*)fb; 
}

void oled_clear(void) { 
  memset(fb, 0, sizeof(fb)); 
}

void oled_flush(void) {
  uint8_t i, j;
  
  spibus_begin(SPIBUS_DEV_OLED);
  
  // Push screen buffer to screen - exact LoopA implementation
  for (j = 0; j < 64; j++) {
    oled_cmd(0x15);
    oled_data(0x00 + 0x1C);
    
    oled_cmd(0x75);
    oled_data(j);
    
    oled_cmd(0x5C);
    
    // Send entire row - DC pin stays high for data
    HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET);
    spibus_tx(SPIBUS_DEV_OLED, fb[j], 128, 200);
  }
  
  spibus_end(SPIBUS_DEV_OLED);
}
