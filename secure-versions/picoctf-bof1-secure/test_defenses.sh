#!/bin/bash

echo "Testing picoCTF Defenses (3 versions)"
echo "====================================="
echo

PAYLOAD_CMD='python3 -c "import sys, struct; sys.stdout.buffer.write(b\"A\"*44 + struct.pack(\"<I\", 0x08049186))"'

echo "[1] Original vulnerable:"
eval $PAYLOAD_CMD | ../../vulnerable-programs/picoctf-bof1/vuln 2>&1
echo

echo "[2] Version 1 - Safe Code:"
eval $PAYLOAD_CMD | ./vuln_v1_safe_code 2>&1
echo

echo "[3] Version 2 - Stack Canary:"
eval $PAYLOAD_CMD | ./vuln_v2_canary 2>&1
echo

echo "[4] Version 3 - All Defenses:"
eval $PAYLOAD_CMD | ./vuln_v3_all_defenses 2>&1
echo

echo "====================================="
echo "Test complete"
