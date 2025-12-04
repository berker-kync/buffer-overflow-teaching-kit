# Debug Tools Used During Exploit Development

These tools were created to solve the GDB vs runtime address problem and verify exploit behavior.

## Critical Tools:

**stack_debug.c** THE KEY TOOL
- Added debug print to reveal actual runtime buffer address
- Solved the GDB environment shift issue
- Showed buffer at `0xffffcb08` (not `0xffffcce8` from GDB)

**get_addr.c**
- Initial attempt to find buffer address
- Revealed addresses differ between programs

**test_offset.c**
- Verified the 36-byte offset calculation
- Confirmed buffer to return address distance

**check_env.c, getenv_stack.c**
- Tested environment variable method for shellcode placement
- Alternative approach (didn't use in final exploit)

## Exploit Versions:

- `seedlabs_final_fix.py` - The breakthrough version with EDX clearing
- `seedlabs_final.py` - Pre-EDX fix attempt
- `seedlabs_exploit_robust.py` - Earlier iteration

These show the evolution toward the working exploit in `../../exploits/lab3-seedlabs/exploit.py`
