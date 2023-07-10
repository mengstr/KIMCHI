#include "6502.h"
#include "6502ShiftRotate.h" 

// Shift & Rotate Instructions
// All shift and rotate instructions preserve the bit shifted out in the carry flag.
//
//      ASL arithmetic shift left (shifts in a zero bit on the right)
//      LSR logical shift right (shifts in a zero bit on the left)
//      ROL rotate left (shifts in carry bit on the right)
//      ROR rotate right (shifts in zero bit on the left)


void asl() {
    value = ((uint16_t)read6502(ea));
    result = value << 1;
    if (result & 0xFF00) status_CARRY=FLAG_CARRY;else status_CARRY=0;
    if (result & 0x00FF) status_ZERO=0; else status_ZERO=FLAG_ZERO;
    if (result & 0x80) status_SIGN=FLAG_SIGN; else status_SIGN=0;
    write6502(ea, result);
}


void aslAcc() {
    value = (uint16_t)a;
    result = value << 1;
    if (result & 0xFF00) status_CARRY=FLAG_CARRY;else status_CARRY=0;
    if (result & 0x00FF) status_ZERO=0; else status_ZERO=FLAG_ZERO;
    if (result & 0x80) status_SIGN=FLAG_SIGN; else status_SIGN=0;
    a=(uint8_t)(result & 0x00FF);
}


void lsr() {
    value = ((uint16_t)read6502(ea));
    result = value >> 1;
    if (value & 1) status_CARRY=FLAG_CARRY; else status_CARRY=0;
    if (result & 0x00FF) status_ZERO=0; else status_ZERO=FLAG_ZERO;
    if (result & 0x80) status_SIGN=FLAG_SIGN; else status_SIGN=0;
    write6502(ea, result);
}


void lsrAcc() {
    value = (uint16_t)a;
    result = value >> 1;
    if (value & 1) status_CARRY=FLAG_CARRY; else status_CARRY=0;
    if (result & 0x00FF) status_ZERO=0; else status_ZERO=FLAG_ZERO;
    if (result & 0x80) status_SIGN=FLAG_SIGN; else status_SIGN=0;
    a=(uint8_t)(result & 0x00FF);
}


void rol() {
    value = ((uint16_t)read6502(ea));
    result = (value << 1) | (status_CARRY);
    if (result & 0xFF00) status_CARRY=FLAG_CARRY;else status_CARRY=0;
    if (result & 0x00FF) status_ZERO=0; else status_ZERO=FLAG_ZERO;
    if (result & 0x80) status_SIGN=FLAG_SIGN; else status_SIGN=0;
    write6502(ea, result);
}


void rolAcc() {
    value = (uint16_t)a;
    result = (value << 1) | (status_CARRY);
    if (result & 0xFF00) status_CARRY=FLAG_CARRY;else status_CARRY=0;
    if (result & 0x00FF) status_ZERO=0; else status_ZERO=FLAG_ZERO;
    if (result & 0x80) status_SIGN=FLAG_SIGN; else status_SIGN=0;
    a=(uint8_t)(result & 0x00FF);
}


void ror() {
    value = ((uint16_t)read6502(ea));
    result = (value >> 1) | ((status_CARRY) << 7);
    if (value & 1) status_CARRY=FLAG_CARRY; else status_CARRY=0;
    if (result & 0x00FF) status_ZERO=0; else status_ZERO=FLAG_ZERO;
    if (result & 0x80) status_SIGN=FLAG_SIGN; else status_SIGN=0;
    write6502(ea, result);
}


void rorAcc() {
    value = (uint16_t)a;
    result = (value >> 1) | ((status_CARRY) << 7);
    if (value & 1) status_CARRY=FLAG_CARRY; else status_CARRY=0;
    if (result & 0x00FF) status_ZERO=0; else status_ZERO=FLAG_ZERO;
    if (result & 0x80) status_SIGN=FLAG_SIGN; else status_SIGN=0;
    a=(uint8_t)(result & 0x00FF);
}
