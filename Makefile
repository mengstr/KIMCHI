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
	-Wall \
	-march=rv32ec \
	-mabi=ilp32e \
	-nostdlib \
	-static-libgcc \
	-I/usr/include/newlib \
	-Ich32v003fun \
	-I. 

LINKER_SCRIPT?=ch32v003fun/ch32v003fun.ld

LDFLAGS+=-T $(LINKER_SCRIPT) -Wl,--gc-sections -Lch32v003fun -lgcc

# ASFLAGS =   -f elf

ROMFILE = kimrom
CFILES	= $(wildcard *.c) $(wildcard ch32v003fun/*.c) $(ROMFILE).c
SFILES  = $(wildcard *.s)

echo CFILES = $(CFILES)

SOBJS   =   $(SFILES:.s=.o)
COBJS   =   $(CFILES:.c=.o)
OBJS	=   $(SOBJS) $(COBJS)

ENFORCESIZE = @(FILESIZE=`stat -f '%z' $1` ; \
		if [ $$FILESIZE -ne $2 ] ; then \
			echo "ERROR: File $1 expected to be $2 bytes, is $$FILESIZE" ; \
			exit 1; \
	fi )

build :  $(TARGET).bin

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

$(ROMFILE).c: $(ROMFILE).bin
	@echo Converting 6530-003 ROM Monitor to C source
	$(call ENFORCESIZE,$(ROMFILE).bin,1024)
	@srec_cat $(ROMFILE).bin -binary -offset 0x1800 -o $(ROMFILE).c -C-array ROM6530

$(ROMFILE).bin: $(ROMFILE).o65
	@echo Linking 6530-003 ROM Monitor
	@ld65 -t none -vm -m $(ROMFILE).map -o $(ROMFILE).bin $(ROMFILE).o65

$(ROMFILE).o65: $(ROMFILE).a65
	@echo Assembling 6530-003 ROM Monitor
	@ca65 -g -o $(ROMFILE).o65 -l $(ROMFILE).lst --feature labels_without_colons $(ROMFILE).a65

.PHONY: flash getfun clean

flash: $(TARGET).bin
	minichlink -w $< flash -b -T

getfun:
	@echo Getting fresh copy of ch32v003fun from github
	@rm -rf ch32v003fun tmpfun
	@git clone --depth 1 https://github.com/cnlohr/ch32v003fun.git tmpfun
	@cp tmpfun/misc/libgcc.a tmpfun/ch32v003fun
	@mv tmpfun/ch32v003fun/ . 
	@rm -rf tmpfun

clean:
	@echo objs=$(OBJS)
	$(RM) $(OBJS) $(TARGET).elf $(TARGET).bin $(TARGET).hex $(TARGET).lst $(TARGET).map $(TARGET).hex || true
	$(RM) $(ROMFILE).o65 $(ROMFILE).lst $(ROMFILE).map $(ROMFILE).bin $(ROMFILE).ptp $(ROMFILE).h65 $(ROMFILE).c || true



# ca65 -g -o kimrom.o65 -l kimrom.lst --feature labels_without_colons kimrom.a65
# ld65 -t none -vm -m kimrom.map -o kimrom.bin kimrom.o65
# srec_cat kimrom.bin -binary -offset 0x1800 -o kimrom.c -C-array ROM6530
