#pragma once
#include <stdint.h>

#define OLED_W 256
#define OLED_H 64

void oled_init(void);
void oled_init_progressive(uint8_t max_step);  // For debugging
uint8_t* oled_framebuffer(void);
void oled_flush(void);
void oled_clear(void);
