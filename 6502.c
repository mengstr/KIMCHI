
#include <stdio.h>
#include <stdint.h>

extern int TRACE;

//6502 defines

#define FLAG_CARRY     0x01
#define FLAG_ZERO      0x02
#define FLAG_INTERRUPT 0x04
#define FLAG_DECIMAL   0x08
#define FLAG_BREAK     0x10
#define FLAG_CONSTANT  0x20
#define FLAG_OVERFLOW  0x40
#define FLAG_SIGN      0x80

#define BASE_STACK     0x100

#define saveaccum(n) a = (uint8_t)((n) & 0x00FF)


//flag modifier macros
#define setcarry() status |= FLAG_CARRY
#define clearcarry() status&=(~FLAG_CARRY)
#define setzero() status |= FLAG_ZERO
#define clearzero() status &= (~FLAG_ZERO)
#define setinterrupt() status |= FLAG_INTERRUPT
#define clearinterrupt() status &= (~FLAG_INTERRUPT)
#define setoverflow() status|=FLAG_OVERFLOW
#define clearoverflow() status &= (~FLAG_OVERFLOW)
#define setsign() status |= FLAG_SIGN
#define clearsign() status &= (~FLAG_SIGN)


//flag calculation macros
#define zerocalc(n) {\
    if ((n) & 0x00FF) clearzero();\
        else setzero();\
}

#define signcalc(n) {\
    if ((n) & 0x0080) setsign();\
        else clearsign();\
}

#define carrycalc(n) {\
    if ((n) & 0xFF00) setcarry();\
        else clearcarry();\
}

#define overflowcalc(n, m, o) { /* n = result, m = accumulator, o = memory */ \
    if (((n) ^ (uint16_t)(m)) & ((n) ^ (o)) & 0x0080) setoverflow();\
        else clearoverflow();\
}


//6502 CPU registers
uint16_t pc; 
uint8_t sp, a, x, y, status; 
 
 
//helper variables
uint32_t instructions = 0; //keep track of total instructions executed
uint16_t ea, reladdr, value, result;
uint8_t opcode;

//externally supplied functions
extern uint8_t read6502(uint16_t address);
extern void write6502(uint16_t address, uint8_t value);

void setPC(uint16_t address) {pc=address;};


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

void reset6502() {
    pc = (uint16_t)read6502(0xFFFC) | ((uint16_t)read6502(0xFFFD) << 8);
    a = 0;
    x = 0;
    y = 0;
    sp = 0xFF;
    status |= FLAG_CONSTANT;
}


static void (*insttable[256])();


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


// 7  bit  0
// ---- ----
// NVss DIZC
// |||| ||||
// |||| |||+- Carry
// |||| ||+-- Zero
// |||| |+--- Interrupt Disable
// |||| +---- Decimal
// ||++------ No CPU effect, see: the B flag
// |+-------- Overflow
// +--------- Negative

void printStatus(void) {
    printf("%c%c%c%c%c%c",
        (status&FLAG_SIGN)?'N':'n',
        (status&FLAG_OVERFLOW)?'O':'o',
        (status&FLAG_DECIMAL)?'D':'d',
        (status&FLAG_INTERRUPT)?'I':'i',
        (status&FLAG_ZERO)?'Z':'z',
        (status&FLAG_CARRY)?'C':'c'
    );
}


//instruction handler functions

static void adc() {
    uint16_t v = ((uint16_t)read6502(ea));
    uint16_t c = (uint16_t)(status&FLAG_CARRY);
    uint16_t res = (uint16_t)a + v + c;

    if (status&FLAG_DECIMAL) {
        uint16_t al = (a & 0x0F) + (v & 0x0F) + c;
        if (al > 9) al += 6;
        uint16_t ah = (a >> 4) + (v >> 4) + ((al > 15) ? 1 : 0);
        zerocalc(res);
        if ((ah & 8) == 0) status &= (~FLAG_SIGN); else status |= FLAG_SIGN;
        if ((~(a ^ v) & (a ^ (ah << 4)) & 0x80) != 0) status|=FLAG_OVERFLOW; else status&=(~FLAG_OVERFLOW);
        if (ah > 9) ah += 6;
        if (ah > 15) status|=FLAG_CARRY; else status&=(~FLAG_CARRY);
        a = (uint8_t)((ah << 4) | (al & 15)) & 0xFF;
    } else {
        zerocalc(res);
        signcalc(res);
        overflowcalc(res, a, v);
        carrycalc(res);
        a = (uint8_t)((res) & 0x00FF);
    }
}

