#include "6502.h"
#include "6502Transfer.h"

// Transfer Instructions
// Load, store, interregister transfer
//
//      LDA load accumulator
//      LDX load X
//      LDY load Y
//      STA store accumulator
//      STX store X
//      STY store Y
//      TAX transfer accumulator to X
//      TAY transfer accumulator to Y
//      TSX transfer stack pointer to X
//      TXA transfer X to accumulator
//      TXS transfer X to stack pointer
//      TYA transfer Y to accumulator


void lda() {
    a = read6502(ea);
    if (a) status_ZERO=0; else status_ZERO=FLAG_ZERO;
    if (a & 0x80) status_SIGN=FLAG_SIGN; else status_SIGN=0;
}

void ldx() {
    x = read6502(ea);
    if (x) status_ZERO=0; else status_ZERO=FLAG_ZERO;
    if (x & 0x80) status_SIGN=FLAG_SIGN; else status_SIGN=0;
}

void ldy() {
    y = read6502(ea);
    if (y) status_ZERO=0; else status_ZERO=FLAG_ZERO;
    if (y & 0x80) status_SIGN=FLAG_SIGN; else status_SIGN=0;
}

void sta() {
    write6502(ea, a);
}

void stx() {
    write6502(ea, x);
}

void sty() {
    write6502(ea, y);
}

void tax() {
    x = a;
    if (x) status_ZERO=0; else status_ZERO=FLAG_ZERO;
    if (x & 0x80) status_SIGN=FLAG_SIGN; else status_SIGN=0;
}

void tay() {
    y = a;
    if (y) status_ZERO=0; else status_ZERO=FLAG_ZERO;
    if (y & 0x80) status_SIGN=FLAG_SIGN; else status_SIGN=0;

}

void tsx() {
    x = sp;
    if (x) status_ZERO=0; else status_ZERO=FLAG_ZERO;
    if (x & 0x80) status_SIGN=FLAG_SIGN; else status_SIGN=0;
}

void txa() {
    a = x;
    if (x) status_ZERO=0; else status_ZERO=FLAG_ZERO;
    if (x & 0x80) status_SIGN=FLAG_SIGN; else status_SIGN=0;
}

void txs() {
    sp = x;
}

void tya() {
    a = y;
    if (a) status_ZERO=0; else status_ZERO=FLAG_ZERO;
    if (a & 0x80) status_SIGN=FLAG_SIGN; else status_SIGN=0;
}
