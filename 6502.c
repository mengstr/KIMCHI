#include "6502.h" 
#include "6502Transfer.h" 
#include "6502Arithmetic.h"
#include "6502Branch.h"
#include "6502Comparison.h"
#include "6502DecInc.h"
#include "6502Flag.h"
#include "6502Interrupt.h"
#include "6502JumpsSubs.h"
#include "6502Logical.h"
#include "6502Other.h"
#include "6502ShiftRotate.h" 
#include "6502Stack.h"

//6502 CPU registers
uint16_t pc; 
uint8_t  sp;
uint8_t  a;
uint8_t  x;
uint8_t  y;

// uint8_t  status; 
uint32_t status_CARRY;
uint32_t status_ZERO;
uint32_t status_INTERRUPT;
uint32_t status_DECIMAL;
uint32_t status_BREAK;
uint32_t status_CONSTANT;
uint32_t status_OVERFLOW;
uint32_t status_SIGN;

//helper variables
uint32_t instructions = 0; //keep track of total instructions executed
uint16_t ea, reladdr, value, result;
uint8_t opcode;

//a few general functions used by various other functions
void push16(uint16_t pushval) {
    write6502(BASE_STACK + sp, (pushval >> 8));
    write6502(BASE_STACK + ((sp - 1) & 0xFF), pushval);
    sp -= 2;
}

void push8(uint8_t pushval) {
    write6502(BASE_STACK + sp--, pushval);
}

uint16_t pull16() {
    uint16_t temp16;
    temp16 = read6502(BASE_STACK + ((sp + 1) & 0xFF)) | ((uint16_t)read6502(BASE_STACK + ((sp + 2) & 0xFF)) << 8);
    sp += 2;
    return(temp16);
}

uint8_t pull8() {
    return (read6502(BASE_STACK + ++sp));
}




static void imm() { //immediate
    ea = pc++;
}

static void zp() { //zero-page
    ea = (uint16_t)read6502((uint16_t)pc++);
}

static void zpx() { //zero-page,X
    ea = ((uint16_t)read6502((uint16_t)pc++) + (uint16_t)x) & 0xFF; //zero-page wraparound
}

static void zpy() { //zero-page,Y
    ea = ((uint16_t)read6502((uint16_t)pc++) + (uint16_t)y) & 0xFF; //zero-page wraparound
}

static void rel() { //relative for branch ops (8-bit immediate value, sign-extended)
    reladdr = (uint16_t)read6502(pc++);
    if (reladdr & 0x80) reladdr |= 0xFF00;
}

static void abso() { //absolute
    ea = (uint16_t)read6502(pc) | ((uint16_t)read6502(pc+1) << 8);
    pc += 2;
}

static void absx() { //absolute,X
    ea = ((uint16_t)read6502(pc) | ((uint16_t)read6502(pc+1) << 8));
    ea += (uint16_t)x;
    pc += 2;
}

static void absy() { //absolute,Y
    ea = ((uint16_t)read6502(pc) | ((uint16_t)read6502(pc+1) << 8));
    ea += (uint16_t)y;
    pc += 2;
}

static void ind() { //indirect
    uint16_t eahelp, eahelp2;
    eahelp = (uint16_t)read6502(pc) | (uint16_t)((uint16_t)read6502(pc+1) << 8);
    eahelp2 = (eahelp & 0xFF00) | ((eahelp + 1) & 0x00FF); //replicate 6502 page-boundary wraparound bug
    ea = (uint16_t)read6502(eahelp) | ((uint16_t)read6502(eahelp2) << 8);
    pc += 2;
}

static void indx() { // (indirect,X)
    uint16_t eahelp;
    eahelp = (uint16_t)(((uint16_t)read6502(pc++) + (uint16_t)x) & 0xFF); //zero-page wraparound for table pointer
    ea = (uint16_t)read6502(eahelp & 0x00FF) | ((uint16_t)read6502((eahelp+1) & 0x00FF) << 8);
}

static void indy() { // (indirect),Y
    uint16_t eahelp, eahelp2;
    eahelp = (uint16_t)read6502(pc++);
    eahelp2 = (eahelp & 0xFF00) | ((eahelp + 1) & 0x00FF); //zero-page wraparound
    ea = (uint16_t)read6502(eahelp) | ((uint16_t)read6502(eahelp2) << 8);
    ea += (uint16_t)y;
}

