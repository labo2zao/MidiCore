#!/bin/bash

# Check if user has correct file versions
# This helps diagnose version mismatch issues

echo "==================================="
echo "File Version Checker"
echo "==================================="
echo ""

# Check if in git repo
if ! git rev-parse --git-dir > /dev/null 2>&1; then
  echo "❌ ERROR: Not in a git repository"
  exit 1
fi

# Check current branch
BRANCH=$(git branch --show-current)
echo "Current branch: $BRANCH"
if [ "$BRANCH" != "copilot/implement-cli-commands-documentation" ]; then
  echo "⚠️  WARNING: You're not on the fix branch!"
  echo "   Expected: copilot/implement-cli-commands-documentation"
  echo "   Actual:   $BRANCH"
  echo ""
fi

# Check latest commit
COMMIT=$(git log --oneline -1 --format="%h %s")
echo "Latest commit: $COMMIT"
echo ""

# Check if files are up to date
echo "Checking file versions..."
echo ""

# Key files that must match
FILES=(
  "Services/cli/module_cli_helpers.h"
  "Services/module_registry/module_registry.h"
  "Services/cli/assist_hold_cli.c"
  "Services/cli/bass_chord_system_cli.c"
  "Services/cli/humanize_cli.c"
)

MISMATCHES=0

for FILE in "${FILES[@]}"; do
  if [ ! -f "$FILE" ]; then
    echo "❌ $FILE - FILE NOT FOUND"
    MISMATCHES=$((MISMATCHES + 1))
    continue
  fi
  
  # Get last commit that modified this file
  FILE_COMMIT=$(git log --oneline -1 --format="%h" -- "$FILE")
  FILE_DATE=$(git log -1 --format="%ai" -- "$FILE")
  
  # Check if file has uncommitted changes
  if git diff --quiet "$FILE" 2>/dev/null; then
    STATUS="✅"
  else
    STATUS="⚠️  MODIFIED"
    MISMATCHES=$((MISMATCHES + 1))
  fi
  
  echo "$STATUS $FILE"
  echo "   Last commit: $FILE_COMMIT ($FILE_DATE)"
done

echo ""
echo "==================================="
if [ $MISMATCHES -eq 0 ]; then
  echo "✅ All files are up to date"
  echo ""
  echo "If you're still seeing errors, try:"
  echo "  make clean && make"
else
  echo "⚠️  Found $MISMATCHES file(s) with issues"
  echo ""
  echo "To fix, run:"
  echo "  git reset --hard origin/copilot/implement-cli-commands-documentation"
  echo "  make clean && make"
fi
echo "==================================="
