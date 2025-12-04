# picoCTF Buffer Overflow - Secure Versions

## Overview

This directory contains three secure versions of the picoCTF buffer overflow program, demonstrating different defense techniques against ret2win attacks.

## Original Vulnerability

Location: `../../vulnerable-programs/picoctf-bof1/vuln.c`

The vulnerable program:
- Uses `gets()` with no bounds checking
- Reads unlimited input into a 32-byte buffer
- Contains a `win()` function that prints the flag
- Allows overflow to overwrite the return address
- Attacker can redirect execution to `win()`

## Defense Versions

### Version 1: Safe Code Practices

**File:** `vuln_v1_safe_code.c`

**Defense:** Input validation + safe functions

**How it works:**
- Uses `fgets()` instead of `gets()`
- Validates input length before processing
- Uses `strncpy()` for safe copying
- Rejects inputs that exceed buffer size

**Result:** Input rejected - overflow prevented at code level

**Compilation:**
```bash
gcc -m32 -fno-stack-protector -no-pie -o vuln_v1_safe_code vuln_v1_safe_code.c
```

---

### Version 2: Stack Canary Protection

**File:** `vuln_v2_canary.c`

**Defense:** Compiler-inserted detection mechanism

**How it works:**
- Code is intentionally vulnerable (still uses `gets()`)
- Compiler inserts canary value on stack
- Canary checked before function returns
- Program aborts if canary was modified

**Result:** Stack smashing detected, program aborted

**Compilation:**
```bash
gcc -m32 -fstack-protector-all -no-pie -o vuln_v2_canary vuln_v2_canary.c
```

---

### Version 3: All Defenses Combined

**File:** `vuln_v3_all_defenses.c`

**Defense:** Safe code + Stack canary + Fortification + PIE

**How it works:**
- Multiple layers of defense
- Input validation (code)
- Safe functions (code)
- Stack canary (compiler)
- Fortified functions (compiler)
- Position-independent executable (compiler)

**Result:** Input rejected at first layer

**Compilation:**
```bash
gcc -m32 -fstack-protector-all -D_FORTIFY_SOURCE=2 -pie \
    -o vuln_v3_all_defenses vuln_v3_all_defenses.c
```

---

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

- **Original:** "picoCTF{...}" flag displayed (vulnerable!)
- **v1:** "Input too long!" (rejected)
- **v2:** "stack smashing detected" (canary caught it)
- **v3:** "Input too long!" (rejected at first layer)

## Attack Payload

The original exploit uses:
```python
payload = b"A" * 44 + p32(win_address)
# 44 bytes to fill buffer + saved EBP
# Then overwrite return address with win() address
```

## Learning Outcomes

1. Defense in depth - multiple independent layers
2. Code-level vs compiler-level defenses
3. Prevention vs detection mechanisms
4. Real-world secure programming practices
5. Understanding modern security protections (PIE/ASLR)

## References

- Original challenge: picoCTF
- Tutorial: `../../tutorial/PICOCTF_TUTORIAL_SECTION.md`
- Exploit: `../../exploits/picoctf-bof1/exploit.py`
