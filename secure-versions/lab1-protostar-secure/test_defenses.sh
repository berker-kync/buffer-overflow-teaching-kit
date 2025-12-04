#!/bin/bash

echo "Testing Protostar Defenses (3 versions)"
echo "========================================"
echo

PAYLOAD=$(python3 -c "import sys; sys.stdout.buffer.write(b'A' * 64 + b'BCDE')")

echo "[1] Original vulnerable:"
echo "$PAYLOAD" | ../../vulnerable-programs/lab1-protostar/stack0
echo

echo "[2] Version 1 - Safe Code:"
echo "$PAYLOAD" | ./lab1_v1_safe_code
echo

echo "[3] Version 2 - Stack Canary:"
echo "$PAYLOAD" | ./lab1_v2_canary 2>&1
echo

echo "[4] Version 3 - All Defenses:"
echo "$PAYLOAD" | ./lab1_v3_all_defenses
echo

echo "========================================"
echo "Test complete"