void adc_abso()     {abso();    adc();  };
void adc_absx()     {absx();    adc();  };
void adc_absy()     {absy();    adc();  };
void adc_imm()      {imm();     adc();  };
void adc_indx()     {indx();    adc();  };
void adc_indy()     {indy();    adc();  };
void adc_zp()       {zp();      adc();  };
void adc_zpx()      {zpx();     adc();  };

void and_abso()     {abso();    and();  };
void and_absx()     {absx();    and();  };
void and_absy()     {absy();    and();  };
void and_imm()      {imm();     and();  };
void and_indx()     {indx();    and();  };
void and_indy()     {indy();    and();  };
void and_zp()       {zp();      and();  };
void and_zpx()      {zpx();     and();  };

void asl_abso()     {abso();    asl();  };
void asl_absx()     {absx();    asl();  };
void asl_zp()       {zp();      asl();  };
void asl_zpx()      {zpx();     asl();  };

void bcc_rel()      {rel();     bcc();  };
void bcs_rel()      {rel();     bcs();  };
void beq_rel()      {rel();     beq();  };

void bit_abso()     {abso();    bit();  };
void bit_zp()       {zp();      bit();  };

void bmi_rel()      {rel();     bmi();  };
void bne_rel()      {rel();     bne();  };
void bpl_rel()      {rel();     bpl();  };
void bvc_rel()      {rel();     bvc();  };
void bvs_rel()      {rel();     bvs();  };

void cmp_abso()     {abso();    cmp();  };
void cmp_absx()     {absx();    cmp();  };
void cmp_absy()     {absy();    cmp();  };
void cmp_imm()      {imm();     cmp();  };
void cmp_indx()     {indx();    cmp();  };
void cmp_indy()     {indy();    cmp();  };
void cmp_zp()       {zp();      cmp();  };
void cmp_zpx()      {zpx();     cmp();  };

void cpx_abso()     {abso();    cpx();  };
void cpx_imm()      {imm();     cpx();  };
void cpx_zp()       {zp();      cpx();  };

void cpy_abso()     {abso();    cpy();  };
void cpy_imm()      {imm();     cpy();  };
void cpy_zp()       {zp();      cpy();  };

void dec_abso()     {abso();    dec();  };
void dec_absx()     {absx();    dec();  };
void dec_zp()       {zp();      dec();  };
void dec_zpx()      {zpx();     dec();  };

void eor_abso()     {abso();    eor();  };
void eor_absx()     {absx();    eor();  };
void eor_absy()     {absy();    eor();  };
void eor_imm()      {imm();     eor();  };
void eor_indx()     {indx();    eor();  };
void eor_indy()     {indy();    eor();  };
void eor_zp()       {zp();      eor();  };
void eor_zpx()      {zpx();     eor();  };

void inc_abso()     {abso();    inc();  };
void inc_absx()     {absx();    inc();  };
void inc_zp()       {zp();      inc();  };
void inc_zpx()      {zpx();     inc();  };

void jmp_abso()     {abso();    jmp();  };
void jmp_ind()      {ind();     jmp();  };

void jsr_abso()     {abso();    jsr();  };

void lda_abso()     {abso();    lda();  };
void lda_absx()     {absx();    lda();  };
void lda_absy()     {absy();    lda();  };
void lda_imm()      {imm();     lda();  };
void lda_indx()     {indx();    lda();  };
void lda_indy()     {indy();    lda();  };
void lda_zp()       {zp();      lda();  };
void lda_zpx()      {zpx();     lda();  };

void ldx_abso()     {abso();    ldx();  };
void ldx_absy()     {absy();    ldx();  };
void ldx_imm()      {imm();     ldx();  };
void ldx_zp()       {zp();      ldx();  };
void ldx_zpy()      {zpy();     ldx();  };

void ldy_abso()     {abso();    ldy();  };
void ldy_absx()     {absx();    ldy();  };
void ldy_imm()      {imm();     ldy();  };
void ldy_zp()       {zp();      ldy();  };
void ldy_zpx()      {zpx();     ldy();  };

void lsr_abso()     {abso();    lsr();  };
void lsr_absx()     {absx();    lsr();  };
void lsr_zp()       {zp();      lsr();  };
void lsr_zpx()      {zpx();     lsr();  };

