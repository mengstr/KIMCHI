#include "ch32v003fun/ch32v003fun.h" 
#include <stdio.h>

void dump(uint32_t addr, uint32_t pages) {
    uint32_t *p=(uint32_t *)addr;
    for (int i=0; i<pages*2; i++) {
        printf(" %08lx:",(uint32_t)p);
        for (int j=0; j<8; j++) {
            printf(" %08lx",*p++);
        }
        printf("\n");
    }
}


 __attribute__ ((aligned (64))) const uint8_t buf[8192]="HELLO WORLD";

int flash() {
	int start;
	int stop;
	int testok = 1;

	SystemInit();

	Delay_Ms(100);

	printf( "Starting\n" );

	// Unkock flash - be aware you need extra stuff for the bootloader.
	FLASH->KEYR = 0x45670123;
	FLASH->KEYR = 0xCDEF89AB;

	// For option bytes.
//	FLASH->OBKEYR = 0x45670123;
//	FLASH->OBKEYR = 0xCDEF89AB;

	// For unlocking programming, in general.
	FLASH->MODEKEYR = 0x45670123;
	FLASH->MODEKEYR = 0xCDEF89AB;

	// printf( "FLASH->CTLR = %08lx\n", FLASH->CTLR );
	// if( FLASH->CTLR & 0x8080 ) 
	// {
	// 	printf( "Flash still locked\n" );
    //     // dummy();
	// 	while(1);
	// }

	uint32_t * ptr = (uint32_t*)0x08003700;

    // dump(0x08003600);
    // dump(0x08003700);
    // dump(0x08003800);

	// printf( "FLASH->CTLR = %08lx\n", FLASH->CTLR );

	//Erase Page
	FLASH->CTLR = CR_PAGE_ER;
	FLASH->ADDR = (intptr_t)ptr;
	FLASH->CTLR = CR_STRT_Set | CR_PAGE_ER;
	start = SysTick->CNT;
	// while( FLASH->STATR & FLASH_STATR_BSY );  // Takes about 3ms. 2672
	Delay_Ms(1000);
	stop = SysTick->CNT;
	printf( "%d cycles for page erase\n", stop - start );

    // dump(0x08003600);
    // dump(0x08003700);
    // dump(0x08003800);

	// printf( "Memory at %p: %08lx %08lx\n", ptr, ptr[0], ptr[1] );

	if( ptr[0] != 0xffffffff )
	{
		printf( "WARNING/FAILURE: Flash general erasure failed\n" );
		testok = 0;
	}

	// Clear buffer and prep for flashing.
	FLASH->CTLR = CR_PAGE_PG;  // synonym of FTPG.
	FLASH->CTLR = CR_BUF_RST | CR_PAGE_PG;
	FLASH->ADDR = (intptr_t)ptr;  // This can actually happen about anywhere toward the end here.

	// Note: It takes about 6 clock cycles for this to finish.
	start = SysTick->CNT;
	while( FLASH->STATR & FLASH_STATR_BSY );  // No real need for this.
	stop = SysTick->CNT;
	printf( "%d cycles for buffer reset\n", stop - start );


	int i;
	start = SysTick->CNT;
	for( i = 0; i < 16; i++ )
	{
		ptr[i] = 0xabcd1234 + i; //Write to the memory
		FLASH->CTLR = CR_PAGE_PG | FLASH_CTLR_BUF_LOAD; // Load the buffer.
		// while( FLASH->STATR & FLASH_STATR_BSY );  // Only needed if running from RAM.
	}
	stop = SysTick->CNT;
	printf( "%d cycles for loading data in\n", stop - start );

	// Actually write the flash out. (Takes about 3ms)
	FLASH->CTLR = CR_PAGE_PG|CR_STRT_Set;

	start = SysTick->CNT;
	while( FLASH->STATR & FLASH_STATR_BSY );
	stop = SysTick->CNT;
	printf( "%d cycles for page write\n",stop - start );

	// printf( "FLASH->STATR = %08lx\n", FLASH->STATR );

	// printf( "Memory at: %08lx: %08lx %08lx\n", (uint32_t)ptr, ptr[0], ptr[1] );


	if( ptr[0] != 0xabcd1234 )
	{
		printf( "WARNING/FAILURE: Flash general erasure failed\n" );
		testok = 0;
	}

    dump((uint32_t)buf,2);
    for (int i=0; i<sizeof(buf); i+=256) printf( "%d ",buf[i]);

    	printf( "\nTest results: %s %d\n", testok?"PASS":"FAIL",buf[i]);
	while(1);
}
