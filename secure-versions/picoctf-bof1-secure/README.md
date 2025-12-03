# picoCTF Buffer Overflow – Secure Versions (ret2win Defense)

## 1. Original Vulnerability (ret2win)

The original program (`vuln.c`) lives in:

`vulnerable-programs/picoctf-bof1/vuln.c`

Key points:

- It uses `gets(buffer)` to read input into a **32-byte** buffer.
- `gets()` does **not** check input length → you can overflow past the buffer.
- There is a `win()` function that is **never called normally**.
- By overflowing the buffer, the attacker overwrites the **return address** and makes it jump to `win()` instead of returning to `main()`.

Example attack:

```bash
cd ~/buffer-overflow-teaching-kit/vulnerable-programs/picoctf-bof1

# Compile vulnerable version
gcc -m32 -fno-stack-protector -no-pie -o vuln vuln.c

# Confirm it’s non-PIE (fixed addresses)
file vuln

# Find win() address
objdump -d vuln | grep win

# Use 44 bytes of padding + win() address
python3 -c 'import sys, struct; sys.stdout.buffer.write(b"A"*44 + struct.pack("<I", 0x08049186))' | ./vuln
## How to Compile All Versions

### Option 1: Use the compilation script (recommended)
```bash
chmod +x COMPILE_ALL.sh
./COMPILE_ALL.sh
```

### Option 2: Compile manually
```bash
# Version 1: No win() function
gcc -m32 -fno-stack-protector -no-pie -o vuln_v1_no_win vuln_v1_no_win.c

# Version 2: Safe input (fgets)
gcc -m32 -fno-stack-protector -no-pie -o vuln_v2_safe_input vuln_v2_safe_input.c

# Version 3: Stack canary
gcc -m32 -fstack-protector-all -no-pie -o vuln_v3 vuln_v3_canary.c

# Version 4: ASLR/PIE + canary
gcc -m32 -fstack-protector-all -pie -o vuln_v4 vuln_v4_aslr.c

# Version 5: All defenses combined
gcc -m32 -fstack-protector-all -D_FORTIFY_SOURCE=2 -pie -o vuln_v5 vuln_v5_all_defenses.c
```

---

## Testing the Defenses

After compiling, run the test script:
```bash
chmod +x test_defenses.sh
./test_defenses.sh
```

This will test all five versions against the original ret2win exploit.

---
