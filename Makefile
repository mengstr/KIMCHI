TARGET:=kimchi

CC		=riscv64-unknown-elf-gcc
SIZE	=riscv64-unknown-elf-size
OBJDUMP	=riscv64-unknown-elf-objdump
OBJCOPY=riscv64-unknown-elf-objcopy

# PC SP ACC X Y STATUS
# -ffixed-s11 \
# -ffixed-s10 \
# -ffixed-s9 \
# -ffixed-s8 \
# -ffixed-s7 \
# -ffixed-s6 \

CFLAGS+= \
	-g -Os \
	-flto -ffunction-sections \
	-Wall -Wshadow \
	-march=rv32ec \
	-mabi=ilp32e \
	-nostdlib \
	-static-libgcc \
	-I/usr/include/newlib \
	-Ich32v003fun \
	-I. 

LINKER_SCRIPT?=ch32v003fun/ch32v003fun.ld

LDFLAGS+=-T $(LINKER_SCRIPT) -Wl,--gc-sections -Lch32v003fun -lgcc

ROMFILE = kimrom
CFILES	= $(wildcard *.c) 
SFILES  = $(wildcard *.s)

SOBJS   =   $(SFILES:.s=.o)
COBJS   =   $(CFILES:.c=.o)
OBJS	=   $(SOBJS) $(COBJS) ch32v003fun/ch32v003fun.o

ENFORCESIZE = @(FILESIZE=`stat -f '%z' $1` ; \
		if [ $$FILESIZE -ne $2 ] ; then \
			echo "ERROR: File $1 expected to be $2 bytes, is $$FILESIZE" ; \
			exit 1; \
	fi )

build : ch32v003fun $(ROMFILE).h $(TARGET).bin
		@$(MAKE) -C TFBOK

%.o: %.c
	@echo Compiling $<
	@$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET).elf: $(OBJS)
	@echo Linking $^
	@$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

$(TARGET).bin : $(TARGET).elf
	@$(SIZE) $^
	@echo Creating info files
	@$(OBJDUMP) -S $^ > $(TARGET).lst
	@$(OBJDUMP) -t $^ > $(TARGET).map
	@$(OBJCOPY) -O binary $< $(TARGET).bin
	@$(OBJCOPY) -O ihex $< $(TARGET).hex

$(SOBJS) : $(SFILES)
	 $(AS) $(ASFLAGS) $< -o $@

$(ROMFILE).h: $(ROMFILE).bin
	@echo Converting 6530-003 ROM Monitor to .h
	$(call ENFORCESIZE,$(ROMFILE).bin,1024)
	@srec_cat $(ROMFILE).bin -binary -offset 0x1800 -o $(ROMFILE).h -C-array ROM6530

$(ROMFILE).bin: $(ROMFILE).o65
	@echo Linking 6530-003 ROM Monitor
	@ld65 -t none -vm -m $(ROMFILE).map -o $(ROMFILE).bin $(ROMFILE).o65

$(ROMFILE).o65: $(ROMFILE).a65
	@echo Assembling 6530-003 ROM Monitor
	@ca65 -g -o $(ROMFILE).o65 -l $(ROMFILE).lst --feature labels_without_colons $(ROMFILE).a65

ch32v003fun:
	@echo Getting fresh copy of ch32v003fun from github
	@rm -rf ch32v003fun tmpfun
	@git clone --depth 1 https://github.com/cnlohr/ch32v003fun.git tmpfun
	@cp tmpfun/misc/libgcc.a tmpfun/ch32v003fun
	@mv tmpfun/ch32v003fun/ . 
	@rm -rf tmpfun
	@$(CC) $(CFLAGS) -c -o ch32v003fun/ch32v003fun.o ch32v003fun/ch32v003fun.c

.PHONY: flash clean distclean

flash: $(TARGET).bin
	minichlink -D -w $< flash -b -T

clean:
	@$(RM) $(OBJS) $(TARGET).elf $(TARGET).bin $(TARGET).hex $(TARGET).lst $(TARGET).map $(TARGET).hex
	@$(RM) $(ROMFILE).o65 $(ROMFILE).lst $(ROMFILE).map $(ROMFILE).bin $(ROMFILE).ptp $(ROMFILE).h65 $(ROMFILE).h
	@$(MAKE) -C TFBOK clean

distclean: clean
	@$(RM) -rf ch32v003fun/
