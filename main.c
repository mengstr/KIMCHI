#include "ch32v003fun/ch32v003fun.h" 
#include <stdio.h>

const int TRACE=0;
const int GTRACE=0;

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
// PA1  
// PA2  
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
// PD7 
//

// http://www.zimmers.net/anonftp/pub/cbm/src/kim-1/6530-002-003.PDF
// http://retro.hansotten.nl/6502-sbc/kim-1-manuals-and-software/
// http://www.erich-foltyn.eu/Technique/KIM1-Circuit-Diagram.html

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


extern uint_fast16_t pc; 
extern uint_fast8_t sp, a, x, y, status; 


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
uint_fast8_t read6502(uint_fast16_t address);
void write6502(uint_fast16_t address, uint_fast8_t value);
extern void setPC(uint_fast16_t address);

extern const uint8_t ROM6530[];

#define MUX         GPIOD->BSHR
#define KEYSEG      GPIOC->BSHR
#define MUX_DIR     GPIOD->CFGLR
#define KEYSEG_DIR  GPIOC->CFGLR

uint8_t RAM[1024];
uint8_t RAM_1780[128];
uint8_t sad=0xff,padd=0xff,sbd=0xff,pbdd=0xff;

uint_fast8_t read6502(uint_fast16_t address) {
    address=address&0xFFFF;
    if (address<1024) {
        if (TRACE) printf("Read from RAM %04x, value is %02x\n",address,RAM[address]);
        return RAM[address];
    }
    if (address>=0x1C00) {
        if (TRACE) printf("Read from ROM %05x, value is %02x\n",address,ROM6530[address&0x3FF]);
        return ROM6530[address&0x3FF];
    }
    if (address==SAD) {
        if (GTRACE) printf("Read from SAD, value %02x\n", sad); 
        return sad;
    }
    if (address==SBD) {
        if (GTRACE) printf("Read from SBD, value %02x\n", sbd); 
        return sbd;
    }
    if (address==PADD){
        if (GTRACE) printf("Read from PADD, value %02x\n", padd);
        return padd;
    }
    if (address==PBDD){
        if (GTRACE) printf("Read from PBDD, value %02x\n", pbdd);
        return pbdd;
    }
    if (address>=0x1780 && address<0x1800) {
        if (TRACE) printf("Read from SCRATCH %04x, value %02x\n",address,RAM_1780[address&0x7F]);
        return RAM_1780[address&0x7F];
    }
    printf("ERROR: Read from %04x, value ?\n",address);
    return 0;
}

// out0=.......1  in0=.......8
// out1=......10  in1=......80
// out2=.....100  in1=.....800
// out3=....1000  in1=....8000
// out4=...10000  in1=...80000
// out5=..100000  in1=..800000
// out6=.1000000  in6=.8000000
// out7=10000000  in7=80000000

// reset0=00010000 set0=00000001
// reset1=00020000 set1=00000002
// reset6=00400000 set6=00000040
// reset7=00800000 set7=00000080

