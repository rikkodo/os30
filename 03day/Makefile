IPL=hello-os.asm
LST=asm.lst
IMG=os.img

EDIMG=edimg.py


default :
		make $(IMG)

run : $(IMG)
		qemu-system-i386 -fda $(IMG)

$(IMG) : $(IPL)
		$(EDIMG) -ipl $(IPL) -o $(IMG) -l $(LST)


clean :
		rm $(LST) $(IMG)
