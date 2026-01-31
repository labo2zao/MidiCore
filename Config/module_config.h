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

/** @brief SRIO DOUT LED polarity configuration
 * Set to 0 if LEDs are ACTIVE HIGH (1=ON, 0=OFF)
 * Set to 1 if LEDs are ACTIVE LOW  (0=ON, 1=OFF) - MidiCore default
 */
#ifndef SRIO_DOUT_LED_ACTIVE_LOW
#define SRIO_DOUT_LED_ACTIVE_LOW 1
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

// =============================================================================
// DEBUG OUTPUT CONFIGURATION
// =============================================================================

/**
 * @brief Debug output method selection
 * 
 * ============================================================================
 * HOW TO CHOOSE DEBUG OUTPUT METHOD:
 * ============================================================================
 * 
 * Set MODULE_DEBUG_OUTPUT to ONE of:
 * - DEBUG_OUTPUT_SWV      → ST-Link SWO (recommended for debugging)
 * - DEBUG_OUTPUT_USB_CDC  → USB Virtual COM (MIOS Studio compatible)
 * - DEBUG_OUTPUT_UART     → Hardware UART (fallback)
 * - DEBUG_OUTPUT_NONE     → Disabled (production)
 * 
 * ============================================================================
 * OPTION 1: SWV (Serial Wire Viewer) ⭐ RECOMMENDED FOR DEBUGGING
 * ============================================================================
 * 
 * Advantages:
 * ✅ NO USB conflicts - uses ST-Link, not USB
 * ✅ Always reliable - works even if USB fails
 * ✅ High bandwidth - up to 2 MHz
 * ✅ Real-time traces - minimal latency
 * ✅ Best for USB MIDI devices
 * 
 * Setup:
 * 1. Set: MODULE_DEBUG_OUTPUT = DEBUG_OUTPUT_SWV
 * 2. In Debug Config → Debugger → Serial Wire Viewer:
 *    - Enable: ☑
 *    - Core Clock: 168000000 (168 MHz)
 *    - SWO Clock: 2000000 (2 MHz)
 *    - Port 0: ☑ Enabled
 * 3. View: Window → Show View → SWV → SWV ITM Data Console
 * 4. Click "Start Trace" button
 * 
 * ============================================================================
 * OPTION 2: USB CDC (Virtual COM Port) - MIOS STUDIO COMPATIBLE
 * ============================================================================
 * 
 * Advantages:
 * ✅ MIOS Studio compatible
 * ✅ Standalone (no debugger needed)
 * ✅ Standard serial terminal
 * 
 * Disadvantages:
 * ⚠️ May conflict with USB MIDI during debugging
 * ⚠️ Requires USB enumeration working
 * 
 * Setup:
 * 1. Set: MODULE_DEBUG_OUTPUT = DEBUG_OUTPUT_USB_CDC
 * 2. Set: MODULE_ENABLE_USB_CDC = 1 (below)
 * 3. Connect USB cable
 * 4. Open MIOS Studio or terminal (COM port)
 * 
 * ============================================================================
 * OPTION 3: Hardware UART - Fallback
 * ============================================================================
 * 
 * Setup:
 * 1. Set: MODULE_DEBUG_OUTPUT = DEBUG_OUTPUT_UART
 * 2. Configure port in App/tests/test_debug.h
 * 3. Connect UART adapter (115200 baud)
 * 
 * ============================================================================
 * OPTION 4: Disabled - Production
 * ============================================================================
 * 
 * Set: MODULE_DEBUG_OUTPUT = DEBUG_OUTPUT_NONE
 * 
 * ============================================================================
 * BEST PRACTICE: Use SWV for debugging + USB CDC for MIOS terminal
 * ============================================================================
 * 
 * Set:
 * - MODULE_DEBUG_OUTPUT = DEBUG_OUTPUT_SWV      (debug traces via ST-Link)
 * - MODULE_ENABLE_USB_CDC = 1                   (MIOS terminal via USB)
 * 
 * This gives you:
 * ✅ Debug traces via SWV (no USB conflicts)
 * ✅ MIOS Studio terminal via USB CDC
 * ✅ CLI commands via USB CDC
 * ✅ Both working simultaneously!
 * 
 * See: docs/DEBUG_OUTPUT_GUIDE.md for complete guide
 * ============================================================================
 */

