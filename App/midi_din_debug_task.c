#include "midi_din_debug_task.h"

#include <stdio.h>
#include <string.h>

#include "cmsis_os2.h"

#include "Config/project_config.h"
#include "Config/module_config.h"
#include "Services/midi/midi_din.h"

#if MODULE_ENABLE_USB_CDC
#include "Services/usb_cdc/usb_cdc.h"
#endif

#ifndef DEBUG_MIDI_DIN_MONITOR
#define DEBUG_MIDI_DIN_MONITOR 0
#endif

#ifndef DEBUG_MIDI_DIN_MONITOR_PERIOD_MS
#define DEBUG_MIDI_DIN_MONITOR_PERIOD_MS 500
#endif

static osThreadId_t s_midi_mon_tid = NULL;

static void debug_write(const char* s)
{
  if (!s) return;
  const size_t n = strlen(s);
  if (n == 0) return;
#if MODULE_ENABLE_USB_CDC
  usb_cdc_send((const uint8_t*)s, (uint16_t)n);
#else
  // Fallback to UART if CDC not enabled
  extern UART_HandleTypeDef huart1;
  (void)HAL_UART_Transmit(&huart1, (uint8_t*)s, (uint16_t)n, 50);
#endif
}

static void mon_task(void* arg)
{
  (void)arg;
  char line[192];

  for (;;) {
    debug_write("\r\n[MIDI DIN] stats\r\n");

    for (uint8_t p = 0; p < MIDI_DIN_PORTS; ++p) {
      midi_din_stats_t st;
      midi_din_get_stats(p, &st);

      if (st.last_len == 0) {
        snprintf(line, sizeof(line), "  P%u: rxB=%lu txB=%lu msg=%lu syx=%lu drop=%lu last=-\r\n",
                 (unsigned)(p + 1), (unsigned long)st.rx_bytes, (unsigned long)st.tx_bytes,
                 (unsigned long)st.rx_msgs, (unsigned long)st.rx_sysex_chunks,
                 (unsigned long)st.rx_drops);
      } else if (st.last_len == 1) {
        snprintf(line, sizeof(line), "  P%u: rxB=%lu txB=%lu msg=%lu syx=%lu drop=%lu last=%02X\r\n",
                 (unsigned)(p + 1), (unsigned long)st.rx_bytes, (unsigned long)st.tx_bytes,
                 (unsigned long)st.rx_msgs, (unsigned long)st.rx_sysex_chunks,
                 (unsigned long)st.rx_drops, (unsigned)st.last_bytes[0]);
      } else if (st.last_len == 2) {
        snprintf(line, sizeof(line), "  P%u: rxB=%lu txB=%lu msg=%lu syx=%lu drop=%lu last=%02X %02X\r\n",
                 (unsigned)(p + 1), (unsigned long)st.rx_bytes, (unsigned long)st.tx_bytes,
                 (unsigned long)st.rx_msgs, (unsigned long)st.rx_sysex_chunks,
                 (unsigned long)st.rx_drops, (unsigned)st.last_bytes[0], (unsigned)st.last_bytes[1]);
      } else {
        snprintf(line, sizeof(line), "  P%u: rxB=%lu txB=%lu msg=%lu syx=%lu drop=%lu last=%02X %02X %02X\r\n",
                 (unsigned)(p + 1), (unsigned long)st.rx_bytes, (unsigned long)st.tx_bytes,
                 (unsigned long)st.rx_msgs, (unsigned long)st.rx_sysex_chunks,
                 (unsigned long)st.rx_drops, (unsigned)st.last_bytes[0], (unsigned)st.last_bytes[1],
                 (unsigned)st.last_bytes[2]);
      }
      debug_write(line);
    }

    osDelay(DEBUG_MIDI_DIN_MONITOR_PERIOD_MS);
  }
}

void midi_din_debug_task_create(void)
{
#if DEBUG_MIDI_DIN_MONITOR
  if (s_midi_mon_tid != NULL) {
    return;
  }

  const osThreadAttr_t attr = {
      .name = "midi_din_mon",
      .priority = (osPriority_t)osPriorityLow,
      .stack_size = 1024,  /* Increased from 768: uses snprintf (500+ bytes stack) + 192B line buffer */
  };

  s_midi_mon_tid = osThreadNew(mon_task, NULL, &attr);
#endif
}