void ora_abso()     {abso();    ora();  };
void ora_absx()     {absx();    ora();  };
void ora_absy()     {absy();    ora();  };
void ora_imm()      {imm();     ora();  };
void ora_indx()     {indx();    ora();  };
void ora_indy()     {indy();    ora();  };
void ora_zp()       {zp();      ora();  };
void ora_zpx()      {zpx();     ora();  };

void rol_abso()     {abso();    rol();  };
void rol_absx()     {absx();    rol();  };
void rol_zp()       {zp();      rol();  };
void rol_zpx()      {zpx();     rol();  };

void ror_abso()     {abso();    ror();  };
void ror_absx()     {absx();    ror();  };
void ror_zp()       {zp();      ror();  };
void ror_zpx()      {zpx();     ror();  };

void sbc_abso()     {abso();    sbc();  };
void sbc_absx()     {absx();    sbc();  };
void sbc_absy()     {absy();    sbc();  };
void sbc_imm()      {imm();     sbc();  };
void sbc_indx()     {indx();    sbc();  };
void sbc_indy()     {indy();    sbc();  };
void sbc_zp()       {zp();      sbc();  };
void sbc_zpx()      {zpx();     sbc();  };

void sta_abso()     {abso();    sta();  };
void sta_absx()     {absx();    sta();  };
void sta_absy()     {absy();    sta();  };
void sta_indx()     {indx();    sta();  };
void sta_indy()     {indy();    sta();  };
void sta_zp()       {zp();      sta();  };
void sta_zpx()      {zpx();     sta();  };

void stx_abso()     {abso();    stx();  };
void stx_zp()       {zp();      stx();  };
void stx_zpy()      {zpy();     stx();  };

void sty_abso()     {abso();    sty();  };
void sty_zp()       {zp();      sty();  };
void sty_zpx()      {zpx();     sty();  };

void illegalOp()    {brk();             };


