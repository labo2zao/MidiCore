/**
 * @file module_tests.h
 * @brief Unified module testing framework for MidiCore
 * 
 * This file provides a centralized testing infrastructure for all MidiCore modules.
 * Tests can be selected at compile time or runtime to isolate and validate individual modules.
 */

#ifndef MODULE_TESTS_H
#define MODULE_TESTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// =============================================================================
// TEST SELECTION
// =============================================================================

/**
 * @brief Available module tests
 * These can be enabled via compiler defines or runtime selection
 * 
 * Note: Enum values use _ID suffix to avoid conflicts with preprocessor defines
 * (e.g., MODULE_TEST_AINSER64_ID vs MODULE_TEST_AINSER64 define)
 */
typedef enum {
  MODULE_TEST_NONE_ID = 0,
  MODULE_TEST_GDB_DEBUG_ID,     // Test GDB debug / UART verification
  MODULE_TEST_AINSER64_ID,      // Test AINSER64 analog inputs
  MODULE_TEST_SRIO_ID,          // Test SRIO DIN (Digital Inputs)
  MODULE_TEST_SRIO_DOUT_ID,     // Test SRIO DOUT (Digital Outputs - LEDs)
  MODULE_TEST_MIDI_DIN_ID,      // Test MIDI DIN I/O
  MODULE_TEST_ROUTER_ID,        // Test MIDI router
  MODULE_TEST_LOOPER_ID,        // Test looper recording/playback
  MODULE_TEST_LFO_ID,           // Test LFO module (waveforms, modulation)
  MODULE_TEST_HUMANIZER_ID,     // Test Humanizer module (velocity/timing)
  MODULE_TEST_UI_ID,            // Test UI/OLED (general)
  MODULE_TEST_UI_PAGE_SONG_ID,      // Test Song Mode UI page
  MODULE_TEST_UI_PAGE_MIDI_MONITOR_ID, // Test MIDI Monitor UI page
  MODULE_TEST_UI_PAGE_SYSEX_ID,     // Test SysEx UI page
  MODULE_TEST_UI_PAGE_CONFIG_ID,    // Test Config Editor UI page
  MODULE_TEST_UI_PAGE_LIVEFX_ID,    // Test LiveFX UI page
  MODULE_TEST_UI_PAGE_RHYTHM_ID,    // Test Rhythm Trainer UI page
  MODULE_TEST_UI_PAGE_HUMANIZER_ID, // Test Humanizer/LFO UI page
  MODULE_TEST_PATCH_SD_ID,      // Test patch loading from SD
  MODULE_TEST_PRESSURE_ID,      // Test pressure sensor I2C
  MODULE_TEST_USB_HOST_MIDI_ID, // Test USB Host MIDI
  MODULE_TEST_USB_DEVICE_MIDI_ID, // Test USB Device MIDI (receive from DAW, print to UART, send test data)
  MODULE_TEST_OLED_SSD1322_ID,  // Test OLED SSD1322 driver (GPIO, SPI, display patterns)
  MODULE_TEST_FOOTSWITCH_ID,    // Test footswitch mapping validation (requires 8 footswitches)
  MODULE_TEST_ALL_ID,           // Run all tests sequentially
} module_test_t;

// =============================================================================
// TEST FUNCTIONS
// =============================================================================

/**
 * @brief Initialize the test framework
 * Call this before running any tests
 */
void module_tests_init(void);

/**
 * @brief Run a specific module test
 * @param test Test to run (see module_test_t)
 * @return 0 on success, negative on error
 * @note Most tests run in infinite loops and do not return
 */
int module_tests_run(module_test_t test);

/**
 * @brief Get the name of a test
 * @param test Test identifier
 * @return String name of the test
 */
const char* module_tests_get_name(module_test_t test);

// =============================================================================
// INDIVIDUAL MODULE TEST FUNCTIONS
// =============================================================================

/**
 * @brief Test GDB debug and UART verification
 * Simple test that prints to UART to verify debug connection
 * Ideal for verifying UART is working and baud rate is correct
 * @note This function runs forever
 */
void module_test_gdb_debug_run(void);

/**
 * @brief Test AINSER64 module (analog inputs)
 * Reads all 64 channels and outputs values via UART or OLED
 * @note This function runs forever
 */
