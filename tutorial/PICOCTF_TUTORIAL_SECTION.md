# Lab 2: picoCTF-Style Buffer Overflow – ret2win Attack
**Time:** 40–50 minutes  
**Difficulty:** Beginner → Intermediate  
**Goal:** Overflow a buffer and jump straight to a “win” function that was never called  
**Author:** Latha Shree K P  

---

## Introduction – From Simple Overflow to Controlling Execution
In the previous lab (Protostar Stack0) you changed a variable by overflowing a buffer.  
Now we go one step further: we will **take full control of the program flow** and force it to execute a function that was never called.

This technique is called **ret2win** (“return-to-win”) and is one of the cleanest ways to learn return-address overwriting.

---

## Section 1: Understanding the Vulnerable Program
Location: `vulnerable-programs/picoctf-bof1/picoctf_bof1.c`

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void win() {
    puts("Congratulations! You redirected execution to win()");
    puts("This is a classic ret2win attack.");
    system("/bin/sh");          // gives you a shell as a bonus
}

void vulnerable() {
    char buffer[32];            // only 32 bytes!
    puts("What's your name?");
    gets(buffer);               // ← DANGEROUS – no bounds checking
    printf("Hello, %s!\n", buffer);
}

int main() {
    setbuf(stdout, NULL);
    setbuf(stdin, NULL);
    
    puts("Welcome to picoCTF-style Buffer Overflow 1");
    vulnerable();
    puts("Exiting normally...");
    return 0;
}
