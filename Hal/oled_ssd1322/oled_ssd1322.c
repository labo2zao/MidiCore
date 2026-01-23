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
// CRITICAL: SPI Mode 0 (CPOL=0, CPHA=0) per MIOS32 convention
// - Clock idle = LOW (CPOL=0)
// - Data sampled on RISING edge (CPHA=0, first edge)
// - Data changes on FALLING edge
//
// MIOS32 uses dual clock pins (E1=PC8, E2=PC9) for dual COM mode
#define SCL_LOW() do { \
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET); \
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET); \
} while(0)

#define SCL_HIGH() do { \
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET); \
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET); \
} while(0)

// Precise delay using DWT cycle counter (better than NOPs)
// At 168 MHz: 1 cycle = 5.95 ns, so 17 cycles ≈ 100 ns
static inline void delay_cycles(uint32_t cycles) {
  uint32_t start = DWT->CYCCNT;
  while ((DWT->CYCCNT - start) < cycles);
}

static inline void spi_write_byte(uint8_t byte) {
  // MIOS32-compatible bit-bang sequence
  // Clock starts at idle LOW, data is sampled on rising edge (Mode 0)
  for (uint8_t i = 0; i < 8; i++) {
    // 1. Set data line (MSB first)
    if (byte & 0x80)
      HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_SET);
    else
      HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_RESET);

    // 2. Keep clock LOW for data setup time
    //    MIOS32 does 5x SCLK_0 calls for setup delay
    SCL_LOW();  // ensure clock is LOW
    delay_cycles(17);  // 17 cycles @ 168 MHz ≈ 101 ns setup time

    // 3. Clock LOW→HIGH (rising edge) - SSD1322 samples on this edge
    //    MIOS32 does 3x SCLK_1 calls for hold time
    SCL_HIGH();
    delay_cycles(17);  // 17 cycles @ 168 MHz ≈ 101 ns hold time

    byte <<= 1;  // shift to next bit (MSB first)
  }

  // 4. Return clock to idle LOW state (as MIOS32 does at end of SerDataShift)
  SCL_LOW();
  SCL_LOW();  // MIOS32 does this twice for safety
}

// Send command (DC=0) byte
static void cmd(uint8_t c) {
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET);  // DC=0 (command mode)
  delay_cycles(10);  // 10 cycles @ 168 MHz ≈ 60 ns for DC stabilization
  spi_write_byte(c);
}

// Send data (DC=1) byte
static void data(uint8_t d) {
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET);  // DC=1 (data mode)
  delay_cycles(10);  // 10 cycles @ 168 MHz ≈ 60 ns for DC stabilization
  spi_write_byte(d);
}

// Helper: send command with two data bytes
static void cmd_data2(uint8_t c, uint8_t d1, uint8_t d2) {
  cmd(c);
  data(d1);
  data(d2);
}

