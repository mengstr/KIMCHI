#include <stdio.h>
#include "ch32v003fun/ch32v003fun.h" 
#include "6502.h"
#include "kimrom.h"

#define INITVECTORS     // If defined the NMI & IRQ vectors areset to $1C00
#define PC200           // If defined the PC will be set to $0200 at start instead of $0000

#define TRACE          0


//
//      CH32V003F4P6 SSOP20
// +------------------------------+      +--------------------------------+
// |                               \____/                                 |
// |                                                                      |
// | PD4/A7/UCK/T2CH1ETR/OPO/T1CH4ETR        T2CH2/AETR/UCTS/T1CH4/A4/PD3 |
// | PD5/A5/UTX/T2CH4/URX                       T1CH1/T2CH3/T1CH2N/A3/PD2 |
// | PD6/A6/URX/T2CH3/UTX                   AETR2/T1CH3N/SCL/URX/SWIO/PD1 | 
// | PD7/NRST/T2CH4/OPP1/UCK                    T1CH2/T2CH2/URTS/MISO/PC7 | 
// | PA1/OSCI/A1/T1CH2/OPN0                   T1CH1CH3N/UCTS/SDA/MOSI/PC6 |
// | PA2/OSCO/A0/T1CH2N/OPP0/AETR2   T1ETR/T2CH1ETR/SCL/UCK/T1CH3/SCK/PC5 |
// | VSS                                       T1CH4/MCO/T1CH1CH2N/A2/PC4 |
// | PD0/T1CH1N/OPN1/SDA/UTX                       T1CH3/T1CH1N/UCTS//PC3 |
// | VDD                             URTS/T1BKIN/AETR/T2CH2/T1ETR/SCL/PC2 |
// | PC0/T2CH3/UTX/NSS/T1CH3        NSS/T2CH4/T2CH1ETR/T1BKIN/URX/SDA/PC1 |
// |                                                                      |
// +----------------------------------------------------------------------+
//
//
// PA1 --> Key RS 
// PA2 --> Key ST 
// PC0 --> PA0 Key Column for 6 D PC
// PC1 --> PA1 Key Column for 5 C GO
// PC2 --> PA2 Key Column for 4 B +
// PC3 --> PA3 Key Column for 3 A DA
// PC4 --> PA4 Key Column for 2 9 AD
// PC5 --> PA5 Key Column for 1 8 F 
// PC6 --> PA6 Key Column for 0 7 E 
// PC7 --> PA7 TTY In 
// PD0 
// PD1 
// PD2 --> PB0 TTY Out 
// PD3 --> PB1 Display/Key Row mux LSB  
// PD4 --> PB2 Display/Key Row mux 
// PD5 --> PB3 Display/Key Row mux 
// PD6 --> PB4 Display/Key Row mux MSB 
// PD7 --> Switch SST
//

// K0 $0000 – $03FF 1024 bytes of RAM (8*6102)
// K1 $0400 – $07FF free
// K2 $0800 – $0BFF free
// K3 $0C00 – $0FFF free
// K4 $1000 – $13FF free
// K5 $1400 – $16FF free
//    $1700 – $173F I/O, timer of 6530-003
//    $1740 – $177F I/O, timer of 6530-002
//    $1780 – $17BF 64 bytes RAM of 6530-003
//    $17C0 – $17FF 64 bytes RAM of 6530-002
// K6 $1800 – $1BFF 1024 bytes ROM of 6530-003
// K7 $1C00 – $1FFF 1024 bytes ROM of 6530-002

