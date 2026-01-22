#include "Hal/oled_ssd1322/oled_ssd1322.h"
#include "Config/oled_pins.h"
#include "Hal/delay_us.h"
#include "main.h"
#include <string.h>

// Framebuffer: flat array compatible with MidiCore UI (256×64 pixels, 4-bit grayscale)
// Size: OLED_W * OLED_H / 2 = 256 * 64 / 2 = 8192 bytes
// Layout: 64 rows × 128 bytes per row (each byte = 2 pixels)
static uint8_t fb[OLED_W * OLED_H / 2] __attribute__((section(".ccmram")));

// Software SPI bit-bang implementation (MIOS32 compatible)
// CS is hardwired to GND, so no CS control needed
//
// CRITICAL TIMING NOTES:
// - SPI Mode 3: Clock idle HIGH, sample on rising edge
// - Setup data BEFORE clock transition
// - SSD1322 requires minimum 100ns setup/hold times
static inline void spi_write_byte(uint8_t byte) {
  for (uint8_t i = 0; i < 8; i++) {
    // Clock low first (prepare for rising edge)
    HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, GPIO_PIN_RESET);
    
    // Set data bit (MSB first) - MUST be stable before clock rises
    if (byte & 0x80) {
      HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_SET);
    } else {
      HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_RESET);
    }
    
    // Small delay for data setup time (SSD1322: min 100ns)
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
    
    // Clock high - SSD1322 samples data on this rising edge
    HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, GPIO_PIN_SET);
    
    // Hold time (SSD1322: min 100ns)
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
    
    byte <<= 1;
  }
  
  // Leave clock low after byte (ready for next byte)
  HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, GPIO_PIN_RESET);
}

// Low-level command transmission
// SSD1322 requires DC (Data/Command) signal:
// - DC=0 (low) for commands
// - DC=1 (high) for data
// PA8 (LCD_RS in MIOS32) is used as DC
//
// CRITICAL: DC must be stable BEFORE starting SPI transmission
static void cmd(uint8_t c) {
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET); // DC=0 for command
  __NOP(); __NOP(); __NOP(); __NOP(); // Let DC stabilize (setup time)
  spi_write_byte(c);
}

static void data(uint8_t d) {
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET); // DC=1 for data
  __NOP(); __NOP(); __NOP(); __NOP(); // Let DC stabilize (setup time)
  spi_write_byte(d);
}

// Helper for commands with 2 data bytes
static void cmd_data2(uint8_t c, uint8_t d1, uint8_t d2) {
  cmd(c);
  data(d1);
  data(d2);
}

