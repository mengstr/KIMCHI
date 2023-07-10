#include "6502.h" 
#include "6502Logical.h"

// Logical Operations
// 
//      AND and (with accumulator)
//      EOR exclusive or (with accumulator)
//      ORA (inclusive) or with accumulator

void and(void) {
    value = ((uint16_t)read6502(ea));
    a = (uint16_t)(a & value) & 0xFF;
    if (a) status_ZERO=0; else status_ZERO=FLAG_ZERO;
    if (a & 0x80) status_SIGN=FLAG_SIGN; else status_SIGN=0;
}


void eor(void) {
    value = ((uint16_t)read6502(ea));
    a = (uint16_t)(a ^ value)&0xFF;
    if (a) status_ZERO=0; else status_ZERO=FLAG_ZERO;
    if (a & 0x80) status_SIGN=FLAG_SIGN; else status_SIGN=0;
}


void ora(void) {
    value = ((uint16_t)read6502(ea));
    a = (uint16_t)(a | value)&0xFF;
    if (a) status_ZERO=0; else status_ZERO=FLAG_ZERO;
    if (a & 0x80) status_SIGN=FLAG_SIGN; else status_SIGN=0;
}