void module_test_ainser64_run(void);

/**
 * @brief Test SRIO DIN module with MIDI output (Digital Inputs → MIDI)
 * 
 * Complete end-to-end test of the button input signal chain:
 * - Reads button inputs from 74HC165 shift registers (SRIO DIN)
 * - Generates MIDI Note On/Off messages when buttons are pressed/released
 * - Routes MIDI to USB MIDI OUT and DIN MIDI OUT1 (if router enabled)
 * 
 * Button to MIDI mapping:
 * - Button 0-63 → MIDI Notes 36-99 (C2 to D#7)
 * - Note On velocity: 100
 * - Note Off velocity: 0
 * - Channel: 1
 * 
 * Hardware tested:
 * - 74HC165 shift register chain (DIN inputs)
 * - SPI communication (MISO, SCK, /PL pins)
 * - MIDI Router (if enabled)
 * - USB MIDI output (if router enabled)
 * - DIN MIDI OUT1 (if router enabled)
 * 
 * Connect:
 * - Buttons to 74HC165 inputs (active low with pull-ups)
 * - USB cable to computer to receive MIDI notes
 * - Or DIN MIDI OUT1 to external synth/device
 * 
 * @note This function runs forever
 * @note Enable MODULE_ENABLE_ROUTER for MIDI output, otherwise only button detection
 */
void module_test_srio_run(void);

/**
 * @brief Test SRIO DOUT module (Digital Outputs - LEDs)
 * 
 * Comprehensive test of the LED output signal chain:
 * - Writes patterns to 74HC595 shift registers (SRIO DOUT)
 * - Cycles through 7 different visual patterns
 * - Tests all DOUT bytes and individual bits
 * - Verifies SPI MOSI, SCK, and RCLK signals
 * 
 * Test Patterns (2 seconds each):
 * 1. All LEDs ON (0x00) - Tests power and common connections
 * 2. All LEDs OFF (0xFF) - Tests LED disable
 * 3. Alternating (0xAA/0x55) - Tests adjacent bits independently
 * 4. Running light - Tests each individual LED sequentially
 * 5. Binary counter - Tests byte-level control and timing
 * 6. Wave pattern - Tests multi-chip synchronization
 * 7. Checkerboard (0x55) - Tests precise bit pattern accuracy
 * 
 * Hardware tested:
 * - 74HC595 shift register chain (DOUT outputs)
 * - SPI communication (MOSI, SCK pins)
 * - RCLK latch control (RC1 pin)
 * - All 8 DOUT bytes (64 LEDs)
 * - Daisy-chained register operation
 * 
 * Hardware connections (MIOS32 mbhp_doutx4):
 * - STM32 PB15 (SPI2 MOSI) → 74HC595 Pin 14 (SER - serial data)
 * - STM32 PB13 (SPI2 SCK)  → 74HC595 Pin 11 (SRCLK - shift clock)
 * - STM32 PB12 (RC1)       → 74HC595 Pin 12 (RCLK - register clock/latch)
 * - 74HC595 Pin 9 (QH')    → Next 74HC595 Pin 14 (daisy chain)
 * 
 * Visual verification:
 * - LEDs connected to 74HC595 outputs (active low typical)
 * - Watch patterns cycle to verify all outputs work
 * - Each pattern should be clearly visible for 2 seconds
 * 
 * Common issues diagnosed:
 * - No LEDs light: Check power, MOSI connection, RCLK pulse
 * - Random pattern: Check SPI clock, data line integrity
 * - Only some LEDs work: Check daisy chain connections (QH')
 * - Pattern frozen: Check RCLK (latch) signal
 * 
 * @note This function runs forever (continuous pattern cycling)
 * @note LEDs are typically active low (0 = ON, 1 = OFF)
 */
void module_test_srio_dout_run(void);

/**
 * @brief Test MIDI DIN module
 * Echoes MIDI input to output, shows status
 * @note This function runs forever
 */
void module_test_midi_din_run(void);

