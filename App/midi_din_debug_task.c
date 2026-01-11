#include "midi_din_debug_task.h"

#include <stdio.h>
#include <string.h>

#include "cmsis_os2.h"

#include "Config/project_config.h"
#include "Services/midi/midi_din.h"

#include "main.h" // huart1

extern UART_HandleTypeDef huart1;

static osThreadId_t s_midi_mon_tid = NULL;

static void debug_uart_write(const char* s)
{
  // Best-effort debug: never block forever.
  (void)HAL_UART_Transmit(&huart1, (uint8_t*)s, (uint16_t)strlen(s), 50);
}

static void mon_task(void* arg)
{
  (void)arg;
  char line[192];

  for (;;) {
    debug_uart_write("\r\n[MIDI DIN] stats\r\n");

    for (uint8_t p = 0; p < MIDI_DIN_PORTS; ++p) {
      midi_din_stats_t st;
      midi_din_get_stats(p, &st);

      if (st.last_short_len == 0) {
        snprintf(line, sizeof(line), "  P%u: rxB=%lu txB=%lu msg=%lu syx=%lu drop=%lu last=-\r\n",
                 (unsigned)(p + 1), (unsigned long)st.rx_bytes, (unsigned long)st.tx_bytes,
                 (unsigned long)st.rx_msgs, (unsigned long)st.rx_sysex_chunks,
                 (unsigned long)st.rx_drops);
      } else if (st.last_short_len == 1) {
        snprintf(line, sizeof(line), "  P%u: rxB=%lu txB=%lu msg=%lu syx=%lu drop=%lu last=%02X\r\n",
                 (unsigned)(p + 1), (unsigned long)st.rx_bytes, (unsigned long)st.tx_bytes,
                 (unsigned long)st.rx_msgs, (unsigned long)st.rx_sysex_chunks,
                 (unsigned long)st.rx_drops, (unsigned)st.last_short[0]);
      } else if (st.last_short_len == 2) {
        snprintf(line, sizeof(line), "  P%u: rxB=%lu txB=%lu msg=%lu syx=%lu drop=%lu last=%02X %02X\r\n",
                 (unsigned)(p + 1), (unsigned long)st.rx_bytes, (unsigned long)st.tx_bytes,
                 (unsigned long)st.rx_msgs, (unsigned long)st.rx_sysex_chunks,
                 (unsigned long)st.rx_drops, (unsigned)st.last_short[0], (unsigned)st.last_short[1]);
      } else {
        snprintf(line, sizeof(line), "  P%u: rxB=%lu txB=%lu msg=%lu syx=%lu drop=%lu last=%02X %02X %02X\r\n",
                 (unsigned)(p + 1), (unsigned long)st.rx_bytes, (unsigned long)st.tx_bytes,
                 (unsigned long)st.rx_msgs, (unsigned long)st.rx_sysex_chunks,
                 (unsigned long)st.rx_drops, (unsigned)st.last_short[0], (unsigned)st.last_short[1],
                 (unsigned)st.last_short[2]);
      }
      debug_uart_write(line);
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
      .stack_size = 768,
  };

  s_midi_mon_tid = osThreadNew(mon_task, NULL, &attr);
#endif
}