// 6530-002 PA0 Key Column for 6 D PC
// 6530-002 PA1 Key Column for 5 C GO
// 6530-002 PA2 Key Column for 4 B +
// 6530-002 PA3 Key Column for 3 A DA
// 6530-002 PA4 Key Column for 2 9 AD
// 6530-002 PA5 Key Column for 1 8 F
// 6530-002 PA6 Key Column for 0 7 E
// 6530-002 PA7 TTY In
// 6530-002 PB0 TTY Out
// 6530-002 PB1 Display/Key Row mux LSB 
// 6530-002 PB2 Display/Key Row mux
// 6530-002 PB3 Display/Key Row mux
// 6530-002 PB4 Display/Key Row mux MSB
// 6530-002 PB5
// 6530-002 PB6 N/A
// 6530-002 PB7

// 6530-003 PA0
// 6530-003 PA1
// 6530-003 PA2
// 6530-003 PA3
// 6530-003 PA4
// 6530-003 PA5
// 6530-003 PA6
// 6530-003 PA7
// 6530-003 PB0
// 6530-003 PB1
// 6530-003 PB2
// 6530-003 PB3
// 6530-003 PB4
// 6530-003 PB5
// 6530-003 PB6 N/A
// 6530-003 PB7


#define TIMER1          0x1704
#define TIMER8          0x1705
#define TIMER64         0x1706      // Get current timer count
#define TIMER1024       0x1707      // Get underrun status in MSB
#define TIMER1irq       0x170C
#define TIMER8irq       0x170D
#define TIMER64irq      0x170E
#define TIMER1024irq    0x170F

#define SAD     0x1740  // 6530 A DATA
#define PADD    0x1741  // 6530 A DATA DIRECTION 0=in 1=out
#define SBD     0x1742  // 6530 B DATA
#define PBDD    0x1743  // 6530 B DATA DIRECTION 0=in 1=out


//  Writing to  Sets Divide   IRQ
//  Address     Ratio To      
// ------------------------------------
//  0x1704      1             Disabled
//  0x1705      8             Disabled
//  0x1706      64            Disabled
//  0x1707      1024          Disabled
//  0x170C      1             Enabled
//  0x170D      8             Enabled
//  0x170E      64            Enabled
//  0x170F      1024          Enabled

// PC     s11 
// SP     s10
// ACC    s9
// X      s8
// Y      s7
// STATUS s6

// Reg     ABI      Description            Saver
// -----------------------------------------------
// x0      zero     Zero constant          ---    
// x1      ra       Return address         Caller 
// x2      sp       Stack pointer          Callee 
// x3      gp       Global pointer         ---    
// x4      tp       Thread pointer         ---    
// x5-x7   t0-t2    Temporaries            Caller 
// x8      s0/fp    Saved / frame pointer  Callee 
// x9      s1       Saved register         Callee 
// x10-x11 a0-a1    Fn args/return values  Caller 
// x12-x17 a2-a7    Fn args                Caller 
// x18-x27 s2-s11   Saved registers        Callee 
// x28-x31 t3-t6    Temporaries            Caller 

void reset6502();
uint32_t exec6502(uint32_t count);
uint8_t read6502(uint16_t address);
void write6502(uint16_t address, uint8_t data);
extern void setPC(uint16_t address);
extern void printStatus(void);

extern const uint8_t ROM6530[];

// extern const unsigned long DecimalTest_length;
// extern const uint8_t DecimalTest[];

#define MUX         GPIOD->BSHR
#define KEYSEG      GPIOC->BSHR
#define MUX_DIR     GPIOD->CFGLR
#define KEYSEG_DIR  GPIOC->CFGLR

volatile uint8_t RAM[1024];
uint8_t RAM_1780[128];
uint8_t sad=0xff,padd=0xff,sbd=0xff,pbdd=0xff;


