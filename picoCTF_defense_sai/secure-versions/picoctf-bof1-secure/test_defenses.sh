#!/bin/bash

echo "Testing picoCTF ret2win defenses"
echo "================================"
echo

# This is the same payload you already used:
# 44 'A's + address of win() (0x08049186)
PAYLOAD_CMD='python3 -c "import sys, struct; sys.stdout.buffer.write(b\"A\"*44 + struct.pack(\"<I\", 0x08049186))"'

echo "[1] Original vulnerable binary (SHOULD call win)"
eval $PAYLOAD_CMD | ../../vulnerable-programs/picoctf-bof1/vuln
echo

echo "[2] Version 1 - no win() function (attack should FAIL / maybe crash)"
eval $PAYLOAD_CMD | ./vuln_v1_no_win
echo

echo "[3] Version 2 - safe input (fgets) (overflow should NOT happen)"
eval $PAYLOAD_CMD | ./vuln_v2_safe_input
echo

echo "[4] Version 3 - stack canary (should detect stack smashing)"
eval $PAYLOAD_CMD | ./vuln_v3 2>&1
echo

echo "[5] Version 4 - ASLR/PIE + canary (ret2win should not be reliable)"
eval $PAYLOAD_CMD | ./vuln_v4 2>&1
echo

echo "[6] Version 5 - all defenses (should be fully protected)"
eval $PAYLOAD_CMD | ./vuln_v5 2>&1
echo

echo "================================"
echo "All tests complete!"
