#pragma once
// Helper macros to place large zero-initialized buffers into CCMRAM.
// This mirrors MIOS32's use of fast tightly coupled memory for bulk state
// buffers so main SRAM stays available for the kernel and stacks.

#ifndef CCM_BSS
#define CCM_BSS __attribute__((section(".ccmram")))
#endif

