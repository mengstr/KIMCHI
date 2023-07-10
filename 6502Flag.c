#include "6502.h" 
#include "6502Flag.h"

// Flag Instructions
//
//      CLC clear carry
//      CLD clear decimal (BCD arithmetics disabled)
//      CLI clear interrupt disable
//      CLV clear overflow
//      SEC set carry
//      SED set decimal (BCD arithmetics enabled)
//      SEI set interrupt disable


void clc(void) {
    status_CARRY=0;
}


void cld(void) {
    status_DECIMAL=0;
}


void cli(void) {
    status_INTERRUPT=0;
}


void clv(void) {
    status_OVERFLOW=0;
}


void sec(void) {
    status_CARRY=FLAG_CARRY;
}


void sed(void) {
    status_DECIMAL=FLAG_DECIMAL;
}


void sei(void) {
    status_INTERRUPT=FLAG_INTERRUPT;
}
