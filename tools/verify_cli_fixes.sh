#!/bin/bash
# CLI Warning Fixes Verification Script
# Checks that all compilation warnings have been properly fixed

echo "=========================================="
echo "CLI Warning Fixes Verification"
echo "=========================================="
echo ""

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

ERRORS=0

# Check 1: Verify init wrapper functions exist
echo "1. Checking init wrapper functions..."
FILES_TO_CHECK=(
  "Services/cli/envelope_cc_cli.c:envelope_cc_cli_init"
  "Services/cli/expression_cli.c:expression_cli_init"
  "Services/cli/gate_time_cli.c:gate_time_cli_init"
  "Services/cli/harmonizer_cli.c:harmonizer_cli_init"
)

for entry in "${FILES_TO_CHECK[@]}"; do
  IFS=':' read -ra PARTS <<< "$entry"
  FILE="${PARTS[0]}"
  FUNC="${PARTS[1]}"
  
  if grep -q "static int ${FUNC}(void)" "$FILE"; then
    echo -e "  ${GREEN}✓${NC} $FILE has $FUNC wrapper"
  else
    echo -e "  ${RED}✗${NC} $FILE missing $FUNC wrapper"
    ERRORS=$((ERRORS + 1))
  fi
done
echo ""

# Check 2: Verify .max_tracks removed
echo "2. Checking .max_tracks removed..."
FILES_TO_CHECK=(
  "Services/cli/one_finger_chord_cli.c"
  "Services/cli/performance_cli.c"
  "Services/cli/program_change_mgr_cli.c"
  "Services/cli/zones_cli.c"
)

for FILE in "${FILES_TO_CHECK[@]}"; do
  if grep -q "\.max_tracks" "$FILE"; then
    echo -e "  ${RED}✗${NC} $FILE still has .max_tracks"
    ERRORS=$((ERRORS + 1))
  else
    echo -e "  ${GREEN}✓${NC} $FILE has .max_tracks removed"
  fi
done
echo ""

# Check 3: Verify description shortened
echo "3. Checking arpeggiator description length..."
FILE="Services/cli/arpeggiator_cli_integration.c"
DESC=$(grep "\.description = " "$FILE" | grep "Pattern")
if [[ ${#DESC} -gt 0 ]]; then
  # Extract just the description string
  DESC_STR=$(echo "$DESC" | sed -n 's/.*description = "\(.*\)".*/\1/p')
  LEN=${#DESC_STR}
  if [[ $LEN -le 64 ]]; then
    echo -e "  ${GREEN}✓${NC} Description length: $LEN chars (≤64)"
  else
    echo -e "  ${RED}✗${NC} Description too long: $LEN chars (>64)"
    ERRORS=$((ERRORS + 1))
  fi
else
  echo -e "  ${YELLOW}⚠${NC}  Could not find pattern description"
fi
echo ""

# Check 4: Verify no old init function references
echo "4. Checking for old init function references..."
OLD_INITS=(
  "envelope_cc_init"
  "expression_init"
  "gate_time_init"
  "harmonizer_init"
)

for FUNC in "${OLD_INITS[@]}"; do
  FILE=$(echo "$FUNC" | sed 's/_init/_cli.c/')
  FILE="Services/cli/${FILE}"
  
  # Check if old init is used in .init field (should use wrapper instead)
  if grep -q "\.init = ${FUNC}," "$FILE"; then
    echo -e "  ${RED}✗${NC} $FILE still uses ${FUNC} directly in .init"
    ERRORS=$((ERRORS + 1))
  else
    echo -e "  ${GREEN}✓${NC} $FILE uses wrapper (not ${FUNC} directly)"
  fi
done
echo ""

# Summary
echo "=========================================="
if [[ $ERRORS -eq 0 ]]; then
  echo -e "${GREEN}✓ All checks passed!${NC}"
  echo "All CLI compilation warnings have been fixed."
else
  echo -e "${RED}✗ $ERRORS check(s) failed!${NC}"
  echo "Please review the errors above."
  exit 1
fi
echo "=========================================="
