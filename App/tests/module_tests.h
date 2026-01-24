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
  MODULE_TEST_BREATH_ID,        // Test breath controller (pressure sensor + expression/MIDI CC output)
  MODULE_TEST_USB_HOST_MIDI_ID, // Test USB Host MIDI
  MODULE_TEST_USB_DEVICE_MIDI_ID, // Test USB Device MIDI (receive from DAW, print to UART, send test data)
  MODULE_TEST_OLED_SSD1322_ID,  // Test OLED SSD1322 driver (GPIO, SPI, display patterns)
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
 * @brief Test MIDI DIN module with LiveFX transform and MIDI learn
 * 
 * Comprehensive test of MIDI DIN I/O with real-time processing:
 * - MIDI I/O: Receives from DIN IN, sends to DIN OUT
 * - LiveFX Transform: Transpose, velocity scaling, force-to-scale
 * - MIDI Learn: Map CC messages to LiveFX parameters
 * 
 * Features tested:
 * - MIDI DIN input/output communication
 * - Real-time MIDI message transformation
 * - Live parameter control via CC messages
 * - Transpose (-12 to +12 semitones)
 * - Velocity scaling (0-200%)
 * - Musical scale quantization (force-to-scale)
 * 
 * MIDI Learn Commands (Channel 1):
 * - CC 20: Enable/Disable LiveFX (value > 64 = enabled)
 * - CC 21: Transpose down (-1 semitone)
 * - CC 22: Transpose up (+1 semitone)
 * - CC 23: Transpose reset (0)
 * - CC 24: Velocity scale down (-10%)
 * - CC 25: Velocity scale up (+10%)
 * - CC 26: Velocity scale reset (100%)
 * - CC 27: Force-to-scale toggle (value > 64 = on)
 * - CC 28: Scale type (0-11)
 * - CC 29: Scale root (0=C, 1=C#, ..., 11=B)
 * 
 * Test workflow:
 * 1. Connect MIDI controller to DIN IN1
 * 2. Connect DIN OUT1 to synth/DAW
 * 3. Send CC 20 (value 127) to enable LiveFX
 * 4. Send CC 22 to transpose notes up
 * 5. Play notes - they will be transposed and sent to OUT
 * 6. Send CC 27 (value 127) to enable force-to-scale
 * 7. Play notes - they will snap to selected scale
 * 8. Monitor UART debug output for status
 * 
 * Hardware requirements:
 * - MIDI DIN IN/OUT hardware (UART-based)
 * - MIDI controller or test device
 * - UART debug connection (115200 baud)
 * 
 * Enable with: MODULE_TEST_MIDI_DIN=1
 * Requires: MODULE_ENABLE_MIDI_DIN=1, MODULE_ENABLE_LIVEFX=1, MODULE_ENABLE_ROUTER=1
 * 
 * @note This function runs forever
 */
void module_test_midi_din_run(void);

/**
 * @brief Test MIDI Router module - Comprehensive
 * 
 * Comprehensive validation of the MIDI routing matrix (16x16 nodes).
 * 
 * Test Phases:
 * 1. Router initialization - matrix setup, node mapping display
 * 2. Basic routing - single source to single destination configuration
 * 3. Channel filtering - per-channel route control (16 MIDI channels)
 * 4. Message types - Note On/Off, CC, PC, Pressure, Pitch Bend routing
 * 5. Multi-destination routing - one source to multiple outputs
 * 6. Route modification - dynamic enable/disable functionality
 * 7. Channel validation - mask filtering with multiple channels
 * 8. Routing table - complete active route display with labels
 * 
 * Features tested:
 * - Route enable/disable per connection
 * - Channel filtering (16-bit chanmask) for each route
 * - All MIDI message types: Note, CC, PC, Pressure, Pitch Bend
 * - Multi-destination routing (1→N outputs)
 * - Route labels (16-character names)
 * - Dynamic route modification at runtime
 * - Continuous monitoring with periodic statistics
 * 
 * Hardware tested:
 * - DIN IN1-4 → DIN OUT1-4 routing
 * - USB Device (4 ports) ↔ DIN routing
 * - USB Host IN/OUT → DIN routing
 * - Internal nodes (Looper, Keys) → Output routing
 * - Channel-specific filtering per route
 * 
 * The test configures multiple example routes, sends test messages through
 * each route type, validates channel filtering, and displays the complete
 * routing table. After automated tests, it enters continuous monitoring mode
 * with periodic status updates.
 * 
 * Monitor UART output (115200 baud) to see:
 * - Detailed test phase descriptions
 * - Test message routing verification
 * - Complete routing table with channel masks
 * - Test summary with ✓ indicators
 * - Continuous monitoring status
 * 
 * Duration: ~5 seconds for automated tests, then continuous monitoring
 * 
 * @note This function runs forever in monitoring mode after tests complete
 */
void module_test_router_run(void);

/**
 * @brief Test Looper module
 * 
 * Comprehensive automated test of the MIDI Looper module covering all features:
 * 
 * **Core Features (Phases 1-7):**
 * - Multi-track recording and playback (4 tracks)
 * - Recording, playback, and overdub modes
 * - Quantization modes (OFF, 1/16, 1/8, 1/4 notes)
 * - Mute/Solo track controls
 * - Scene management (8 scenes with track snapshots)
 * - Transport controls (tempo, time signature, tap tempo)
 * - Advanced features (LFO, humanizer, undo/redo)
 * 
 * **Extended Features (Phases 8-11):**
 * - Step mode with cursor control (step read/write)
 * - Track randomization (velocity/timing variations)
 * - Multi-track simultaneous playback
 * - Save/Load tracks to SD card
 * 
 * **Advanced Testing (Phases 12-17):**
 * - Scene chaining and automation
 * - Router integration (multi-source MIDI)
 * - Stress testing (rapid input, buffer limits)
 * - Error recovery and edge cases
 * - Performance benchmarks
 * - Humanizer/LFO modulation validation
 * 
 * Test sequence (27 phases total, ~190-250s runtime):
 * 
 * **Phase 1-7: Core Features**
 * 1. Initialize looper and configure transport (120 BPM, 4/4 time)
 * 2. Test basic recording with MIDI note sequence (C4, E4, G4)
 * 3. Test playback and event export
 * 4. Test overdub mode (add C5 to existing loop)
 * 5. Test quantization modes and verification
 * 6. Test mute/solo controls and audibility checks
 * 7. Test scene save/load and scene switching
 * 
 * **Phase 8-11: Extended Features**
 * 8. Test advanced features (tempo tap, humanizer, LFO, undo/redo)
 * 9. **Test step mode - manual cursor control (step read/write)**
 *    - Enable/disable step mode
 *    - Step forward event-by-event (step read)
 *    - Step forward by fixed ticks
 *    - Step backward navigation
 *    - Direct cursor positioning (step write)
 *    - Step size configuration
 * 10. **Test track randomization**
 *     - Configure randomization parameters
 *     - Apply velocity and timing randomization
 *     - Verify event modifications
 * 11. **Test multi-track simultaneous operation**
 *     - Record different patterns on tracks 1-3
 *     - Test multi-track mute/solo
 *     - Validate all tracks playing together
 * 
 * **Phase 12-17: Advanced Testing**
 * 12. **Test save/load to SD card**
 *     - Save track to file
 *     - Clear and reload track
 *     - Verify event restoration
 * 13. **Test scene chaining**
 *     - Configure scene chain (0→1→2→0)
 *     - Test automatic scene transitions
 *     - Verify chain configuration and enable/disable
 * 14. **Test router integration**
 *     - MIDI recording from DIN IN, USB Device, USB Host
 *     - Verify multi-source event routing
 *     - Validate event attribution
 * 15. **Test stress conditions**
 *     - Rapid MIDI note sequence (50ms intervals)
 *     - Near-buffer capacity (100+ events)
 *     - Extended recording time (16 beats, 8 seconds)
 * 16. **Test error recovery**
 *     - Invalid track indices
 *     - Rapid state transitions
 *     - Operations on empty tracks
 *     - Extreme parameter values
 *     - Concurrent track operations
 * 17. **Test performance benchmarks**
 *     - Event recording speed (ms per event)
 *     - Event export performance
 *     - State transition latency
 *     - Scene operation timing
 * 
 * **Phase 18: Humanizer/LFO Validation**
 * 18. **Test humanizer/LFO validation**
 *     - Record identical notes
 *     - Apply humanization and compare
 *     - Validate velocity/timing variations
 *     - Test LFO configuration and BPM sync
 * 
 * **Phase 19-25: Professional Features**
 * 19. **Test global transpose**
 *     - Transpose all tracks up/down by semitones
 *     - Verify transpose settings and read-back
 * 20. **Test track quantization**
 *     - Record off-beat notes
 *     - Apply quantization (1/16 note = 24 ticks)
 *     - Compare before/after tick positions
 * 21. **Test copy/paste**
 *     - Copy track data to clipboard
 *     - Paste to different track
 *     - Verify event count matches
 * 22. **Test footswitch control**
 *     - Configure 5 footswitch actions (play/stop, record, mute, solo, scene)
 *     - Simulate footswitch press/release
 *     - Verify state changes
 * 23. **Test MIDI learn**
 *     - Start MIDI learn mode
 *     - Map CC#80 to Play/Stop
 *     - Map Note C5 to Mute Track 0
 *     - Test cancel functionality
 *     - Display mapping count
 * 24. **Test quick-save/load**
 *     - Save session to slot (tempo, scene, track data)
 *     - Modify current session
 *     - Load from slot and verify restoration
 *     - Test 4 slots (0-3)
 *     - Clear slot operation
 * 25. **Test event editing**
 *     - Export events for examination
 *     - Edit velocity (80→127)
 *     - Edit tick position
 *     - Edit note pitch (E4→G4)
 *     - Verify changes applied
 * 
 * **Phase 27: CC Automation Layer**
 * 27. **Test CC Automation Layer (Production API)**
 *     - Start/stop automation recording
 *     - Record CC messages (CC10/Pan, CC1/Mod Wheel, CC7/Volume)
 *     - Export automation events (tick, CC number, value, channel)
 *     - Manual event addition
 *     - Enable/disable automation playback
 *     - Synchronized CC playback with loop (2 seconds demonstration)
 *     - Clear automation functionality
 * 
 * **Phase 28: Continuous Monitor**
 * After phase 27, enters continuous monitoring mode that:
 * - Allows live MIDI recording/playback via DIN IN or USB
 * - Prints status updates every 30 seconds
 * - Shows track states, event counts, mute/solo status
 * 
 * Hardware requirements:
 * - UART connection for debug output (115200 baud)
 * - Optional: MIDI DIN or USB input for live testing
 * - Optional: MIDI output to hear playback
 * - Optional: SD card for save/load testing
 * - Optional: Router module for integration testing
 * - Optional: Footswitches for footswitch control testing
 * 
 * Expected duration: ~190-250 seconds for automated tests (27 phases)
 * 
 * Output: Comprehensive UART debug log with:
 * - Phase-by-phase progress and results
 * - MIDI event details (note values, velocities, timing)
 * - Track state information (recording/playback/overdub)
 * - Scene management and chaining operations
 * - Step mode cursor positions and navigation
 * - Randomization effects (before/after comparison)
 * - Multi-track status and interactions
 * - Save/load results and verification
 * - Router integration (multi-source events)
 * - Stress test results (capacity, timing)
 * - Error recovery validation
 * - Performance benchmark measurements
 * - Humanizer/LFO modulation validation
 * - Global transpose operations
 * - Track quantization (before/after comparison)
 * - Copy/paste verification
 * - Footswitch action configuration
 * - MIDI learn mapping creation
 * - Quick-save/load session management
 * - Event editing validation
 * - **CC Automation Layer (production API)**
 * - **CC recording/playback synchronized with loop**
 * - **Manual CC event addition and export**
 * - Test summary with PASS/FAIL results for all 27 phases
 * 
 * Usage: Enable MODULE_TEST_LOOPER=1 in test configuration
 * Connect UART terminal (115200 baud) to observe test execution
 * Optionally connect MIDI input/output for live interaction
 * Optionally connect SD card for save/load testing
 * 
 * @note This function runs forever after completing automated tests
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
 * @brief Test UI/OLED module with comprehensive automated navigation
 * 
 * Comprehensive automated test of the complete UI system including OLED display,
 * page navigation, button/encoder input handling, status line updates, and all
 * OLED test modes (including SSD1322 enhancements).
 * 
 * Features tested:
 * - OLED SSD1322 display initialization and rendering
 * - All UI pages: Looper, Timeline, Pianoroll, Song, MIDI Monitor, SysEx,
 *   Config, LiveFX, Rhythm, Humanizer (if enabled), OLED Test
 * - Direct page navigation via ui_set_page()
 * - Button-based navigation (Button 5 cycles through all pages)
 * - Rotary encoder navigation on OLED test page
 * - All 29 OLED test modes (0-28) including:
 *   - Pattern/Grayscale tests (0-6)
 *   - Animations (7-10, 15-19)
 *   - Advanced graphics (11-14, 17-19)
 *   - Hardware driver tests (20-27)
 *   - Vortex tunnel demo (28)
 * - Encoder stress test (rapid forward/backward, large jumps)
 * - Status line updates with various message lengths
 * - Graphics rendering validation
 * 
 * Hardware requirements:
 * - OLED Display: SSD1322 256x64 (grayscale, Software SPI)
 * - Control Input: Buttons + rotary encoder (via SRIO DIN or GPIO)
 * 
 * Test sequence (6 phases):
 * 1. Initialize OLED and UI subsystem
 * 2. Test direct page navigation through all UI pages (3s per page)
 * 3. Test button-based navigation (Button 5, full cycle)
 * 4. Test all 29 OLED test modes with encoder navigation
 * 5. Encoder stress test (rapid changes, large jumps)
 * 6. Status line validation with various messages
 * 7. Enter manual testing mode for visual verification
 * 
 * Expected duration: ~2-3 minutes for automated tests
 * 
 * Output: Comprehensive UART debug log with test results including:
 * - Phase-by-phase progress
 * - Pass/fail status for each test
 * - Detailed mode descriptions for OLED tests
 * - Final summary with all test results
 * 
 * Usage: Enable MODULE_TEST_UI=1 in test configuration
 * Connect OLED display and UART terminal (115200 baud)
 * Observe automated test execution, then test with actual buttons/encoders
 * 
 * @note This function runs forever after completing automated tests
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
 * @brief Test Breath Controller module (pressure sensor + expression/MIDI output)
 * 
 * Comprehensive test of the complete breath controller signal chain:
 * - Pressure sensor I2C communication (XGZP6847D or generic 16-bit sensors)
 * - Raw sensor value reading (24-bit or 16-bit ADC)
 * - Pressure conversion to Pascal (Pa) units
 * - Expression module pressure-to-CC mapping
 * - MIDI CC output generation (typically CC#2 for breath controller)
 * - Real-time value monitoring via UART debug output
 * 
 * Hardware tested:
 * - I2C pressure sensor (XGZP6847D 24-bit or generic 16-bit I2C ADC)
 * - Pressure module: sensor reading and calibration
 * - Expression module: curve application (linear/expo/S-curve), deadband, hysteresis
 * - MIDI Router: CC message routing to USB/DIN outputs
 * - Bidirectional support: Push (inhale) and Pull (exhale) for accordion/bellows
 * 
 * Test output (UART debug):
 * - Sensor type and I2C configuration
 * - Raw sensor values (0-16777215 for 24-bit, 0-65535 for 16-bit)
 * - Pressure in Pascal (Pa) - signed relative to atmospheric zero
 * - 12-bit mapped value (0-4095)
 * - MIDI CC number and value (0-127)
 * - Update rate and timing statistics
 * 
 * Configuration sources:
 * - SD card: pressure.ngc (sensor config), expression.ngc (CC mapping)
 * - Defaults: If SD files missing, uses hardcoded defaults
 * 
 * Typical breath controller mapping:
 * - CC#2 (Breath Controller) for wind instruments
 * - CC#11 (Expression) for general use
 * - Bidirectional: CC#11 (push/inhale), CC#2 (pull/exhale) for accordion
 * 
 * Hardware connections:
 * - I2C sensor: SCL/SDA (typically I2C1 or I2C2)
 * - Pull-up resistors: 4.7kΩ on SCL/SDA
 * - Power: 3.3V to sensor VCC, GND
 * 
 * Troubleshooting output:
 * - I2C communication errors (NACK, timeout)
 * - Sensor not detected
 * - Incorrect I2C address
 * - Missing configuration files
 * - Invalid pressure readings
 * 
 * Performance characteristics:
 * - Latency: <5ms from breath to MIDI CC output
 * - Resolution: 12-bit (4096 levels) mapped to 7-bit MIDI (0-127)
 * - Update rate: Configurable 5-50ms (default 20ms)
 * - Smoothing: EMA filter to reduce jitter
 * 
 * Use cases:
 * - Wind controller setup and calibration
 * - Accordion bellows pressure mapping
 * - Breath-controlled synthesizers
 * - Expression pedal alternative
 * - Hardware validation and troubleshooting
 * 
 * @note This function runs forever (continuous monitoring loop)
 * @note Requires MODULE_ENABLE_PRESSURE and expression module enabled
 * @note Connect UART terminal at 115200 baud to view debug output
 * @note Blow/suck on breath sensor to see values change
 */
void module_test_breath_run(void);

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
