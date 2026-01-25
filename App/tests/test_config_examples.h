/**
 * @file test_config_examples.h
 * @brief Example configurations for module testing
 * 
 * This file provides example configurations for testing different modules.
 * Copy the relevant section to your build configuration or use as reference.
 */

#ifndef TEST_CONFIG_EXAMPLES_H
#define TEST_CONFIG_EXAMPLES_H

// =============================================================================
// EXAMPLE 1: Test AINSER64 Module Only (Minimal Build)
// =============================================================================
/*
 * Use Case: Test analog input hardware and MCP3208 communication
 * 
 * Build Defines:
 *   MODULE_TEST_AINSER64=1
 * 
 * module_config.h Settings:
 *   MODULE_ENABLE_AINSER64=1
 *   MODULE_ENABLE_SPI_BUS=1
 *   MODULE_ENABLE_AIN=1
 *   (All other modules can be 0)
 * 
 * Expected Behavior:
 *   - Continuously scans 64 analog channels
 *   - Outputs values via UART2
 *   - Updates at ~10ms intervals
 * 
 * Hardware Required:
 *   - MCP3208 on SPI1
 *   - 74HC4051 multiplexers
 *   - Analog inputs connected
 *   - UART2 for debug output
 */

// =============================================================================
// EXAMPLE 2: Test SRIO Module (Digital I/O)
// =============================================================================
/*
 * Use Case: Test button matrix and LED outputs
 * 
 * Build Defines:
 *   MODULE_TEST_SRIO=1
 * 
 * module_config.h Settings:
 *   MODULE_ENABLE_SRIO=1
 *   MODULE_ENABLE_SPI_BUS=1
 *   (All other modules can be 0)
 * 
 * Expected Behavior:
 *   - Reads DIN shift registers (74HC165)
 *   - Outputs DIN state as hex via UART
 *   - Optionally toggles DOUT (74HC595)
 * 
 * Hardware Required:
 *   - 74HC165 for DIN
 *   - 74HC595 for DOUT
 *   - Buttons connected to DIN
 *   - LEDs connected to DOUT
 *   - UART2 for debug output
 */

// =============================================================================
// EXAMPLE 3: Test MIDI DIN with Router
// =============================================================================
/*
 * Use Case: Test MIDI input/output and routing
 * 
 * Build Defines:
 *   MODULE_TEST_MIDI_DIN=1
 * 
 * module_config.h Settings:
 *   MODULE_ENABLE_MIDI_DIN=1
 *   MODULE_ENABLE_ROUTER=1
 *   MODULE_ENABLE_SRIO=1
 *   (For button input)
 * 
 * Expected Behavior:
 *   - Reads buttons from SRIO
 *   - Sends MIDI notes on button press
 *   - Routes MIDI through configured paths
 *   - Echoes to UART and USB Host
 * 
 * Hardware Required:
 *   - MIDI DIN connectors (IN/OUT)
 *   - UART configured for MIDI
 *   - Button matrix on SRIO
 *   - Optional: USB Host for MIDI output
 */

// =============================================================================
// EXAMPLE 4: Test Looper with Full MIDI Stack
// =============================================================================
/*
 * Use Case: Test MIDI recording and playback
 * 
 * Build Defines:
 *   MODULE_TEST_LOOPER=1
 * 
 * module_config.h Settings:
 *   MODULE_ENABLE_LOOPER=1
 *   MODULE_ENABLE_ROUTER=1
 *   MODULE_ENABLE_MIDI_DIN=1
 *   MODULE_ENABLE_PATCH=1
 *   (For loading looper config)
 * 
 * Expected Behavior:
 *   - Automatically cycles: REC (7s) → PLAY (8s) → STOP (2s)
 *   - Records MIDI events during REC phase
 *   - Plays back during PLAY phase
 *   - Repeats indefinitely
 * 
 * Hardware Required:
 *   - MIDI input for recording
 *   - MIDI output for playback
 *   - Optional: SD card for patch config
 */

// =============================================================================
// EXAMPLE 5: Test UI/OLED Display
// =============================================================================
/*
 * Use Case: Test graphics and user interface
 * 
 * Build Defines:
 *   MODULE_TEST_UI=1
 * 
 * module_config.h Settings:
 *   MODULE_ENABLE_UI=1
 *   MODULE_ENABLE_OLED=1
 *   MODULE_ENABLE_SPI_BUS=1
 *   MODULE_ENABLE_INPUT=1
 *   (For encoders/buttons)
 * 
 * Expected Behavior:
 *   - Displays UI on OLED
 *   - Responds to encoder rotation
 *   - Shows different pages
 *   - Updates at ~30 Hz
 * 
 * Hardware Required:
 *   - SSD1322 OLED display on SPI
 *   - Encoders for navigation
 *   - Buttons for selection
 */

// =============================================================================
// EXAMPLE 6: Test SD Card and Patch Loading
// =============================================================================
/*
 * Use Case: Test file system and configuration loading
 * 
 * Build Defines:
 *   MODULE_TEST_PATCH_SD=1
 * 
 * module_config.h Settings:
 *   MODULE_ENABLE_PATCH=1
 *   (All other modules can be 0 for minimal test)
 * 
 * Expected Behavior:
 *   - Attempts to mount SD card
 *   - Returns 0 on success, -1 on failure
 *   - Can be extended to load/save patches
 * 
 * Hardware Required:
 *   - SD card slot (SDIO or SPI)
 *   - Formatted SD card
 *   - UART2 for status output
 */

// =============================================================================
// EXAMPLE 7: Integration Test (Multiple Modules)
// =============================================================================
/*
 * Use Case: Test interaction between modules
 * 
 * Build Defines:
 *   MODULE_TEST_ROUTER=1
 * 
 * module_config.h Settings:
 *   MODULE_ENABLE_AINSER64=1
 *   MODULE_ENABLE_SRIO=1
 *   MODULE_ENABLE_ROUTER=1
 *   MODULE_ENABLE_MIDI_DIN=1
 *   MODULE_ENABLE_LOOPER=1
 * 
 * Expected Behavior:
 *   - Buttons from SRIO control looper
 *   - AINSER64 generates MIDI CC
 *   - Router forwards messages
 *   - Looper records/plays MIDI
 * 
 * Hardware Required:
 *   - All hardware combined
 *   - Comprehensive system test
 */

// =============================================================================
// EXAMPLE 8: Production Configuration (No Tests)
// =============================================================================
/*
 * Use Case: Normal operation, all features enabled
 * 
 * Build Defines:
 *   (None - no MODULE_TEST_xxx defined)
 * 
 * module_config.h Settings:
 *   (All required modules = 1 per your hardware)
 * 
 * Expected Behavior:
 *   - Full MidiCore application runs
 *   - app_entry_start() is called
 *   - All configured modules active
 *   - Normal user operation
 * 
 * Hardware Required:
 *   - Complete MidiCore hardware
 */

#endif // TEST_CONFIG_EXAMPLES_H
