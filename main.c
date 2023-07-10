#include <stdio.h>
#include "ch32v003fun/ch32v003fun.h" 
#include "kimrom.h"

const int TRACE=0;
const int GTRACE=0;
uint8_t DEBUG = 0;

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

// http://www.zimmers.net/anonftp/pub/cbm/src/kim-1/6530-002-003.PDF
// http://retro.hansotten.nl/6502-sbc/kim-1-manuals-and-software/
// http://www.erich-foltyn.eu/Technique/KIM1-Circuit-Diagram.html
// https://github.com/maksimKorzh/KIM-1/tree/main/software/First_book_of_KIM_sources/Games

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


extern uint16_t pc; 
extern uint8_t sp, a, x, y, status; 


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
void write6502(uint16_t address, uint8_t value);
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

uint8_t read6502(uint16_t address) {
    address=address&0xFFFF;
    if (address<1024) {
        if ((TRACE>0)) printf("R RAM %04x val=%02x pc=%04x\n",address,RAM[address],pc);
        return RAM[address];
    }
    if (address>=0x1C00) {
        if (TRACE) printf("R ROM %05x val=%02x\n",address,ROM6530[address&0x3FF]);
        return ROM6530[address&0x3FF];
    }
    if (address==SAD) {
        sad=GPIOC->INDR|0x80;   // Always use KEYPAD
        if (GTRACE) printf("R SAD val=%02x\n", sad); 
        return sad;
    }
    if (address==SBD) {
        printf("EY! R SBD val=%02x\n", sbd); 
        return sbd;
    }
    if (address==PADD){
        if (GTRACE) printf("EY! R PADD val=%02x\n", padd);
        return padd;
    }
    if (address==PBDD){
        if (GTRACE) printf("EY! R PBDD val=%02x\n", pbdd);
        return pbdd;
    }
    if (address>=0x1780 && address<0x1800) {
        if (TRACE) printf("R SCRATCH %04x val=%02x\n",address,RAM_1780[address&0x7F]);
        return RAM_1780[address&0x7F];
    }
    printf("EY! R %04x\n",address);
    return 0;
}

void write6502(uint16_t address, uint8_t value) {
    value=value&0xFF;
    address=address&0xFFFF;

    if (address<1024) {
        if ((TRACE>0)) printf("W RAM %04x val=%02x pc=%04x\n",address,value,pc);
        RAM[address]=value;
        return;
    }
    if (address==SAD) {
        uint8_t valueInv=value^0xFF;
        uint32_t b=(valueInv<<16)|value;
        KEYSEG=b;
        if (TRACE) printf("W SAD val=%02x KEYSEG=%08lx\n", value,b); 
        if (GTRACE) printf("W SAD val=%02x KEYSEG=%08lx\n", value,b); 
        return;
        }
    if (address==SBD) {
        value=0xff&(value<<2);
        uint8_t valueInv=value^0xFF;
        uint32_t b=(valueInv<<16)|value<<0;
        MUX=b;
        if (TRACE) printf("W SBD val=%02x MUX=%08lx\n", value,b); 
        if (GTRACE) printf("W SBD val=%02x MUX=%08lx\n", value,b); 
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
            (value&0x01 ?        0x1 :        0x8) |
            (value&0x02 ?       0x10 :       0x80) |
            (value&0x04 ?      0x100 :      0x800) |
            (value&0x08 ?     0x1000 :     0x8000) |
            (value&0x10 ?    0x10000 :    0x80000) |
            (value&0x20 ?   0x100000 :   0x800000) |
            (value&0x40 ?  0x1000000 :  0x8000000) |
            (value&0x80 ? 0x10000000 : 0x80000000);
        uint32_t pubits=
            (value&0x01 ? 0 : 0x01) |
            (value&0x02 ? 0 : 0x02) |
            (value&0x04 ? 0 : 0x04) |
            (value&0x08 ? 0 : 0x08) |
            (value&0x10 ? 0 : 0x10) |
            (value&0x20 ? 0 : 0x20) |
            (value&0x40 ? 0 : 0x40) |
            (value&0x80 ? 0 : 0x80);

        KEYSEG_DIR=dirbits;
        GPIOC->OUTDR=pubits;

        if (TRACE) printf("W PADD val=%02x KEYSEG_DIR=%08lx GPIOC_OUTDR=%08lx\n", value,dirbits,pubits);
        if (GTRACE) printf("W PADD val=%02x KEYSEG_DIR=%08lx GPIOC_OUTDR=%08lx\n", value,dirbits,pubits);

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
        value=0xff&(value<<2);
        uint32_t b=
            (value&0x01 ?        0x1 :        0x8) |
            (value&0x02 ?       0x10 :       0x80) |
            (value&0x04 ?      0x100 :      0x800) |
            (value&0x08 ?     0x1000 :     0x8000) |
            (value&0x10 ?    0x10000 :    0x80000) |
            (value&0x20 ?   0x100000 :   0x800000) |
            (value&0x40 ?  0x1000000 :  0x8000000) |
            (value&0x80 ? 0x10000000 : 0x80000000);
        MUX_DIR=b;
        if (TRACE) printf("W PBDD val=%02x MUX_DIR=%08lx\n", value,b); 
        if (GTRACE) printf("W PBDD val=%02x MUX_DIR=%08lx\n", value,b); 
        return;
        }
    if (address>=0x1780 && address<0x1800) {
        if (TRACE) printf("W SCRATCH %04x val=%02x\n",address,value);
        RAM_1780[address&0x7F]=value;
        return;
    }
    printf("EY! W %04x val=%02x\n",address,value);
}

