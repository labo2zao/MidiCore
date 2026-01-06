
#include "Services/midi/midi_din.h"
#include "Hal/uart_midi/hal_uart_midi.h"
#include "Services/router/router.h"

static uint8_t running_status[4];

void midi_din_init(void) {
    hal_uart_midi_init();
    for (int i=0;i<4;i++) running_status[i]=0;
}

static void process_byte(uint8_t port, uint8_t b) {
    static router_msg_t msg;
    if (b & 0x80) {
        running_status[port] = b;
        msg.b0 = b;
        msg.type = ROUTER_MSG_1B;
        router_process(ROUTER_NODE_DIN_IN1 + port, &msg);
    } else if (running_status[port]) {
        msg.b0 = running_status[port];
        msg.b1 = b;
        msg.type = ROUTER_MSG_2B;
        router_process(ROUTER_NODE_DIN_IN1 + port, &msg);
    }
}

void midi_din_tick(void) {
    for (uint8_t p=0;p<4;p++) {
        uint8_t b;
        if (hal_uart_midi_rx_available(p) && hal_uart_midi_read_byte(p, &b)) {
            process_byte(p, b);
        }
    }
}
