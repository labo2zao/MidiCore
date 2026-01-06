
#include "Hal/uart_midi/hal_uart_midi.h"
#include "stm32f4xx_hal.h"

// Expected CubeMX handles
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart5;

static UART_HandleTypeDef* ports[4] = {
    &huart1, &huart2, &huart3, &huart5
};

static uint8_t rx_buf[4];
static uint8_t rx_ready[4];

void hal_uart_midi_init(void) {
    for (int i=0;i<4;i++) rx_ready[i]=0;
}

void hal_uart_midi_send_byte(uint8_t port, uint8_t b) {
    if (port >= 4) return;
    HAL_UART_Transmit(ports[port], &b, 1, 10);
}

uint8_t hal_uart_midi_rx_available(uint8_t port) {
    if (port >= 4) return 0;
    if (!rx_ready[port]) {
        HAL_UART_Receive_IT(ports[port], &rx_buf[port], 1);
    }
    return rx_ready[port];
}

uint8_t hal_uart_midi_read_byte(uint8_t port, uint8_t* b) {
    if (port >= 4 || !rx_ready[port]) return 0;
    *b = rx_buf[port];
    rx_ready[port] = 0;
    return 1;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    for (int i=0;i<4;i++) {
        if (huart == ports[i]) {
            rx_ready[i] = 1;
        }
    }
}
