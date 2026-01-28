#!/bin/bash
# Comprehensive CLI System Verification Script
# Checks ALL CLI files for common compilation issues

set -e

echo "=========================================="
echo "MidiCore CLI System - Complete Verification"
echo "=========================================="
echo ""

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

ERRORS=0
WARNINGS=0

# =============================================================================
# Test 1: Check all CLI files exist
# =============================================================================
echo -e "${BLUE}Test 1: Checking CLI file existence...${NC}"

CLI_FILES=(
  "ain_cli.c"
  "ainser_map_cli.c"
  "arpeggiator_cli_integration.c"
  "assist_hold_cli.c"
  "bass_chord_system_cli.c"
  "bellows_expression_cli.c"
  "bellows_shake_cli.c"
  "bootloader_cli.c"
  "cc_smoother_cli.c"
  "channelizer_cli.c"
  "chord_cli.c"
  "config_cli.c"
  "config_io_cli.c"
  "din_map_cli.c"
  "dout_map_cli.c"
  "dream_cli.c"
  "envelope_cc_cli.c"
  "expression_cli.c"
  "footswitch_cli.c"
  "gate_time_cli.c"
  "harmonizer_cli.c"
  "humanize_cli.c"
  "instrument_cli.c"
  "legato_cli.c"
  "lfo_cli.c"
  "livefx_cli.c"
  "log_cli.c"
  "looper_cli.c"
  "metronome_cli.c"
  "midi_converter_cli.c"
  "midi_delay_cli.c"
  "midi_filter_cli.c"
  "midi_monitor_cli.c"
  "musette_detune_cli.c"
  "note_repeat_cli.c"
  "note_stabilizer_cli.c"
  "one_finger_chord_cli.c"
  "patch_cli.c"
  "performance_cli.c"
  "program_change_mgr_cli.c"
  "quantizer_cli.c"
  "register_coupling_cli.c"
  "rhythm_trainer_cli.c"
  "router_cli.c"
  "scale_cli.c"
  "strum_cli.c"
  "swing_cli.c"
  "system_cli.c"
  "ui_cli.c"
  "usb_cdc_cli.c"
  "usb_host_midi_cli.c"
  "usb_midi_cli.c"
  "usb_msc_cli.c"
  "velocity_compressor_cli.c"
  "watchdog_cli.c"
  "zones_cli.c"
)

FOUND=0
for FILE in "${CLI_FILES[@]}"; do
  if [ -f "Services/cli/$FILE" ]; then
    FOUND=$((FOUND + 1))
  else
    echo -e "  ${RED}✗${NC} Missing: $FILE"
    ERRORS=$((ERRORS + 1))
  fi
done

echo -e "  ${GREEN}✓${NC} Found $FOUND / ${#CLI_FILES[@]} CLI files"
echo ""

# =============================================================================
# Test 2: Check module descriptors exist
# =============================================================================
echo -e "${BLUE}Test 2: Checking module descriptors...${NC}"

CHECKED=0
for FILE in "${CLI_FILES[@]}"; do
  if [ -f "Services/cli/$FILE" ]; then
    if grep -q "static module_descriptor_t s_.*_descriptor" "Services/cli/$FILE"; then
      CHECKED=$((CHECKED + 1))
    else
      # Some files like router_cli.c may use different patterns
      if grep -q "router_cli_register" "Services/cli/$FILE" && [ "$FILE" = "router_cli.c" ]; then
        CHECKED=$((CHECKED + 1))
      elif grep -q "// No module descriptor" "Services/cli/$FILE"; then
        # Intentionally no descriptor (e.g., utility CLI files)
        CHECKED=$((CHECKED + 1))
      else
        echo -e "  ${YELLOW}⚠${NC}  $FILE may be missing module descriptor"
        WARNINGS=$((WARNINGS + 1))
      fi
    fi
  fi
done

echo -e "  ${GREEN}✓${NC} Checked $CHECKED / ${#CLI_FILES[@]} files for descriptors"
echo ""

