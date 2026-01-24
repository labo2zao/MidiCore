// SPDX-License-Identifier: MIT

/**
 * @file footswitch.c
 * @brief Footswitch input service implementation
 */

#include "Services/footswitch/footswitch.h"
#include "Config/footswitch_pins.h"
#include "Config/module_config.h"
#include "main.h"

#include <string.h>

// =============================================================================
// Private Types
// =============================================================================

typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
} footswitch_gpio_pin_t;

// =============================================================================
// Private Variables
// =============================================================================

static bool g_initialized = false;
static footswitch_callback_t g_callback = NULL;

// Debouncing state
static uint8_t g_button_state[FOOTSWITCH_NUM_SWITCHES];  // Current debounced state (0=released, 1=pressed)
static uint8_t g_debounce_counter[FOOTSWITCH_NUM_SWITCHES];
static const uint8_t DEBOUNCE_THRESHOLD = 3;  // 3 consistent reads (30ms at 10ms scan rate)

#ifndef FOOTSWITCH_USE_SRIO
// GPIO mode: Pin mapping
static const footswitch_gpio_pin_t g_gpio_pins[FOOTSWITCH_NUM_SWITCHES] = {
    {FOOTSWITCH_GPIO_FS0_PORT, FOOTSWITCH_GPIO_FS0_PIN},
    {FOOTSWITCH_GPIO_FS1_PORT, FOOTSWITCH_GPIO_FS1_PIN},
    {FOOTSWITCH_GPIO_FS2_PORT, FOOTSWITCH_GPIO_FS2_PIN},
    {FOOTSWITCH_GPIO_FS3_PORT, FOOTSWITCH_GPIO_FS3_PIN},
    {FOOTSWITCH_GPIO_FS4_PORT, FOOTSWITCH_GPIO_FS4_PIN},
    {FOOTSWITCH_GPIO_FS5_PORT, FOOTSWITCH_GPIO_FS5_PIN},
    {FOOTSWITCH_GPIO_FS6_PORT, FOOTSWITCH_GPIO_FS6_PIN},
    {FOOTSWITCH_GPIO_FS7_PORT, FOOTSWITCH_GPIO_FS7_PIN}
};
#endif

// =============================================================================
// Private Functions
// =============================================================================

#ifdef FOOTSWITCH_USE_SRIO
/**
 * @brief Read 8 bits from 74HC165 shift register using bit-bang SPI
 * @return 8-bit value from shift register
 */
static uint8_t footswitch_srio_read_byte(void)
{
    uint8_t result = 0;
    
    // Pulse /PL low to latch parallel inputs
    HAL_GPIO_WritePin(FOOTSWITCH_SRIO_PL_PORT, FOOTSWITCH_SRIO_PL_PIN, GPIO_PIN_RESET);
    for(volatile int i=0; i<10; i++); // Short delay
    HAL_GPIO_WritePin(FOOTSWITCH_SRIO_PL_PORT, FOOTSWITCH_SRIO_PL_PIN, GPIO_PIN_SET);
    for(volatile int i=0; i<10; i++); // Short delay
    
    // Clock out 8 bits
    for (uint8_t bit = 0; bit < 8; bit++) {
        // Read current bit on MISO
        GPIO_PinState bit_val = HAL_GPIO_ReadPin(FOOTSWITCH_SRIO_MISO_PORT, FOOTSWITCH_SRIO_MISO_PIN);
        if (bit_val == GPIO_PIN_RESET) {
            result |= (1 << bit); // Active low, invert logic
        }
        
        // Clock pulse (rising edge shifts next bit)
        HAL_GPIO_WritePin(FOOTSWITCH_SRIO_SCK_PORT, FOOTSWITCH_SRIO_SCK_PIN, GPIO_PIN_SET);
        for(volatile int i=0; i<10; i++); // Short delay
        HAL_GPIO_WritePin(FOOTSWITCH_SRIO_SCK_PORT, FOOTSWITCH_SRIO_SCK_PIN, GPIO_PIN_RESET);
        for(volatile int i=0; i<10; i++); // Short delay
    }
    
    return result;
}
#endif

// =============================================================================
// Public Functions
// =============================================================================