/**
 * @brief Test MIDI Router module
 * 
 * Comprehensive test of the MIDI routing matrix (16x16 nodes).
 * 
 * Tests:
 * - Route configuration (enable/disable)
 * - Channel filtering with chanmask
 * - Message routing between nodes (DIN, USB, Looper, etc.)
 * - Label assignment
 * - Multiple simultaneous routes
 * - Different MIDI message types (Note, CC, Sysex)
 * 
 * Hardware tested:
 * - DIN IN1-4 → DIN OUT1-4 routing
 * - USB Device IN/OUT → DIN routing
 * - Looper → Output routing with channel filtering
 * 
 * The test configures example routes and sends test messages to verify
 * the routing matrix operates correctly. Monitor UART output to see
 * routing table and test message flow.
 * 
 * @note This function runs forever
 */
void module_test_router_run(void);

/**
 * @brief Test Looper module
 * Tests recording, playback, overdub, quantization
 * @note This function runs forever
 */
void module_test_looper_run(void);

/**
 * @brief Test LFO module
 * 
 * Tests the Low Frequency Oscillator module functionality:
 * - All 6 waveforms (sine, triangle, saw, square, random, sample & hold)
 * - Rate control (0.01 - 10.0 Hz)
 * - Depth control (0 - 100%)
 * - BPM sync modes (free-running and synced to tempo)
 * - Modulation targets (velocity, timing, pitch)
 * - Phase reset functionality
 * 
 * Outputs waveform values to UART for verification.
 * Can be visualized using a serial plotter or oscilloscope.
 * 
 * @note This function runs forever
 */
void module_test_lfo_run(void);

/**
 * @brief Test Humanizer module
 * 
 * Tests the MIDI humanization module:
 * - Velocity humanization (0-32 range)
 * - Timing humanization (0-6 ticks)
 * - Intensity control (0-100%)
 * - Groove-aware micro-variations
 * - Enable/disable functionality
 * 
 * Sends test MIDI notes through the humanizer and outputs
 * the modified velocity/timing values to UART for verification.
 * 
 * @note This function runs forever
 */
void module_test_humanizer_run(void);

/**
 * @brief Test UI/OLED module
 * 
 * Tests the complete UI system including OLED display, page navigation,
 * button/encoder input handling, and status line updates.
 * 
 * Features tested:
 * - OLED SSD1322 display initialization and rendering
 * - UI page cycling (Looper, Timeline, Pianoroll, Router, Patch)
 * - Button input simulation and handling
 * - Rotary encoder input simulation
 * - Status line updates
 * - Graphics rendering
 * 
 * Hardware requirements:
 * - OLED Display: SSD1322 256x64 (grayscale)
 * - Control Input: Buttons + rotary encoder (via SRIO or GPIO)
 * 
 * Test sequence:
 * 1. Initialize OLED and UI subsystem
 * 2. Cycle through all available UI pages (auto-demo)
 * 3. Simulate button presses
 * 4. Simulate encoder rotation
 * 5. Update status messages
 * 6. Enter manual testing mode for visual verification
 * 
 * Usage: Enable MODULE_TEST_UI=1 in test configuration
 * Connect OLED display and observe automatic page cycling,
 * then test with actual buttons/encoders.
 */
void module_test_ui_run(void);

/**
 * @brief Test Song Mode UI page
 * 
 * Tests the Song Mode page which displays:
 * - Scene grid (A-H scenes)
 * - 4-track clip matrix
 * - Active scene indicator
 * - Scene playback controls
 * 
 * Tests button navigation, scene selection, and visual rendering.
 * 
 * @note This function runs forever
 */
void module_test_ui_page_song_run(void);

/**
 * @brief Test MIDI Monitor UI page
 * 
 * Tests the MIDI Monitor page which displays:
 * - Real-time MIDI message stream
 * - Message type (Note On/Off, CC, etc.)
 * - Channel, data bytes
 * - Message timestamps
 * 
 * Simulates MIDI messages and verifies display updates.
 * 
 * @note This function runs forever
 */
void module_test_ui_page_midi_monitor_run(void);

/**
 * @brief Test SysEx UI page
 * 
 * Tests the SysEx management page:
 * - SysEx message display
 * - Send/receive functionality
 * - Message formatting
 * - Navigation controls
 * 
 * @note This function runs forever
 */
void module_test_ui_page_sysex_run(void);

/**
 * @brief Test Config Editor UI page
 * 
 * Tests the system configuration page:
 * - Parameter editing (MIDI channel, clock source, etc.)
 * - Value adjustment with encoder
 * - Save/load configuration
 * - Navigation through config sections
 * 
 * @note This function runs forever
 */
