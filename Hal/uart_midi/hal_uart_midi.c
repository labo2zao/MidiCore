// HAL UART backend for MIDI DIN.
//
// Design goals (STM32F4/F7 portable):
// - interrupt-driven RX with a small ring buffer per port
// - simple blocking TX (good enough for bring-up/testing)
// - keep USART1 free for debug UART (115200)

#include "hal_uart_midi.h"

#include <string.h>

#include "main.h" // UART handle declarations

// ---- Port mapping -----------------------------------------------------------
// IMPORTANT:
// In this project, USART1 is used as the debug UART (115200). Do NOT map a MIDI
// DIN port to USART1, or you'll get no MIDI events.
//
// We map DIN ports to the UARTs that are configured at 31250 in Core/Src/main.c.
// Adjust here if you change CubeMX mappings.

extern UART_HandleTypeDef huart2; // USART2 (31250)
extern UART_HandleTypeDef huart3; // USART3 (31250)
extern UART_HandleTypeDef huart5; // UART5  (31250)

static UART_HandleTypeDef* const s_midi_uarts[MIDI_DIN_PORTS] = {
    &huart3, // Port 0 -> MIDI2 (USART3: PD8/PD9)
    &huart5, // Port 1 -> MIDI4 (UART5: PC12/PD2)
    &huart2, // Port 2 -> USART2 (PA2/PA3) (optional)
    NULL,    // Port 3 -> unused for now
};

// ---- RX ring buffer ---------------------------------------------------------

#define RX_RING_SIZE 256u

typedef struct {
  volatile uint16_t head;
  volatile uint16_t tail;
  volatile uint32_t drops;
  uint8_t ring[RX_RING_SIZE];
  uint8_t rx_byte; // single-byte ISR buffer
} midi_uart_rx_t;

static midi_uart_rx_t s_rx[MIDI_DIN_PORTS];

static inline uint16_t ring_next(uint16_t v) { return (uint16_t)((v + 1u) & (RX_RING_SIZE - 1u)); }

// Ensure RX_RING_SIZE is power-of-two for the mask above
_Static_assert((RX_RING_SIZE & (RX_RING_SIZE - 1u)) == 0u, "RX_RING_SIZE must be power of 2");

static int port_from_handle(UART_HandleTypeDef* huart)
{
  for (int i = 0; i < MIDI_DIN_PORTS; ++i) {
    if (s_midi_uarts[i] == huart) return i;
  }
  return -1;
}

static void start_rx_it(int port)
{
  UART_HandleTypeDef* huart = s_midi_uarts[port];
  if (!huart) return;

  // Restart 1-byte receive
  (void)HAL_UART_Receive_IT(huart, &s_rx[port].rx_byte, 1);
}

HAL_StatusTypeDef hal_uart_midi_init(void)
{
  memset(s_rx, 0, sizeof(s_rx));

  for (int p = 0; p < MIDI_DIN_PORTS; ++p) {
    start_rx_it(p);
  }
  return HAL_OK;
}

int hal_uart_midi_available(uint8_t port)
{
  if (port >= MIDI_DIN_PORTS || s_midi_uarts[port] == NULL) return 0;
  return s_rx[port].head != s_rx[port].tail;
}

uint8_t hal_uart_midi_read_byte(uint8_t port)
{
  if (port >= MIDI_DIN_PORTS || s_midi_uarts[port] == NULL) return 0;

  midi_uart_rx_t* r = &s_rx[port];
  if (r->head == r->tail) return 0;

  uint8_t b = r->ring[r->tail];
  r->tail = ring_next(r->tail);
  return b;
}

HAL_StatusTypeDef hal_uart_midi_send_byte(uint8_t port, uint8_t byte)
{
  if (port >= MIDI_DIN_PORTS || s_midi_uarts[port] == NULL) return HAL_ERROR;
  return HAL_UART_Transmit(s_midi_uarts[port], &byte, 1, 10);
}

HAL_StatusTypeDef hal_uart_midi_send_bytes(uint8_t port, const uint8_t* data, uint16_t len)
{
  if (port >= MIDI_DIN_PORTS || s_midi_uarts[port] == NULL) return HAL_ERROR;
  if (!data || len == 0) return HAL_OK;
  return HAL_UART_Transmit(s_midi_uarts[port], (uint8_t*)data, len, 50);
}

uint32_t hal_uart_midi_rx_drops(uint8_t port)
{
  if (port >= MIDI_DIN_PORTS) return 0;
  return s_rx[port].drops;
}

// ---- HAL callbacks ----------------------------------------------------------
// NOTE: These are global callbacks invoked by stm32f4xx_hal_uart.c.

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
  int p = port_from_handle(huart);
  if (p < 0) return;

  midi_uart_rx_t* r = &s_rx[p];
  uint16_t next = ring_next(r->head);
  if (next == r->tail) {
    r->drops++;
  } else {
    r->ring[r->head] = r->rx_byte;
    r->head = next;
  }

  start_rx_it(p);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef* huart)
{
  // If an error occurs (noise, framing, overrun), restart RX.
  int p = port_from_handle(huart);
  if (p < 0) return;
  start_rx_it(p);
}