static void (*insttable[256])() = {
/*           |  0      |  1         |  2        |  3        |  4        |  5        |  6        |  7        |  8    |  9        |  A        |  B        |  C        |  D       |  E         |  F       */
/* 0 */      brk,       ora_indx,   illegalOp,  illegalOp,  illegalOp,  ora_zp,     asl_zp,     illegalOp,  php,    ora_imm,    aslAcc,     illegalOp,  illegalOp,  ora_abso,  asl_abso,    illegalOp, /* 0 */
/* 1 */      bpl_rel,   ora_indy,   illegalOp,  illegalOp,  illegalOp,  ora_zpx,    asl_zpx,    illegalOp,  clc,    ora_absy,   illegalOp,  illegalOp,  illegalOp,  ora_absx,  asl_absx,    illegalOp, /* 1 */
/* 2 */      jsr_abso,  and_indx,   illegalOp,  illegalOp,  bit_zp,     and_zp,     rol_zp,     illegalOp,  plp,    and_imm,    rolAcc,     illegalOp,  bit_abso,   and_abso,  rol_abso,    illegalOp, /* 2 */
/* 3 */      bmi_rel,   and_indy,   illegalOp,  illegalOp,  illegalOp,  and_zpx,    rol_zpx,    illegalOp,  sec,    and_absy,   illegalOp,  illegalOp,  illegalOp,  and_absx,  rol_absx,    illegalOp, /* 3 */
/* 4 */      rti,       eor_indx,   illegalOp,  illegalOp,  illegalOp,  eor_zp,     lsr_zp,     illegalOp,  pha,    eor_imm,    lsrAcc,     illegalOp,  jmp_abso,   eor_abso,  lsr_abso,    illegalOp, /* 4 */
/* 5 */      bvc_rel,   eor_indy,   illegalOp,  illegalOp,  illegalOp,  eor_zpx,    lsr_zpx,    illegalOp,  cli,    eor_absy,   illegalOp,  illegalOp,  illegalOp,  eor_absx,  lsr_absx,    illegalOp, /* 5 */
/* 6 */      rts,       adc_indx,   illegalOp,  illegalOp,  illegalOp,  adc_zp,     ror_zp,     illegalOp,  pla,    adc_imm,    rorAcc,     illegalOp,  jmp_ind,    adc_abso,  ror_abso,    illegalOp, /* 6 */
/* 7 */      bvs_rel,   adc_indy,   illegalOp,  illegalOp,  illegalOp,  adc_zpx,    ror_zpx,    illegalOp,  sei,    adc_absy,   illegalOp,  illegalOp,  illegalOp,  adc_absx,  ror_absx,    illegalOp, /* 7 */
/* 8 */      illegalOp, sta_indx,   illegalOp,  illegalOp,  sty_zp,     sta_zp,     stx_zp,     illegalOp,  dey,    illegalOp,  txa,        illegalOp,  sty_abso,   sta_abso,  stx_abso,    illegalOp, /* 8 */
/* 9 */      bcc_rel,   sta_indy,   illegalOp,  illegalOp,  sty_zpx,    sta_zpx,    stx_zpy,    illegalOp,  tya,    sta_absy,   txs,        illegalOp,  illegalOp,  sta_absx,  illegalOp,   illegalOp, /* 9 */
/* A */      ldy_imm,   lda_indx,   ldx_imm,    illegalOp,  ldy_zp,     lda_zp,     ldx_zp,     illegalOp,  tay,    lda_imm,    tax,        illegalOp,  ldy_abso,   lda_abso,  ldx_abso,    illegalOp, /* A */
/* B */      bcs_rel,   lda_indy,   illegalOp,  illegalOp,  ldy_zpx,    lda_zpx,    ldx_zpy,    illegalOp,  clv,    lda_absy,   tsx,        illegalOp,  ldy_absx,   lda_absx,  ldx_absy,    illegalOp, /* B */
/* C */      cpy_imm,   cmp_indx,   illegalOp,  illegalOp,  cpy_zp,     cmp_zp,     dec_zp,     illegalOp,  iny,    cmp_imm,    dex,        illegalOp,  cpy_abso,   cmp_abso,  dec_abso,    illegalOp, /* C */
/* D */      bne_rel,   cmp_indy,   illegalOp,  illegalOp,  illegalOp,  cmp_zpx,    dec_zpx,    illegalOp,  cld,    cmp_absy,   illegalOp,  illegalOp,  illegalOp,  cmp_absx,  dec_absx,    illegalOp, /* D */
/* E */      cpx_imm,   sbc_indx,   illegalOp,  illegalOp,  cpx_zp,     sbc_zp,     inc_zp,     illegalOp,  inx,    sbc_imm,    nop,        sbc_imm,    cpx_abso,   sbc_abso,  inc_abso,    illegalOp, /* E */
/* F */      beq_rel,   sbc_indy,   illegalOp,  illegalOp,  illegalOp,  sbc_zpx,    inc_zpx,    illegalOp,  sed,    sbc_absy,   illegalOp,  illegalOp,  illegalOp,  sbc_absx,  inc_absx,    illegalOp  /* F */
};

void reset6502(void) {
    pc = (uint16_t)read6502(0xFFFC) | ((uint16_t)read6502(0xFFFD) << 8);
    a = 0;
    x = 0;
    y = 0;
    sp = 0xFF;
    status_CONSTANT = FLAG_CONSTANT;
}

void nmi6502(void) {
    push16(pc);
    push8(  //push CPU status to stack
            status_CARRY        |
            status_ZERO         |
            status_INTERRUPT    |
            status_DECIMAL      |
            // FLAG_BREAK       |
            FLAG_CONSTANT       |
            status_OVERFLOW     |
            status_SIGN
        ); 
    status_INTERRUPT = FLAG_INTERRUPT;
    pc = (uint16_t)read6502(0xFFFA) | ((uint16_t)read6502(0xFFFB) << 8);
}

void irq6502(void) {
    push16(pc);
    push8(  //push CPU status to stack
            status_CARRY        |
            status_ZERO         |
            status_INTERRUPT    |
            status_DECIMAL      |
            // FLAG_BREAK       |
            FLAG_CONSTANT       |
            status_OVERFLOW     |
            status_SIGN
        ); 
    status_INTERRUPT = FLAG_INTERRUPT;
    pc = (uint16_t)read6502(0xFFFE) | ((uint16_t)read6502(0xFFFF) << 8);
}

uint32_t exec6502(uint32_t count) {
    while (count>0) {
        opcode = read6502(pc++);
        status_CONSTANT = FLAG_CONSTANT;
        (*insttable[opcode])();
        instructions++;
        count--;
    }
    return instructions;
}