static void and() {
    value = ((uint16_t)read6502(ea));
    result = (uint16_t)a & value;
    zerocalc(result);
    signcalc(result);
    saveaccum(result);
}

static void asl() {
    value = ((uint16_t)read6502(ea));
    result = value << 1;
    carrycalc(result);
    zerocalc(result);
    signcalc(result);
    write6502(ea, result);
}

static void aslAcc() {
    value = (uint16_t)a;
    result = value << 1;
    carrycalc(result);
    zerocalc(result);
    signcalc(result);
    a=(uint8_t)(result & 0x00FF);
}

static void bcc() {
    if ((status&FLAG_CARRY) == 0) {
        pc += reladdr;
    }
}

static void bcs() {
    if ((status&FLAG_CARRY) == FLAG_CARRY) {
        pc += reladdr;
    }
}

static void beq() {
    if ((status & FLAG_ZERO) == FLAG_ZERO) {
        pc += reladdr;
    }
}

static void bit() {
    value = ((uint16_t)read6502(ea));
    result = (uint16_t)a & value;
   
    zerocalc(result);
    status = (status & 0x3F) | (uint8_t)(value & 0xC0);
}

static void bmi() {
    if ((status & FLAG_SIGN) == FLAG_SIGN) {
        pc += reladdr;
    }
}

static void bne() {
    if ((status & FLAG_ZERO) == 0) {
        pc += reladdr;
    }
}

static void bpl() {
    if ((status & FLAG_SIGN) == 0) {
        pc += reladdr;
    }
}

static void brk() {
    // printf("iCnt=%ld PC=%04x A=%02x X=%02x Y=%02x\n",instructions,pc,a,x,y);
    // for(;;);
    pc++;
    push16(pc); //push next instruction address onto stack
    push8(status | FLAG_BREAK); //push CPU status to stack
    setinterrupt(); //set interrupt flag
    pc = (uint16_t)read6502(0xFFFE) | ((uint16_t)read6502(0xFFFF) << 8);
}

static void bvc() {
    if ((status & FLAG_OVERFLOW) == 0) {
        pc += reladdr;
    }
}

static void bvs() {
    if ((status & FLAG_OVERFLOW) == FLAG_OVERFLOW) {
        pc += reladdr;
    }
}

static void clc() {
    clearcarry();
}

static void cld() {
    status &= (~FLAG_DECIMAL);
}

static void cli() {
    clearinterrupt();
}

static void clv() {
    clearoverflow();
}

static void cmp() {
    value = ((uint16_t)read6502(ea));
    result = (uint16_t)a - value;
   
    if (a >= (uint8_t)(value & 0x00FF)) setcarry();
        else clearcarry();
    if (a == (uint8_t)(value & 0x00FF)) setzero();
        else clearzero();
    signcalc(result);
}

static void cpx() {
    value = ((uint16_t)read6502(ea));
    result = (uint16_t)x - value;
   
    if (x >= (uint8_t)(value & 0x00FF)) setcarry();
        else clearcarry();
    if (x == (uint8_t)(value & 0x00FF)) setzero();
        else clearzero();
    signcalc(result);
}

static void cpy() {
    value = ((uint16_t)read6502(ea));
    result = (uint16_t)y - value;
   
    if (y >= (uint8_t)(value & 0x00FF)) setcarry();
        else clearcarry();
    if (y == (uint8_t)(value & 0x00FF)) setzero();
        else clearzero();
    signcalc(result);
}