# =============================================================================
# Test 3: Check for DEFINE_PARAM macro usage
# =============================================================================
echo -e "${BLUE}Test 3: Checking parameter wrapper macros...${NC}"

FILES_WITH_PARAMS=0
for FILE in "${CLI_FILES[@]}"; do
  if [ -f "Services/cli/$FILE" ]; then
    if grep -q "DEFINE_PARAM" "Services/cli/$FILE"; then
      FILES_WITH_PARAMS=$((FILES_WITH_PARAMS + 1))
    fi
  fi
done

echo -e "  ${GREEN}✓${NC} $FILES_WITH_PARAMS files use DEFINE_PARAM macros"
echo ""

# =============================================================================
# Test 4: Check for registration functions
# =============================================================================
echo -e "${BLUE}Test 4: Checking registration functions...${NC}"

FILES_WITH_REGISTER=0
for FILE in "${CLI_FILES[@]}"; do
  if [ -f "Services/cli/$FILE" ]; then
    if grep -q "_register_cli\|_cli_register" "Services/cli/$FILE"; then
      FILES_WITH_REGISTER=$((FILES_WITH_REGISTER + 1))
    fi
  fi
done

echo -e "  ${GREEN}✓${NC} $FILES_WITH_REGISTER files have registration functions"
echo ""

# =============================================================================
# Test 5: Check init wrapper functions for void init()
# =============================================================================
echo -e "${BLUE}Test 5: Checking init wrapper functions...${NC}"

CRITICAL_FILES=(
  "envelope_cc_cli.c"
  "expression_cli.c"
  "gate_time_cli.c"
  "harmonizer_cli.c"
)

INIT_WRAPPERS_OK=0
for FILE in "${CRITICAL_FILES[@]}"; do
  if grep -q "static int .*_cli_init(void)" "Services/cli/$FILE"; then
    INIT_WRAPPERS_OK=$((INIT_WRAPPERS_OK + 1))
  else
    echo -e "  ${RED}✗${NC} $FILE missing init wrapper"
    ERRORS=$((ERRORS + 1))
  fi
done

echo -e "  ${GREEN}✓${NC} $INIT_WRAPPERS_OK / ${#CRITICAL_FILES[@]} critical files have init wrappers"
echo ""

# =============================================================================
# Test 6: Check for .max_tracks usage (should NOT exist)
# =============================================================================
echo -e "${BLUE}Test 6: Checking for invalid .max_tracks field...${NC}"

MAX_TRACKS_FOUND=0
for FILE in "${CLI_FILES[@]}"; do
  if [ -f "Services/cli/$FILE" ]; then
    if grep -q "\.max_tracks" "Services/cli/$FILE"; then
      echo -e "  ${RED}✗${NC} $FILE still has .max_tracks field"
      ERRORS=$((ERRORS + 1))
      MAX_TRACKS_FOUND=$((MAX_TRACKS_FOUND + 1))
    fi
  fi
done

if [ $MAX_TRACKS_FOUND -eq 0 ]; then
  echo -e "  ${GREEN}✓${NC} No .max_tracks fields found (correct)"
else
  echo -e "  ${RED}✗${NC} Found $MAX_TRACKS_FOUND files with .max_tracks"
fi
echo ""

# =============================================================================
# Test 7: Check for correct function name patterns
# =============================================================================
echo -e "${BLUE}Test 7: Checking function naming patterns...${NC}"

# Check chord_cli.c uses chord_is_enabled not chord_get_enabled
if grep -q "chord_is_enabled" "Services/cli/chord_cli.c"; then
  echo -e "  ${GREEN}✓${NC} chord_cli.c uses correct function name (chord_is_enabled)"
else
  if grep -q "chord_get_enabled" "Services/cli/chord_cli.c"; then
    echo -e "  ${RED}✗${NC} chord_cli.c uses wrong function name (chord_get_enabled)"
    ERRORS=$((ERRORS + 1))
  fi
fi

echo ""

