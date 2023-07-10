#include "6502.h"
#include "6502Stack.h"

// Stack Instructions
// 
// These instructions transfer the accumulator or status register (flags) to 
// and from the stack. The processor stack is a last-in-first-out (LIFO) 
// stack of 256 bytes length, implemented at addresses $0100 - $01FF. The 
// stack grows down as new values are pushed onto it with the current 
// insertion point maintained in the stack pointer register.
//
// (When a byte is pushed onto the stack, it will be stored in the address 
// indicated by the value currently in the stack pointer, which will be then 
// decremented by 1. Conversely, when a value is pulled from the stack, the 
// stack pointer is incremented. The stack pointer is accessible by the TSX 
// and TXS instructions.)
//
//      PHA push accumulator
//      PHP push processor status register (with break flag set)
//      PLA pull accumulator
//      PLP pull processor status register

void pha(void) {
    push8(a);
}

void php(void) {
    push8(
        status_CARRY        |
        status_ZERO         |
        status_INTERRUPT    |
        status_DECIMAL      |
        FLAG_BREAK          |
        FLAG_CONSTANT       |
        status_OVERFLOW     |
        status_SIGN
    );
}

void pla(void) {
    a = pull8();
    if (a) status_ZERO=0; else status_ZERO=FLAG_ZERO;
    if (a & 0x80) status_SIGN=FLAG_SIGN; else status_SIGN=0;
}

void plp(void) {
    uint8_t v = pull8();
    status_CARRY    = v&FLAG_CARRY;
    status_ZERO     = v&FLAG_ZERO;
    status_INTERRUPT= v&FLAG_INTERRUPT;
    status_DECIMAL  = v&FLAG_DECIMAL;
    // status_BREAK    = v&FLAG_BREAK;
    // status_CONSTANT = v&FLAG_CONSTANT;
    status_OVERFLOW = v&FLAG_OVERFLOW;
    status_SIGN     = v&FLAG_SIGN;
}


