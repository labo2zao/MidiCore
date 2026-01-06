
# Module_Patch_SD_TXT_v1

Minimal SD patch engine using FATFS.

## Format
key=value
key2=value2

## Features
- Load / Save patch from SD
- Key-value store in RAM
- MIOS32-like TXT patch philosophy

## Requirements
- CubeMX FATFS enabled
- SDIO or SPI SD configured
- ff.h available

## Usage
patch_init();
patch_load("0:/patches/default.txt");
patch_get("router_mode", buf, sizeof(buf));
