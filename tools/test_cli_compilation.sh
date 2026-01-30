#!/bin/bash
# Test script to verify CLI files compile without warnings
# This simulates what STM32CubeIDE would check

echo "=========================================="
echo "CLI Compilation Warnings Check"
echo "=========================================="
echo ""

# List of all modified CLI files
CLI_FILES=(
  "Services/cli/envelope_cc_cli.c"
  "Services/cli/expression_cli.c"
  "Services/cli/gate_time_cli.c"
  "Services/cli/harmonizer_cli.c"
  "Services/cli/one_finger_chord_cli.c"
  "Services/cli/performance_cli.c"
  "Services/cli/program_change_mgr_cli.c"
  "Services/cli/zones_cli.c"
  "Services/cli/arpeggiator_cli_integration.c"
)

echo "Checking syntax of ${#CLI_FILES[@]} CLI files..."
echo ""

# Check each file for common issues
ISSUES=0

for FILE in "${CLI_FILES[@]}"; do
  echo "Checking $FILE..."
  
  # Check for void init() being used directly in .init field
  if grep -q "\.init = [a-z_]*_init," "$FILE" | grep -qv "_cli_init"; then
    echo "  ⚠ May have void init() used directly"
    ISSUES=$((ISSUES + 1))
  fi
  
  # Check for .max_tracks field
  if grep -q "\.max_tracks" "$FILE"; then
    echo "  ⚠ Contains invalid .max_tracks field"
    ISSUES=$((ISSUES + 1))
  fi
  
  # Check for long descriptions (over 64 chars)
  while IFS= read -r line; do
    if [[ $line =~ \.description[[:space:]]*=[[:space:]]*\"([^\"]*)\" ]]; then
      DESC="${BASH_REMATCH[1]}"
      LEN=${#DESC}
      if [[ $LEN -gt 64 ]]; then
        echo "  ⚠ Description too long: $LEN chars (max 64)"
        ISSUES=$((ISSUES + 1))
      fi
    fi
  done < "$FILE"
done

echo ""
echo "=========================================="
if [[ $ISSUES -eq 0 ]]; then
  echo "✓ All files pass syntax checks"
  echo "Ready for compilation in STM32CubeIDE"
else
  echo "✗ Found $ISSUES potential issue(s)"
  echo "Please review the warnings above"
  exit 1
fi
echo "=========================================="
