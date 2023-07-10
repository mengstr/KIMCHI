#include "6502.h" 
#include "6502DecInc.h"

// Decrements & Increments
//
//      DEC decrement (memory)
//      DEX decrement X
//      DEY decrement Y
//      INC increment (memory)
//      INX increment X
//      INY increment Y


void dec(void) {
    value = ((uint16_t)read6502(ea));
    result = (value - 1) & 0xFF;
    if (result) status_ZERO=0; else status_ZERO=FLAG_ZERO;
    if (result & 0x80) status_SIGN=FLAG_SIGN; else status_SIGN=0;
    write6502(ea, result);
}


void dex(void) {
    x=(x-1)&0xFF;
    if (x) status_ZERO=0; else status_ZERO=FLAG_ZERO;
    if (x & 0x80) status_SIGN=FLAG_SIGN; else status_SIGN=0;
}


void dey(void) {
    y=(y-1)&0xFF;
    if (y) status_ZERO=0; else status_ZERO=FLAG_ZERO;
    if (y & 0x80) status_SIGN=FLAG_SIGN; else status_SIGN=0;
}


void inc(void) {
    value = ((uint16_t)read6502(ea));
    result = (value + 1) & 0xFF;
    if (result) status_ZERO=0; else status_ZERO=FLAG_ZERO;
    if (result & 0x80) status_SIGN=FLAG_SIGN; else status_SIGN=0;
    write6502(ea, result);
}


void inx(void) {
    x=(x+1)&0xFF;
    if (x) status_ZERO=0; else status_ZERO=FLAG_ZERO;
    if (x & 0x80) status_SIGN=FLAG_SIGN; else status_SIGN=0;
}


void iny(void) {
    y=(y+1)&0xFF;
    if (y) status_ZERO=0; else status_ZERO=FLAG_ZERO;
    if (y & 0x80) status_SIGN=FLAG_SIGN; else status_SIGN=0;
}
