# Getting Started with MidiCore

> 🇫🇷 [Version française disponible](README_FR.md)

Quick start guides to help you integrate and use MidiCore in your projects.

## Available Guides

### Integration
- **[What To Do Now](WHAT_TO_DO_NOW.md)** - Clear action plan and latest updates

## Quick Start Workflow

1. **Read** this guide to understand module integration
2. **Configure** modules using [Module Configuration](../configuration/README_MODULE_CONFIG.md)
3. **Test** your setup with [Testing Quick Start](../testing/TESTING_QUICKSTART.md)
4. **Explore** [User Guides](../user-guides/) for feature documentation

## Need Help?

- **Configuration issues?** → See [Configuration](../configuration/)
- **Build errors?** → Check [What To Do Now](WHAT_TO_DO_NOW.md)
- **Testing?** → Browse [Testing Documentation](../testing/)

---

## Integration Guide

### Add AINSER64 and OLED SSD1322 Modules

#### Copy into your project
Copy folders:
- `Config/`
- `Hal/`
- `Services/`

#### Init order (after RTOS kernel init)
```c
spibus_init();
hal_ainser64_init();
ain_init();
oled_init();
```

#### FreeRTOS task (example)
Call `ain_tick_5ms()` every 5ms.

#### Key mapping
Current mapping:
```
key = ((bank*8 + adc_channel) * 8 + step)  // 0..63
```

If your physical wiring differs, change mapping in `Services/ain/ain.c`.

---

## Project Integration

### MidiCore merged project (router + DIN + patch + stubs looper/ui)

#### What is included
- AINSER64 scan + velocity events
- OLED SSD1322 driver + demo
- Router 16 nodes with per-route channel masks
- MIDI DIN IN/OUT (USART1/2/3 + UART5) at 31250 baud
- Patch SD TXT (FATFS) minimal key=value store
- USB MIDI transport stubs (your CubeMX project is currently Custom HID)
- Looper/UI compile-safe placeholders (next to implement fully)

#### CubeMX notes
- Ensure USART1 baudrate is set to 31250 (currently may be default).
- Ensure RX interrupts enabled for USART1/2/3/UART5.
- USB Device is Custom HID in this project; to enable USB MIDI later:
  - Switch USB Device class to MIDI in CubeMX
  - Generate `usbd_midi_if.*` and define `ENABLE_USBD_MIDI` in build flags.

#### Build notes
- If you see `undefined reference to powf`, add `m` in MCU GCC Linker → Libraries.

#### Patch load at boot (Option A)
- `app_init` mounts SD (0:) then loads `0:/patches/bank01/patch.txt` and applies [router] routes.

---

## AccordeonInstrument Modules BUNDLE v1

This bundle contains the currently *implemented* (non-empty) modules:
- `Config/`: pin mappings for AINSER64 + OLED
- `Hal/`: spi_bus, delay_us, ainser64_hw (MCP3208+4051+bank gating), oled_ssd1322
- `Services/`: ain (filtering+calib+velocity C=A+B + event queue)
- `App/`: bring-up tasks (AinTask 5ms + OLED demo)

### How to use (CubeIDE)
1. Start from your CubeMX-generated project (the one that already generates OK).
2. Copy these folders next to `Core/`:
   - Config, Hal, Services, App
3. Add include paths:
   - `${workspace_loc:/${ProjName}/Config}`
   - `${workspace_loc:/${ProjName}/Hal}`
   - `${workspace_loc:/${ProjName}/Services}`
   - `${workspace_loc:/${ProjName}/App}`
4. Ensure linker includes libm (for powf):
   - add `m` in MCU GCC Linker > Libraries
5. Call `app_init_and_start()` once after RTOS init (see `App/README.md`).

### Notes
- This is a *module bundle*, not a replacement for CubeMX files (Core/, .ioc, startup, linker).
- It will compile when merged into your CubeMX project with the agreed pinout.