static void dec() {
    value = ((uint16_t)read6502(ea));
    result = value - 1;
    zerocalc(result);
    signcalc(result);
    write6502(ea, result);
}

static void dex() {
    x--;
    zerocalc(x);
    signcalc(x);
}

static void dey() {
    y--;
    zerocalc(y);
    signcalc(y);
}

static void eor() {
    value = ((uint16_t)read6502(ea));
    result = (uint16_t)a ^ value;
    zerocalc(result);
    signcalc(result);
    saveaccum(result);
}

static void inc() {
    value = ((uint16_t)read6502(ea));
    result = value + 1;
    zerocalc(result);
    signcalc(result);
    write6502(ea, result);
}

static void inx() {
    x++;
    zerocalc(x);
    signcalc(x);
}

static void iny() {
    y++;
    zerocalc(y);
    signcalc(y);
}

static void jmp() {
    pc = ea;
}

static void jsr() {
    push16(pc - 1);
    pc = ea;
}

static void lda() {
    value = ((uint16_t)read6502(ea));
    a = (uint8_t)(value & 0x00FF);
    zerocalc(a);
    signcalc(a);
}

static void ldx() {
    value = ((uint16_t)read6502(ea));
    x = (uint8_t)(value & 0x00FF);
    zerocalc(x);
    signcalc(x);
}

static void ldy() {
    value = ((uint16_t)read6502(ea));
    y = (uint8_t)(value & 0x00FF);
    zerocalc(y);
    signcalc(y);
}

static void lsr() {
    value = ((uint16_t)read6502(ea));
    result = value >> 1;
    if (value & 1) setcarry(); else clearcarry();
    zerocalc(result);
    signcalc(result);
    write6502(ea, result);
}

static void lsrAcc() {
    value = (uint16_t)a;
    result = value >> 1;
    if (value & 1) setcarry(); else clearcarry();
    zerocalc(result);
    signcalc(result);
    a=(uint8_t)(result & 0x00FF);
}

static void nop() {
}

static void ora() {
    value = ((uint16_t)read6502(ea));
    result = (uint16_t)a | value;
    zerocalc(result);
    signcalc(result);
    saveaccum(result);
}

static void pha() {
    push8(a);
}

static void php() {
    push8(status | FLAG_BREAK);
}

static void pla() {
    a = pull8();
    zerocalc(a);
    signcalc(a);
}

static void plp() {
    status = pull8() | FLAG_CONSTANT;
}

static void rol() {
    value = ((uint16_t)read6502(ea));
    result = (value << 1) | (status&FLAG_CARRY);
    carrycalc(result);
    zerocalc(result);
    signcalc(result);
    write6502(ea, result);
}

static void rolAcc() {
    value = (uint16_t)a;
    result = (value << 1) | (status&FLAG_CARRY);
    carrycalc(result);
    zerocalc(result);
    signcalc(result);
    a=(uint8_t)(result & 0x00FF);
}

static void ror() {
    value = ((uint16_t)read6502(ea));
    result = (value >> 1) | ((status&FLAG_CARRY) << 7);
    if (value & 1) setcarry(); else clearcarry();
    zerocalc(result);
    signcalc(result);
    write6502(ea, result);
}

static void rorAcc() {
    value = (uint16_t)a;
    result = (value >> 1) | ((status&FLAG_CARRY) << 7);
    if (value & 1) setcarry(); else clearcarry();
    zerocalc(result);
    signcalc(result);
    a=(uint8_t)(result & 0x00FF);
}

static void rti() {
    status = pull8();
    value = pull16();
    pc = value;
}

static void rts() {
    value = pull16();
    pc = value + 1;
}


