IPL=hello-os.asm

LST=asm.lst

IMG=os.img

EDIMG=../bin/edimg.py


default : $(IMG)
		:

run : $(IMG)
		qemu-system-i386 -fda $(IMG)

$(IMG) : $(IPL)
		$(EDIMG) -ipl $(IPL) -o $(IMG) -l $(LST)


clean :
		rm $(LST) $(IMG)
