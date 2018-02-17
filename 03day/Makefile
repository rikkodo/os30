IPL=ipl.asm
IPL_IMG=ipl.img
LST=asm.lst
IMG=os.img

EDIMG=edimg.py


default :
		make $(IMG)

run : $(IMG)
		qemu-system-i386 qemu-system-i386 -drive file=${IMG},index=0,if=floppy,format=raw

$(IMG) : $(IPL_IMG)
		$(EDIMG) -ipl $(IPL_IMG) -o $(IMG)

$(IPL_IMG) : $(IPL)
		nasm -l $(LST) -o $(IPL_IMG) $(IPL)

clean :
		rm $(LST) $(IMG) $(IPL_IMG)
