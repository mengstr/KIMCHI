#include "6502.h" 
#include "6502Comparison.h"

// Comparisons
//
// Generally, comparison instructions subtract the operand from the given 
// register without affecting this register. Flags are still set as with a 
// normal subtraction and thus the relation of the two values becomes 
// accessible by the Zero, Carry and Negative flags.
// (See the branch instructions below for how to evaluate flags.)
//
// Relation R − Op       Z   C   N
// -------------------------------------------------
// Register < Operand    0   0   sign bit of result
// Register = Operand    1   1   0
// Register > Operand    0   1   sign bit of result
//
//      CMP compare (with accumulator)
//      CPX compare with X
//      CPY compare with Y


void cmp(void) {
    value = ((uint16_t)read6502(ea));
    result = (uint16_t)a - value;
    if (a >= (uint8_t)(value & 0x00FF)) status_CARRY=FLAG_CARRY; else status_CARRY=0;
    if (a == (uint8_t)(value & 0x00FF)) status_ZERO=FLAG_ZERO; else status_ZERO=0;
    if ((result) & 0x80) status_SIGN=FLAG_SIGN; else status_SIGN=0;
}


void cpx(void) {
    value = ((uint16_t)read6502(ea));
    result = (uint16_t)x - value;
    if (x >= (uint8_t)(value & 0x00FF)) status_CARRY=FLAG_CARRY; else status_CARRY=0;
    if (x == (uint8_t)(value & 0x00FF)) status_ZERO=FLAG_ZERO; else status_ZERO=0;
    if ((result) & 0x80) status_SIGN=FLAG_SIGN; else status_SIGN=0;
}


void cpy(void) {
    value = ((uint16_t)read6502(ea));
    result = (uint16_t)y - value;
    if (y >= (uint8_t)(value & 0x00FF)) status_CARRY=FLAG_CARRY; else status_CARRY=0;
    if (y == (uint8_t)(value & 0x00FF)) status_ZERO=FLAG_ZERO; else status_ZERO=0;
    if ((result) & 0x80) status_SIGN=FLAG_SIGN; else status_SIGN=0;
}

