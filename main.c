#include "ch32v003fun/ch32v003fun.h" 
#include <stdio.h>

const int TRACE=1;

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
#define PBDD    0x1743  // 6530 B DATA DIRECTION


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

uint8_t RAM[1024];
uint8_t RAM_1780[128];
uint8_t sad,padd,sbd,pbdd;

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
        if (TRACE) printf("Read from SAD, value %02x\n", sad); 
        return sad;
    }
    if (address==SBD) {
        if (TRACE) printf("Read from SBD, value %02x\n", sbd); 
        return sbd;
    }
    if (address==PADD){
        if (TRACE) printf("Read from PADD, value %02x\n", padd);
        return padd;
    }
    if (address==PBDD){
        if (TRACE) printf("Read from PBDD, value %02x\n", pbdd);
        return pbdd;
    }
    if (address>=0x1780 && address<0x1800) {
        if (TRACE) printf("Read from SCRATCH %04x, value %02x\n",address,RAM_1780[address&0x7F]);
        return RAM_1780[address&0x7F];
    }
    printf("ERROR: Read from %04x, value ?\n",address);
    return 0;
}

void write6502(uint_fast16_t address, uint_fast8_t value) {
    value=value&0xFF;
    address=address&0xFFFF;
    if (address<1024) {
        if (TRACE) printf("Write to RAM %04x, value %02x\n",address,value);
        RAM[address]=value;
        return;
    }
    if (address==SAD) {
        if (TRACE) printf("Write to SAD, value %02x\n", value); 
        return;
        }
    if (address==SBD) {
        if (TRACE) printf("Write to SBD, value %02x\n", value); 
        return;
        }
    if (address==PADD){
        if (TRACE) printf("Write to PADD, value %02x\n", value); 
        return;
        }
    if (address==PBDD){
        if (TRACE) printf("Write to PBDD, value %02x\n", value); 
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

void DisplayMode(void) {
	// GPIO C7 Input with pullup, C0 C1 C2 C3 C4 C5 C6 Output Push-Pull
	GPIOC->CFGLR = (
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0) |
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*1) |
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*2) |
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*3) |
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4) |
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*5) |
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*6) |
		GPIO_CNF_IN_PUPD<<(4*7)
	);
    GPIOC->OUTDR = 0x80;

}

void KeypadMode(void) {
	// GPIO C0 C1 C2 C3 C4 C5 C6 C7 Input with pullup, Output Push-Pull
	GPIOC->CFGLR = (
		GPIO_CNF_IN_PUPD<<(4*0) |
		GPIO_CNF_IN_PUPD<<(4*1) |
		GPIO_CNF_IN_PUPD<<(4*2) |
		GPIO_CNF_IN_PUPD<<(4*3) |
		GPIO_CNF_IN_PUPD<<(4*4) |
		GPIO_CNF_IN_PUPD<<(4*5) |
		GPIO_CNF_IN_PUPD<<(4*6) |
		GPIO_CNF_IN_PUPD<<(4*7) 
	);
    GPIOC->OUTDR = 0xFF;
}


int main() {
	SystemInit();

	// Enable GPIOs
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD;

    // PA1 --> KEY RS (reset 6502)
    // PA2 --> KEY ST (nmi 6502)
    //
	// GPIO A1 A2 Input with pulldown
	GPIOA->CFGLR = (
		GPIO_CNF_IN_PUPD<<(4*1) |
		GPIO_CNF_IN_PUPD<<(4*2)
	);

    // PC0 --> PA0 Key Column for 6 D PC
    // PC1 --> PA1 Key Column for 5 C GO
    // PC2 --> PA2 Key Column for 4 B +
    // PC3 --> PA3 Key Column for 3 A DA
    // PC4 --> PA4 Key Column for 2 9 AD
    // PC5 --> PA5 Key Column for 1 8 F 
    // PC6 --> PA6 Key Column for 0 7 E 
    // PC7 --> PA7 TTY In 
    //
    DisplayMode();

    // PD0 
    // PD1 
    // PD2 --> PB0 TTY Out 
    // PD3 --> PB1 Display/Key Row mux LSB  
    // PD4 --> PB2 Display/Key Row mux 
    // PD5 --> PB3 Display/Key Row mux 
    // PD6 --> PB4 Display/Key Row mux MSB 
    // PD7 --> SST
	// GPIO D0 D1 D7 Input with pulldown D2 D3 D4 D5 D5 Output Push-Pull
	GPIOD->CFGLR = (
		GPIO_CNF_IN_PUPD<<(4*0) |
		GPIO_CNF_IN_PUPD<<(4*1) |
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP )<<(4*2) |
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP )<<(4*3) |
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP )<<(4*4) |
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP )<<(4*5) |
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP )<<(4*6) |
		GPIO_CNF_IN_PUPD<<(7*2) 
	);

    // for (;;) {
    //     DisplayMode();

    //     for (int i=4; i<10; i++) {
    //         GPIOD->BSHR=(1<<(16+3))|(1<<(16+4))|(1<<(16+5))|(1<<(16+6));    
    //         Delay_Ms(3);
    //         GPIOD->BSHR=i<<3;    
    //         GPIOC->BSHR=(1<<(16+0))|(1<<(16+1))|(1<<(16+2))|(1<<(16+3))|(1<<(16+4))|(1<<(16+5))|(1<<(16+6))|(1<<(16+7));    
    //         // GPIOC->BSHR=(1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<6);    
    //         if (i==4) GPIOC->BSHR=(1<<0)|(1<<3)|(1<<4)|(1<<5);        //C
    //         if (i==5) GPIOC->BSHR=(1<<2)|(1<<4)|(1<<5)|(1<<6);        //h   
    //         if (i==6) GPIOC->BSHR=(1<<4);                             //i  
    //         if (i==7) GPIOC->BSHR=(1<<0)|(1<<1)|(1<<2)|(1<<4)|(1<<5);      //M     
    //         if (i==8) GPIOC->BSHR=(1<<6);                           // -
    //         if (i==9) GPIOC->BSHR=(1<<1)|(1<<2);                    // 1    
    //         Delay_Ms(1);
    //     }
    // }

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


    reset6502();

    if (TRACE) printf("Start\n");
    printf("%ld\n",exec6502(1000000));
    for (;;);
}





