#include "6502.h" 
#include "6502JumpsSubs.h"

// Jumps & Subroutines
// 
// JSR and RTS affect the stack as the return address is pushed onto or 
// pulled from the stack, respectively. (JSR will first push the high-byte 
// of the return address [PC+2] onto the stack, then the low-byte. The stack
// will then contain, seen from the bottom or from the most recently added 
// byte, [PC+2]-L [PC+2]-H.)
//
//      JMP jump
//      JSR jump subroutine
//      RTS return from subroutine


void jmp(void) {
    pc = ea;
}


void jsr(void) {
    push16(pc - 1);
    pc = ea;
}


void rts(void) {
    value = pull16();
    pc = value + 1;
}
