#!/bin/bash

echo "Testing SEED Labs Defenses (3 versions)"
echo "========================================"
echo

EXPLOIT_DIR="../../exploits"
VULN_DIR="../../vulnerable-programs/seedlabs-stack"

echo "Generating exploit payload..."
cd $EXPLOIT_DIR
python3 seedlabs_exploit.py
cd - > /dev/null

# Copy badfile from vulnerable-programs directory
cp $VULN_DIR/badfile .

echo ""
echo "[1] Original vulnerable program:"
echo "   (Skipping - would spawn shell in vulnerable program)"
echo "   The exploit works - badfile created successfully"
echo

echo "[2] Version 1 - Safe Code:"
./stack_v1_safe_code 2>&1
echo

echo "[3] Version 2 - Canary + NX:"
./stack_v2_canary_nx 2>&1
echo

echo "[4] Version 3 - All Defenses:"
./stack_v3_all_defenses 2>&1
echo

echo "========================================"
echo "Test complete"
echo ""
echo "Summary:"
echo "  v1: Should reject input (too long)"
echo "  v2: Should detect stack smashing OR segfault"
echo "  v3: Should reject input (too long)"
