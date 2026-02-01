/**
 * @file midicore_main_task.c
 * @brief MidiCore Main Task - Cooperative service-based architecture
 * 
 * Single main task that calls service tick functions cooperatively,
 * following embedded best practices for deterministic execution.
 * 
 * @note This file implements the core scheduler loop. All functional
 *       logic lives in service modules, not here.
 */

#include "App/midicore_main_task.h"
#include "Config/module_config.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include "App/tests/test_debug.h"
#include <string.h>

/* ============================================================================
 * SERVICE INCLUDES
 * ============================================================================
 * Include headers for all services that provide tick functions.
 * Services are conditionally compiled based on module_config.h.
 */

#if MODULE_ENABLE_AIN
#include "Services/ain/ain.h"
#include "App/ain_midi_task.h"  /* For ain_midi_process_events() */
#endif

#if MODULE_ENABLE_PRESSURE
#include "Services/pressure/pressure_i2c.h"
#include "Services/expression/expression.h"
#endif

#if MODULE_ENABLE_USB_MIDI
#include "Services/usb_midi/usb_midi.h"
#endif

#if MODULE_ENABLE_USB_CDC
#include "Services/usb_cdc/usb_cdc.h"
#endif

#if MODULE_ENABLE_MIDI_DIN
#include "Services/midi/midi_din.h"
#endif

#if MODULE_ENABLE_LOOPER
#include "Services/looper/looper.h"
#endif

#if MODULE_ENABLE_EXPRESSION
#include "Services/expression/expression.h"
#endif

#if MODULE_ENABLE_UI
#include "Services/ui/ui.h"
#endif

#if MODULE_ENABLE_CLI
#include "Services/cli/cli.h"
#endif

#if MODULE_ENABLE_WATCHDOG
#include "Services/watchdog/watchdog.h"
#endif

#if MODULE_ENABLE_ROUTER
#include "Services/router/router.h"
#endif

#include "Services/midi/midi_delayq.h"

#if MODULE_ENABLE_USB_MIDI
#include "Services/midicore_query/midicore_query.h"
#endif

#if MODULE_ENABLE_SRIO
#include "Services/srio/srio.h"
#include "Services/srio/srio_user_config.h"
#endif

#if MODULE_ENABLE_INPUT
#include "Services/input/input.h"
#endif

/* ============================================================================
 * FORWARD DECLARATIONS
 * ============================================================================
 */

/* Service tick functions - called from main loop */
static void ain_service_tick(uint32_t tick);
static void pressure_service_tick(uint32_t tick);
static void midi_io_service_tick(uint32_t tick);
static void ui_service_tick(uint32_t tick);
static void cli_service_tick(uint32_t tick);
static void watchdog_service_tick(uint32_t tick);
static void expression_service_tick(uint32_t tick);
static void srio_service_tick(uint32_t tick);
static void input_service_tick(uint32_t tick);

/* AIN MIDI processing - converts AIN events to MIDI */
static void ain_midi_service_tick(uint32_t tick);

/* ============================================================================
 * PRIVATE STATE
 * ============================================================================
 */

static osThreadId_t s_main_task_handle = NULL;
static volatile uint32_t s_tick_count = 0;
static volatile uint8_t s_running = 0;

/* SRIO state for cooperative mode */
#if MODULE_ENABLE_SRIO && defined(SRIO_ENABLE)
static uint8_t s_din_prev[SRIO_DIN_BYTES];
static uint8_t s_din_cur[SRIO_DIN_BYTES];
static uint8_t s_dout_buf[SRIO_DOUT_BYTES];
static uint8_t s_srio_initialized = 0;
#endif

/* Input service configuration defaults */
#define INPUT_DEBOUNCE_MS       20    /* Button debounce time in ms */
#define INPUT_SHIFT_HOLD_MS     500   /* Long-press time for shift function */
#define INPUT_SHIFT_BUTTON_ID   10    /* Physical button ID for SHIFT */

/* Input service state */
#if MODULE_ENABLE_INPUT
static uint32_t s_input_ms = 0;
static uint8_t s_input_initialized = 0;
#endif

/* ============================================================================
 * MAIN TASK IMPLEMENTATION
 * ============================================================================
 */

/**
 * @brief MidiCore Main Task - Cooperative scheduler
 * 
 * This is the heart of the system. It runs a tight loop with vTaskDelayUntil()
 * to ensure deterministic timing, calling service tick functions cooperatively.
 * 
 * Design principles:
 * - Single task, minimal stack usage
 * - Deterministic tick period (1-2ms)
 * - Non-blocking service calls
 * - No dynamic memory allocation
 * - No logging in the critical path
 */