void write6502(uint_fast16_t address, uint_fast8_t value) {
    value=value&0xFF;
    address=address&0xFFFF;
    if (address<1024) {
        if (TRACE) printf("Write to RAM %04x, value %02x\n",address,value);
        RAM[address]=value;
        return;
    }
    if (address==SAD) {
        uint8_t valueInv=value^0xFF;
        uint32_t b=(valueInv<<16)|value;
        KEYSEG=b;
        if (TRACE) printf("Write to SAD, value %02x KEYSEG=%08lx\n", value,b); 
        if (GTRACE) printf("Write to SAD, value %02x KEYSEG=%08lx\n", value,b); 
        return;
        }
    if (address==SBD) {
        value=0xff&(value<<2);
        uint8_t valueInv=value^0xFF;
        uint32_t b=(valueInv<<16)|value<<0;
        uint32_t bb=b;
        // b=0x00df0020;
        MUX=b;
        if (TRACE) printf("Write to SBD, value %02x MUX=%08lx was=%08lx\n", value,b,bb); 
        if (GTRACE) printf("Write to SBD, value %02x MUX=%08lx was=%08lx\n", value,b,bb); 
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
        uint32_t b=
            (value&0x01 ?        0x1 :        0x8) |
            (value&0x02 ?       0x10 :       0x80) |
            (value&0x04 ?      0x100 :      0x800) |
            (value&0x08 ?     0x1000 :     0x8000) |
            (value&0x10 ?    0x10000 :    0x80000) |
            (value&0x20 ?   0x100000 :   0x800000) |
            (value&0x40 ?  0x1000000 :  0x8000000) |
            (value&0x80 ? 0x10000000 : 0x80000000);
            b=0x81111111;
KEYSEG_DIR=b;
        if (TRACE) printf("Write to PADD, value %02x KEYSEG_DIR=%08lx\n", value,b);
        if (GTRACE) printf("Write to PADD, value %02x KEYSEG_DIR=%08lx\n", value,b);
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
// b=0x81111188;
        MUX_DIR=b;
        if (TRACE) printf("Write to PBDD, value %02x MUX_DIR=%08lx\n", value,b); 
        if (GTRACE) printf("Write to PBDD, value %02x MUX_DIR=%08lx\n", value,b); 
        return;
        }
    if (address>=0x1780 && address<0x1800) {
        if (TRACE) printf("Write to SCRATCH %04x, value %02x\n",address,value);
        RAM_1780[address&0x7F]=value;
        return;
    }
    printf("ERROR: Write to %04x, value %02x\n",address,value);
}

// 0.27MIPS

// void DisplayMode(void) {
// 	// GPIO C7 Input with pullup, C0 C1 C2 C3 C4 C5 C6 Output Push-Pull
// 	uint32_t cc= (
// 		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0) |
// 		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*1) |
// 		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*2) |
// 		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*3) |
// 		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4) |
// 		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*5) |
// 		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*6) |
// 		GPIO_CNF_IN_PUPD<<(4*7)
// 	);
//     GPIOC->CFGLR=cc;
//     printf("Displaymode GPIOC->CFGLR=%08lx\n",cc);

//     GPIOC->OUTDR = 0x80;
// }

// void KeypadMode(void) {
// 	// GPIO C0 C1 C2 C3 C4 C5 C6 C7 Input with pullup, Output Push-Pull
// 	GPIOC->CFGLR = (
// 		GPIO_CNF_IN_PUPD<<(4*0) |
// 		GPIO_CNF_IN_PUPD<<(4*1) |
// 		GPIO_CNF_IN_PUPD<<(4*2) |
// 		GPIO_CNF_IN_PUPD<<(4*3) |
// 		GPIO_CNF_IN_PUPD<<(4*4) |
// 		GPIO_CNF_IN_PUPD<<(4*5) |
// 		GPIO_CNF_IN_PUPD<<(4*6) |
// 		GPIO_CNF_IN_PUPD<<(4*7) 
// 	);
//     GPIOC->OUTDR = 0xFF;
// }


// volatile void Minus1(void) {
//     printf("Minus1(start)\n");
//     Delay_Ms(1000);
// 	uint32_t cc= (
// 		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0) |
// 		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*1) |
// 		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*2) |
// 		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*3) |
// 		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4) |
// 		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*5) |
// 		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*6) |
// 		GPIO_CNF_IN_PUPD<<(4*7)
// 	);
//     GPIOC->CFGLR=cc;
//     printf("GPIOC->CFGLR=%08lx\n",cc);


//     uint32_t dd = (
//         GPIO_CNF_IN_PUPD<<(4*0) |
//         GPIO_CNF_IN_PUPD<<(4*1) |
//         (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP )<<(4*2) |
//         (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP )<<(4*3) |
//         (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP )<<(4*4) |
//         (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP )<<(4*5) |
//         (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP )<<(4*6) |
//         GPIO_CNF_IN_PUPD<<(4*7) 
// 	);
//     GPIOD->CFGLR=dd;
//     printf("GPIOD->CFGLR=%08lx\n",dd);