uint8_t data;

int main() {
	SystemInit();
    // for (int i=0; i<DecimalTest_length; i++) RAM[0x0200+i]=DecimalTest[i];

	// Enable GPIOs
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD;

    if (TRACE) printf("Start\n");
    reset6502();

    write6502(0x17fa,0x00);
    write6502(0x17fb,0x1C);
    write6502(0x17fe,0x00);
    write6502(0x17ff,0x1C);

    write6502(PADD, 0b01111111);
    write6502(PBDD, 0b00011111);
    write6502(SBD,  0b00001000);
    write6502(SAD , 0b01000000);
    Delay_Ms(500);

//    0200 D8       START CLD         clr dc mode
//    0201 A9 00          LDA #0      zero into A
//    0203 85 FB    STORE STA POINTH
//    0205 85 FA          STA POINTL
//    0207 85 F9          STA INH
//    0209 20 1F 1F       JSR SCANDS  light display
//    020C 20 6A 1F       JSR GETKEY  test keys
//    020F 4C 03 02       JMP STORE
// pc=0x200;
// RAM[0x200]=0xD8;
// RAM[0x201]=0xA9; RAM[0x202]=0x00;
// RAM[0x203]=0x85; RAM[0x204]=0xFB;
// RAM[0x205]=0x85; RAM[0x206]=0xFA;
// RAM[0x207]=0x85; RAM[0x208]=0xF9;
// RAM[0x209]=0x20; RAM[0x20A]=0x1F; RAM[0x20B]=0x1F;
// RAM[0x20C]=0x20; RAM[0x20D]=0x6A; RAM[0x20E]=0x1F;
// RAM[0x20F]=0x4C; RAM[0x210]=0x03; RAM[0x211]=0x02;

// RAM[0x10]=0xAA;
// RAM[0x11]=0x55;
// RAM[0x200]=0xA5; RAM[0x201]=0x10;
// RAM[0x202]=0x85; RAM[0x203]=0xFB;
// RAM[0x204]=0x85; RAM[0x205]=0xFA;
// RAM[0x206]=0x85; RAM[0x207]=0xF9;
// RAM[0x208]=0x00;

int p=0x200;
RAM[p++]=0xA2;RAM[p++]=0x0D;RAM[p++]=0xBD;RAM[p++]=0xCC;RAM[p++]=0x02;RAM[p++]=0x95;RAM[p++]=0xD5;RAM[p++]=0xCA;RAM[p++]=0x10;RAM[p++]=0xF8;RAM[p++]=0xA2;RAM[p++]=0x05;RAM[p++]=0xA0;RAM[p++]=0x01;RAM[p++]=0xF8;RAM[p++]=0x18;RAM[p++]=0xB5;RAM[p++]=0xD5;RAM[p++]=0x75;RAM[p++]=0xD7;RAM[p++]=0x95;RAM[p++]=0xD5;RAM[p++]=0xCA;RAM[p++]=0x88; 
RAM[p++]=0x10;RAM[p++]=0xF6;RAM[p++]=0xB5;RAM[p++]=0xD8;RAM[p++]=0x10;RAM[p++]=0x02;RAM[p++]=0xA9;RAM[p++]=0x99;RAM[p++]=0x75;RAM[p++]=0xD5;RAM[p++]=0x95;RAM[p++]=0xD5;RAM[p++]=0xCA;RAM[p++]=0x10;RAM[p++]=0xE5;RAM[p++]=0xA5;RAM[p++]=0xD5;RAM[p++]=0x10;RAM[p++]=0x0D;RAM[p++]=0xA9;RAM[p++]=0x00;RAM[p++]=0x85;RAM[p++]=0xE2;RAM[p++]=0xA2; 
RAM[p++]=0x02;RAM[p++]=0x95;RAM[p++]=0xD5;RAM[p++]=0x95;RAM[p++]=0xDB;RAM[p++]=0xCA;RAM[p++]=0x10;RAM[p++]=0xF9;RAM[p++]=0x38;RAM[p++]=0xA5;RAM[p++]=0xE0;RAM[p++]=0xE5;RAM[p++]=0xDD;RAM[p++]=0x85;RAM[p++]=0xE0;RAM[p++]=0xA2;RAM[p++]=0x01;RAM[p++]=0xB5;RAM[p++]=0xDE;RAM[p++]=0xE9;RAM[p++]=0x00;RAM[p++]=0x95;RAM[p++]=0xDE;RAM[p++]=0xCA; 
RAM[p++]=0x10;RAM[p++]=0xF7;RAM[p++]=0xB0;RAM[p++]=0x0C;RAM[p++]=0xA9;RAM[p++]=0x00;RAM[p++]=0xA2;RAM[p++]=0x03;RAM[p++]=0x95;RAM[p++]=0xDD;RAM[p++]=0xCA;RAM[p++]=0x10;RAM[p++]=0xFB;RAM[p++]=0x20;RAM[p++]=0xBD;RAM[p++]=0x02;RAM[p++]=0xA5;RAM[p++]=0xDE;RAM[p++]=0xA6;RAM[p++]=0xDF;RAM[p++]=0x09;RAM[p++]=0xF0;RAM[p++]=0xA4;RAM[p++]=0xE1; 
RAM[p++]=0xF0;RAM[p++]=0x20;RAM[p++]=0xF0;RAM[p++]=0x9C;RAM[p++]=0xF0;RAM[p++]=0xA4;RAM[p++]=0xA2;RAM[p++]=0xFE;RAM[p++]=0xA0;RAM[p++]=0x5A;RAM[p++]=0x18;RAM[p++]=0xA5;RAM[p++]=0xD9;RAM[p++]=0x69;RAM[p++]=0x05;RAM[p++]=0xA5;RAM[p++]=0xD8;RAM[p++]=0x69;RAM[p++]=0x00;RAM[p++]=0xB0;RAM[p++]=0x04;RAM[p++]=0xA2;RAM[p++]=0xAD;RAM[p++]=0xA0; 
RAM[p++]=0xDE;RAM[p++]=0x98;RAM[p++]=0xA4;RAM[p++]=0xE2;RAM[p++]=0xF0;RAM[p++]=0x04;RAM[p++]=0xA5;RAM[p++]=0xD5;RAM[p++]=0xA6;RAM[p++]=0xD6;RAM[p++]=0x85;RAM[p++]=0xFB;RAM[p++]=0x86;RAM[p++]=0xFA;RAM[p++]=0xA5;RAM[p++]=0xD9;RAM[p++]=0xA6;RAM[p++]=0xD8;RAM[p++]=0x10;RAM[p++]=0x05;RAM[p++]=0x38;RAM[p++]=0xA9;RAM[p++]=0x00;RAM[p++]=0xE5; 
RAM[p++]=0xD9;RAM[p++]=0x85;RAM[p++]=0xF9;RAM[p++]=0xA9;RAM[p++]=0x02;RAM[p++]=0x85;RAM[p++]=0xE3;RAM[p++]=0xD8;RAM[p++]=0x20;RAM[p++]=0x1F;RAM[p++]=0x1F;RAM[p++]=0x20;RAM[p++]=0x6A;RAM[p++]=0x1F;RAM[p++]=0xC9;RAM[p++]=0x13;RAM[p++]=0xF0;RAM[p++]=0xC0;RAM[p++]=0xB0;RAM[p++]=0x03;RAM[p++]=0x20;RAM[p++]=0xAD;RAM[p++]=0x02;RAM[p++]=0xC6; 
RAM[p++]=0xE3;RAM[p++]=0xD0;RAM[p++]=0xED;RAM[p++]=0xF0;RAM[p++]=0xB7;RAM[p++]=0xC9;RAM[p++]=0x0A;RAM[p++]=0x90;RAM[p++]=0x05;RAM[p++]=0x49;RAM[p++]=0x0F;RAM[p++]=0x85;RAM[p++]=0xE1;RAM[p++]=0x60;RAM[p++]=0xAA;RAM[p++]=0xA5;RAM[p++]=0xDD;RAM[p++]=0xF0;RAM[p++]=0xFA;RAM[p++]=0x86;RAM[p++]=0xDD;RAM[p++]=0xA5;RAM[p++]=0xDD;RAM[p++]=0x38; 
RAM[p++]=0xF8;RAM[p++]=0xE9;RAM[p++]=0x05;RAM[p++]=0x85;RAM[p++]=0xDC;RAM[p++]=0xA9;RAM[p++]=0x00;RAM[p++]=0xE9;RAM[p++]=0x00;RAM[p++]=0x85;RAM[p++]=0xDB;RAM[p++]=0x60;RAM[p++]=0x45;RAM[p++]=0x01;RAM[p++]=0x00;RAM[p++]=0x99;RAM[p++]=0x81;RAM[p++]=0x00;RAM[p++]=0x99;RAM[p++]=0x97;RAM[p++]=0x02;RAM[p++]=0x08;RAM[p++]=0x00;RAM[p++]=0x00; 
RAM[p++]=0x01;RAM[p++]=0x01;
//;00000A000A

    RAM[0xFA]=0x00;
    RAM[0xFB]=0x02;

    // RAM[0xFA]=0x97;
    // RAM[0xFB]=0x03;
    
    for (;;) {
        if (GTRACE) {
            if (pc==0x1c4f) printf("START\n");
            if (pc==0x1e88) printf("INITS\n");
            if (pc==0x1c8c) printf("INIT1\n");
            if (pc==0x1F19) printf("SCAND\n");
            if (pc==0x1F6A) printf("GETKEY\n");
            if (pc==0x1F7A) printf("KEYIN\n");
            if (pc==0x1C77) printf("TTYKB\n");
            if (pc==0x1F48) printf("CONVD\n");
        }
        exec6502(1);
        pc=pc&0xffff;
        x=x&0xff;
        y=y&0xff;
        // if (pc<1024) printf("PC=%04x A=%02x X=%02x Y=%02x N1=%02x N2=%02x DA=%02x AR=%02x\n",pc,a,x,y,RAM[0x211],RAM[0x212],RAM[0x215],RAM[0x213]);

        // if (pc<1024) printf("PC=%04x N1=%02x N2=%02x DA=%02x AR=%02x\n",pc,RAM[0x211],RAM[0x212],RAM[0x213],RAM[0x214]);

        // if (pc==0x302) {
        //     printf("N1H=%d N1L=%d N2H=%d,%d N2L=%d\n",RAM[0x0219],RAM[0x021a],RAM[0x021f],RAM[0x0220],RAM[0x021b]);
        // }
        // if (pc>=0x302 && pc<=0x321) {
        //     printf("PC=%04x a=%02x x=%02x y=%02x ",pc,a,x,y);
        //     printStatus();
        //     printf("\n");
        // }

        // if (pc==0x205) for(;;);
        // poll_input();
        // Delay_Us(20);
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