void module_test_ui_page_config_run(void);

/**
 * @brief Test LiveFX UI page
 * 
 * Tests the Live Effects page:
 * - Transpose control
 * - Velocity scaling
 * - Scale/chord modes
 * - Real-time parameter adjustment
 * 
 * @note This function runs forever
 */
void module_test_ui_page_livefx_run(void);

/**
 * @brief Test Rhythm Trainer UI page
 * 
 * Tests the Rhythm Trainer page:
 * - Rhythm subdivision display (1/16, 1/8, 1/4, etc.)
 * - Metronome click visualization
 * - Timing accuracy feedback
 * - Practice mode controls
 * 
 * @note This function runs forever
 */
void module_test_ui_page_rhythm_run(void);

/**
 * @brief Test Humanizer/LFO UI page
 * 
 * Tests the combined Humanizer/LFO control page:
 * - Humanizer parameters (velocity, timing, intensity)
 * - LFO parameters (waveform, rate, depth, target)
 * - Mode switching between Humanizer and LFO views
 * - BPM sync toggle
 * - Visual waveform representation
 * 
 * @note This function runs forever
 */
void module_test_ui_page_humanizer_run(void);

/**
 * @brief Test Patch/SD module
 * Tests SD card mounting, file I/O, patch loading
 * @return 0 on success, negative on error
 */
int module_test_patch_sd_run(void);

/**
 * @brief Test Pressure sensor module
 * Tests I2C communication and value mapping
 * @note This function runs forever
 */
void module_test_pressure_run(void);

/**
 * @brief Test USB Host MIDI module
 * Tests USB device detection and MIDI communication
 * @note This function runs forever
 */
void module_test_usb_host_midi_run(void);

/**
 * @brief Test USB Device MIDI module
 * 
 * Tests USB Device MIDI functionality by receiving MIDI data from a DAW 
 * via USB and printing it to UART for debugging. Also sends test MIDI 
 * messages periodically to verify bidirectional communication.
 * 
 * Features tested:
 * - USB Device MIDI reception from DAW/computer
 * - MIDI packet decoding (status, data bytes)
 * - UART debug output of received MIDI data
 * - Periodic transmission of test MIDI messages via USB
 * - Note On/Off message transmission
 * - CC (Control Change) message transmission
 * 
 * Hardware requirements:
 * - USB connection to computer (DAW or MIDI monitoring software)
 * - UART connection for debug output (default: UART2 at 115200 baud)
 * 
 * Test sequence:
 * 1. Initialize USB Device MIDI and debug UART
 * 2. Print configuration information to UART
 * 3. Start listening for incoming USB MIDI from DAW
 * 4. Print each received MIDI message in human-readable format
 * 5. Periodically send test Note On/Off messages via USB
 * 6. Continue forever, logging all MIDI activity
 * 
 * UART output format:
 * - [RX] Cable:0 90 3C 64 (Note On Ch:1 Note:60 Vel:100)
 * - [TX] Sending test Note On: Cable:0 90 3C 64
 * 
 * Usage: 
 * - Enable MODULE_TEST_USB_DEVICE_MIDI=1 in test configuration
 * - Connect USB to computer with DAW or MIDI monitoring tool
 * - Connect UART to serial terminal (115200 baud)
 * - Send MIDI from DAW and observe UART output
 * - DAW should receive test MIDI messages every 2 seconds
 * 
 * @note This function runs forever
 */
void module_test_usb_device_midi_run(void);