// Define output method constants
#define DEBUG_OUTPUT_NONE     0  // No debug output
#define DEBUG_OUTPUT_SWV      1  // SWV/ITM via ST-Link (recommended for debugging)
#define DEBUG_OUTPUT_USB_CDC  2  // USB CDC Virtual COM (MIOS Studio compatible)
#define DEBUG_OUTPUT_UART     3  // Hardware UART (fallback)

// ============================================================================
// 👇 CHANGE THIS LINE TO CHOOSE DEBUG OUTPUT METHOD 👇
// ============================================================================
#ifndef MODULE_DEBUG_OUTPUT
#define MODULE_DEBUG_OUTPUT DEBUG_OUTPUT_UART  // ⭐ RECOMMENDED: SWV for debugging
// #define MODULE_DEBUG_OUTPUT DEBUG_OUTPUT_USB_CDC  // Alternative: USB CDC for MIOS Studio
// #define MODULE_DEBUG_OUTPUT DEBUG_OUTPUT_UART     // Alternative: Hardware UART
// #define MODULE_DEBUG_OUTPUT DEBUG_OUTPUT_NONE     // Alternative: Disabled
#endif
// ============================================================================

// =============================================================================
// CLI TERMINAL SELECTION  
// =============================================================================
/**
 * Choose where CLI output appears:
 * 
 * CLI_OUTPUT_USB_CDC (1):
 *   - CLI on USB CDC terminal (MIOS Studio)
 *   - Works immediately, no wait
 *   - Best for production with MIOS Studio
 *   - Separates CLI from debug output (clean)
 * 
 * CLI_OUTPUT_UART (2):
 *   - CLI on hardware UART terminal  
 *   - Best for hardware debugging
 *   - Independent of USB
 *   - NOTE: If MODULE_DEBUG_OUTPUT also UART, CLI shares terminal with debug
 *           (CLI prompt adds newline to avoid overwriting debug messages)
 * 
 * CLI_OUTPUT_MIOS (3):
 *   - CLI on MIOS terminal via MIDI SysEx protocol
 *   - Uses midicore_debug_send_message() SysEx (NOT USB CDC text)
 *   - Standard MidiCore behavior for MIOS Studio compatibility
 *   - Format: F0 00 00 7E 32 00 0D 40 <text> F7
 *   - Best for production with MIOS Studio
 *   - IMPORTANT: Device recognition via USB MIDI queries (handled by MidiIOTask)
 *   - CLI terminal is separate and independent from queries
 *   - No wait for USB CDC - starts immediately
 * 
 * CLI_OUTPUT_DEBUG (4):
 *   - CLI follows MODULE_DEBUG_OUTPUT setting
 *   - CLI appears on same terminal as debug messages
 *   - Simple unified output
 *   - NOTE: CLI prompt adds newline when MODULE_DEBUG_MIDICORE_QUERIES active
 *           to avoid overwriting MidiCore query debug messages
 * 
 * RECOMMENDED CONFIGURATIONS:
 * 
 * For MIOS Studio Production:
 *   #define MODULE_DEBUG_OUTPUT  DEBUG_OUTPUT_UART    (debug on UART)
 *   #define MODULE_CLI_OUTPUT    CLI_OUTPUT_MIOS      (CLI on USB CDC)
 *   → Debug messages on UART, CLI on MIOS terminal (separate, clean)
 * 
 * For Hardware Debugging:
 *   #define MODULE_DEBUG_OUTPUT  DEBUG_OUTPUT_UART    (debug on UART)
 *   #define MODULE_CLI_OUTPUT    CLI_OUTPUT_UART      (CLI also on UART)
 *   → Everything on UART (unified, CLI adds newlines for separation)
 * 
 * For Minimal Setup:
 *   #define MODULE_DEBUG_OUTPUT  DEBUG_OUTPUT_UART
 *   #define MODULE_CLI_OUTPUT    CLI_OUTPUT_DEBUG     (follow debug)
 *   → CLI follows debug output (unified, auto newline separation)
 */
#define CLI_OUTPUT_USB_CDC    1
#define CLI_OUTPUT_UART       2
#define CLI_OUTPUT_MIOS       3
#define CLI_OUTPUT_DEBUG      4

#ifndef MODULE_CLI_OUTPUT
#define MODULE_CLI_OUTPUT  CLI_OUTPUT_MIOS  // Default: MIOS terminal (standard behavior)
#endif

