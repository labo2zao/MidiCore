#include "ain_raw_debug_task.h"

#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
#include "Services/ain/ain.h"
#include "Config/project_config.h"
#include "Config/module_config.h"

#if MODULE_ENABLE_USB_CDC
#include "Services/usb_cdc/usb_cdc.h"
#endif

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static int buf_append(char* buf, size_t buf_sz, int len, const char* fmt, ...) {
  if (!buf || buf_sz == 0) return len;
  if (len < 0) len = 0;
  if ((size_t)len >= buf_sz) return len; // no space left

  va_list ap;
  va_start(ap, fmt);
  const int n = vsnprintf(buf + len, buf_sz - (size_t)len, fmt, ap);
  va_end(ap);

  if (n <= 0) return len;
  // If truncated, vsnprintf returns the number of chars that *would* have been written.
  // Clamp.
  const size_t written = (size_t)n;
  const size_t max_add = buf_sz - 1 - (size_t)len;
  return len + (int)((written > max_add) ? max_add : written);
}

#if DEBUG_AIN_RAW_DUMP

static osThreadId_t s_task = NULL;

static void debug_write(const char* s) {
  if (!s) return;
  const size_t n = strlen(s);
  if (n == 0) return;
#if MODULE_ENABLE_USB_CDC
  usb_cdc_send((const uint8_t*)s, (uint16_t)n);
#else
  // Fallback to UART if CDC not enabled (requires external UART handle)
  extern UART_HandleTypeDef huart1;
  (void)HAL_UART_Transmit(&huart1, (uint8_t*)s, (uint16_t)n, 100);
#endif
}

static void AinRawDebugTask(void* argument) {
  (void)argument;

  uint16_t raw[AIN_NUM_KEYS];
  char line[240];

  debug_write("AIN raw debug: ON\r\n");

  for (;;) {
    ain_debug_get_raw(raw, AIN_NUM_KEYS);

    // Print 8 ports x 8 channels in MIOS32-style order (J6..J13, A0..A7)
    for (uint8_t port = 0; port < 8; ++port) {
      int len = snprintf(line, sizeof(line), "J%u:", (unsigned)(port + 6));
      for (uint8_t a = 0; a < 8; ++a) {
        // In MIOS32 mapping, the channel order is reversed inside a port.
        const uint8_t key = (uint8_t)(port * 8 + (7 - a));
        len = buf_append(line, sizeof(line), len, " A%u=%4u", (unsigned)a, (unsigned)raw[key]);
      }
      len = buf_append(line, sizeof(line), len, "\r\n");
      debug_write(line);
    }

    debug_write("\r\n");
    osDelay(DEBUG_AIN_RAW_DUMP_PERIOD_MS);
  }
}

void ain_raw_debug_task_create(void) {
  if (s_task) return;

  const osThreadAttr_t attr = {
    .name = "AinRawDbg",
    .priority = (osPriority_t)osPriorityLow,
    .stack_size = 512,
  };
  s_task = osThreadNew(AinRawDebugTask, NULL, &attr);
}

#else

void ain_raw_debug_task_create(void) {
  // disabled
}

#endif
