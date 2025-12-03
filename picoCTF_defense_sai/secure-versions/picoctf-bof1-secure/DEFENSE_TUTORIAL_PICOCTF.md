# picoCTF Buffer Overflow – Secure Versions & Defense Report
Created by: Sai Monish Guduru  
Group 4 – Defense Team  
CSC-544 Network Programming Project  

---

## 📌 Overview
This directory contains **five secure versions** of the picoCTF Buffer Overflow ret2win program.  
Each version demonstrates a **different defense technique** against buffer overflow attacks.

These secure versions defend against the exploit written by Group 2.

---

# ✅ Version List & Purpose

## **🔹 Version 1 — Remove win() Function**
**File:** `vuln_v1_no_win.c`  
**Defense Used:** No win() function → removes attacker’s target  
**Effect:** ret2win exploit cannot redirect execution because the win() function no longer exists.

✔️ Attack fails  
✔️ Program prints input normally  
✖️ Still overflow-vulnerable (gets)

---

## **🔹 Version 2 — Safe Input (fgets instead of gets)**  
**File:** `vuln_v2_safe_input.c`  
**Defense Used:** Replace `gets()` with `fgets()`  
**Effect:** Prevents buffer overflow entirely by limiting input size.

✔️ No overflow  
✔️ win() unreachable  
✔️ Exploit fails immediately  

---

## **🔹 Version 3 — Stack Canary**
**File:** `vuln_v3_canary.c`  
**Compiled With:**  

gcc -m32 -fstack-protector-all -no-pie -o vuln_v3 vuln_v3_canary.c
**Defense Used:** Stack Canary  
**Effect:** Program detects overflow → terminates safely.

✔️ Detects overflow  
✔️ Program aborts before return address overwrite  
✔️ Exploit fails  
✖️ Still vulnerable if bypassed (but harder)

---

## **🔹 Version 4 — ASLR + Canary + PIE**
**File:** `vuln_v4_aslr.c`  
**Compiled With:**  
gcc -m32 -fstack-protector-all -pie -o vuln_v4 vuln_v4_aslr.c
**Defense Used:**  
- PIE (randomizes function addresses)  
- ASLR (random stack)  
- Stack Canary  

**Effect:**  
✔️ win() address changes every run → ret2win impossible  
✔️ Canary stops stack smashing  
✔️ Exploit becomes unreliable  

---

## **🔹 Version 5 — All Defenses Combined**
**File:** `vuln_v5_all_defenses.c`  
**Compiled With:**  
gcc -m32 -fstack-protector-all -D_FORTIFY_SOURCE=2 -pie -o vuln_v5 vuln_v5_all_defenses.c
**Defense Used:**  
- Input validation  
- fgets() safe input  
- Stack canary  
- PIE  
- ASLR  
- Fortify source  

**Effect:**  
✔️ Fully protected  
✔️ Cannot overflow  
✔️ Cannot smash stack  
✔️ Cannot locate win()  
✔️ Exploit completely fails  

---

# 🧪 Test Script Used  
**File:** `test_defenses.sh`  
This script automatically tests:

1. Original vulnerable program  
2. Version 1  
3. Version 2  
4. Version 3  
5. Version 4  
6. Version 5  

Expected outputs:  
- Original → **win() triggered**  
- V1 → **cannot jump to win()**  
- V2 → **overflow prevented**  
- V3 → **stack canary triggered**  
- V4 → **stack canary + ASLR**  
- V5 → **fully secure**  

---

# 📌 Summary Table

| Version | Defense Technique | Result |
|--------|-------------------|--------|
| v1 | Remove win() | Exploit target removed |
| v2 | Safe input (`fgets`) | Overflow prevented |
| v3 | Stack Canary | Detects overflow |
| v4 | PIE + ASLR + Canary | Randomizes memory; stops ret2win |
| v5 | All defenses | Fully protected |

---

# ✅ Final Notes
- All versions successfully prevented the Group 2 ret2win exploit.  
- Version 5 represents a **production-grade secure C program**.  
- This work completes the **Defense Team’s requirement** for the picoCTF portion.

---

# ✔️ End of Report
