# SEED Labs Buffer Overflow - Secure Versions

## Overview

This directory contains three secure versions of the SEED Labs buffer overflow program, demonstrating different defense techniques against shellcode injection attacks.

## Original Vulnerability

Location: `../../vulnerable-programs/lab3-seedlabs/stack.c`

The vulnerable program:
- Uses `strcpy()` with no bounds checking
- Reads 517 bytes into a 24-byte buffer
- Stack is executable
- No stack protection
- Fixed addresses

## Defense Versions

### Version 1: Safe Code Practices

**File:** `lab3_v1_safe_code.c`

**Defense:** Input validation + safe functions

**How it works:**
- Validates input length before copying
- Uses `strncpy()` instead of `strcpy()`
- Rejects inputs that are too large

**Result:** Input rejected - overflow prevented at code level

**Compilation:**
```bash
gcc -m32 -fno-stack-protector -z execstack -no-pie -o lab3_v1_safe_code lab3_v1_safe_code.c
```

### Version 2: Compiler and OS Protection

**File:** `lab3_v2_canary_nx.c`

**Defense:** Stack canary + Non-executable stack

**How it works:**
- Stack canary detects buffer overflow
- NX bit prevents shellcode execution
- Code is intentionally vulnerable - defenses are in compilation flags

**Result:** Stack smashing detected, program aborted

**Compilation:**
```bash
gcc -m32 -fstack-protector-all -z noexecstack -no-pie -o lab3_v2_canary_nx lab3_v2_canary_nx.c
```

### Version 3: All Defenses Combined

**File:** `lab3_v3_all_defenses.c`

**Defense:** Safe code + Stack canary + NX + Fortification

**How it works:**
- Multiple layers of defense
- Input validation (code)
- Safe functions (code)
- Stack canary (compiler)
- NX bit (OS)
- Fortified functions (compiler)

**Result:** Input rejected at first layer

**Compilation:**
```bash
gcc -m32 -fstack-protector-all -D_FORTIFY_SOURCE=2 -z noexecstack -no-pie \
    -o lab3_v3_all_defenses lab3_v3_all_defenses.c
```

## How to Compile

Use the compilation script:
```bash
./COMPILE_ALL.sh
```

Or compile manually using the commands above.

## How to Test

Run the test script:
```bash
./test_defenses.sh
```

This will:
1. Generate the exploit payload
2. Test each secure version
3. Show how each defense stops the attack

## Expected Results

- **v1**: Input rejected (too long)
- **v2**: Stack smashing detected, aborted
- **v3**: Input rejected (too long)

## Learning Outcomes

1. Defense in depth - multiple independent layers
2. Code-level vs compiler-level vs OS-level defenses
3. Prevention vs detection vs mitigation
4. Real-world secure programming practices
