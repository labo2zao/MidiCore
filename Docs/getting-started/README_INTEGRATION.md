# Add-on: AINSER64-like + OLED SSD1322 (SPI2)

## Copy into your project
Copy folders:
- Config/
- Hal/
- Services/

## Init order (after RTOS kernel init)
- spibus_init()
- hal_ainser64_init()
- ain_init()
- oled_init()

## FreeRTOS task (example)
Call `ain_tick_5ms()` every 5ms.

## Key mapping
Current mapping:
key = ((bank*8 + adc_channel) * 8 + step)  // 0..63

If your physical wiring differs, change mapping in `Services/ain/ain.c`.