// Progressive initialization for debugging - stops at specified step
// step: 0-15 (0=minimal, 15=full init)
void oled_init_progressive(uint8_t max_step) {
  // Ensure DWT cycle counter is initialized (used by delay_cycles)
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  // Set initial SPI lines states for Mode 0 (CPOL=0, CPHA=0):
  // CRITICAL: MIOS32 uses SPI Mode 0, clock must idle LOW (not HIGH)
  SCL_LOW();  // clock idle low (required for Mode 0)
  HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_RESET);  // data line low
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET);      // DC high (defaults to data mode)

  // Delay to allow OLED power supply to stabilize (min 100 ms, using 300 ms for safety)
  for (uint16_t ctr = 0; ctr < 300; ++ctr) {
    delay_us(1000);  // 300 ms total
  }

  // The OLED module uses an on-board RC reset circuit – no direct RST control from MCU.
  // After power-up, the module's RC circuit holds RST low then high automatically.
  // Wait an additional 300 ms for the reset cycle to complete and VDD to stabilize.
  delay_us(300000);  // 300 ms

  // Progressive initialization - add one command at a time
  // Step 0: Minimal (unlock + display ON + all pixels ON)
  cmd(0xFD); data(0x12);       // Unlock OLED driver IC
  if (max_step == 0) {
    cmd(0xAF);                 // Display ON
    cmd(0xA5);                 // All pixels ON (test pattern)
    return;
  }

  // Step 1: + Display OFF before config
  cmd(0xAE);                  // Display OFF during configuration
  if (max_step == 1) {
    cmd(0xAF);                 // Display ON
    cmd(0xA5);                 // All pixels ON
    return;
  }

  // Step 2: + Column Address
  // CRITICAL: During INIT, MIOS32 sends 2 bytes to set full window range!
  cmd(0x15); data(0x1C); data(0x5B);  // Set Column Address (start=0x1C, end=0x5B)
  if (max_step == 2) {
    cmd(0xAF); cmd(0xA5);
    return;
  }

  // Step 3: + Row Address  
  // CRITICAL: During INIT, MIOS32 sends 2 bytes to set full window range!
  cmd(0x75); data(0x00); data(0x3F);  // Set Row Address (start=0x00, end=0x3F)
  if (max_step == 3) {
    cmd(0xAF); cmd(0xA5);
    return;
  }

  // Step 4: + MUX ratio
  cmd(0xCA); data(0x3F);       // Set MUX ratio to 64
  if (max_step == 4) {
    cmd(0xAF); cmd(0xA5);
    return;
  }

  // Step 5: + Remap (dual COM mode)
  cmd(0xA0); data(0x14); data(0x11);  // Set Remap
  if (max_step == 5) {
    cmd(0xAF); cmd(0xA5);
    return;
  }

  // Step 6: + Display Clock
  cmd(0xB3); data(0x00); data(0x0C);  // Set Display Clock
  if (max_step == 6) {
    cmd(0xAF); cmd(0xA5);
    return;
  }

  // Step 7: + Segment Output Current (contrast)
  cmd(0xC1); data(0xFF);       // Set Segment Output Current to max
  if (max_step == 7) {
    cmd(0xAF); cmd(0xA5);
    return;
  }

  // Step 8: + Master Current Control
  cmd(0xC7); data(0x0F);       // Master Current Control to max
  if (max_step == 8) {
    cmd(0xAF); cmd(0xA5);
    return;
  }

  // Step 9: + Linear gray scale table
  cmd(0xB9);                  // Use default linear gray scale table
  cmd(0x00);                  // (Send 0x00 as command to enable linear table)
  if (max_step == 9) {
    cmd(0xAF); cmd(0xA5);
    return;
  }

  // Step 10: + Phase Length
  cmd(0xB1); data(0x56);       // Phase Length
  if (max_step == 10) {
    cmd(0xAF); cmd(0xA5);
    return;
  }

  // Step 11: + Pre-charge Voltage
  cmd(0xBB); data(0x00);       // Pre-charge Voltage
  if (max_step == 11) {
    cmd(0xAF); cmd(0xA5);
    return;
  }

  // Step 12: + Second Pre-charge Period
  cmd(0xB6); data(0x08);       // Second Pre-charge Period
  if (max_step == 12) {
    cmd(0xAF); cmd(0xA5);
    return;
  }

  // Step 13: + VCOMH Voltage
  cmd(0xBE); data(0x00);       // Set VCOMH Voltage
  if (max_step == 13) {
    cmd(0xAF); cmd(0xA5);
    return;
  }

  // Step 14: + Normal Display mode
  cmd(0xA4 | 0x02);           // Normal Display mode (0xA6)
  if (max_step == 14) {
    cmd(0xAF); cmd(0xA5);
    return;
  }

  // Step 15: Full init with SIMPLE WHITE SCREEN TEST
  // CRITICAL: Exit test mode (0xA5) before writing to RAM!
  cmd(0xA4 | 0x02);  // Normal display mode (0xA6 - matches MIOS32 exactly)
  
  // ULTRA-SIMPLE TEST: Fill entire screen with WHITE
  // This bypasses framebuffer complexity and tests RAM write directly
  for (uint8_t row = 0; row < 64; ++row) {
    cmd(0x15);                 // Set Column Address
    data(0x1C);                // Column start ONLY (MIOS32 style - 1 byte)
    cmd(0x75);                 // Set Row Address
    data(row);                 // Row start ONLY (MIOS32 style - 1 byte)
    cmd(0x5C);                 // Write RAM command
    for (uint16_t i = 0; i < 64; ++i) {
      data(0xFF);              // White pixels
      data(0xFF);              // White pixels
    }
  }

  // CRITICAL: MIOS32 sends Display ON AFTER writing RAM!
  cmd(0xAE | 1);  // Display ON (0xAF - matches MIOS32 exactly)

  delay_us(2000000);  // Wait 2 seconds to see white screen

  // Clear framebuffer for future use
  memset(fb, 0x00, sizeof(fb));
}

void oled_init(void) {
  // Call full progressive init (step 15 = complete sequence)
  oled_init_progressive(15);
}

void oled_flush(void) {
  // Transfer the local framebuffer to OLED GDDRAM (SSD1322 VRAM)
  // Send data row by row, matching MIOS32 sequence EXACTLY
  // CRITICAL: MIOS32 sends ONLY ONE data byte for 0x15/0x75, NOT two!
  // This contradicts datasheet but is what actually works with SSD1322
  for (uint8_t row = 0; row < 64; ++row) {
    cmd(0x15);                // Set Column Address
    data(0x1C);               // Column start ONLY (MIOS32 doesn't send end!)
    cmd(0x75);                // Set Row Address  
    data(row);                // Row start ONLY (MIOS32 doesn't send end!)
    cmd(0x5C);                // Write RAM command (begin data write)
    uint8_t *row_ptr = &fb[row * 128];
    for (uint16_t i = 0; i < 128; ++i) {
      data(row_ptr[i]);       // Write 128 bytes of pixel data for this row
    }
  }
}

uint8_t *oled_framebuffer(void) {
  return fb;
}

void oled_clear(void) {
  memset(fb, 0x00, sizeof(fb));
}

// MIOS32-compatible test screen function
// Matches testScreen() from github.com/midibox/mios32/apps/mios32_test/app_lcd/ssd1322/app.c
// Left half: gradient pattern, Right half: full white
// This test bypasses the framebuffer and writes directly to OLED RAM
void oled_test_mios32_pattern(void) {
  uint16_t x = 0;
  uint16_t y = 0;

  // Render test screen exactly like MIOS32
  for (y = 0; y < 64; y++) {
    cmd(0x15);                    // Set Column Address
    data(0x1C);                   // Column start (MIOS32 sends only 1 byte)

    cmd(0x75);                    // Set Row Address
    data(y);                      // Row start (MIOS32 sends only 1 byte)

    cmd(0x5C);                    // Write RAM command

    for (x = 0; x < 64; x++) {    // 64 column pairs = 128 bytes = 256 pixels
      if (x < 32) {               // Left half: pattern
        // MIOS32 pattern: gradient based on position
        // Original MIOS32 code had: if (x || 4 == 0 || y || 4 == 0)
        // This is always true (logical OR with constants), creating gradient
        data(y & 0x0F);           // First pixel: gradient based on Y
        data(0x00);               // Second pixel: black
      } else {                    // Right half: full white
        data(0xFF);               // First pixel: white
        data(0xFF);               // Second pixel: white
      }
    }
  }
}
