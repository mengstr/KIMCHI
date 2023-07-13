#include <stdint.h>

void reset6502();
void nmi6502(void);
uint32_t exec6502(uint32_t count);

//externally supplied functions
extern uint8_t read6502(uint16_t address);
extern void write6502(uint16_t address, uint8_t data);

#define FLAG_CARRY      0x01
#define FLAG_ZERO       0x02
#define FLAG_INTERRUPT  0x04
#define FLAG_DECIMAL    0x08
#define FLAG_BREAK      0x10
#define FLAG_CONSTANT   0x20
#define FLAG_OVERFLOW   0x40
#define FLAG_SIGN       0x80

#define BASE_STACK     0x100

#define overflowcalc(n, m, o) { /* n = result, m = accumulator, o = memory */ \
    if (((n) ^ (uint16_t)(m)) & ((n) ^ (o)) & 0x80) status_OVERFLOW=FLAG_OVERFLOW;\
        else status_OVERFLOW=0; \
}


//6502 CPU registers
extern uint16_t pc; 
extern uint8_t  sp;
extern uint8_t  a;
extern uint8_t  x;
extern uint8_t  y;
// extern uint8_t  status; 

extern uint32_t status_CARRY;
extern uint32_t status_ZERO;
extern uint32_t status_INTERRUPT;
extern uint32_t status_DECIMAL;
extern uint32_t status_BREAK;
extern uint32_t status_CONSTANT;
extern uint32_t status_OVERFLOW;
extern uint32_t status_SIGN;

extern uint16_t ea;
extern uint16_t reladdr;
extern uint16_t value;
extern uint16_t result;
extern uint8_t opcode;
extern uint32_t instructions;


extern uint16_t pull16();
extern uint8_t pull8();
extern void push16(uint16_t pushval);
extern void push8(uint8_t pushval);


