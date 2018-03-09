DIR_BIN=bin
DIR_LST=lst

BINS= \
		$(DIR_BIN)/ipl.bin \
		$(DIR_BIN)/haribote.bin

IMG=os.img


default :
		make $(IMG)

run : $(IMG)
		qemu-system-i386 -drive file=${IMG},index=0,if=floppy,format=raw

$(IMG) : $(BINS) Makefile
		mformat -f 1440 -C -B $(DIR_BIN)/ipl.bin -i $@
		mcopy $(DIR_BIN)/haribote.bin -i $@ ::

$(DIR_BIN)/%.bin : *.asm $(DIR_LST) $(DIR_BIN)
		nasm -l $(DIR_LST)/$*.lst -o $@ $*.asm

$(DIR_LST) :
		@if [ ! -d $@ ]; \
			then echo "mkdir -p $@"; mkdir -p $@; \
		fi

$(DIR_BIN) :
		@if [ ! -d $@ ]; \
			then echo "mkdir -p $@"; mkdir -p $@; \
		fi

clean :
		rm $(DIR_LST)/* $(DIR_BIN)/*.bin
