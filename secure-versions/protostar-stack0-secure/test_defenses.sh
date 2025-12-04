#!/bin/bash

echo "Testing Protostar Defenses (3 versions)"
echo "========================================"
echo

PAYLOAD=$(python3 -c "import sys; sys.stdout.buffer.write(b'A' * 64 + b'BCDE')")

echo "[1] Original vulnerable:"
echo "$PAYLOAD" | ../../vulnerable-programs/protostar-stack0/stack0
echo

echo "[2] Version 1 - Safe Code:"
echo "$PAYLOAD" | ./stack0_v1_safe_code
echo

echo "[3] Version 2 - Stack Canary:"
echo "$PAYLOAD" | ./stack0_v2_canary 2>&1
echo

echo "[4] Version 3 - All Defenses:"
echo "$PAYLOAD" | ./stack0_v3_all_defenses
echo

echo "========================================"
echo "Test complete"
