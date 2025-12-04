# Buffer Overflow Teaching Kit

Educational resource for learning/teaching buffer overflow vulnerabilities - CSC-544 Network Programming Course Project

**Course:** CSC-544 Network Programming  
**Institution:** University of North Carolina Wilmington  
**Team:** Group Project - Fall 2025

## Course Instructor

Aydogan, Ahmet Furkan, Ph.D

## Team

**Project Leader:** Berker Koyuncu  
**Group 1 (Environment Setup):** Srinivasulu, Hushikeash - Insley, Tim Michael  
**Group 2 (Exploits):** Townsend, Jake Michael - Druhen, John Michel Eric  
**Group 3 (Tutorials):** Krishnapura Prabhakara, Latha Shree - Kesamchetti, Kyathi Lekha  
**Group 4 (Defenses):** Guduru, Sai Monish - Entha, Thulasiram

---

## Overview

This repository contains a complete teaching kit demonstrating three different types of buffer overflow vulnerabilities, their exploits, and multiple defense strategies. The project progresses from basic variable overwriting to advanced shellcode injection, making it suitable for students at various skill levels.

## Objectives

- Understand how buffer overflows work at the memory level
- Learn to identify vulnerable code patterns
- Develop working exploits for educational purposes
- Implement multiple defense mechanisms
- Understand modern security protections

---

## Repository Structure

```
buffer-overflow-teaching-kit/
│
├── vulnerable-programs/    # Three vulnerable C programs
│   ├── protostar-stack0/   # Lab 1: Variable overwrite
│   ├── picoctf-bof1/       # Lab 2: Function redirection
│   └── seedlabs-stack/     # Lab 3: Shellcode injection
│
├── exploits/               # Working exploits for each vulnerability
│   ├── protostar-stack0/
│   ├── picoctf-bof1/
│   └── seedlabs_exploit.py
│
├── tutorial/               # Step-by-step tutorial sections
│   ├── PROTOSTAR_TUTORIAL_SECTION.md
│   ├── PICOCTF_TUTORIAL_SECTION.md
│   └── SEEDLABS_TUTORIAL_SECTION.md
│
└── secure-versions/        # Defense implementations
    ├── protostar-stack0-secure/    # 3 secure versions
    ├── picoctf-bof1-secure/        # 3 secure versions
    └── seedlabs-stack-secure/      # 3 secure versions
```

---

## Three Labs - Progressive Difficulty

### Lab 1: Protostar Stack0 (Beginner)

**Vulnerability Type:** Adjacent variable overwrite  
**Time:** 20-30 minutes  
**Difficulty:** Easy

Learn the fundamentals of buffer overflows by overwriting a variable adjacent to a buffer.

**Files:**
- Vulnerable program: `vulnerable-programs/protostar-stack0/`
- Exploit: `exploits/protostar-stack0/exploit.py`
- Tutorial: `tutorial/PROTOSTAR_TUTORIAL_SECTION.md`
- Defenses: `secure-versions/protostar-stack0-secure/`

---

### Lab 2: picoCTF Buffer Overflow (Intermediate)

**Vulnerability Type:** Return address overwrite (ret2win)  
**Time:** 30-40 minutes  
**Difficulty:** Moderate

Learn to redirect program execution by overwriting the return address to jump to an existing function.

**Files:**
- Vulnerable program: `vulnerable-programs/picoctf-bof1/`
- Exploit: `exploits/picoctf-bof1/exploit.py`
- Tutorial: `tutorial/PICOCTF_TUTORIAL_SECTION.md`
- Defenses: `secure-versions/picoctf-bof1-secure/`

---

### Lab 3: SEED Labs Shellcode (Advanced)

**Vulnerability Type:** Code injection  
**Time:** 45-60 minutes  
**Difficulty:** Advanced

Learn to inject and execute custom shellcode to spawn a root shell.

**Files:**
- Vulnerable program: `vulnerable-programs/seedlabs-stack/`
- Exploit: `exploits/seedlabs_exploit.py`
- Tutorial: `tutorial/SEEDLABS_TUTORIAL_SECTION.md`
- Defenses: `secure-versions/seedlabs-stack-secure/`

---

## Quick Start

### Prerequisites

**System Requirements:**
- Linux environment (Ubuntu 20.04+ recommended)
- 32-bit compilation support (`gcc-multilib`)
- Python 3.x
- GDB (for debugging)

**Install Dependencies:**

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y gcc-multilib python3 gdb

# Verify installation
gcc --version
python3 --version
```

### Running a Lab

**Example: Lab 1 (Protostar)**

```bash
# 1. Navigate to the repository
cd buffer-overflow-teaching-kit

# 2. Compile the vulnerable program
cd vulnerable-programs/protostar-stack0
gcc -m32 -fno-stack-protector -no-pie -o stack0 stack0.c

