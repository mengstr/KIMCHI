#include "6502.h" 
#include "6502Other.h"

// Other
//
//      BIT bit test (accumulator & memory)
//      NOP no operation



// BIT
// Test Bits in Memory with Accumulator
// 
// Bits 7 and 6 of operand are transfered to bit 7 and 6 of SR (N,V);
// the zero-flag is set to the result of operand AND accumulator.
// 
// A AND M, M7 -> N, M6 -> V
// 
// N   Z   C   I   D   V
// M7  +   -   -   -   M6
//
// addressing  assembler   opc     bytes   cycles
// zeropage    BIT oper    $24     2       3  
// absolute    BIT oper    $2C     3       4  
//
void bit(void) {
    value = ((uint16_t)read6502(ea));
    result = (uint16_t)a & value;
    if (result & 0xFF) status_ZERO=0; else status_ZERO=FLAG_ZERO;
    if (value & 0x80) status_SIGN=FLAG_SIGN; else status_SIGN=0;
    if (value & 0x40) status_OVERFLOW=FLAG_OVERFLOW; else status_OVERFLOW=0;
}


// NOP
// No Operation
// 
// ---
// 
// N   Z   C   I   D   V
// -   -   -   -   -   -
// 
// addressing   assembler   opc     bytes   cycles
// implied	    NOP	        $EA	    1	    2  
//
void nop(void) {
}
