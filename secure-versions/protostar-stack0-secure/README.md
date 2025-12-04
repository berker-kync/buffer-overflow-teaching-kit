# Protostar Stack0 - Secure Versions

## Overview

This directory contains three secure versions of the Protostar Stack0 program, demonstrating different defense techniques against buffer overflow attacks.

## Original Vulnerability

Location: `../../vulnerable-programs/protostar-stack0/stack0.c`

The vulnerable program:
- Uses `gets()` with no bounds checking
- Reads unlimited input into a 64-byte buffer
- Allows overflow to overwrite adjacent `modified` variable
- If `modified` changes from 0, attack succeeds

## Defense Versions

### Version 1: Safe Code Practices

**File:** `stack0_v1_safe_code.c`

**Defense:** Input validation + safe functions

**How it works:**
- Uses `fgets()` instead of `gets()`
- Validates input length before copying
- Uses `strncpy()` for safe copying
- Rejects inputs that are too large

**Result:** Input rejected - overflow prevented at code level

**Compilation:**
```bash
gcc -m32 -fno-stack-protector -no-pie -o stack0_v1_safe_code stack0_v1_safe_code.c
```

### Version 2: Stack Canary Protection

**File:** `stack0_v2_canary.c`

**Defense:** Compiler-inserted detection mechanism

**How it works:**
- Code is intentionally vulnerable (still uses `gets()`)
- Compiler inserts canary value on stack
- Canary checked before function returns
- Program aborts if canary was modified

**Result:** Stack smashing detected, program aborted

**Compilation:**
```bash
gcc -m32 -fstack-protector-all -no-pie -o stack0_v2_canary stack0_v2_canary.c
```

### Version 3: All Defenses Combined

**File:** `stack0_v3_all_defenses.c`

**Defense:** Safe code + Stack canary + Fortification

**How it works:**
- Multiple layers of defense
- Input validation (code)
- Safe functions (code)
- Stack canary (compiler)
- Fortified functions (compiler)

**Result:** Input rejected at first layer

**Compilation:**
```bash
gcc -m32 -fstack-protector-all -D_FORTIFY_SOURCE=2 -no-pie \
    -o stack0_v3_all_defenses stack0_v3_all_defenses.c
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
1. Test the original vulnerable program
2. Test each secure version
3. Show how each defense stops the attack

## Expected Results

- **Original:** "you have changed the 'modified' variable" (vulnerable!)
- **v1:** "Input too long!" (rejected)
- **v2:** "stack smashing detected" (canary caught it)
- **v3:** "Input too long!" (rejected at first layer)

## Learning Outcomes

1. Defense in depth - multiple independent layers
2. Code-level vs compiler-level defenses
3. Prevention vs detection
4. Real-world secure programming practices
