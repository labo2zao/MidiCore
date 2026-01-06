#pragma once
#include <stdint.h>
#include "Services/patch/patch_adv.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Apply router configuration from patch entries.
 *
 * Convention:
 * - Section: [router]
 * - Entries with key starting "route" are parsed as route definitions.
 *   Value format examples:
 *     DIN_IN1->DIN_OUT2
 *     DIN1->OUT2
 *     USB_IN->DIN_OUT1
 *     DIN_IN1->USB_OUT
 *
 * Optional channel selection appended after space:
 *     DIN1->OUT2 ch=1..4,6,10..12
 *
 * Conditions:
 * - Use key?cond=value (cond is evaluated using ctx)
 * - If cond evaluates false, route is skipped.
 */
void patch_router_apply(const patch_ctx_t* ctx);

#ifdef __cplusplus
}
#endif
