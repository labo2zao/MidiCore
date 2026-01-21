/**
 * @file module_config.h
 * @brief MidiCore Module Configuration
 * 
 * This file allows enabling/disabling individual modules at compile time.
 * Each module can be independently tested and configured.
 * 
 * Usage:
 * - Define MODULE_ENABLE_xxx to 1 to enable a module
 * - Define MODULE_ENABLE_xxx to 0 to disable a module
 * - Can be overridden via compiler flags: -DMODULE_ENABLE_AINSER64=0
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// HARDWARE MODULES
// =============================================================================

/** @brief Enable AINSER64 analog input module (MCP3208 + 74HC4051) */
#ifndef MODULE_ENABLE_AINSER64
#define MODULE_ENABLE_AINSER64 1
#endif

/** @brief AINSER64 LED mode: 0=simple toggle (low memory), 1=PWM breathing (MIOS32-style) */
#ifndef AINSER64_LED_MODE_PWM
#define AINSER64_LED_MODE_PWM 1
#endif

/** @brief Enable SRIO module (74HC165/595 shift register I/O) */
#ifndef MODULE_ENABLE_SRIO
#define MODULE_ENABLE_SRIO 1
#endif

/** @brief Enable SPI bus shared resource management */
#ifndef MODULE_ENABLE_SPI_BUS
#define MODULE_ENABLE_SPI_BUS 1
#endif

/** @brief Enable OLED SSD1322 display module */
#ifndef MODULE_ENABLE_OLED
#define MODULE_ENABLE_OLED 1
#endif

// =============================================================================
// MIDI MODULES
// =============================================================================

/** @brief Enable MIDI DIN input/output (UART-based) */
#ifndef MODULE_ENABLE_MIDI_DIN
#define MODULE_ENABLE_MIDI_DIN 1
#endif

/** @brief Enable MIDI router (message routing between nodes) */
#ifndef MODULE_ENABLE_ROUTER
#define MODULE_ENABLE_ROUTER 1
#endif

/** @brief Enable MIDI delay queue (for timing/humanization) */
#ifndef MODULE_ENABLE_MIDI_DELAYQ
#define MODULE_ENABLE_MIDI_DELAYQ 1
#endif

/** @brief Enable USB Device MIDI */
#ifndef MODULE_ENABLE_USB_MIDI
#define MODULE_ENABLE_USB_MIDI 1  // Disabled by default (requires USB config)
#endif

/** @brief Enable USB Host MIDI */
#ifndef MODULE_ENABLE_USBH_MIDI
#define MODULE_ENABLE_USBH_MIDI 0  // Disabled - Using USB Device mode instead
#endif

// =============================================================================
// SERVICES MODULES
// =============================================================================

/** @brief Enable AIN service (analog input processing, velocity detection) */
#ifndef MODULE_ENABLE_AIN
#define MODULE_ENABLE_AIN 1
#endif

/** @brief Enable Looper service (MIDI recording/playback) */
#ifndef MODULE_ENABLE_LOOPER
#define MODULE_ENABLE_LOOPER 1
#endif

/** @brief Enable LFO service (Low Frequency Oscillator for modulation) */
#ifndef MODULE_ENABLE_LFO
#define MODULE_ENABLE_LFO 1
#endif

/** @brief Enable Humanizer service (MIDI humanization/groove) */
#ifndef MODULE_ENABLE_HUMANIZER
#define MODULE_ENABLE_HUMANIZER 1
#endif

/** @brief Enable Patch management (SD card patch loading/saving) */
#ifndef MODULE_ENABLE_PATCH
#define MODULE_ENABLE_PATCH 1
#endif

/** @brief Enable Input service (button/encoder handling) */
#ifndef MODULE_ENABLE_INPUT
#define MODULE_ENABLE_INPUT 1
#endif

/** @brief Enable UI service (user interface, pages, menus) */
#ifndef MODULE_ENABLE_UI
#define MODULE_ENABLE_UI 1
#endif

/** @brief Enable UI Song Mode page */
#ifndef MODULE_ENABLE_UI_PAGE_SONG
#define MODULE_ENABLE_UI_PAGE_SONG 1
#endif

/** @brief Enable UI MIDI Monitor page */
#ifndef MODULE_ENABLE_UI_PAGE_MIDI_MONITOR
#define MODULE_ENABLE_UI_PAGE_MIDI_MONITOR 1
#endif

/** @brief Enable UI SysEx page */
#ifndef MODULE_ENABLE_UI_PAGE_SYSEX
#define MODULE_ENABLE_UI_PAGE_SYSEX 1
#endif

/** @brief Enable UI Config Editor page */
#ifndef MODULE_ENABLE_UI_PAGE_CONFIG
#define MODULE_ENABLE_UI_PAGE_CONFIG 1
#endif

/** @brief Enable Expression pedal/pressure service */
#ifndef MODULE_ENABLE_EXPRESSION
#define MODULE_ENABLE_EXPRESSION 1
#endif

/** @brief Enable Pressure sensor (I2C-based) */
#ifndef MODULE_ENABLE_PRESSURE
#define MODULE_ENABLE_PRESSURE 1
#endif

/** @brief Enable Velocity curve processing */
#ifndef MODULE_ENABLE_VELOCITY
#define MODULE_ENABLE_VELOCITY 1
#endif

/** @brief Enable Humanize service (timing/velocity randomization) */
#ifndef MODULE_ENABLE_HUMANIZE
#define MODULE_ENABLE_HUMANIZE 1
#endif

/** @brief Enable LiveFX module (transpose, velocity scale, force-to-scale) */
#ifndef MODULE_ENABLE_LIVEFX
#define MODULE_ENABLE_LIVEFX 1
#endif

