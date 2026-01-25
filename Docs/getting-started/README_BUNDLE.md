# AccordeonInstrument Modules BUNDLE v1

This bundle contains the currently *implemented* (non-empty) modules produced in this chat:
- Config/: pin mappings for AINSER64 + OLED
- Hal/: spi_bus, delay_us, ainser64_hw (MCP3208+4051+bank gating), oled_ssd1322
- Services/: ain (filtering+calib+velocity C=A+B + event queue)
- App/: bring-up tasks (AinTask 5ms + OLED demo)

## How to use (CubeIDE)
1. Start from your CubeMX-generated project (the one that already generates OK).
2. Copy these folders next to Core/:
   - Config, Hal, Services, App
3. Add include paths:
   - ${workspace_loc:/${ProjName}/Config}
   - ${workspace_loc:/${ProjName}/Hal}
   - ${workspace_loc:/${ProjName}/Services}
   - ${workspace_loc:/${ProjName}/App}
4. Ensure linker includes libm (for powf):
   - add 'm' in MCU GCC Linker > Libraries
5. Call `app_init_and_start()` once after RTOS init (see App/README.md).

## Notes
- This is a *module bundle*, not a replacement for CubeMX files (Core/, .ioc, startup, linker).
- It will compile when merged into your CubeMX project with the agreed pinout.