void oled_init(void) {
  // Initialize GPIO pins - CRITICAL: Set proper initial states
  HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, GPIO_PIN_RESET);  // Clock low (idle)
  HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_RESET);  // Data low
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET);      // DC/RST high (not in reset)

  // Power stabilization delay - CRITICAL for SSD1322
  // Display needs time for internal power supply to stabilize
  // Per datasheet: minimum 100ms, LoopA uses 300ms for safety
  for (uint16_t ctr = 0; ctr < 300; ++ctr)
    delay_us(1000); // 300ms total
  
  // ===== On-Board RC Reset Circuit (on OLED module) =====
  // The OLED display module has an on-board RC (resistor-capacitor) circuit
  // that automatically generates a reset pulse when power is applied.
  // There is NO RST connection from the STM32 board to the OLED module.
  //
  // At power-up:
  // 1. OLED module's capacitor is discharged → RST = 0V (reset asserted)
  // 2. Capacitor charges through resistor → RST rises to 3.3V
  // 3. This creates the required reset pulse (SSD1322 datasheet: min 2μs)
  // 4. STM32 just waits for reset cycle to complete
  //
  // Per SSD1322 datasheet Section 8.9:
  // After RST rises and VDD is stable, wait before sending commands.
  // The OLED module's RC time constant determines reset pulse duration.
  
  delay_us(300000); // 300ms power-up delay for OLED module RC reset + init (MIOS32 LoopA: 300×1ms)
  
  // Only 3 signals from STM32 to OLED:
  // PA8 = DC (Data/Command select) - used by cmd() and data() functions
  // PC8 = SCL (Clock)
  // PC11 = SDA (Data)
  
  // ===== SSD1322 Initialization Sequence =====
  // EXACT copy from MIOS32 modules/app_lcd/ssd1322/app_lcd.c
  // https://github.com/midibox/mios32/blob/master/modules/app_lcd/ssd1322/app_lcd.c
  
  // 1. Unlock OLED driver IC MCU interface (0xFD)
  cmd(0xfd); data(0x12);  // Unlock commands (default locked after reset)
  
  // 2. Display OFF while configuring (0xAE)
  cmd(0xae);
  
  // 3. Set Column Address (0x15) - 0x1C to 0x5B for 256px width
  // Column address 28-91 (each address = 4 pixels, 64 addresses × 4 = 256 pixels)
  cmd(0x15); data(0x1c); data(0x5b);
  
  // 4. Set Row Address (0x75) - 0x00 to 0x3F for 64 rows
  cmd(0x75); data(0x00); data(0x3f);
  
  // 5. Set MUX Ratio (0xCA) - 64MUX
  cmd(0xca); data(0x3f);  // 64 rows
  
  // 6. Set Re-map / Color Depth (0xA0)
  // Byte 1: 0x14 = Enable Dual COM mode, column address 127 mapped to SEG0
  // Byte 2: 0x11 = Enable Dual COM Line mode
  cmd(0xa0); data(0x14); data(0x11);
  
  // 7. Set Display Clock Divide Ratio / Oscillator Frequency (0xB3)
  // CRITICAL: MIOS32 sends TWO SEPARATE data bytes, not one packed byte!
  cmd(0xb3);
  data(0);    // Clock divide ratio (A[3:0])
  data(12);   // Oscillator frequency (A[7:4])
  
  // 8. Set Segment Output Current (0xC1) - contrast
  cmd(0xc1); data(0xff);  // Maximum contrast
  
  // 9. Set Master Current Control (0xC7)
  cmd(0xc7); data(0x0f);  // Maximum (1/16)
  
  // 10. Select Default Linear Gray Scale table (0xB9)
  cmd(0xb9);
  // CRITICAL: MIOS32 sends 0x00 as a COMMAND, not data!
  cmd(0x00);  // Enable linear gray scale table
  
  // 11. Set Phase Length (0xB1)
  cmd(0xb1); data(0x56);  // Phase 1: 5 DCLKs, Phase 2: 6 DCLKs
  
  // 12. Set Pre-charge Voltage (0xBB)
  cmd(0xbb); data(0x0);   // 0.30 × VCC
  
  // 13. Set Second Pre-charge Period (0xB6)
  cmd(0xb6); data(0x08);  // 8 DCLKs
  
  // 14. Set VCOMH Voltage (0xBE)
  cmd(0xbe); data(0x00);  // 0.72 × VCC
  
  // 15. Normal Display (not inverted) (0xA6)
  // This is 0xa4 | 0x02 in MIOS32 code = 0xa6
  cmd(0xa4 | 0x02);  // 0xA6 = Normal display mode
  
  // 16. Clear display RAM before turning on (MIOS32 APP_LCD_Clear equivalent)
  // This matches the MIOS32 clear screen function exactly
  for (uint8_t row = 0; row < 64; row++) {
    // Set column address (start only, auto-increment enabled)
    cmd(0x15); data(0x1c);
    
    // Set row address
    cmd(0x75); data(row);
    
    // Write RAM command
    cmd(0x5c);
    
    // Send 128 bytes of zeros (256 pixels)
    for (uint16_t i = 0; i < 128; i++) {
      data(0x00);
    }
  }
  
  // 17. Display ON (0xAF)
  cmd(0xaf);
  
  // Wait for display to stabilize after wake-up
  delay_us(100000); // 100ms stabilization
  
  // 23. Send test pattern to verify display is working
  // White bar (4 rows) + gray fill (60 rows)
  memset(fb, 0xFF, 512);      // First 4 rows: white (0xFF)
  memset(fb + 512, 0x77, 7680); // Remaining 60 rows: medium gray (0x77)
  
  // Display test pattern for 1 second
  oled_flush();
  delay_us(1000000); // 1 second
  
  // Clear framebuffer for UI use
  memset(fb, 0x00, sizeof(fb));
}

void oled_flush(void) {
  // Transfer framebuffer to display RAM
  // CRITICAL: Match MIOS32 LoopA APP_LCD_Clear() row-by-row approach EXACTLY
  //
  // MIOS32 sends commands in this exact sequence per row:
  // 1. Set column address (0x15) with ONE data byte (start column only)
  // 2. Set row address (0x75) with ONE data byte (row number only)  
  // 3. Write RAM command (0x5C)
  // 4. Send pixel data
  //
  // This differs from typical init which sends start+end for both commands
  
  for (uint8_t row = 0; row < 64; row++) {
    // Set column start address - CRITICAL: Only ONE data byte!
    cmd(0x15);
    data(0x1C);  // Column 0x1C (28) - MIOS32: "data(0+0x1c)" - no end address
    
    // Set row address - CRITICAL: Only ONE data byte!
    cmd(0x75);
    data(row);  // Current row - MIOS32: "data(j)" - no end address
    
    // Write RAM command
    cmd(0x5C);
    
    // Send 128 bytes for this row (256 pixels at 4bpp)
    // MIOS32 sends pairs: "APP_LCD_Data(0); APP_LCD_Data(0);" per column
    uint8_t *row_data = &fb[row * 128];
    for (uint16_t i = 0; i < 128; i++) {
      data(row_data[i]);
    }
  }
}

uint8_t *oled_framebuffer(void) {
  return fb;
}

void oled_clear(void) {
  memset(fb, 0x00, sizeof(fb));
}
