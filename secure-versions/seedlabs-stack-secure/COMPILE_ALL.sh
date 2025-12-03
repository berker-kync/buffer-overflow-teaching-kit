#!/bin/bash

echo "Compiling SEED Labs secure versions (3 versions)..."
echo

echo "[1/3] Compiling v1 (safe code)..."
gcc -m32 -fno-stack-protector -z execstack -no-pie -o stack_v1_safe_code stack_v1_safe_code.c
echo "  Done: stack_v1_safe_code"

echo "[2/3] Compiling v2 (canary + NX)..."
gcc -m32 -fstack-protector-all -z noexecstack -no-pie -o stack_v2_canary_nx stack_v2_canary_nx.c
echo "  Done: stack_v2_canary_nx (canary + non-executable stack)"

echo "[3/3] Compiling v3 (all defenses)..."
gcc -m32 -fstack-protector-all -D_FORTIFY_SOURCE=2 -z noexecstack -no-pie -o stack_v3_all_defenses stack_v3_all_defenses.c
echo "  Done: stack_v3_all_defenses (all defenses)"

echo
echo "All versions compiled successfully"
echo "Run ./test_defenses.sh to test"
