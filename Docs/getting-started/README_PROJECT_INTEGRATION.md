# MidiCore merged project (router + DIN + patch + stubs looper/ui)

## What is included
- AINSER64 scan + velocity events
- OLED SSD1322 driver + demo
- Router 16 nodes with per-route channel masks
- MIDI DIN IN/OUT (USART1/2/3 + UART5) at 31250
- Patch SD TXT (FATFS) minimal key=value store
- USB MIDI transport stubs (your CubeMX project is currently Custom HID)
- Looper/UI compile-safe placeholders (next to implement fully)

## CubeMX notes
- Ensure USART1 baudrate is set to 31250 (currently may be default).
- Ensure RX interrupts enabled for USART1/2/3/UART5.
- USB Device is Custom HID in this project; to enable USB MIDI later:
  - Switch USB Device class to MIDI in CubeMX
  - Generate usbd_midi_if.* and define ENABLE_USBD_MIDI in build flags.

## Build notes
- If you see `undefined reference to powf`, add `m` in MCU GCC Linker -> Libraries.

## Patch load at boot (Option A)
- app_init mounts SD (0:) then loads 0:/patches/bank01/patch.txt and applies [router] routes.
