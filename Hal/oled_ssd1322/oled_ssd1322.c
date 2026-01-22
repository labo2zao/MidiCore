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
// CRITICAL: SPI Mode 3 (CPOL=1, CPHA=1) per SSD1322 datasheet
// - Clock idle = HIGH (CPOL=1)
// - Data sampled on FALLING edge (CPHA=1, second edge)
// - Data MUST be stable *before* the falling edge
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

static inline void spi_write_byte(uint8_t byte) {
  for (uint8_t i = 0; i < 8; i++) {
    // 1. Set data line first (MSB of byte) – must be stable before clock falls
    if (byte & 0x80)
      HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_SET);
    else
      HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_RESET);

    // 2. Clock HIGH→LOW (falling edge) - SSD1322 samples on this falling edge
    //    Stretch (~8 NOPs) for data setup time before sampling
    SCL_LOW();
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP__;

    // 3. Clock LOW→HIGH (rising edge) - return clock to idle HIGH (CPOL=1)
    //    Stretch (~8 NOPs) for data hold time after sampling
    SCL_HIGH();
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP__;

    byte <<= 1;  // shift to next bit (MSB first)
  }

  SCL_HIGH();  // ensure clock remains high between bytes (idle state for Mode 3)
}

// Send command (DC=0) byte
static void cmd(uint8_t c) {
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET);  // DC=0 (command mode)
  __NOP(); __NOP(); __NOP(); __NOP();  // small delay to let DC stabilize
  spi_write_byte(c);
}

// Send data (DC=1) byte
static void data(uint8_t d) {
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET);  // DC=1 (data mode)
  __NOP(); __NOP(); __NOP(); __NOP();  // let DC line settle
  spi_write_byte(d);
}

// Helper: send command with two data bytes
static void cmd_data2(uint8_t c, uint8_t d1, uint8_t d2) {
  cmd(c);
  data(d1);
  data(d2);
}

void oled_init(void) {
  // Set initial SPI lines states for Mode 3 (CPOL=1, CPHA=1):
  SCL_HIGH();  // clock idle high
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

  // OLED connections: PA8 = DC, PC8 = SCL, PC11 = SDA (CS tied to GND)
  // Following SSD1322 init sequence from MIOS32 (LoopA):
  cmd(0xFD); data(0x12);       // 1. Unlock OLED driver IC (was locked after reset)
  cmd(0xAE);                  // 2. Display OFF during configuration
  cmd(0x15); data(0x1C); data(0x5B);  // 3. Set Column Address 0x1C to 0x5B (256px)
  cmd(0x75); data(0x00); data(0x3F);  // 4. Set Row Address 0x00 to 0x3F (64 rows)
  cmd(0xCA); data(0x3F);       // 5. Set MUX ratio to 64 (0x3F)
  cmd(0xA0); data(0x14); data(0x11);  // 6. Set Remap (dual COM mode, dual COM line)
  cmd(0xB3); data(0x00); data(0x0C);  // 7. Set Display Clock Div and Oscillator freq
  cmd(0xC1); data(0xFF);       // 8. Set Segment Output Current (contrast) to max
  cmd(0xC7); data(0x0F);       // 9. Master Current Control to max
  cmd(0xB9);                  // 10. Use default linear gray scale table
  cmd(0x00);                  //     (Send 0x00 as command to enable linear table)
  cmd(0xB1); data(0x56);       // 11. Phase Length (front 5 DCLK, second 6 DCLK)
  cmd(0xBB); data(0x00);       // 12. Pre-charge Voltage = 0.30 * VCC
  cmd(0xB6); data(0x08);       // 13. Second Pre-charge Period = 8 DCLKs
  cmd(0xBE); data(0x00);       // 14. Set VCOMH Voltage = 0.72 * VCC
  cmd(0xA4 | 0x02);           // 15. Normal Display mode (0xA6)

  // 16. Clear display RAM (fill all pixels with 0) before turning display on
  for (uint8_t row = 0; row < 64; ++row) {
    cmd(0x15); data(0x1C);     // set column start address
    cmd(0x75); data(row);      // set current row address
    cmd(0x5C);                // write RAM command
    for (uint16_t i = 0; i < 128; ++i) {
      data(0x00);             // write 128 bytes of 0x00 (256 pixels of 0)
    }
  }

  cmd(0xAF);  // 17. Display ON

  delay_us(100000);  // wait 100 ms after turning display on

  // Show test pattern (white bar on top 4 rows, gray on rest) for 1s
  memset(fb, 0xFF, 512);         // first 4 rows (512 bytes) = 0xFF (white)
  memset(fb + 512, 0x77, 7680);  // remaining 60 rows = 0x77 (medium gray)
  oled_flush();
  delay_us(1000000);  // 1 second delay

  // Clear the test pattern from framebuffer for normal UI use
  memset(fb, 0x00, sizeof(fb));
}

void oled_flush(void) {
  // Transfer the local framebuffer to OLED GDDRAM (SSD1322 VRAM)
  // Send data row by row, as done in MIOS32 LoopA (single-byte column & row addresses)
  for (uint8_t row = 0; row < 64; ++row) {
    cmd(0x15); data(0x1C);    // set column start (only) to 0x1C
    cmd(0x75); data(row);     // set current row (only)
    cmd(0x5C);               // write RAM command (begin data write)
    uint8_t *row_ptr = &fb[row * 128];
    for (uint16_t i = 0; i < 128; ++i) {
      data(row_ptr[i]);      // write 128 bytes of pixel data for this row
    }
  }
}

uint8_t *oled_framebuffer(void) {
  return fb;
}

void oled_clear(void) {
  memset(fb, 0x00, sizeof(fb));
}