/** @brief Enable Scale module (musical scale quantization) */
#ifndef MODULE_ENABLE_SCALE
#define MODULE_ENABLE_SCALE 1
#endif

/** @brief Enable Router Hooks (LiveFX/Monitor integration) */
#ifndef MODULE_ENABLE_ROUTER_HOOKS
#define MODULE_ENABLE_ROUTER_HOOKS 1
#endif

/** @brief Enable Rhythm Trainer (pedagogical timing practice tool) */
#ifndef MODULE_ENABLE_RHYTHM_TRAINER
#define MODULE_ENABLE_RHYTHM_TRAINER 1
#endif

/** @brief Enable Metronome (synchronized click track) */
#ifndef MODULE_ENABLE_METRONOME
#define MODULE_ENABLE_METRONOME 1
#endif

/** @brief Enable Config I/O (SD card configuration file read/write) */
#ifndef MODULE_ENABLE_CONFIG_IO
#define MODULE_ENABLE_CONFIG_IO 1
#endif

/** @brief Enable Zones configuration (keyboard split/layers) */
#ifndef MODULE_ENABLE_ZONES
#define MODULE_ENABLE_ZONES 1
#endif

/** @brief Enable Instrument configuration */
#ifndef MODULE_ENABLE_INSTRUMENT
#define MODULE_ENABLE_INSTRUMENT 1
#endif

/** @brief Enable Digital output mapping (DOUT) */
#ifndef MODULE_ENABLE_DOUT
#define MODULE_ENABLE_DOUT 1
#endif

// =============================================================================
// SYSTEM MODULES
// =============================================================================

/** @brief Enable System status/diagnostics */
#ifndef MODULE_ENABLE_SYSTEM_STATUS
#define MODULE_ENABLE_SYSTEM_STATUS 1
#endif

/** @brief Enable Boot reason detection */
#ifndef MODULE_ENABLE_BOOT_REASON
#define MODULE_ENABLE_BOOT_REASON 1
#endif

/** @brief Enable Watchdog service */
#ifndef MODULE_ENABLE_WATCHDOG
#define MODULE_ENABLE_WATCHDOG 1
#endif

/** @brief Enable Safe mode (fallback for SD card errors) */
#ifndef MODULE_ENABLE_SAFE_MODE
#define MODULE_ENABLE_SAFE_MODE 1
#endif

/** @brief Enable Logging service */
#ifndef MODULE_ENABLE_LOG
#define MODULE_ENABLE_LOG 1
#endif

// =============================================================================
// DEBUG/TEST MODULES
// =============================================================================

/** @brief Enable AIN raw debug task (UART output of ADC values) */
#ifndef MODULE_ENABLE_AIN_RAW_DEBUG
#define MODULE_ENABLE_AIN_RAW_DEBUG 0  // Disabled by default
#endif

/** @brief Enable MIDI DIN debug monitoring */
#ifndef MODULE_ENABLE_MIDI_DIN_DEBUG
#define MODULE_ENABLE_MIDI_DIN_DEBUG 0  // Disabled by default
#endif

// NOTE: DIN_SELFTEST has been removed - use MODULE_TEST_SRIO instead
// The old DIN_SELFTEST was superseded by the comprehensive MODULE_TEST_SRIO test

// NOTE: MODULE_ENABLE_LOOPER_SELFTEST has been removed - use MODULE_TEST_LOOPER instead
// The old LOOPER_SELFTEST was replaced by the new MODULE_TEST_LOOPER test framework

// =============================================================================
// CONFIGURATION VALIDATION
// =============================================================================

// Dependency checks
#if MODULE_ENABLE_AIN && !MODULE_ENABLE_AINSER64
#warning "AIN module requires AINSER64 module. Enable MODULE_ENABLE_AINSER64."
#endif

#if MODULE_ENABLE_INPUT && !MODULE_ENABLE_SRIO
#warning "INPUT module typically requires SRIO module for buttons. Enable MODULE_ENABLE_SRIO if needed."
#endif

#if MODULE_ENABLE_OLED && !MODULE_ENABLE_SPI_BUS
#error "OLED module requires SPI_BUS module. Enable MODULE_ENABLE_SPI_BUS."
#endif

#if MODULE_ENABLE_AINSER64 && !MODULE_ENABLE_SPI_BUS
#error "AINSER64 module requires SPI_BUS module. Enable MODULE_ENABLE_SPI_BUS."
#endif

#if MODULE_ENABLE_ROUTER && !MODULE_ENABLE_MIDI_DIN && !MODULE_ENABLE_USB_MIDI && !MODULE_ENABLE_USBH_MIDI
#warning "ROUTER module enabled but no MIDI transport (DIN/USB) is enabled."
#endif

// =============================================================================
// MODULE STATUS REPORTING
// =============================================================================

/** @brief Returns a bitmask of enabled modules for diagnostics */
static inline uint32_t module_config_get_enabled_mask(void) {
  uint32_t mask = 0;
  #if MODULE_ENABLE_AINSER64
  mask |= (1u << 0);
  #endif
  #if MODULE_ENABLE_SRIO
  mask |= (1u << 1);
  #endif
  #if MODULE_ENABLE_MIDI_DIN
  mask |= (1u << 2);
  #endif
  #if MODULE_ENABLE_ROUTER
  mask |= (1u << 3);
  #endif
  #if MODULE_ENABLE_LOOPER
  mask |= (1u << 4);
  #endif
  #if MODULE_ENABLE_UI
  mask |= (1u << 5);
  #endif
  #if MODULE_ENABLE_OLED
  mask |= (1u << 6);
  #endif
  #if MODULE_ENABLE_PATCH
  mask |= (1u << 7);
  #endif
  return mask;
}

#ifdef __cplusplus
}
#endif