# 3. Read the tutorial
cat ../../tutorial/PROTOSTAR_TUTORIAL_SECTION.md

# 4. Run the exploit
cd ../../exploits/protostar-stack0
python3 exploit.py | ../../vulnerable-programs/protostar-stack0/stack0

# 5. Check the defenses
cd ../../secure-versions/protostar-stack0-secure
./COMPILE_ALL.sh
./test_defenses.sh
```

---

## Defense Implementations

Each lab includes three secure versions demonstrating different defense techniques:

### Version 1: Safe Code Practices
- Input validation
- Safe string functions (fgets, strncpy)
- Bounds checking

### Version 2: Compiler Protections
- Stack canaries
- Detection mechanisms
- Compile-time enforcement

### Version 3: All Defenses Combined
- Multiple layers of protection
- Defense in depth
- Production-ready security

**Example: Testing SEED Labs Defenses**

```bash
cd secure-versions/seedlabs-stack-secure
./COMPILE_ALL.sh          # Compile all versions
./test_defenses.sh        # Test against exploit
```

---

## Educational Approach

### Progressive Learning Path

1. **Start Simple (Lab 1)**
   - Understand basic memory layout
   - Learn what buffers are
   - See how overflow affects adjacent memory

2. **Build Complexity (Lab 2)**
   - Understand the stack structure
   - Learn about return addresses
   - Control program execution flow

3. **Advanced Techniques (Lab 3)**
   - Inject custom code
   - Bypass security mechanisms
   - Achieve arbitrary code execution

### Hands-On Learning

Each lab includes:
- Detailed tutorials with step-by-step instructions
- Visual memory diagrams
- Troubleshooting guides
- Working exploit code
- Multiple defense implementations

---

## Security & Ethics Notice

**Important:** This educational material is intended for:
- Academic learning purposes only
- Understanding defensive security measures
- Ethical security research in controlled environments

**Do NOT:**
- Use these techniques on systems you don't own
- Apply these methods without explicit authorization
- Share exploits for malicious purposes

Unauthorized computer access is illegal. This material is for learning how to defend systems, not attack them.

---

## Project Components

### Environment Setup (Group 1)

Complete environment configuration for all three labs:
- Ubuntu VM setup and configuration
- 32-bit library installation
- Security protection disabling for educational purposes
- Testing and verification procedures

### Exploits (Group 2)

All three exploits have been successfully developed and tested:
- Protostar Stack0: Variable overwrite
- picoCTF Buffer Overflow: ret2win attack
- SEED Labs: Shellcode injection with NOP sled

### Tutorials (Group 3)

Comprehensive step-by-step tutorials for each lab:
- Beginner-friendly explanations
- Memory layout diagrams
- Common pitfalls and solutions
- Testing procedures

### Defenses (Group 4)

Three secure versions for each vulnerability:
- Code-level defenses
- Compiler-level protections
- Combined defense strategies
- Comparative analysis

---

## Technical Details

### Compilation Flags

**Vulnerable versions:**
```bash
gcc -m32 -fno-stack-protector -z execstack -no-pie -o vuln vuln.c
```

**Secure versions:**
```bash
# Version 1: No special flags (code-level defenses)
gcc -m32 -fno-stack-protector -no-pie -o secure1 secure1.c

# Version 2: Stack canary
gcc -m32 -fstack-protector-all -no-pie -o secure2 secure2.c

# Version 3: All protections
gcc -m32 -fstack-protector-all -D_FORTIFY_SOURCE=2 -z noexecstack -pie -o secure3 secure3.c
```

### Environment Setup

The SEED Labs exploit requires specific environment setup:
- Disable ASLR: `sudo sysctl -w kernel.randomize_va_space=0`
- Set-UID root permissions for demonstration purposes
- 32-bit compilation for consistent memory addresses

---

## Documentation

Each component includes comprehensive documentation:

- **READMEs:** Setup and usage instructions
- **Tutorials:** Step-by-step learning guides
- **Defense Comparisons:** Detailed analysis of security mechanisms
- **Code Comments:** Inline explanations

---

## Testing

All exploits and defenses have been tested and verified:

**Exploits:** All three working and spawn shells/change variables as expected  
**Defenses:** All secure versions successfully prevent exploits  
**Documentation:** Tutorials tested with students for clarity

---

## Acknowledgments

- Dr. Ahmet Furkan Aydogan for course guidance and project supervision
- SEED Labs project for shellcode injection concepts
- picoCTF for ret2win challenge inspiration
- Protostar for classic stack overflow exercises

---

## License

This project is licensed under the MIT License - see the LICENSE file for details.

---

## References

- SEED Labs: https://seedsecuritylabs.org/
- picoCTF: https://picoctf.org/
- Protostar: https://exploit.education/protostar/