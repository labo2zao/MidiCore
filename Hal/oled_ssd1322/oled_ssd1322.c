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

// MIOS32 uses GPIO write repetition for timing (not explicit delays!)
// Each GPIO write takes ~2-3 cycles @ 168 MHz
// MIOS32 timing: 5x SCLK_0 (setup) + 3x SCLK_1 (hold)
//
// For stability testing, we can slow down by adding more GPIO writes
#define SPI_TIMING_MIOS32      // EXACT MIOS32 timing (5 setup + 3 hold)
//#define SPI_TIMING_MEDIUM    // 2x slower (10 setup + 6 hold)
//#define SPI_TIMING_SLOW      // 4x slower (20 setup + 12 hold)

#ifdef SPI_TIMING_MEDIUM
  #define SPI_SETUP_REPS  10   // 2x slower than MIOS32
  #define SPI_HOLD_REPS   6
#elif defined(SPI_TIMING_SLOW)
  #define SPI_SETUP_REPS  20   // 4x slower than MIOS32
  #define SPI_HOLD_REPS   12
#else // SPI_TIMING_MIOS32 (default - EXACT MIOS32)
  #define SPI_SETUP_REPS  5    // EXACT MIOS32: 5x SCLK_0
  #define SPI_HOLD_REPS   3    // EXACT MIOS32: 3x SCLK_1
#endif

static inline void spi_write_byte(uint8_t byte) {
  // EXACT MIOS32 bit-bang sequence using GPIO write repetition for timing
  // MIOS32 does: 5x SCLK_0 (setup) + 3x SCLK_1 (hold) + 2x SCLK_0 (cleanup)
  // Clock starts at idle LOW, data is sampled on rising edge (Mode 0)
  
  for (uint8_t i = 0; i < 8; i++) {
    // 1. Set data line (MSB first)
    if (byte & 0x80)
      HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_SET);
    else
      HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_RESET);

    // 2. Setup time: MIOS32 does 5x SCLK_0 (5 GPIO writes with clock LOW)
    //    Each GPIO write provides natural delay from CPU instruction execution
    for (uint8_t j = 0; j < SPI_SETUP_REPS; j++) {
      SCL_LOW();
    }

    // 3. Hold time: MIOS32 does 3x SCLK_1 (3 GPIO writes with clock HIGH)
    //    Rising edge occurs on first SCLK_1 - display samples data here
    for (uint8_t j = 0; j < SPI_HOLD_REPS; j++) {
      SCL_HIGH();
    }

    byte <<= 1;  // shift to next bit (MSB first)
  }

  // 4. Cleanup: MIOS32 does 2x SCLK_0 to return clock to idle LOW state
  SCL_LOW();
  SCL_LOW();
}

// Send command (DC=0) byte
static void cmd(uint8_t c) {
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET);  // DC=0 (command mode)
  // No explicit delay - MIOS32 doesn't add delay between DC set and data shift
  spi_write_byte(c);
}

// Send data (DC=1) byte
static void data(uint8_t d) {
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET);  // DC=1 (data mode)
  // No explicit delay - MIOS32 doesn't add delay between DC set and data shift
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
  // MIOS32 INCLUDES cmd(0x00) after cmd(0xB9) despite "OLD" comment!
  // This is what actually works on hardware - following MIOS32 exactly
  cmd(0xB9);                  // Set_Linear_Gray_Scale_Table
  cmd(0x00);                  // Enable gray scale table (MIOS32 includes this!)
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

  // Step 14: Skip normal display mode for now (test later in step 15)
  // Just add a delay to verify step 13 worked
  if (max_step == 14) {
    cmd(0xAF); cmd(0xA5);
    return;
  }

  // Step 15: Full init with SIMPLE WHITE SCREEN TEST
  // EXACT MIOS32 SEQUENCE:
  // 1. Set normal display mode (0xA6)
  // 2. Clear/write to RAM (display still OFF)
  // 3. Turn display ON (0xAF)
  
  // Set normal display mode (0xA6) - display still OFF
  cmd(0xA4 | 0x02);  // Normal display mode (0xA6 - matches MIOS32)
  
  // Write white screen to RAM (MIOS32 does this while display is OFF!)
  // MIOS32 sequence: 1 byte address + 128 bytes data per row
  for (uint8_t row = 0; row < 64; ++row) {
    cmd(0x15);                 // Set Column Address
    data(0x1C);                // Column start ONLY (1 byte like MIOS32)
    cmd(0x75);                 // Set Row Address
    data(row);                 // Row start ONLY (1 byte like MIOS32)
    cmd(0x5C);                 // Write RAM command
    // CRITICAL: Data MUST immediately follow 0x5C per datasheet!
    // MIOS32 writes 128 bytes per row (64 iterations × 2 bytes)
    for (uint16_t i = 0; i < 64; ++i) {
      data(0xFF);              // White pixels
      data(0xFF);              // White pixels (128 bytes total)
    }
  }
  
  // NOW turn display ON (after RAM write - EXACT MIOS32 sequence!)
  cmd(0xAE | 1);  // Display ON (0xAF)

  delay_us(1000000);  // Wait 1 second to see white screen

  // Clear framebuffer for future use
  memset(fb, 0x00, sizeof(fb));
}

