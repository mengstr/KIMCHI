#include "6502.h" 
#include "6502Interrupt.h"

// Interrupts
// 
// A hardware interrupt (maskable IRQ and non-maskable NMI), will cause the 
// processor to put first the address currently in the program counter onto 
// the stack (in HB-LB order), followed by the value of the status register.
// (The stack will now contain, seen from the bottom or from the most recently
// added byte, SR PC-L PC-H with the stack pointer pointing to the address 
// below the stored contents of status register.) Then, the processor will 
// divert its control flow to the address provided in the two word-size 
// interrupt vectors at $FFFA (IRQ) and $FFFE (NMI).
//
// A set interrupt disable flag will inhibit the execution of an IRQ, but not
// of a NMI, which will be executed anyways. The break instruction (BRK) 
// behaves like a NMI, but will push the value of PC+2 onto the stack to be
// used as the return address. Also, as with any software initiated transfer 
// of the status register to the stack, the break flag will be found set on 
// the respective value pushed onto the stack. Then, control is transferred 
// to the address in the NMI-vector at $FFFE. In any way, the interrupt
// disable flag is set to inhibit any further IRQ as control is transferred 
// to the interrupt handler specified by the respective interrupt vector.
//
// The RTI instruction restores the status register from the stack and behaves
// otherwise like the JSR instruction. The break flag is always ignored as 
// the status is read from the stack, as it isn't a real processor flag anyway.
//
//      BRK break / software interrupt
//      RTI return from interrupt


void brk(void) {
    pc++;
    push16(pc); //push next instruction address onto stack
    push8(  //push CPU status to stack
            status_CARRY        |
            status_ZERO         |
            status_INTERRUPT    |
            status_DECIMAL      |
            FLAG_BREAK          |
            FLAG_CONSTANT       |
            status_OVERFLOW     |
            status_SIGN
        ); 
    status_INTERRUPT=FLAG_INTERRUPT;
    pc = (uint16_t)read6502(0xFFFE) | ((uint16_t)read6502(0xFFFF) << 8);
}


void rti(void) {
    uint8_t v = pull8();
    status_CARRY    = v&FLAG_CARRY;
    status_ZERO     = v&FLAG_ZERO;
    status_INTERRUPT= v&FLAG_INTERRUPT;
    status_DECIMAL  = v&FLAG_DECIMAL;
    // status_BREAK    = v&FLAG_BREAK;
    // status_CONSTANT = v&FLAG_CONSTANT;
    status_OVERFLOW = v&FLAG_OVERFLOW;
    status_SIGN     = v&FLAG_SIGN;

    pc = pull16();
}