/** @brief Enable USB CDC (Virtual COM Port / ACM) - MidiCore & MIOS Studio compatible
 * 
 * When enabled (MODULE_ENABLE_USB_CDC=1):
 * - Adds CDC ACM interface to USB device (composite with MIDI)
 * - Exposes virtual COM port for terminal/debug communication
 * - Compatible with MIOS Studio (requires proper descriptor strings)
 * - Provides MIOS32-compatible API shims (MIOS32_USB_CDC_*)
 * 
 * When disabled (MODULE_ENABLE_USB_CDC=0, default):
 * - USB device remains MIDI-only
 * - No CDC code compiled (saves Flash/RAM)
 * - Default behavior for existing users
 * 
 * Requirements:
 * - USB_OTG_FS configured in CubeMX
 * - Available endpoints and FIFO space (see USB_CONFIGURATION_GUIDE.md)
 * - Composite device descriptors (bDeviceClass=0)
 * 
 * Testing:
 * - Windows: Device Manager → Ports (COM & LPT)
 * - macOS: /dev/tty.usbmodem*
 * - Linux: /dev/ttyACM*
 * 
 * See Docs/usb/CDC_INTEGRATION.md for setup and usage
 * 
 * Note: USB CDC can be enabled even if MODULE_DEBUG_OUTPUT != DEBUG_OUTPUT_USB_CDC
 * This allows MIOS terminal via USB CDC while using SWV for debug traces.
 */
#ifndef MODULE_ENABLE_USB_CDC
#define MODULE_ENABLE_USB_CDC 1  // Enabled - RAM optimized via FreeRTOS heap reduction and buffer optimization
#endif

/** @brief Enable USB MSC (Mass Storage Class) - SD card access via USB
 * 
 * When enabled (MODULE_ENABLE_USB_MSC=1):
 * - Exposes SD card as USB Mass Storage device
 * - Allows direct file editing from MIOS Studio and PC
 * - Composite device: MIDI + CDC + MSC
 * - Automatic SD card locking when accessed via USB
 * 
 * When disabled (MODULE_ENABLE_USB_MSC=0, default):
 * - No MSC functionality
 * - SD card only accessible via firmware
 * 
 * Requirements:
 * - SD card initialized and working
 * - FATFS configured
 * - USB_OTG_FS with sufficient endpoints
 * 
 * Safety:
 * - Automatic arbitration prevents SD corruption
 * - Firmware pauses SD access when USB host mounts
 * - Safe unmount handling
 * 
 * See Docs/usb/MSC_INTEGRATION.md for setup and usage
 */
#ifndef MODULE_ENABLE_USB_MSC
#define MODULE_ENABLE_USB_MSC 0  // Disabled by default (opt-in feature)
#endif

/** @brief Enable USB Host MIDI - MIOS32-style dual-mode configuration */
#ifndef MODULE_ENABLE_USBH_MIDI
#define MODULE_ENABLE_USBH_MIDI 0  /* Disable Host for now - testing Device only */  // Enabled for MIOS32-style dual Host/Device support
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

/** @brief Enable UI Looper Pianoroll page
 *  Note: Pianoroll is the main accordion UI page and must be enabled.
 *  Uses 24KB in CCMRAM for active note map + ~13KB stack for event buffers.
 *  Memory placement: active[16][128] in CCMRAM to preserve RAM.
 */
#ifndef MODULE_ENABLE_UI_PAGE_PIANOROLL
#define MODULE_ENABLE_UI_PAGE_PIANOROLL 1  // Enabled by default (main accordion page)
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

/** @brief Enable MIDI Delay FX (tempo-synced echo/delay effect)
 *  Note: Most synths have built-in delay effects. Disable to save 3KB RAM.
 *  When disabled, saves: ~3KB RAM (64 events × 12 bytes × 4 tracks)
 */
#ifndef MODULE_ENABLE_MIDI_DELAY_FX
#define MODULE_ENABLE_MIDI_DELAY_FX 0  // Disabled by default (most synths have delay)
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

/** @brief Enable Bootloader (USB MIDI firmware update) */
#ifndef MODULE_ENABLE_BOOTLOADER
#define MODULE_ENABLE_BOOTLOADER 1
#endif

