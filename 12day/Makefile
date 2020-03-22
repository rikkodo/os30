DIR_BIN=bin
DIR_LST=lst

OBJECT_BOOTPACK= \
	$(DIR_BIN)/bootpack.o \
	$(DIR_BIN)/fifo.o \
	$(DIR_BIN)/interrupt.o \
	$(DIR_BIN)/graphic.o \
	$(DIR_BIN)/dsctbl.o \
	$(DIR_BIN)/mouse.o \
	$(DIR_BIN)/keyboard.o \
	$(DIR_BIN)/fonts.o \
	$(DIR_BIN)/memory.o \
	$(DIR_BIN)/sheet.o \
	$(DIR_BIN)/timer.o \
	$(DIR_BIN)/mysprintf.o

default :
		make $(DIR_BIN)
		make $(DIR_LST)
		make os.img

run32 :
		qemu-system-i386 -drive file=os.img,index=0,if=floppy,format=raw -m 32m

run :
		qemu-system-i386 -drive file=os.img,index=0,if=floppy,format=raw

os.img : $(DIR_BIN)/ipl.bin $(DIR_BIN)/haribote.sys Makefile
		mformat -f 1440 -C -B $(DIR_BIN)/ipl.bin -i $@
		mcopy $(DIR_BIN)/haribote.sys -i $@ ::

$(DIR_BIN)/%.bin : %.asm Makefile
		nasm -l $(DIR_LST)/$*.lst -o $@ $*.asm

$(DIR_BIN)/nasmfunc.o : nasmfunc.asm Makefile
		nasm -g -f elf nasmfunc.asm -o $@

$(DIR_BIN)/%.o : %.c  Makefile
		gcc -Wall -Wextra -fno-pie -c -march=i486 -m32 -nostdlib $*.c -o $@

$(DIR_BIN)/bootpack.hrb : $(OBJECT_BOOTPACK) har.ld $(DIR_BIN) $(DIR_BIN)/nasmfunc.o Makefile
		gcc -Wall -Wextra -fno-pie -march=i486 -m32 -nostdlib -T har.ld $(OBJECT_BOOTPACK) $(DIR_BIN)/nasmfunc.o -o $@

$(DIR_BIN)/haribote.sys : $(DIR_BIN)/haribote.bin $(DIR_BIN)/bootpack.hrb Makefile
		cat $(DIR_BIN)/haribote.bin $(DIR_BIN)/bootpack.hrb > $@

$(DIR_LST) :
		@if [ ! -d $@ ]; \
			then echo "mkdir -p $@"; mkdir -p $@; \
		fi

$(DIR_BIN) :
		@if [ ! -d $@ ]; \
			then echo "mkdir -p $@"; mkdir -p $@; \
		fi

clean :
		rm $(DIR_LST)/* $(DIR_BIN)/*