/**
 * @brief Test OLED SSD1322 driver module
 * 
 * Comprehensive test of the SSD1322 OLED display driver with detailed
 * UART debug output for systematic validation of the software SPI
 * bit-bang implementation and display communication.
 * 
 * Features tested:
 * - GPIO pin control (PA8, PC8, PC9, PC11) with read-back verification
 * - Software SPI bit-bang timing (Mode 0: CPOL=0, CPHA=0)
 * - OLED initialization sequence with duration measurement
 * - Display test patterns (white, black, checkerboard, stripes, gradient)
 * - DWT cycle counter timing precision
 * - MIOS32-compatible pin mapping
 * 
 * Hardware requirements:
 * - OLED Display: SSD1322 256x64 grayscale
 * - Connections: PA8=DC, PC8/PC9=SCL, PC11=SDA, GND, 3.3V
 * - UART connection for debug output (default: UART2 at 115200 baud)
 * 
 * Test sequence:
 * 1. Print pin mapping and timing specifications
 * 2. Test GPIO control with PASS/FAIL for each pin
 * 3. Initialize OLED and measure init duration (~2100 ms expected)
 * 4. Display test patterns:
 *    - All white (2 sec)
 *    - All black (2 sec)
 *    - Checkerboard (2 sec)
 *    - Horizontal stripes (2 sec)
 *    - Grayscale gradient (2 sec)
 * 5. Print test summary and troubleshooting guidance
 * 
 * UART output includes:
 * - Pin mapping (MIOS32 compatible: J15_SER/RS, J15_E1/E2, J15_RW)
 * - SPI timing specs vs SSD1322 datasheet requirements
 * - GPIO test results (PASS/FAIL per pin)
 * - Init duration measurement
 * - Pattern test progress
 * - Troubleshooting tips if display doesn't work
 * 
 * SPI Timing verification:
 * - Setup time: 101 ns (meets >15 ns requirement)
 * - Hold time: 101 ns (meets >10 ns requirement)
 * - Clock period: ~200 ns (meets >100 ns requirement)
 * - Clock frequency: ~5 MHz (under 10 MHz max)
 * 
 * Troubleshooting steps provided:
 * - Power verification (3.3V at OLED VCC)
 * - Wiring check (all 5 connections)
 * - GPIO read-back test results
 * - Logic analyzer guidance (if available)
 * 
 * Usage:
 * - Enable MODULE_TEST_OLED_SSD1322=1 in test configuration
 * - Connect OLED display and UART terminal (115200 baud)
 * - Observe test patterns on display and monitor UART for diagnostics
 * 
 * @return 0 on success, negative on error
 * @note Returns after pattern tests complete (unlike most tests that run forever)
 */
int module_test_oled_ssd1322_run(void);

/**
 * @brief Test Footswitch module
 * 
 * Comprehensive test of the footswitch mapping system (8 footswitches).
 * 
 * Tests:
 * - Footswitch input detection via SRIO DIN
 * - Mapping configuration for all 13 action types
 * - Action execution (Play/Stop, Record, Overdub, Undo, Redo, etc.)
 * - Looper integration
 * - Button press/release detection
 * - Real-time status display
 * 
 * Hardware requirements:
 * - 8 footswitches connected to SRIO DIN inputs (buttons 0-7)
 * - SRIO hardware (74HC165 shift registers)
 * - UART connection for debug output (115200 baud)
 * 
 * Test sequence:
 * 1. Initialize SRIO for button input
 * 2. Initialize looper module
 * 3. Configure 8 footswitch mappings to test all actions:
 *    - FS0: Play/Stop (Track 0)
 *    - FS1: Record (Track 0)
 *    - FS2: Overdub (Track 0)
 *    - FS3: Undo (Track 0)
 *    - FS4: Mute (Track 1)
 *    - FS5: Tap Tempo
 *    - FS6: Trigger Scene (Scene A/0)
 *    - FS7: Clear (Track 0)
 * 4. Monitor button presses continuously
 * 5. Display action triggered for each footswitch press
 * 6. Display looper state changes
 * 
 * UART output format:
 * - Footswitch mapping table
 * - Button press/release events
 * - Action triggered for each footswitch
 * - Looper state changes (Play, Record, etc.)
 * 
 * Usage:
 * - Enable MODULE_TEST_FOOTSWITCH=1 in test configuration
 * - Connect 8 footswitches to SRIO DIN inputs (buttons 0-7)
 * - Connect UART to serial terminal (115200 baud)
 * - Press footswitches and observe action execution
 * - Verify each footswitch triggers correct action
 * 
 * @note This function runs forever
 */
void module_test_footswitch_run(void);

// =============================================================================
// COMPILE-TIME TEST SELECTION HELPERS
// =============================================================================

/**
 * @brief Get the test selected at compile time
 * @return Selected test or MODULE_TEST_NONE if no compile-time selection
 */
module_test_t module_tests_get_compile_time_selection(void);

#ifdef __cplusplus
}
#endif

#endif // MODULE_TESTS_H
