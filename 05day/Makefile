DIR_BIN=bin
DIR_LST=lst

CSOURCE= \
	bootpack.c \
	fonts.c

default :
		make os.img

run :
		qemu-system-i386 -drive file=os.img,index=0,if=floppy,format=raw

os.img : $(DIR_BIN)/ipl.bin $(DIR_BIN)/haribote.sys Makefile
		mformat -f 1440 -C -B $(DIR_BIN)/ipl.bin -i $@
		mcopy $(DIR_BIN)/haribote.sys -i $@ ::

$(DIR_BIN)/%.bin : %.asm $(DIR_LST) $(DIR_BIN) Makefile
		nasm -l $(DIR_LST)/$*.lst -o $@ $*.asm

$(DIR_BIN)/%.o : %.asm $(DIR_BIN) Makefile
		nasm -g -f elf $*.asm -o $@

$(DIR_BIN)/bootpack.hrb : $(CSOURCE) har.ld $(DIR_BIN) $(DIR_BIN)/nasmfunc.o Makefile
		gcc -march=i486 -m32 -nostdlib -T har.ld -g $(CSOURCE) $(DIR_BIN)/nasmfunc.o -o $@

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
