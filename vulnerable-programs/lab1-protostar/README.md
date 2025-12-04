# Assignment 2: Protostar Stack0

## Program Overview
This program demonstrates a simple buffer overflow where overflowing a buffer affects a neighboring variable on the stack.

## Vulnerability Type
* **Vulnerability:** Variable Overwrite (Stack Overflow)
* **Goal:** The `modified` variable is set to 0. You must overflow `buffer` to change `modified` to any non-zero value.

## Compilation Instructions
```bash
gcc -m32 -fno-stack-protector -z execstack -Wno-error=implicit-function-declaration stack0.c -o stack0
