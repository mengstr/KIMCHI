#include "6502.h" 
#include "6502Arithmetic.h"

// Arithmetic Operations
// 
//      ADC add with carry (prepare by CLC)
//      SBC subtract with carry (prepare by SEC)

void adc(void) {
    uint16_t v = ((uint16_t)read6502(ea));
    uint16_t c = (uint16_t)(status_CARRY);
    uint16_t res = (uint16_t)a + v + c;

    if (status_DECIMAL) {
        uint16_t al = (a & 0x0F) + (v & 0x0F) + c;
        if (al > 9) al += 6;
        uint16_t ah = (a >> 4) + (v >> 4) + ((al > 15) ? 1 : 0);
        if ((res) & 0x00FF) status_ZERO=0; else status_ZERO=FLAG_ZERO;
        if ((ah & 8) == 0) status_SIGN=0; else status_SIGN=FLAG_SIGN;
        if ((~(a ^ v) & (a ^ (ah << 4)) & 0x80) != 0) status_OVERFLOW=FLAG_OVERFLOW; else status_OVERFLOW=0;
        if (ah > 9) ah += 6;
        if (ah > 15) status_CARRY=FLAG_CARRY; else status_CARRY=0;
        a = (uint8_t)((ah << 4) | (al & 15)) & 0xFF;
    } else {
        if ((res) & 0x00FF) status_ZERO=0; else status_ZERO=FLAG_ZERO;
        if ((res) & 0x80) status_SIGN=FLAG_SIGN; else status_SIGN=0;
        overflowcalc(res, a, v);
        if ((res) & 0xFF00) status_CARRY=FLAG_CARRY;else status_CARRY=0;
        a = (uint8_t)((res) & 0x00FF);
    }
}


void sbc(void) {
    int16_t aIn = (int16_t)a;
    int16_t vIn = ((int16_t)read6502(ea));
    int16_t cIn = (int16_t)(status_CARRY);
    int16_t r = aIn - vIn - (1-cIn);

    if (status_DECIMAL) {
        int16_t al=(aIn&0x0F) - (vIn & 0x0F) - (1-cIn);
        if (al < 0) al -= 6;

        int16_t ah = (aIn >> 4) - (vIn >> 4) - ((al < 0) ? 1 : 0);

        status_ZERO = ((r & 0xFF) == 0) ? FLAG_ZERO : 0;
        status_SIGN = ((r & 0x80) != 0) ? FLAG_SIGN : 0;
        status_OVERFLOW = (((aIn ^ vIn) & (aIn ^  r) & 0x80) != 0) ? FLAG_OVERFLOW : 0;
        status_CARRY = ((r & 0x100) != 0) ? 0 : FLAG_CARRY;

        if (ah < 0) ah -= 6;

        a=(uint8_t)((ah << 4) | (al & 0x0F)) & 0xFF;
        // if (C) status|=FLAG_CARRY; else status&=(~FLAG_CARRY);
        // if (Z) status|=FLAG_ZERO; else status&=(~FLAG_ZERO);
        // if (V) status|=FLAG_OVERFLOW; else status&=(~FLAG_OVERFLOW);
        // if (N) status|=FLAG_SIGN; else status&=(~FLAG_SIGN);

    } else {

        status_ZERO = ((r & 0xFF) == 0) ? FLAG_ZERO : 0;
        status_SIGN = ((r & 0x80) != 0) ? FLAG_SIGN : 0;
        status_OVERFLOW = (((aIn ^ vIn) & (aIn ^ r) & 0x80) != 0) ? FLAG_OVERFLOW : 0;
        status_CARRY = ((r & 0x100) != 0) ? 0 : FLAG_CARRY;

        a=(uint8_t)(r&0x0FF);
        // if (C) status|=FLAG_CARRY; else status&=(~FLAG_CARRY);
        // if (Z) status|=FLAG_ZERO; else status&=(~FLAG_ZERO);
        // if (V) status|=FLAG_OVERFLOW; else status&=(~FLAG_OVERFLOW);
        // if (N) status|=FLAG_SIGN; else status&=(~FLAG_SIGN);
    }
}
