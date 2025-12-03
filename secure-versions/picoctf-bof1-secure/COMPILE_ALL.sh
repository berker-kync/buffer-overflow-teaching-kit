#!/bin/bash

echo "Compiling all picoCTF secure versions..."
echo

# Version 1: Remove win() function
echo "[1/5] Compiling v1 (no win function)..."
gcc -m32 -fno-stack-protector -no-pie -o vuln_v1_no_win vuln_v1_no_win.c
echo "  ✓ vuln_v1_no_win compiled"

# Version 2: Safe input
echo "[2/5] Compiling v2 (safe input)..."
gcc -m32 -fno-stack-protector -no-pie -o vuln_v2_safe_input vuln_v2_safe_input.c
echo "  ✓ vuln_v2_safe_input compiled"

# Version 3: Stack canary
echo "[3/5] Compiling v3 (stack canary)..."
gcc -m32 -fstack-protector-all -no-pie -o vuln_v3 vuln_v3_canary.c
echo "  ✓ vuln_v3 compiled (with canary)"

# Version 4: PIE/ASLR
echo "[4/5] Compiling v4 (ASLR/PIE)..."
gcc -m32 -fstack-protector-all -pie -o vuln_v4 vuln_v4_aslr.c
echo "  ✓ vuln_v4 compiled (with PIE)"

# Version 5: All defenses
echo "[5/5] Compiling v5 (all defenses)..."
gcc -m32 -fstack-protector-all -D_FORTIFY_SOURCE=2 -pie -o vuln_v5 vuln_v5_all_defenses.c
echo "  ✓ vuln_v5 compiled (all defenses)"

echo
echo "All versions compiled successfully!"
echo "Run ./test_defenses.sh to test all versions"
