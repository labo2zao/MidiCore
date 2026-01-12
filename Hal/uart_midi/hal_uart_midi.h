#pragma once
#include <stdint.h>
#include "stm32f4xx_hal.h"

// Initialize UART MIDI backend (sets up RX ring + starts interrupts).
HAL_StatusTypeDef hal_uart_midi_init(void);

// Blocking send of a single byte on given MIDI port.
HAL_StatusTypeDef hal_uart_midi_send_byte(uint8_t port, uint8_t byte);

// Convenience for sending an array (blocking).
HAL_StatusTypeDef hal_uart_midi_send_bytes(uint8_t port, const uint8_t* data, uint16_t len);

// Non-blocking RX API: returns non-zero if at least one byte is available.
int hal_uart_midi_available(uint8_t port);

// Read one byte from RX ring buffer (0 if none available).
uint8_t hal_uart_midi_read_byte(uint8_t port);

// Diagnostic: number of RX bytes dropped because the ring buffer was full.
uint32_t hal_uart_midi_rx_drops(uint8_t port);
