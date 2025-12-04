# Assignment 1: picoCTF Buffer Overflow
**Vulnerability:** Uses `gets()` which allows buffer overflow.
**Goal:** Overwrite return address to jump to `win()`.
**Compile:** `gcc -m32 -fno-stack-protector -no-pie -Wno-error=implicit-function-declaration vuln.c -o vuln`