/** 
 * @brief Bootloader Build Mode Configuration
 * 
 * Controls which parts of the project are built:
 * - 0 (BOOTLOADER_MODE_FULL): Build full project as single ELF (no bootloader separation)
 *   Uses STM32F407VGTX_FLASH.ld (1024KB from 0x08000000)
 * 
 * - 2 (BOOTLOADER_MODE_BOOTLOADER_ONLY): Build only bootloader (32KB)
 *   Uses STM32F407VGTX_FLASH_BOOT.ld (32KB from 0x08000000)
 *   Bootloader code only, no application code included
 * 
 * - 3 (BOOTLOADER_MODE_APP_ONLY): Build only application (992KB)
 *   Uses STM32F407VGTX_FLASH_APP.ld (992KB from 0x08008000)
 *   Application code only, expects bootloader at 0x08000000
 * 
 * NOTE: Set this via build configuration or compiler defines:
 *       -DBOOTLOADER_MODE=0  (or 2, or 3)
 */
#ifndef BOOTLOADER_MODE
#define BOOTLOADER_MODE 0  /* Default: Full project build */
#endif

/* Bootloader mode constants */
#define BOOTLOADER_MODE_FULL             0  /* Full project (no separation) */
#define BOOTLOADER_MODE_BOOTLOADER_ONLY  2  /* Bootloader only */
#define BOOTLOADER_MODE_APP_ONLY         3  /* Application only */

/** @brief Enable Logging service */
#ifndef MODULE_ENABLE_LOG
#define MODULE_ENABLE_LOG 1
#endif

/** @brief Enable CLI (Command Line Interface) for UART terminal control */
#ifndef MODULE_ENABLE_CLI
#define MODULE_ENABLE_CLI 1
#endif

/** @brief Enable MidiCore query debug messages in production mode
 * 
 * When enabled (MODULE_DEBUG_MIDICORE_QUERIES=1):
 * - MidiCore query processing debug messages are output via debug system
 * - Shows query reception, processing, and response sending
 * - Useful for debugging MIOS Studio terminal connection issues
 * - Output destination depends on MODULE_DEBUG_OUTPUT setting
 * 
 * Debug messages include:
 * - [MIDICORE-Q] Received query len:X cable:Y
 * - [MIDICORE-Q] dev_id:XX cmd:XX type:XX
 * - [MIDICORE-R] Sending type:XX "text" cable:Y
 * - [MIDICORE-R] Sent X bytes (success=1/0)
 * - [MIDICORE-R] ERROR messages if sending fails
 * 
 * When disabled (MODULE_DEBUG_MIDICORE_QUERIES=0):
 * - No MidiCore query debug output (saves code space)
 * - Query processing still works normally
 * 
 * Recommended: Enable for development/troubleshooting, can disable for production
 */
#ifndef MODULE_DEBUG_MIDICORE_QUERIES
#define MODULE_DEBUG_MIDICORE_QUERIES 1
#endif

/** @brief Enable Module Registry (required for CLI module control) */
#ifndef MODULE_ENABLE_MODULE_REGISTRY
#define MODULE_ENABLE_MODULE_REGISTRY 1
#endif

/** @brief Enable Stack Monitor (FreeRTOS stack usage monitoring)
 * 
 * When enabled:
 * - Provides runtime monitoring of task stack usage
 * - Detects stack overflows via 0xA5 pattern checking
 * - Configurable warning/critical thresholds
 * - CLI commands for stack inspection (stack, stack_all, stack_monitor)
 * - Minimal overhead (~512 bytes RAM for monitor task)
 * 
 * Recommended for development and production monitoring.
 */
#ifndef MODULE_ENABLE_STACK_MONITOR
#define MODULE_ENABLE_STACK_MONITOR 1
#endif

/** @brief Enable Test Module (runtime module testing via CLI)
 * 
 * WARNING: This module is deprecated and incomplete.
 * The test_cli.c file is not included in the build, causing linker errors.
 * 
 * Set to 0 to disable (recommended for production builds).
 */
#ifndef MODULE_ENABLE_TEST
#define MODULE_ENABLE_TEST 0
#endif

// =============================================================================
// PRODUCTION / TEST MODE CONFIGURATION
// =============================================================================

