#pragma once
#include <stdint.h>

#define OLED_W 256
#define OLED_H 64

void oled_init(void);
uint8_t* oled_framebuffer(void);
void oled_flush(void);
void oled_clear(void);