/* http://srecord.sourceforge.net/ */
const unsigned char DecimalTest[] =
{
0x20, 0x21, 0x02, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA,
0xEA, 0xEA, 0x00, 0xEA, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xA9, 0x01, 0x8D,
0x10, 0x02, 0xA9, 0x00, 0x8D, 0x11, 0x02, 0xA9, 0x00, 0x8D, 0x12, 0x02,
0xAD, 0x12, 0x02, 0x29, 0x0F, 0x8D, 0x1B, 0x02, 0xAD, 0x12, 0x02, 0x29,
0xF0, 0x8D, 0x1C, 0x02, 0x09, 0x0F, 0x8D, 0x1D, 0x02, 0xAD, 0x11, 0x02,
0x29, 0x0F, 0x8D, 0x1A, 0x02, 0xAD, 0x11, 0x02, 0x29, 0xF0, 0x8D, 0x19,
0x02, 0xA0, 0x00, 0x20, 0x99, 0x02, 0x20, 0x5E, 0x03, 0x20, 0x2F, 0x03,
0xD0, 0x36, 0xA0, 0x01, 0x20, 0x99, 0x02, 0x20, 0x5E, 0x03, 0x20, 0x2F,
0x03, 0xD0, 0x29, 0xA0, 0x01, 0x20, 0xEC, 0x02, 0x20, 0x6B, 0x03, 0x20,
0x2F, 0x03, 0xD0, 0x1C, 0xA0, 0x00, 0x20, 0xEC, 0x02, 0x20, 0x6B, 0x03,
0x20, 0x2F, 0x03, 0xD0, 0x0F, 0xEE, 0x11, 0x02, 0xD0, 0xB7, 0xEE, 0x12,
0x02, 0xD0, 0x9D, 0xA9, 0x00, 0x8D, 0x10, 0x02, 0x60, 0xF8, 0xC0, 0x01,
0xAD, 0x11, 0x02, 0x6D, 0x12, 0x02, 0x8D, 0x13, 0x02, 0x08, 0x68, 0x8D,
0x16, 0x02, 0xD8, 0xC0, 0x01, 0xAD, 0x11, 0x02, 0x6D, 0x12, 0x02, 0x8D,
0x17, 0x02, 0x08, 0x68, 0x8D, 0x18, 0x02, 0xC0, 0x01, 0xAD, 0x1A, 0x02,
0x6D, 0x1B, 0x02, 0xC9, 0x0A, 0xA2, 0x00, 0x90, 0x06, 0xE8, 0x69, 0x05,
0x29, 0x0F, 0x38, 0x0D, 0x19, 0x02, 0x7D, 0x1C, 0x02, 0x08, 0xB0, 0x04,
0xC9, 0xA0, 0x90, 0x03, 0x69, 0x5F, 0x38, 0x8D, 0x14, 0x02, 0x08, 0x68,
0x8D, 0x15, 0x02, 0x68, 0x8D, 0x1F, 0x02, 0x60, 0xF8, 0xC0, 0x01, 0xAD,
0x11, 0x02, 0xED, 0x12, 0x02, 0x8D, 0x13, 0x02, 0x08, 0x68, 0x8D, 0x16,
0x02, 0xD8, 0xC0, 0x01, 0xAD, 0x11, 0x02, 0xED, 0x12, 0x02, 0x8D, 0x17,
0x02, 0x08, 0x68, 0x8D, 0x18, 0x02, 0x60, 0xC0, 0x01, 0xAD, 0x1A, 0x02,
0xED, 0x1B, 0x02, 0xA2, 0x00, 0xB0, 0x06, 0xE8, 0xE9, 0x05, 0x29, 0x0F,
0x18, 0x0D, 0x19, 0x02, 0xFD, 0x1C, 0x02, 0xB0, 0x02, 0xE9, 0x5F, 0x8D,
0x14, 0x02, 0x60, 0xAD, 0x13, 0x02, 0xCD, 0x14, 0x02, 0xD0, 0x26, 0xAD,
0x16, 0x02, 0x4D, 0x1E, 0x02, 0x29, 0x80, 0xD0, 0x1C, 0xAD, 0x16, 0x02,
0x4D, 0x1F, 0x02, 0x29, 0x40, 0xD0, 0x12, 0xAD, 0x16, 0x02, 0x4D, 0x20,
0x02, 0x29, 0x02, 0xD0, 0x08, 0xAD, 0x16, 0x02, 0x4D, 0x15, 0x02, 0x29,
0x01, 0x60, 0xAD, 0x1F, 0x02, 0x8D, 0x1E, 0x02, 0xAD, 0x18, 0x02, 0x8D,
0x20, 0x02, 0x60, 0x20, 0x0F, 0x03, 0xAD, 0x18, 0x02, 0x8D, 0x1E, 0x02,
0x8D, 0x1F, 0x02, 0x8D, 0x20, 0x02, 0x8D, 0x15, 0x02, 0x60,
};
const unsigned long DecimalTest_termination = 0x00000000;
const unsigned long DecimalTest_start       = 0x00000200;
const unsigned long DecimalTest_finish      = 0x0000037E;
const unsigned long DecimalTest_length      = 0x0000017E;

