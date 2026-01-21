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
  // Initialize GPIO pins - CRITICAL: Set proper initial states
  HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET);    // CS high (idle)
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET);  // DC low (command mode)
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET);  // RST high (not in reset)

  // Power stabilization delay - CRITICAL for SSD1322
  // Display needs time for internal power supply to stabilize
  // Per datasheet: minimum 100ms, LoopA uses 300ms for safety
  for (uint16_t ctr = 0; ctr < 300; ++ctr)
    delay_us(1000); // 300ms total
  
  // Hardware reset sequence - CRITICAL for waking up display
  // Per SSD1322 datasheet Section 8.9:
  // 1. RST = LOW for minimum 2μs
  // 2. Wait minimum 2μs after RST = HIGH before sending commands
  // We use much longer delays for reliability
  
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET); // Assert reset (active low)
  delay_us(100000); // Hold reset for 100ms (datasheet minimum: 2μs)
  
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET);   // Release reset
  delay_us(200000); // Wait 200ms for display to complete internal reset (datasheet minimum: 2μs)
  
  // Begin SPI transaction
  spibus_begin(SPIBUS_DEV_OLED);

  // SSD1322 initialization sequence
  // Per datasheet Section 8: Initialization must occur in specific order
  
  // Step 1: Unlock driver IC (required before any configuration)
  cmd(0xFD); data(0x12); // Unlock OLED driver IC (0x12 = unlock, 0x16 = lock)
  
  // Step 2: Display OFF during configuration (prevents artifacts)
  cmd(0xAE); // Display OFF
  
  // Step 3: Set display start line and offset
  cmd(0xA1); data(0x00); // Set display start line to 0
  cmd(0xA2); data(0x00); // Set display offset to 0
  
  // Step 4: Set column address (critical for 256-pixel width)
  cmd(0x15); // Set Column Address
  data(0x1C); // Start column 28 (offset for 256px display)
  data(0x5B); // End column 91 (28+64-1, as each column = 4 pixels)
  
  // Step 5: Set row address
  cmd(0x75); // Set Row Address
  data(0x00); // Start row 0
  data(0x3F); // End row 63 (64 rows total)
  
  // Step 6: Set MUX ratio (number of COM lines)
  cmd(0xCA); data(0x3F); // Set MUX Ratio to 64 lines (0x3F = 63+1)
  
  // Step 7: Set re-mapping and data format - CRITICAL
  cmd(0xA0); // Set Re-map / Color Depth
  data(0x14); // [7:6]=00 Disable Column Address Remap, [5:4]=01 Nibble Remap, [2]=1 Dual COM, [1:0]=00 4bpp
  data(0x11); // [6]=1 Enable COM Split Odd Even, [4]=1 Scan from COM[N-1] to COM0
  
  // Step 8: Configure display timing
  cmd(0xB3); // Display Clock Divider / Oscillator Frequency
  data(0x00); // Divide ratio = 1 (no division)
  data(0x0C); // Oscillator Frequency = 12 (higher = faster refresh, LoopA uses 0x0C)
  
  // Step 9: Set GPIO pins (not used, but configure anyway)
  cmd(0xB5); data(0x00); // GPIO = low, disable
  
  // Step 10: Enable internal VDD regulator
  cmd(0xAB); data(0x01); // Enable internal VDD regulator (0x01 = enable, 0x00 = disable)
  
  // Step 11: Configure display enhancement
  cmd(0xB4); // Display Enhancement A
  data(0xA0); // Enable external VSL
  data(0xFD); // Enhanced low GS display quality
  
  // Step 12: Set contrast levels - CRITICAL for visibility
  cmd(0xC1); data(0xFF); // Set Contrast Current (0xFF = maximum)
  cmd(0xC7); data(0x0F); // Master Contrast Current Control (0x0F = maximum)
  
  // Step 13: Select gray scale table
  cmd(0xB9); // Select Default Linear Gray Scale table
  
  // Step 14: Configure phase lengths and precharge
  cmd(0xB1); data(0x56); // Set Phase Length (Phase 1 = 5 DCLKs, Phase 2 = 6 DCLKs)
  cmd(0xBB); data(0x00); // Set Pre-charge voltage to ~0.20 × VCC
  cmd(0xB6); data(0x08); // Set Second Precharge Period = 8 DCLKs
  
  // Step 15: Set VCOMH voltage
  cmd(0xBE); data(0x00); // Set VCOMH deselect level to ~0.70 × VCC
  
  // Step 16: Configure display mode
  cmd(0xA6); // Set Normal Display (not inverted)
  cmd(0xA9); // Exit partial display mode
  
  // Step 17: Clear display RAM before turning on
  cmd(0x15); data(0x1C); data(0x5B); // Set column address
  cmd(0x75); data(0x00); data(0x3F); // Set row address
  cmd(0x5C); // Write RAM command
  
  // Send all zeros to clear display RAM
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET);
  uint8_t zero = 0x00;
  for (uint16_t i = 0; i < 128 * 64; i++) {
    spibus_tx(SPIBUS_DEV_OLED, &zero, 1, 100);
  }
  
  // Step 18: Turn display ON - CRITICAL
  cmd(0xAF); // Display ON (wake from sleep mode)
  
  // Wait for display to fully wake up and stabilize
  spibus_end(SPIBUS_DEV_OLED);
  delay_us(100000); // 100ms stabilization after Display ON
  
  // Now write test pattern to verify display is working
  spibus_begin(SPIBUS_DEV_OLED);
  // Now write test pattern to verify display is working
  spibus_begin(SPIBUS_DEV_OLED);
  
  // Set address window for test pattern
  cmd(0x15); data(0x1C); data(0x5B); // Column address
  cmd(0x75); data(0x00); data(0x3F); // Row address
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
  
  spibus_end(SPIBUS_DEV_OLED);
  
  // Wait to see test pattern (display is already ON)
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
  
  // Push framebuffer to display row by row (exact LoopA implementation)
  // Framebuffer: 64 rows × 128 bytes per row
  for (uint8_t row = 0; row < 64; row++) {
    // Set column address for this row (columns 0x1C-0x5B)
    cmd(0x15);
    data(0x1C); // Start column 28
    data(0x5B); // End column 91
    
    // Set row address
    cmd(0x75);
    data(row); // Current row
    data(0x3F); // End row (not used since we set it per row, but LoopA sends it)
    
    // Write RAM command
    cmd(0x5C);
    
    // Send 128 bytes (256 pixels at 4bpp) for this row
    HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET);
    uint8_t* row_data = &fb[row * 128]; // Calculate row offset
    spibus_tx(SPIBUS_DEV_OLED, row_data, 128, 200);
  }
  
  spibus_end(SPIBUS_DEV_OLED);
}
