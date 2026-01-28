#!/bin/bash

echo "======================================"
echo "CLI Configuration Check"
echo "======================================"
echo ""

# Check for .cproject file (Eclipse/CubeIDE)
if [ -f ".cproject" ]; then
    echo "Checking .cproject for MODULE_ENABLE_CLI..."
    if grep -q "MODULE_ENABLE_CLI" .cproject; then
        echo "✓ MODULE_ENABLE_CLI found in .cproject"
        grep "MODULE_ENABLE_CLI" .cproject | head -3
    else
        echo "✗ MODULE_ENABLE_CLI NOT found in .cproject"
    fi
    echo ""
fi

# Check for preprocessor definitions in common locations
echo "Checking source files for #if MODULE_ENABLE_CLI..."
echo ""

echo "App/app_init.c:"
grep -n "#if MODULE_ENABLE_CLI" App/app_init.c | head -5

echo ""
echo "Services/cli/cli.c:"
grep -n "#if MODULE_ENABLE_CLI" Services/cli/cli.c | head -5

echo ""
echo "======================================"
echo "Instructions:"
echo "======================================"
echo ""
echo "If MODULE_ENABLE_CLI is NOT found in .cproject:"
echo "  1. Open project in CubeIDE"
echo "  2. Right-click project → Properties"
echo "  3. C/C++ Build → Settings"
echo "  4. MCU GCC Compiler → Preprocessor"
echo "  5. Add: MODULE_ENABLE_CLI=1"
echo "  6. Clean and rebuild"
echo ""
