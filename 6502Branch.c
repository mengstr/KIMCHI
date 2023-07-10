#include "6502.h" 
#include "6502Branch.h"

// Conditional Branch Instructions
//
// Branch targets are relative, signed 8-bit address offsets. (An offset 
// of #0 corresponds to the immedately following address — or a rather 
// odd and expensive NOP.)
//
//      BCC branch on carry clear
//      BCS branch on carry set
//      BEQ branch on equal (zero set)
//      BMI branch on minus (negative set)
//      BNE branch on not equal (zero clear)
//      BPL branch on plus (negative clear)
//      BVC branch on overflow clear
//      BVS branch on overflow set


void bcc(void) {
    if ((status_CARRY) == 0) {
        pc += reladdr;
    }
}


void bcs(void) {
    if (status_CARRY) {
        pc += reladdr;
    }
}


void beq(void) {
    if (status_ZERO) {
        pc += reladdr;
    }
}


void bmi(void) {
    if (status_SIGN) {
        pc += reladdr;
    }
}


void bne(void) {
    if (!status_ZERO) {
        pc += reladdr;
    }
}


void bpl(void) {
    if (!status_SIGN) {
        pc += reladdr;
    }
}


void bvc(void) {
    if (!status_OVERFLOW) {
        pc += reladdr;
    }
}


void bvs(void) {
    if (status_OVERFLOW) {
        pc += reladdr;
    }
}
