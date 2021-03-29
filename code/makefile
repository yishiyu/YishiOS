##################################################
# Makefile
##################################################


.PHONY : run

run : yishi.bin
	dd if=yishi.bin of=yishi.img bs=512 count=1 conv=notrunc
	bochs -f bochsrc

yishi.bin : boot.asm
	nasm -o yishi.bin boot.asm

