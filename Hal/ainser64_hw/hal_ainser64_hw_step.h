#pragma once
#include <stdint.h>

/**
 * Efficient primitive: read one step (0..7) of one bank (0..7) -> 8 values (ADC channels 0..7).
 * This corresponds to 8 keys per call.
 */
int hal_ainser64_read_bank_step(uint8_t bank, uint8_t step, uint16_t out8[8]);

void hal_ainser64_init(void);