/**
 * @brief Production Mode - Final MidiCore build configuration
 * 
 * When PRODUCTION_MODE=1:
 * - All production modules are enabled (MIDI, Router, OLED, Looper, etc.)
 * - ALL test/debug code is excluded (saves ~25KB Flash)
 * - Production-grade initialization (oled_init_newhaven, etc.)
 * - Optimized binary for deployment
 * 
 * When PRODUCTION_MODE=0 (Development/Test):
 * - Test modules can be individually enabled
 * - Debug tasks available
 * - Test UI pages compiled
 * - Hardware verification tools available
 * 
 * This is the MASTER FLAG for production builds.
 * Set to 1 for final MidiCore hex compilation.
 */
#ifndef PRODUCTION_MODE
#define PRODUCTION_MODE 1  // Default: Production mode (final hex compilation)
#endif

// =============================================================================
// DEBUG/TEST MODULES (Automatically disabled in PRODUCTION_MODE)
// =============================================================================

/** @brief Enable AIN raw debug task (UART output of ADC values) */
#ifndef MODULE_ENABLE_AIN_RAW_DEBUG
#if PRODUCTION_MODE
#define MODULE_ENABLE_AIN_RAW_DEBUG 0  // Always disabled in production
#else
#define MODULE_ENABLE_AIN_RAW_DEBUG 0  // Disabled by default (can enable for testing)
#endif
#endif

/** @brief Enable MIDI DIN debug monitoring */
#ifndef MODULE_ENABLE_MIDI_DIN_DEBUG
#if PRODUCTION_MODE
#define MODULE_ENABLE_MIDI_DIN_DEBUG 0  // Always disabled in production
#else
#define MODULE_ENABLE_MIDI_DIN_DEBUG 0  // Disabled by default (can enable for testing)
#endif
#endif

/** @brief Enable USB MIDI debug output via UART
 * 
 * When enabled, outputs detailed USB enumeration and descriptor information
 * to UART for troubleshooting USB device issues.
 * 
 * To enable:
 * 1. Set this to 1, OR
 * 2. Uncomment #define USBD_MIDI_DEBUG in USB_DEVICE/Class/MIDI/Inc/usbd_midi_debug.h
 * 
 * Requires: printf redirected to UART
 * Output: USB setup requests, descriptor dumps, enumeration state
 * Usage: See Docs/USB_DEBUG_UART_QUICKSTART.md
 */
#ifndef MODULE_ENABLE_USB_MIDI_DEBUG
#if PRODUCTION_MODE
#define MODULE_ENABLE_USB_MIDI_DEBUG 0  // Always disabled in production
#else
#define MODULE_ENABLE_USB_MIDI_DEBUG 0  // Disabled by default (can enable for testing)
#endif
#endif

/** @brief Enable OLED SSD1322 test functions and test page
 * 
 * When enabled (MODULE_TEST_OLED=1):
 * - Compiles OLED test functions for hardware verification
 * - oled_init() - Simple MidiCore test init
 * - oled_init_progressive() - Step-by-step debug init
 * - oled_test_*() - Test patterns (checkerboard, gradients, etc.)
 * - ui_page_oled_test - Complete OLED test UI page
 * - RUNS OLED test at startup (StartDefaultTask)
 * 
 * When disabled (MODULE_TEST_OLED=0):
 * - No test code compiled (saves ~25KB Flash)
 * - Main application runs at startup
 * 
 * Production builds (PRODUCTION_MODE=1): Always disabled
 * Test builds (PRODUCTION_MODE=0): Set to 1 for OLED testing
 * 
 * To run OLED hardware test:
 * 1. Set PRODUCTION_MODE=0 (enable dev/test mode)
 * 2. Set MODULE_TEST_OLED=1 (compile test code AND run at startup)
 * 3. Rebuild and flash
 * 
 * Note: Production always uses oled_init_newhaven() regardless of this setting.
 */
#ifndef MODULE_TEST_OLED
#if PRODUCTION_MODE
#define MODULE_TEST_OLED 0  // Always disabled in production (saves ~25KB Flash)
#else
#define MODULE_TEST_OLED 0  // Disabled by default (set to 1 for OLED testing)
#endif
#endif

// MODULE_TEST_OLED_SSD1322 is now automatically set based on MODULE_TEST_OLED
// No need for separate flag - kept for backwards compatibility
#ifndef MODULE_TEST_OLED_SSD1322
#define MODULE_TEST_OLED_SSD1322 MODULE_TEST_OLED
#endif



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