#define DECIMALTEST_TERMINATION 0x00000000
#define DECIMALTEST_START       0x00000200
#define DECIMALTEST_FINISH      0x0000037E
#define DECIMALTEST_LENGTH      0x0000017E




uint8_t read6502(uint16_t address) {
    address=address&0xFFFF;
    if (address<1024) {
        if ((TRACE>0)) printf("R RAM %04x val=%02x pc=%04x\n",address,RAM[address],pc);
        return RAM[address];
    }
    if (address>=0x1C00) {
        if (TRACE>0) printf("R ROM %05x val=%02x\n",address,ROM6530[address&0x3FF]);
        return ROM6530[address&0x3FF];
    }
    if (address==SAD) {
        sad=GPIOC->INDR|0x80;   // Always use KEYPAD
        if (TRACE>0) printf("R SAD val=%02x\n", sad); 
        return sad;
    }
    if (address==SBD) {
        printf("EY! R SBD val=%02x\n", sbd); 
        return sbd;
    }
    if (address==PADD){
        if (TRACE>0) printf("EY! R PADD val=%02x\n", padd);
        return padd;
    }
    if (address==PBDD){
        if (TRACE>0) printf("EY! R PBDD val=%02x\n", pbdd);
        return pbdd;
    }
    if (address>=0x1780 && address<0x1800) {
        if (TRACE>0) printf("R SCRATCH %04x val=%02x\n",address,RAM_1780[address&0x7F]);
        return RAM_1780[address&0x7F];
    }
    printf("EY! R %04x\n",address);
    return 0;
}