static void MidiCore_MainTask(void *argument)
{
  (void)argument;
  
  dbg_printf("\r\n");
  dbg_printf("================================================\r\n");
  dbg_printf("  MidiCore_MainTask: Cooperative Architecture\r\n");
  dbg_printf("  Tick period: %u ms\r\n", MIDICORE_MAIN_TICK_MS);
  dbg_printf("  Stack size: %u bytes\r\n", MIDICORE_MAIN_STACK_SIZE);
  dbg_printf("================================================\r\n");
  dbg_printf("\r\n");
  
  /* Initialize services that need runtime init */
#if MODULE_ENABLE_EXPRESSION
  midi_delayq_init();
  expression_init();
#endif

  /* Initialize SRIO for button/LED handling */
#if MODULE_ENABLE_SRIO && defined(SRIO_ENABLE)
  {
    srio_config_t scfg = {
      .hspi = SRIO_SPI_HANDLE,
      .din_pl_port = SRIO_DIN_PL_PORT,
      .din_pl_pin = SRIO_DIN_PL_PIN,
      .dout_rclk_port = SRIO_DOUT_RCLK_PORT,
      .dout_rclk_pin = SRIO_DOUT_RCLK_PIN,
      .dout_oe_port = NULL,
      .dout_oe_pin = 0,
      .dout_oe_active_low = 1,
      .din_bytes = SRIO_DIN_BYTES,
      .dout_bytes = SRIO_DOUT_BYTES,
    };
    srio_init(&scfg);
    memset(s_din_prev, 0xFF, sizeof(s_din_prev));
    memset(s_din_cur, 0xFF, sizeof(s_din_cur));
    memset(s_dout_buf, 0, sizeof(s_dout_buf));
    srio_write_dout(s_dout_buf);
    s_srio_initialized = 1;
    dbg_printf("[MAIN] SRIO initialized: DIN=%u bytes, DOUT=%u bytes\r\n", 
               SRIO_DIN_BYTES, SRIO_DOUT_BYTES);
  }
#endif

  /* Initialize input service for buttons/encoders */
#if MODULE_ENABLE_INPUT
  {
    input_config_t icfg = {
      .debounce_ms = INPUT_DEBOUNCE_MS,
      .shift_hold_ms = INPUT_SHIFT_HOLD_MS,
      .shift_button_id = INPUT_SHIFT_BUTTON_ID
    };
    input_init(&icfg);
    s_input_initialized = 1;
    dbg_printf("[MAIN] Input service initialized\r\n");
  }
#endif

  /* Wait for USB enumeration to complete before processing MIDI */
  dbg_printf("[MAIN] Waiting for USB enumeration (500ms)...\r\n");
  osDelay(500);
  dbg_printf("[MAIN] USB ready\r\n");
  
  /* Send test message to MIOS Studio terminal to verify connectivity
   * This matches what MODULE_TEST_USB_DEVICE_MIDI does and is required
   * for MIOS Studio to recognize the device. */
#if MODULE_ENABLE_USB_MIDI
  dbg_printf("[MAIN] Sending test message to MIOS Studio terminal...\r\n");
  bool test_sent = midicore_debug_send_message("*** MidiCore Ready ***\r\n", 0);
  if (test_sent) {
    dbg_printf("[MAIN] Test message sent - check MIOS Studio Terminal\r\n");
  } else {
    dbg_printf("[MAIN] WARNING: Failed to send test message to MIOS terminal\r\n");
  }
#endif
  
  dbg_printf("[MAIN] Entering main loop\r\n");
  
  /* Track last wake time for vTaskDelayUntil */
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(MIDICORE_MAIN_TICK_MS);
  
  s_running = 1;
  
  /* Main cooperative loop */
  for (;;) {
    /* Service tick functions - called at appropriate intervals */
    uint32_t tick = s_tick_count;
    
    /* ---- PRIORITY 1: Time-critical services (every tick) ---- */
    
    /* MIDI I/O - process USB/DIN MIDI queues */
    midi_io_service_tick(tick);
    
    /* Expression processing (1ms for smooth CC output) */
    expression_service_tick(tick);
    
    /* Input service timing tick (1ms) */
    input_service_tick(tick);
    
    /* ---- PRIORITY 2: Regular services (every 5ms) ---- */
    
    if ((tick % MIDICORE_TICK_AIN) == 0) {
      ain_service_tick(tick);
      ain_midi_service_tick(tick);
    }
    
    if ((tick % MIDICORE_TICK_PRESSURE) == 0) {
      pressure_service_tick(tick);
    }
    
    /* SRIO scan (every 5ms for responsive buttons/LEDs) */
    if ((tick % MIDICORE_TICK_SRIO) == 0) {
      srio_service_tick(tick);
    }
    
    if ((tick % MIDICORE_TICK_CLI) == 0) {
      cli_service_tick(tick);
    }
    
    /* ---- PRIORITY 3: UI services (every 20ms) ---- */
    
    if ((tick % MIDICORE_TICK_UI) == 0) {
      ui_service_tick(tick);
    }
    
    /* ---- PRIORITY 4: Background services (infrequent) ---- */
    
    if ((tick % MIDICORE_TICK_WATCHDOG) == 0) {
      watchdog_service_tick(tick);
    }
    
    /* Heartbeat logging (every 60 seconds) */
    if ((tick % 60000) == 0 && tick > 0) {
      TaskHandle_t handle = xTaskGetCurrentTaskHandle();
      UBaseType_t hwm = uxTaskGetStackHighWaterMark(handle);
      dbg_printf("[MAIN] Heartbeat: tick=%lu, stack_free=%lu bytes\r\n",
                 (unsigned long)tick,
                 (unsigned long)(hwm * sizeof(StackType_t)));
    }
    
    /* MIOS Studio diagnostics (every 10 seconds) */
#if MODULE_ENABLE_USB_MIDI && MODULE_DEBUG_MIDICORE_QUERIES
    if ((tick % 10000) == 0 && tick > 0 && tick <= 30000) {
      /* Only show in first 30 seconds to avoid spam */
      extern bool usb_midi_get_tx_status(uint32_t*, uint32_t*, uint32_t*);
      uint32_t q_size = 0, q_used = 0, q_drops = 0;
      bool ready = usb_midi_get_tx_status(&q_size, &q_used, &q_drops);
      dbg_printf("[MAIN] USB MIDI Status: ready=%d, queue=%lu/%lu, drops=%lu\r\n",
                 ready, (unsigned long)q_used, (unsigned long)q_size, (unsigned long)q_drops);
      dbg_printf("[MAIN] Waiting for MIOS Studio query...\r\n");
    }
#endif
    
    /* Increment tick counter */
    s_tick_count++;
    
    /* Deterministic delay - CRITICAL for precise timing */
    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
}

/* ============================================================================
 * SERVICE TICK IMPLEMENTATIONS
 * ============================================================================
 * 
 * Each service tick function is:
 * - Non-blocking
 * - Has bounded execution time
 * - Does not call FreeRTOS API
 * - Does not allocate memory
 * - Does not log (except for debugging)
 */

/**
 * @brief AIN service tick - poll analog inputs
 */
static void ain_service_tick(uint32_t tick)
{
  (void)tick;
#if MODULE_ENABLE_AIN
  ain_tick_5ms();
#endif
}

/**
 * @brief Pressure sensor service tick - read I2C sensor
 */
static void pressure_service_tick(uint32_t tick)
{
  (void)tick;
#if MODULE_ENABLE_PRESSURE
  const pressure_cfg_t* c = pressure_get_cfg();
  if (c && c->enable) {
    int32_t raw = 0;
    if (pressure_read_once(&raw) == 0) {
      uint16_t v12 = pressure_to_12b(raw);
      expression_set_raw(v12);
      expression_set_pressure_pa(raw);
    }
  }
#endif
}

/**
 * @brief MIDI I/O service tick - process MIDI queues
 * 
 * This handles:
 * - USB MIDI RX queue processing
 * - USB CDC RX queue processing  
 * - MidiCore query processing
 * - MIDI DIN I/O
 * - Delay queue processing
 */
static void midi_io_service_tick(uint32_t tick)
{
  (void)tick;
  
#if MODULE_ENABLE_USB_MIDI
  /* Process USB MIDI RX queue in task context */
  usb_midi_process_rx_queue();
  
  /* Process MidiCore queries (MIOS Studio detection) */
  midicore_query_process_queued();
#endif

#if MODULE_ENABLE_USB_CDC
  /* Process USB CDC RX queue in task context */
  usb_cdc_process_rx_queue();
#endif

#if MODULE_ENABLE_MIDI_DIN
  /* MIDI DIN tick */
  midi_din_tick();
#endif

#if MODULE_ENABLE_LOOPER
  /* Looper 1ms tick */
  looper_tick_1ms();
#endif

  /* Delay queue tick */
  midi_delayq_tick_1ms();
}

/**
 * @brief Expression service tick - process expression/CC
 */
static void expression_service_tick(uint32_t tick)
{
  (void)tick;
#if MODULE_ENABLE_EXPRESSION
  expression_tick_1ms();
#endif
}

/**
 * @brief UI service tick - update display
 */
static void ui_service_tick(uint32_t tick)
{
  (void)tick;
#if MODULE_ENABLE_UI
  ui_tick_20ms();
#endif
}

/**
 * @brief CLI service tick - process terminal input
 */
static void cli_service_tick(uint32_t tick)
{
  (void)tick;
#if MODULE_ENABLE_CLI
  cli_task();
#endif
}

/**
 * @brief Watchdog service tick - kick the watchdog
 */
static void watchdog_service_tick(uint32_t tick)
{
  (void)tick;
#if MODULE_ENABLE_WATCHDOG
  watchdog_kick();
#endif
}

/**
 * @brief AIN MIDI service tick - convert AIN events to MIDI
 * 
 * This replaces the old AinMidiTask. It processes AIN events
 * and sends them through the router.
 */
static void ain_midi_service_tick(uint32_t tick)
{
  (void)tick;
#if MODULE_ENABLE_AIN && MODULE_ENABLE_ROUTER
  /* Process AIN events and convert to MIDI via ain_midi_process_events() */
  ain_midi_process_events();
#endif
}

/**
 * @brief SRIO service tick - scan DIN and update DOUT
 * 
 * This handles 74HC165 (DIN) and 74HC595 (DOUT) shift register chains.
 * Scans digital inputs and writes digital outputs for buttons/LEDs.
 */
static void srio_service_tick(uint32_t tick)
{
  (void)tick;
#if MODULE_ENABLE_SRIO && defined(SRIO_ENABLE)
  if (!s_srio_initialized) return;
  
  /* Read DIN chain */
  if (srio_read_din(s_din_cur) == 0) {
    /* Check for changes and feed to input service */
#if MODULE_ENABLE_INPUT
    for (uint16_t b = 0; b < SRIO_DIN_BYTES; b++) {
      uint8_t diff = s_din_cur[b] ^ s_din_prev[b];
      if (!diff) continue;
      
      for (uint8_t bit = 0; bit < 8; bit++) {
        if (diff & (1u << bit)) {
          uint16_t phys = (uint16_t)(b * 8u + bit);
          /* Active low: bit=0 means pressed, bit=1 means released */
          uint8_t pressed = !(s_din_cur[b] & (1u << bit));
          input_feed_button(phys, pressed);
        }
      }
    }
#endif
    
    /* Update previous state */
    memcpy(s_din_prev, s_din_cur, SRIO_DIN_BYTES);
  }
  
  /* Write DOUT chain - s_dout_buf can be updated by other services */
  srio_write_dout(s_dout_buf);
#endif
}

/**
 * @brief Input service tick - process button/encoder debouncing and timing
 * 
 * This should be called every 1ms to update debounce counters and
 * detect shift button long-press.
 */
static void input_service_tick(uint32_t tick)
{
  (void)tick;
#if MODULE_ENABLE_INPUT
  if (!s_input_initialized) return;
  
  s_input_ms++;
  input_tick(s_input_ms);
#endif
}

/* ============================================================================
 * PUBLIC API
 * ============================================================================
 */

int midicore_main_task_start(void)
{
  if (s_main_task_handle != NULL) {
    dbg_printf("[MAIN] Main task already started\r\n");
    return 0;
  }
  
  dbg_printf("[MAIN] Creating MidiCore_MainTask...\r\n");
  
  const osThreadAttr_t attr = {
    .name = "MidiCore",
    .priority = MIDICORE_MAIN_PRIORITY,
    .stack_size = MIDICORE_MAIN_STACK_SIZE
  };
  
  s_main_task_handle = osThreadNew(MidiCore_MainTask, NULL, &attr);
  
  if (s_main_task_handle == NULL) {
    dbg_printf("[MAIN] ERROR: Failed to create main task!\r\n");
    return -1;
  }
  
  dbg_printf("[MAIN] MidiCore_MainTask created successfully\r\n");
  return 0;
}

uint32_t midicore_main_get_tick_count(void)
{
  return s_tick_count;
}

uint8_t midicore_main_is_running(void)
{
  return s_running;
}
