
#pragma once
#include <stdint.h>

// Include main.h for portable STM32 HAL (F4/F7/H7 compatibility)
#include "main.h"

void hal_uart_midi_init(void);
void hal_uart_midi_send_byte(uint8_t port, uint8_t b);

// Convenience for sending an array (blocking).
HAL_StatusTypeDef hal_uart_midi_send_bytes(uint8_t port, const uint8_t* data, uint16_t len);
uint8_t hal_uart_midi_rx_available(uint8_t port);
uint8_t hal_uart_midi_read_byte(uint8_t port, uint8_t* b);

// Diagnostic: number of RX bytes dropped because the ring buffer was full.
uint32_t hal_uart_midi_rx_drops(uint8_t port);