# =============================================================================
# Test 8: Check App/app_init.c for CLI task
# =============================================================================
echo -e "${BLUE}Test 8: Checking FreeRTOS CLI task integration...${NC}"

if grep -q "static void CliTask(void \*argument)" "App/app_init.c"; then
  echo -e "  ${GREEN}✓${NC} CLI task function exists in app_init.c"
else
  echo -e "  ${RED}✗${NC} CLI task function missing in app_init.c"
  ERRORS=$((ERRORS + 1))
fi

if grep -q "osThreadNew(CliTask" "App/app_init.c"; then
  echo -e "  ${GREEN}✓${NC} CLI task is created with osThreadNew"
else
  echo -e "  ${RED}✗${NC} CLI task creation missing in app_init.c"
  ERRORS=$((ERRORS + 1))
fi

if grep -q "cli_init();" "App/app_init.c"; then
  echo -e "  ${GREEN}✓${NC} cli_init() is called"
else
  echo -e "  ${RED}✗${NC} cli_init() not called in app_init.c"
  ERRORS=$((ERRORS + 1))
fi

echo ""

# =============================================================================
# Test 9: Check for config getter/setter implementations
# =============================================================================
echo -e "${BLUE}Test 9: Checking config_cli.c implementations...${NC}"

if grep -q "static uint8_t config_get_srio_enable" "Services/cli/config_cli.c"; then
  echo -e "  ${GREEN}✓${NC} config_cli.c has getter implementations"
else
  echo -e "  ${RED}✗${NC} config_cli.c missing getter implementations"
  ERRORS=$((ERRORS + 1))
fi

if grep -q "static void config_set_srio_enable" "Services/cli/config_cli.c"; then
  echo -e "  ${GREEN}✓${NC} config_cli.c has setter implementations"
else
  echo -e "  ${RED}✗${NC} config_cli.c missing setter implementations"
  ERRORS=$((ERRORS + 1))
fi

echo ""

# =============================================================================
# Test 10: Check CLI system files
# =============================================================================
echo -e "${BLUE}Test 10: Checking CLI system files...${NC}"

SYSTEM_FILES=(
  "Services/cli/cli.h"
  "Services/cli/cli.c"
  "Services/cli/cli_module_commands.h"
  "Services/cli/cli_module_commands.c"
  "Services/cli/module_cli_helpers.h"
)

SYSTEM_FILES_OK=0
for FILE in "${SYSTEM_FILES[@]}"; do
  if [ -f "$FILE" ]; then
    SYSTEM_FILES_OK=$((SYSTEM_FILES_OK + 1))
  else
    echo -e "  ${RED}✗${NC} Missing: $FILE"
    ERRORS=$((ERRORS + 1))
  fi
done

echo -e "  ${GREEN}✓${NC} $SYSTEM_FILES_OK / ${#SYSTEM_FILES[@]} system files present"
echo ""

# =============================================================================
# Summary
# =============================================================================
echo "=========================================="
echo "VERIFICATION SUMMARY"
echo "=========================================="
echo ""

if [ $ERRORS -eq 0 ] && [ $WARNINGS -eq 0 ]; then
  echo -e "${GREEN}✓✓✓ ALL TESTS PASSED ✓✓✓${NC}"
  echo ""
  echo "The CLI system is complete and ready for compilation!"
  echo ""
  echo "Next steps:"
  echo "  1. Build the firmware in STM32CubeIDE"
  echo "  2. Flash to hardware"
  echo "  3. Test CLI commands via MIOS Studio terminal"
  echo ""
elif [ $ERRORS -eq 0 ]; then
  echo -e "${YELLOW}⚠ PASSED WITH WARNINGS ⚠${NC}"
  echo ""
  echo "Found $WARNINGS warning(s)"
  echo "System should compile but review warnings above."
  echo ""
else
  echo -e "${RED}✗✗✗ TESTS FAILED ✗✗✗${NC}"
  echo ""
  echo "Found $ERRORS error(s) and $WARNINGS warning(s)"
  echo "Please fix the errors above before compilation."
  echo ""
  exit 1
fi

echo "=========================================="