int footswitch_init(void)
{
    if (g_initialized) {
        return 0; // Already initialized
    }
    
    // Initialize state arrays
    memset(g_button_state, 0, sizeof(g_button_state));
    memset(g_debounce_counter, 0, sizeof(g_debounce_counter));
    
#ifndef FOOTSWITCH_USE_SRIO
    // =========================================================================
    // GPIO Mode Initialization
    // =========================================================================
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    
    // Configure PE2, PE4, PE5, PE6 (FS0-FS3)
    GPIO_InitStruct.Pin = FOOTSWITCH_GPIO_FS0_PIN | FOOTSWITCH_GPIO_FS1_PIN | 
                          FOOTSWITCH_GPIO_FS2_PIN | FOOTSWITCH_GPIO_FS3_PIN;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    
    // Configure PB8, PB9, PB10, PB11 (FS4-FS7)
    GPIO_InitStruct.Pin = FOOTSWITCH_GPIO_FS4_PIN | FOOTSWITCH_GPIO_FS5_PIN | 
                          FOOTSWITCH_GPIO_FS6_PIN | FOOTSWITCH_GPIO_FS7_PIN;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
#else
    // =========================================================================
    // SRIO Bit-Bang Mode Initialization
    // =========================================================================
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // Configure SCK pin as output
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pin = FOOTSWITCH_SRIO_SCK_PIN;
    HAL_GPIO_Init(FOOTSWITCH_SRIO_SCK_PORT, &GPIO_InitStruct);
    
    // Configure MISO pin as input with pull-up
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Pin = FOOTSWITCH_SRIO_MISO_PIN;
    HAL_GPIO_Init(FOOTSWITCH_SRIO_MISO_PORT, &GPIO_InitStruct);
    
    // Configure PL pin as output
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Pin = FOOTSWITCH_SRIO_PL_PIN;
    HAL_GPIO_Init(FOOTSWITCH_SRIO_PL_PORT, &GPIO_InitStruct);
    
    // Set initial states (PL idle HIGH, SCK idle LOW)
    HAL_GPIO_WritePin(FOOTSWITCH_SRIO_PL_PORT, FOOTSWITCH_SRIO_PL_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(FOOTSWITCH_SRIO_SCK_PORT, FOOTSWITCH_SRIO_SCK_PIN, GPIO_PIN_RESET);
    
#endif
    
    g_initialized = true;
    return 0;
}

int footswitch_scan(void)
{
    if (!g_initialized) {
        return -1; // Not initialized
    }
    
    int event_count = 0;
    
    // Read all footswitch inputs
    for (uint8_t fs = 0; fs < FOOTSWITCH_NUM_SWITCHES; fs++) {
        // Read current state
        bool pressed_now = footswitch_read_raw(fs);
        bool was_pressed = (g_button_state[fs] != 0);
        
        // Debouncing: require consistent state for DEBOUNCE_THRESHOLD reads
        if (pressed_now != was_pressed) {
            g_debounce_counter[fs]++;
            if (g_debounce_counter[fs] >= DEBOUNCE_THRESHOLD) {
                // State confirmed, update and trigger event
                g_debounce_counter[fs] = 0;
                g_button_state[fs] = pressed_now ? 1 : 0;
                
                // Call callback if registered
                if (g_callback) {
                    footswitch_event_t event = pressed_now ? FOOTSWITCH_EVENT_PRESS : FOOTSWITCH_EVENT_RELEASE;
                    g_callback(fs, event);
                }
                
                event_count++;
            }
        } else {
            // State is stable, reset debounce counter
            g_debounce_counter[fs] = 0;
        }
    }
    
    return event_count;
}

void footswitch_set_callback(footswitch_callback_t callback)
{
    g_callback = callback;
}

bool footswitch_is_pressed(uint8_t fs_num)
{
    if (fs_num >= FOOTSWITCH_NUM_SWITCHES) {
        return false;
    }
    
    return (g_button_state[fs_num] != 0);
}

bool footswitch_read_raw(uint8_t fs_num)
{
    if (fs_num >= FOOTSWITCH_NUM_SWITCHES) {
        return false;
    }
    
#ifndef FOOTSWITCH_USE_SRIO
    // GPIO mode: Read pin directly
    GPIO_PinState pin_state = HAL_GPIO_ReadPin(g_gpio_pins[fs_num].port, g_gpio_pins[fs_num].pin);
    return (pin_state == GPIO_PIN_RESET); // Active low
#else
    // SRIO mode: Read from shift register
    static uint8_t srio_cache = 0xFF;
    static uint32_t last_read_time = 0;
    
    // Cache SRIO read for current scan cycle (all 8 buttons in one read)
    // Refresh if this is the first button or if enough time has passed
    uint32_t now = HAL_GetTick();
    if (fs_num == 0 || (now != last_read_time)) {
        srio_cache = footswitch_srio_read_byte();
        last_read_time = now;
    }
    
    return ((srio_cache & (1 << fs_num)) == 0); // Active low
#endif
}
