# SEED Labs Stack Overflow

## Vulnerability
Uses `strcpy()` without bounds checking on a 24-byte buffer.

## Compilation
```bash
gcc -m32 -fno-stack-protector -z execstack -o stack stack.c
```

## Requirements
- ASLR disabled
- 32-bit libraries installed

## Testing
```bash
echo "Test" > badfile
./stack
```