// MIOS32-compatible initialization - EXACT replica
// Source: midibox/mios32/modules/app_lcd/ssd1322/app_lcd.c APP_LCD_Init()
void oled_init(void) {
  // Ensure DWT cycle counter is initialized
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  // Set initial SPI lines for Mode 0 (CPOL=0, CPHA=0)
  SCL_LOW();  // Clock idle LOW
  HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET);

  // MIOS32: Wait 300ms for power stabilization
  for (uint16_t ctr = 0; ctr < 300; ++ctr) {
    delay_us(1000);
  }

  // EXACT MIOS32 APP_LCD_Init() sequence:
  cmd(0xfd); data(0x12);                          // Unlock
  cmd(0xae | 0);                                  // Display OFF
  cmd(0x15); data(0x1c); data(0x5b);             // Column Address
  cmd(0x75); data(0x00); data(0x3f);             // Row Address
  cmd(0xca); data(0x3f);                         // Multiplex Ratio
  cmd(0xa0); data(0x14); data(0x11);             // Remap Format
  cmd(0xb3); data(0); data(12);                  // Display Clock
  cmd(0xc1); data(0xff);                         // Contrast Current
  cmd(0xc7); data(0x0f);                         // Master Current
  cmd(0xb9);                                     // Linear Gray Scale Table
  cmd(0x00);                                     // Enable gray scale (MIOS32 has this!)
  cmd(0xb1); data(0x56);                         // Phase Length
  cmd(0xbb); data(0x0);                          // Precharge Voltage
  cmd(0xb6); data(0x08);                         // Precharge Period
  cmd(0xbe); data(0x00);                         // VCOMH
  cmd(0xa4 | 0x02);                              // Display Mode Normal

  // MIOS32: APP_LCD_Clear() - clear display while OFF
  for (uint8_t j = 0; j < 64; j++) {
    cmd(0x15); data(0 + 0x1c);
    cmd(0x75); data(j);
    cmd(0x5c);
    for (uint8_t i = 0; i < 64; i++) {
      data(0);
      data(0);
    }
  }

  cmd(0xae | 1);                                 // Display ON

  // Clear framebuffer for future use
  memset(fb, 0x00, sizeof(fb));
}

void oled_flush(void) {
  // Transfer the local framebuffer to OLED GDDRAM (SSD1322 VRAM)
  // EXACT MIOS32 sequence: 1 byte address + 128 bytes data per row
  for (uint8_t row = 0; row < 64; ++row) {
    cmd(0x15);                // Set Column Address
    data(0x1C);               // Column start ONLY (1 byte like MIOS32)
    cmd(0x75);                // Set Row Address  
    data(row);                // Row start ONLY (1 byte like MIOS32)
    cmd(0x5C);                // Write RAM command
    uint8_t *row_ptr = &fb[row * 128];  // 128 bytes per row
    for (uint16_t i = 0; i < 128; ++i) {
      data(row_ptr[i]);       // Write 128 bytes like MIOS32
    }
  }
}

uint8_t *oled_framebuffer(void) {
  return fb;
}

void oled_clear(void) {
  memset(fb, 0x00, sizeof(fb));
}

// MIOS32-compatible test screen function - EXACT replica
// Source: github.com/midibox/mios32/apps/mios32_test/app_lcd/ssd1322/app.c testScreen()
// Left half: gradient pattern, Right half: full white
// This test bypasses the framebuffer and writes directly to OLED RAM
void oled_test_mios32_pattern(void) {
  uint16_t x = 0;
  uint16_t y = 0;

  // EXACT MIOS32 testScreen() replication
  for (y = 0; y < 64; y++) {
    // APP_LCD_Cmd(0x15); APP_LCD_Data(0x1c);
    cmd(0x15);
    data(0x1c);

    // APP_LCD_Cmd(0x75); APP_LCD_Data(y);
    cmd(0x75);
    data(y);

    // APP_LCD_Cmd(0x5c);
    cmd(0x5c);

    // MIOS32: for (x = 0; x < 64; x++)
    for (x = 0; x < 64; x++) {
      if (x < 32) {
        // Left half: pattern
        // MIOS32 original: if (x || 4 == 0 || y || 4 == 0) - always true
        // APP_LCD_Data(y & 0x0f); APP_LCD_Data(0);
        data(y & 0x0f);
        data(0);
      } else {
        // Right half: full white
        // APP_LCD_Data(0xff); APP_LCD_Data(0xff);
        data(0xff);
        data(0xff);
      }
    }
  }

  // MIOS32: while(1); - wait forever to show pattern
  // We'll just return and let caller decide what to do
}