//     uint32_t ee=4<<3;
//     printf("GPIOD->BSHR=%08lx\n",ee);
//     GPIOD->BSHR=ee;

//     uint32_t ff=(1<<(16+0))|(1<<(16+1))|(1<<(16+2))|(1<<(16+3))|(1<<(16+4))|(1<<(16+5))|(1<<(16+6))|(1<<(16+7));    
//     printf("GPIOC->BSHR=%08lx\n",ff);
//     GPIOC->BSHR=ff;
//     // GPIOC->BSHR=(1<<0)|(1<<1)|(1<<2)|(1<<4)|(1<<5);      //M     
//     Delay_Ms(1000);
//     printf("Minus1(end)\n");
// }

int main() {
	SystemInit();
    printf("Start\n");

	// Enable GPIOs
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD;

    // // PA1 --> KEY RS (reset 6502)
    // // PA2 --> KEY ST (nmi 6502)
    // //
	// // GPIO A1 A2 Input with pulldown
	// GPIOA->CFGLR = (
	// 	GPIO_CNF_IN_PUPD<<(4*1) |
	// 	GPIO_CNF_IN_PUPD<<(4*2)
	// );

    // // PC0 --> PA0 Key Column for 6 D PC
    // // PC1 --> PA1 Key Column for 5 C GO
    // // PC2 --> PA2 Key Column for 4 B +
    // // PC3 --> PA3 Key Column for 3 A DA
    // // PC4 --> PA4 Key Column for 2 9 AD
    // // PC5 --> PA5 Key Column for 1 8 F 
    // // PC6 --> PA6 Key Column for 0 7 E 
    // // PC7 --> PA7 TTY In 
    // //
    // DisplayMode();

    // // PD0 
    // // PD1 
    // // PD2 --> PB0 TTY Out 
    // // PD3 --> PB1 Display/Key Row mux LSB  
    // // PD4 --> PB2 Display/Key Row mux 
    // // PD5 --> PB3 Display/Key Row mux 
    // // PD6 --> PB4 Display/Key Row mux MSB 
    // // PD7 --> SST
	// // GPIO D0 D1 D7 Input with pulldown D2 D3 D4 D5 D5 Output Push-Pull

//-------------------------------------------

    // uint32_t ee=4<<3;
    // printf("GPIOD->BSHR=%08lx\n",ee);
    // GPIOD->BSHR=ee;
    // uint32_t ff=(1<<(16+0))|(1<<(16+1))|(1<<(16+2))|(1<<(16+3))|(1<<(16+4))|(1<<(16+5))|(1<<(16+6))|(1<<(16+7));    
    // printf("xGPIOC->BSHR=%08lx\n",ff);
    // GPIOC->BSHR=ff;
    // GPIOC->BSHR=(1<<0)|(1<<1)|(1<<2)|(1<<4)|(1<<5);      //M     
    // for(;;);
//-------------------------------------------

    // printf("Loop Start\n");

	// uint32_t dd = (
	// 	GPIO_CNF_IN_PUPD<<(4*0) |
	// 	GPIO_CNF_IN_PUPD<<(4*1) |
	// 	(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP )<<(4*2) |
	// 	(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP )<<(4*3) |
	// 	(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP )<<(4*4) |
	// 	(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP )<<(4*5) |
	// 	(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP )<<(4*6) |
	// 	GPIO_CNF_IN_PUPD<<(4*7) 
	// );
    // MUX_DIR=dd;
    // printf("MUX_DIR=%08lx\n",dd);


	// uint32_t cc= (
	// 	(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0) |
	// 	(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*1) |
	// 	(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*2) |
	// 	(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*3) |
	// 	(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4) |
	// 	(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*5) |
	// 	(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*6) |
	// 	GPIO_CNF_IN_PUPD<<(4*7)
	// );
    // KEYSEG_DIR=cc;
    // printf("KEYSEG_DIR=%08lx\n",cc);


    // // for (;;) {

    //     int i=4;
    //     // for (int i=4; i<10; i++) {

    //         uint32_t qq=(1<<(16+3))|(1<<(16+4))|(1<<(16+5))|(1<<(16+6)) | (i<<3);
    //         qq=0x20;
    //         printf("MUX=%08lx\n",qq);
    //         MUX=qq;
    //         Delay_Ms(100);
    //         // GPIOD->BSHR=i<<3;    

    //         // uint32_t ww=(1<<(16+0))|(1<<(16+1))|(1<<(16+2))|(1<<(16+3))|(1<<(16+4))|(1<<(16+5))|(1<<(16+6))|(1<<(16+7));    
    //         uint32_t ww=(1<<(0))|(1<<(16+1))|(1<<(16+2))|(1<<(16+3))|(1<<(16+4))|(1<<(16+5))|(1<<(16+6))|(1<<(16+7));    
    //         printf("KEYSEG=%08lx\n",ww);
    //         KEYSEG=ww;

    //         // // GPIOC->BSHR=(1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<6);    
    //         // uint32_t eee=(1<<0)|(1<<3)|(1<<4)|(1<<5);        //C
    //         // printf("GPIOC->BSHR=%08lx\n",eee);
    //         // GPIOC->BSHR=eee;
    //         // if (i==5) GPIOC->BSHR=(1<<2)|(1<<4)|(1<<5)|(1<<6);        //h   
    //         // if (i==6) GPIOC->BSHR=(1<<4);                             //i  
    //         // if (i==7) GPIOC->BSHR=(1<<0)|(1<<1)|(1<<2)|(1<<4)|(1<<5);      //M     
    //         // if (i==8) GPIOC->BSHR=(1<<6);                           // -
    //         // if (i==9) GPIOC->BSHR=(1<<1)|(1<<2);                    // 1    
    //         Delay_Ms(500);
    //     // }
    // // }
    // Delay_Ms(1000);
    // printf("Loop End\n");

    // for (;;);

    // Minus1();

    // for (;;) {
    //     DisplayMode();
    //     GPIOD->BSHR=(1<<(16+3))|(1<<(16+4))|(1<<(16+5))|(1<<(16+6));    
    //     Delay_Ms(100);

    //     KeypadMode();
    //     Delay_Ms(1);

    //     GPIOD->BSHR=(1<<(16+3))|(1<<(16+4))|(1<<(16+5))|(1<<(16+6));    
    //     Delay_Ms(1);
    //     printf("%02x ",(~(GPIOC->INDR))&0xff );

    //     GPIOD->BSHR=(1<<(16+3))|(1<<(16+4))|(1<<(16+5))|(1<<(16+6));    
    //     GPIOD->BSHR =(1<<3);
    //     Delay_Ms(1);
    //     printf("%02x ",(~(GPIOC->INDR))&0xff );

    //     GPIOD->BSHR=(1<<(16+3))|(1<<(16+4))|(1<<(16+5))|(1<<(16+6));    
    //     GPIOD->BSHR =(1<<4);
    //     Delay_Ms(1);
    //     printf("%02x ",(~(GPIOC->INDR))&0xff );

    //     GPIOD->BSHR=(1<<(16+3))|(1<<(16+4))|(1<<(16+5))|(1<<(16+6));    
    //     GPIOD->BSHR =(1<<3);
    //     GPIOD->BSHR =(1<<4);
    //     Delay_Ms(1);
    //     printf("%02x\n",(~(GPIOC->INDR))&0xff );

    // }


// Displaymode GPIOC->CFGLR=81111111
// Displaymode GPIOD->CFGLR=81111188
// GPIOD->BSHR=00000020                 
// GPIOC->BSHR=00ff0000

// Write to PBDD, value 7c GPIOD->CFGLR=81111188
// Write to SBD, value 40 GPIOD->BSHR=00bf0040
// Write to PADD, value 7f GPIOC->CFGLR=81111111
// Write to SAD, value 00 GPIOC->BSHR=00ff0000

// 6530-002 PB0 TTY Out
// 6530-002 PB1 Display/Key Row mux LSB 
// 6530-002 PB2 Display/Key Row mux
// 6530-002 PB3 Display/Key Row mux
// 6530-002 PB4 Display/Key Row mux MSB
// 6530-002 PB5
// 6530-002 PB6 N/A
// 6530-002 PB7 



// 6530-002 PA0 Key Column for 6 D PC
// 6530-002 PA1 Key Column for 5 C GO
// 6530-002 PA2 Key Column for 4 B +
// 6530-002 PA3 Key Column for 3 A DA
// 6530-002 PA4 Key Column for 2 9 AD
// 6530-002 PA5 Key Column for 1 8 F
// 6530-002 PA6 Key Column for 0 7 E
// 6530-002 PA7 TTY In

// printf("Write6502 Start\n");
// write6502(PADD,0b01111111);
// write6502(PBDD, 0b00011111);
// write6502(SBD,0b00001000);
// write6502(SAD ,0b01110000);
// Delay_Ms(2500);
// printf("Write6502 End\n");


    reset6502();

RAM[0x90]=0x7e;
RAM[0x91]=0x7e;
RAM[0x92]=0x7e;
RAM[0x93]=0x7e;
RAM[0x94]=0x7e;
RAM[0x95]=0x7e;


RAM[0x200]=0x84; RAM[0x201]=0x7F;                   //LIGHT  STY YSAV    save register
RAM[0x202]=0xA0; RAM[0x203]=0x13;                   //       LDY #$13
RAM[0x204]=0xA2; RAM[0x205]=0x05;                   //       LDX #$5     6 digits to show
RAM[0x206]=0xA9; RAM[0x207]=0x7F;                   //       LDA #$7F
RAM[0x208]=0x8D; RAM[0x209]=0x41; RAM[0x20A]=0x17;  //       STA PADD    set directional reg
RAM[0x20B]=0xB5; RAM[0x20C]=0x90;                   //DIGIT  LDA WINDOW,X
RAM[0x20D]=0x8D; RAM[0x20E]=0x40; RAM[0x20F]=0x17;  //       STA SAD     character segments
RAM[0x210]=0x8C; RAM[0x211]=0x42; RAM[0x212]=0x17;  //       STY SBD     character ID    
RAM[0x213]=0xE6; RAM[0x214]=0x7B;                   //WAIT   INC PAUSE
RAM[0x215]=0xD0; RAM[0x216]=0xFC;                   //       BNE WAIT    wait loop
// RAM[0x215]=0xEA; RAM[0x216]=0xEA;                   //       nopnop
RAM[0x217]=0x88; RAM[0x218]=0x88;                   //       DEY DEY  
RAM[0x219]=0xCA;                                    //       DEX
RAM[0x21A]=0x10; RAM[0x21B]=0xEF;                   //       BPL DIGIT
RAM[0x21C]=0x00;                                    //       BRK



    if (TRACE) printf("Start\n");
    write6502(PBDD, 0b11111111);
    // pc=0x200;
    for (;;) {
        // if (pc==0x1e88) printf("INITS\n");
        // if (pc==0x1c8c) printf("INIT1\n");
        // if (pc==0x1F19) printf("SCAND\n");
        // if (pc==0x1F6A) printf("GETKEY\n");
        // if (pc==0x1F7A) printf("KEYIN\n");
        // if (pc==0x1C77) printf("TTYKB\n");
        // if (pc==0x1F48) printf("CONVD\n");
        exec6502(1);
        pc=pc&0xffff;
        x=x&0xff;
        y=y&0xff;
        // printf("PC=%04x A=%02x X=%02x Y=%02x\n",pc,a,x,y);
    }
    if (TRACE) printf("End\n");
 
}





