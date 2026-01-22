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
  // Adapted from MIOS32 LoopA with datasheet references
  
  // 1. Set Command Lock (0xFD)
  // Unlock driver IC MCU interface (default after reset: locked)
  cmd(0xFD); data(0x12);  // 0x12 = Unlock, 0x16 = Lock
  
  // 2. Display OFF (0xAE) - display must be off during config
  cmd(0xAE);
  
  // 3. Set Display Start Line (0xA1) - row 0
  cmd(0xA1); data(0x00);
  
  // 4. Set Display Offset (0xA2) - no offset
  cmd(0xA2); data(0x00);
  
  // 5. Set Re-map and Dual COM Line mode (0xA0)
  // [1:0] = 00: Horizontal address increment
  // [2]   = 1:  Column address 0 mapped to SEG0
  // [4]   = 1:  Enable Dual COM mode (required for 64-row display)
  // [5]   = 0:  Scan from COM0 to COM[N-1]
  // [6]   = 1:  Enable COM Split Odd Even
  cmd_data2(0xA0, 0x14, 0x11);
  
  // 6. Set MUX Ratio (0xCA) - 64 rows (0x3F = 63, but counts from 0)
  cmd(0xCA); data(0x3F);
  
  // 7. Function Selection (0xAB) - Enable internal VDD regulator
  cmd(0xAB); data(0x01);  // 0x01 = Enable internal VDD regulator
  
  // 8. Set Display Clock Divide Ratio / Oscillator Frequency (0xB3)
  // MIOS32 sends TWO separate data bytes: divide ratio, then oscillator
  cmd(0xB3); 
  data(0);    // Clock divide ratio: 0 (divide by 1)
  data(12);   // Oscillator frequency: 12
  
  // 9. Set Display Enhancement A (0xB4)
  // External VSL, Enhanced low GS display quality
  cmd_data2(0xB4, 0xA0, 0xFD);
  
  // 10. Set GPIO (0xB5) - both pins HiZ, input disabled
  cmd(0xB5); data(0x00);
  
  // 11. Set Second Pre-charge Period (0xB6)
  cmd(0xB6); data(0x08);  // 8 DCLKs
  
  // 12. Set Gray Scale Table (0xB9) - Use default linear gray scale
  cmd(0xB9);
  
  // 13. Set Pre-charge voltage (0xBB)
  cmd(0xBB); data(0x00);  // 0.30 × VCC (LoopA setting)
  
  // 14. Set VCOMH Voltage (0xBE)
  cmd(0xBE); data(0x00);  // 0.72 × VCC (LoopA setting)
  
  // 15. Set Contrast Current (0xC1)
  cmd(0xC1); data(0xFF);  // Maximum contrast
  
  // 16. Master Contrast Current Control (0xC7)
  cmd(0xC7); data(0x0F);  // Maximum (no reduction)
  
  // 17. Set Display Enhancement B (0xD1)
  // Normal enhancement, low GS quality
  cmd_data2(0xD1, 0x82, 0x20);
  
  // 18. Set Phase Length (0xB1)
  // [3:0] = Phase 1 period (reset phase length)
  // [7:4] = Phase 2 period (first pre-charge phase length)
  cmd(0xB1); data(0x56);  // Phase 1 = 6 DCLKs, Phase 2 = 5 DCLKs (LoopA)
  
  // 19. Display Mode Normal (0xA6)
  // 0xA4 = All OFF, 0xA5 = All ON, 0xA6 = Normal, 0xA7 = Inverse
  cmd(0xA6);
  
  // 20. Exit Partial Display (0xA9) - use full display
  cmd(0xA9);
  
  // 21. Clear display RAM to prevent garbage
  // Set column address 0x1C-0x5B (28-91 for 256px display)
  cmd(0x15); data(0x1C); data(0x5B);
  // Set row address 0-63
  cmd(0x75); data(0x00); data(0x3F);
  // Write RAM command
  cmd(0x5C);
  // Clear all pixels (send zeros)
  for (uint16_t i = 0; i < 8192; i++) {
    data(0x00);
  }
  
  // 22. Display ON (0xAF) - CRITICAL: Turn display on BEFORE sending test pattern
  cmd(0xAF);
  
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