void write6502(uint16_t address, uint8_t data) {
    data=data&0xFF;
    address=address&0xFFFF;

    if (address<1024) {
        if ((TRACE>0)) printf("W RAM %04x val=%02x pc=%04x\n",address,data,pc);
        RAM[address]=data;
        return;
    }
    if (address==SAD) {
        uint8_t dataInv=data^0xFF;
        uint32_t b=(dataInv<<16)|data;
        KEYSEG=b;
        if (TRACE>0) printf("W SAD val=%02x KEYSEG=%08lx\n", data,b); 
        return;
        }
    if (address==SBD) {
        data=0xff&(data<<2);
        uint8_t dataInv=data^0xFF;
        uint32_t b=(dataInv<<16)|data<<0;
        MUX=b;
        if (TRACE>0) printf("W SBD val=%02x MUX=%08lx\n", data,b); 
        return;
        }

    if (address==PADD){
        // PC0 --> PA0 Key Column for 6 D PC
        // PC1 --> PA1 Key Column for 5 C GO
        // PC2 --> PA2 Key Column for 4 B +
        // PC3 --> PA3 Key Column for 3 A DA
        // PC4 --> PA4 Key Column for 2 9 AD
        // PC5 --> PA5 Key Column for 1 8 F 
        // PC6 --> PA6 Key Column for 0 7 E 
        // PC7 --> PA7 TTY In 
        uint32_t dirbits=
            (data&0x01 ?        0x1 :        0x8) |
            (data&0x02 ?       0x10 :       0x80) |
            (data&0x04 ?      0x100 :      0x800) |
            (data&0x08 ?     0x1000 :     0x8000) |
            (data&0x10 ?    0x10000 :    0x80000) |
            (data&0x20 ?   0x100000 :   0x800000) |
            (data&0x40 ?  0x1000000 :  0x8000000) |
            (data&0x80 ? 0x10000000 : 0x80000000);
        uint32_t pubits=
            (data&0x01 ? 0 : 0x01) |
            (data&0x02 ? 0 : 0x02) |
            (data&0x04 ? 0 : 0x04) |
            (data&0x08 ? 0 : 0x08) |
            (data&0x10 ? 0 : 0x10) |
            (data&0x20 ? 0 : 0x20) |
            (data&0x40 ? 0 : 0x40) |
            (data&0x80 ? 0 : 0x80);

        KEYSEG_DIR=dirbits;
        GPIOC->OUTDR=pubits;

        if (TRACE>0) printf("W PADD val=%02x KEYSEG_DIR=%08lx GPIOC_OUTDR=%08lx\n", data,dirbits,pubits);

        return;
    }

    if (address==PBDD){
        // PD0 
        // PD1 
        // PD2 --> PB0 TTY Out 
        // PD3 --> PB1 Display/Key Row mux LSB  
        // PD4 --> PB2 Display/Key Row mux 
        // PD5 --> PB3 Display/Key Row mux 
        // PD6 --> PB4 Display/Key Row mux MSB 
        // PD7 
        data=0xff&(data<<2);
        uint32_t b=
            (data&0x01 ?        0x1 :        0x8) |
            (data&0x02 ?       0x10 :       0x80) |
            (data&0x04 ?      0x100 :      0x800) |
            (data&0x08 ?     0x1000 :     0x8000) |
            (data&0x10 ?    0x10000 :    0x80000) |
            (data&0x20 ?   0x100000 :   0x800000) |
            (data&0x40 ?  0x1000000 :  0x8000000) |
            (data&0x80 ? 0x10000000 : 0x80000000);
        MUX_DIR=b;
        if (TRACE>0) printf("W PBDD val=%02x MUX_DIR=%08lx\n", data,b); 
        return;
        }
    if (address>=0x1780 && address<0x1800) {
        if (TRACE>0) printf("W SCRATCH %04x val=%02x\n",address,data);
        RAM_1780[address&0x7F]=data;
        return;
    }
    printf("EY! W %04x val=%02x\n",address,data);
}

int main() {
	SystemInit();

	// Enable GPIOs
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD;

    if (TRACE>0) printf("Start\n");
    reset6502();

#ifdef INITVECTORS
    write6502(0x17fa,0x00);
    write6502(0x17fb,0x1C);
    write6502(0x17fe,0x00);
    write6502(0x17ff,0x1C);
#endif

#ifdef PC200
    RAM[0xFA]=0x00;
    RAM[0xFB]=0x02;
#endif



for (int i=0; i<DecimalTest_length; i++) RAM[0x200+i]=DecimalTest[i];

    for (;;) {
        exec6502(1);
        pc=pc&0xffff;
        x=x&0xff;
        y=y&0xff;

        if (TRACE>0) {
            if (pc>=0x302 && pc<=0x321) {
                printf("PC=%04x a=%02x x=%02x y=%02x ",pc,a,x,y);
                printStatus();
                printf("\n");
            }
        }
        // poll_input();
    }
 
}

void handle_debug_input( int numbytes, uint8_t * xdata ) { 

    if (numbytes>0) {
        if (*xdata==27) {
            write6502(SAD , 0b00000000);
            for(;;);
        }
    }
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
        (status_SIGN)?'N':'n',
        (status_OVERFLOW)?'O':'o',
        (status_DECIMAL)?'D':'d',
        (status_INTERRUPT)?'I':'i',
        (status_ZERO)?'Z':'z',
        (status_CARRY)?'C':'c'
    );
}