static void sbc() {
    int16_t aIn = (int16_t)a;
    int16_t vIn = ((int16_t)read6502(ea));
    int16_t cIn = (int16_t)(status&FLAG_CARRY);
    int16_t r = aIn - vIn - (1-cIn);
    // printf("\nsbc(%d-%d) cIn=%d \n",aIn,vIn,cIn);

    if (status&FLAG_DECIMAL) {
        int16_t al=(aIn&0x0F) - (vIn & 0x0F) - (1-cIn);
        if (al < 0) al -= 6;

        int16_t ah = (aIn >> 4) - (vIn >> 4) - ((al < 0) ? 1 : 0);

        int16_t Z = ((r & 0xFF) == 0) ? 1 : 0;
        int16_t N = ((r & 0x80) != 0) ? 1 : 0;
        int16_t V = (((aIn ^ vIn) & (aIn ^  r) & 0x80) != 0) ? 1 : 0;
        int16_t C = ((r & 0x100) != 0) ? 0 : 1;

        if (ah < 0) ah -= 6;

        a=(uint8_t)((ah << 4) | (al & 0x0F)) & 0xFF;
        if (C) status|=FLAG_CARRY; else status&=(~FLAG_CARRY);
        if (Z) status|=FLAG_ZERO; else status&=(~FLAG_ZERO);
        if (V) status|=FLAG_OVERFLOW; else status&=(~FLAG_OVERFLOW);
        if (N) status|=FLAG_SIGN; else status&=(~FLAG_SIGN);
        // printf("[D] =%d($%x) ah,al=%d,%d C=%d Z=%d V=%d N=%d \n",a,a,ah,al,C,Z,V,N);

    } else {

        int16_t Z = ((r & 0xFF) == 0) ? 1 : 0;
        int16_t N = ((r & 0x80) != 0) ? 1 : 0;
        int16_t V = (((aIn ^ vIn) & (aIn ^ r) & 0x80) != 0) ? 1 : 0;
        int16_t C = ((r & 0x100) != 0) ? 0 : 1;
        a=(uint8_t)(r&0x0FF);
        if (C) status|=FLAG_CARRY; else status&=(~FLAG_CARRY);
        if (Z) status|=FLAG_ZERO; else status&=(~FLAG_ZERO);
        if (V) status|=FLAG_OVERFLOW; else status&=(~FLAG_OVERFLOW);
        if (N) status|=FLAG_SIGN; else status&=(~FLAG_SIGN);
        // printf("[B] r=%d ($%x) a=%d ($%x)\n",r,r,a,a);

        // printf("[B] =%d ($%x) C=%d Z=%d V=%d N=%d\n",a,a,C,Z,V,N);



    }
}

static void sec() {
    setcarry();
}

static void sed() {
    status |= FLAG_DECIMAL;
}

static void sei() {
    setinterrupt();
}

static void sta() {
    write6502(ea, a);
}

static void stx() {
    write6502(ea, x);
}

static void sty() {
    write6502(ea, y);
}

static void tax() {
    x = a;
    zerocalc(x);
    signcalc(x);
}

static void tay() {
    y = a;
    zerocalc(y);
    signcalc(y);
}

static void tsx() {
    x = sp;
    zerocalc(x);
    signcalc(x);
}

static void txa() {
    a = x;
    zerocalc(a);
    signcalc(a);
}

static void txs() {
    sp = x;
}

static void tya() {
    a = y;
    zerocalc(a);
    signcalc(a);
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

void nmi6502() {
    push16(pc);
    push8(status);
    status |= FLAG_INTERRUPT;
    pc = (uint16_t)read6502(0xFFFA) | ((uint16_t)read6502(0xFFFB) << 8);
}

void irq6502() {
    push16(pc);
    push8(status);
    status |= FLAG_INTERRUPT;
    pc = (uint16_t)read6502(0xFFFE) | ((uint16_t)read6502(0xFFFF) << 8);
}

uint32_t exec6502(uint32_t count) {
    while (count>0) {
        opcode = read6502(pc++);
        status |= FLAG_CONSTANT;
        (*insttable[opcode])();
        instructions++;
        count--;
    }
    return instructions;
}

