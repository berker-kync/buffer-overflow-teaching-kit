#!/bin/bash

echo "Compiling picoCTF secure versions (3 versions)..."
echo

echo "[1/3] Compiling v1 (safe code)..."
gcc -m32 -fno-stack-protector -no-pie -o lab2_v1_safe_code lab2_v1_safe_code.c
echo "  Done: lab2_v1_safe_code"

echo "[2/3] Compiling v2 (stack canary)..."
gcc -m32 -fstack-protector-all -no-pie -o lab2_v2_canary lab2_v2_canary.c
echo "  Done: lab2_v2_canary (with canary)"

echo "[3/3] Compiling v3 (all defenses)..."
gcc -m32 -fstack-protector-all -D_FORTIFY_SOURCE=2 -pie -o lab2_v3_all_defenses lab2_v3_all_defenses.c
echo "  Done: lab2_v3_all_defenses (all defenses)"

echo
echo "All versions compiled successfully"
echo "Run ./test_defenses.sh to test"
