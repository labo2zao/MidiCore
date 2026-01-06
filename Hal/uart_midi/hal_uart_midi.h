
#pragma once
#include <stdint.h>

void hal_uart_midi_init(void);
void hal_uart_midi_send_byte(uint8_t port, uint8_t b);
uint8_t hal_uart_midi_rx_available(uint8_t port);
uint8_t hal_uart_midi_read_byte(uint8_t port, uint8_t* b);
