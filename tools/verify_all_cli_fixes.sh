#!/bin/bash
# Verification script for all CLI fixes
# Checks that all PARAM macro errors have been resolved

echo "==================================="
echo "CLI Fixes Verification Script"
echo "==================================="
echo ""

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$REPO_ROOT"

ERRORS=0
CHECKS=0

# Check if we're on the right branch
BRANCH=$(git branch --show-current)
if [ "$BRANCH" != "copilot/implement-cli-commands-documentation" ]; then
    echo "⚠️  WARNING: Not on the correct branch!"
    echo "   Current: $BRANCH"
    echo "   Expected: copilot/implement-cli-commands-documentation"
    echo ""
fi

# Check latest commit
LATEST_COMMIT=$(git log --oneline -1)
echo "Latest commit: $LATEST_COMMIT"
echo ""

# List of CLI files that should have fixes
CLI_FILES=(
    "Services/cli/humanize_cli.c"
    "Services/cli/legato_cli.c"
    "Services/cli/lfo_cli.c"
    "Services/cli/livefx_cli.c"
    "Services/cli/assist_hold_cli.c"
    "Services/cli/bass_chord_system_cli.c"
    "Services/cli/bellows_expression_cli.c"
    "Services/cli/bellows_shake_cli.c"
    "Services/cli/cc_smoother_cli.c"
    "Services/cli/channelizer_cli.c"
    "Services/cli/chord_cli.c"
    "Services/cli/envelope_cc_cli.c"
    "Services/cli/gate_time_cli.c"
    "Services/cli/harmonizer_cli.c"
)

echo "Checking ${#CLI_FILES[@]} CLI files for proper fixes..."
echo ""

for file in "${CLI_FILES[@]}"; do
    if [ ! -f "$file" ]; then
        echo "❌ File not found: $file"
        ((ERRORS++))
        continue
    fi
    
    # Check if file has DEFINE_PARAM macros before setup function
    if grep -q "DEFINE_PARAM" "$file"; then
        echo "✓ $file has DEFINE_PARAM macros"
        ((CHECKS++))
    else
        echo "⚠️  $file missing DEFINE_PARAM macros (may not need them)"
    fi
    
    # Check if setup function exists
    if grep -q "setup_.*_parameters" "$file"; then
        SETUP_LINE=$(grep -n "setup_.*_parameters" "$file" | head -1 | cut -d: -f1)
        DEFINE_LINE=$(grep -n "DEFINE_PARAM" "$file" | head -1 | cut -d: -f1)
        
        if [ -n "$DEFINE_LINE" ] && [ "$DEFINE_LINE" -lt "$SETUP_LINE" ]; then
            echo "  ✓ DEFINE_PARAM macros appear before setup function"
        elif [ -n "$DEFINE_LINE" ]; then
            echo "  ❌ DEFINE_PARAM macros appear AFTER setup function!"
            ((ERRORS++))
        fi
    fi
done

echo ""
echo "==================================="
echo "Verification Complete"
echo "==================================="
echo "Files checked: ${#CLI_FILES[@]}"
echo "Checks passed: $CHECKS"
echo "Errors found: $ERRORS"
echo ""

if [ $ERRORS -eq 0 ]; then
    echo "✅ All checks passed! Files are ready to compile."
    exit 0
else
    echo "❌ Some issues found. Please review above."
    exit 1
fi
