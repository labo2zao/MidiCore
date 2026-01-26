// HAL UART backend for MIDI DIN.
//
// Design goals (STM32F4/F7 portable):
// - interrupt-driven RX with a small ring buffer per port
// - simple blocking TX (good enough for bring-up/testing)
// - allow selecting the primary DIN UART via TEST_MIDI_DIN_UART_PORT

#include "hal_uart_midi.h"

#include <string.h>

#include "main.h" // UART handle declarations

// ---- Port mapping -----------------------------------------------------------
// IMPORTANT:
// MIOS32 Hardware Configuration (MIDIbox STM32F4):
//   Debug UART: PA2/PA3 (USART2) @ 115200 baud in test mode
//
//   MIDI IN1:  PA9  (USART1 RX)
//   MIDI OUT1: PA10 (USART1 TX)
//   MIDI IN2:  PD9  (USART3 RX)
//   MIDI OUT2: PD8  (USART3 TX)
//   MIDI IN3:  PD2  (UART5 RX)
//   MIDI OUT3: PC12 (UART5 TX)
//   MIDI IN4:  PB7
//   MIDI OUT4: PC6
//
// Port Mapping:
//   Port 0 (DIN1, primary) -> USART1 (huart1) PA10=TX, PA9=RX   [MIDI OUT1/IN1]
//   Port 1 (DIN2)          -> USART3 (huart3) PD8=TX,  PD9=RX   [MIDI OUT2/IN2]
//   Port 2 (DIN3)          -> UART5  (huart5) PC12=TX, PD2=RX   [MIDI OUT3/IN3]
//
// Note: USART2 (PA2/PA3) is used for debug output @ 115200 in test mode

#ifndef MIDI_DIN_PORTS
#define MIDI_DIN_PORTS 4
#endif

extern UART_HandleTypeDef huart1; // USART1
extern UART_HandleTypeDef huart2; // USART2
extern UART_HandleTypeDef huart3; // USART3
extern UART_HandleTypeDef huart5; // UART5

#ifndef MIDI_DIN_PRIMARY_UART_PORT
#ifdef TEST_MIDI_DIN_UART_PORT
#define MIDI_DIN_PRIMARY_UART_PORT TEST_MIDI_DIN_UART_PORT
#else
#define MIDI_DIN_PRIMARY_UART_PORT 0  // Default to Port 0 (USART1/PA9-PA10)
#endif
#endif

static UART_HandleTypeDef* midi_uart_from_index(uint8_t idx)
{
  switch (idx) {
    case 0: return &huart1;   // USART1: PA10=TX, PA9=RX  (MIDI Port 0 - DIN1)
    case 1: return &huart3;   // USART3: PD8=TX,  PD9=RX  (MIDI Port 1 - DIN2)
    case 2: return &huart5;   // UART5:  PC12=TX, PD2=RX  (MIDI Port 2 - DIN3)
    case 3: return NULL;      // Port 3 not configured (DIN4 would need additional UART)
    default: return NULL;
  }
}

static UART_HandleTypeDef* s_midi_uarts[MIDI_DIN_PORTS];

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
  // Map MIDI ports to UART handles
  // Port 0 (DIN1) uses MIDI_DIN_PRIMARY_UART_PORT index (default: 0 = USART1/PA9-PA10)
  s_midi_uarts[0] = midi_uart_from_index(MIDI_DIN_PRIMARY_UART_PORT);
  s_midi_uarts[1] = midi_uart_from_index(1); // Port 1 -> USART3/PD8-PD9
  s_midi_uarts[2] = midi_uart_from_index(2); // Port 2 -> UART5/PC12-PD2
  s_midi_uarts[3] = midi_uart_from_index(3); // Port 3 -> Not configured

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
