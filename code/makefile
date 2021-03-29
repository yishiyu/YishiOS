##################################################
# Makefile
##################################################


.PHONY : run

run : boot.bin loader.bin
	# 写入MBR
	dd if=boot.bin of=c.img bs=512 count=1 conv=notrunc
	# 写入Boot Sector
	# dd if=loader.bin of=c.img bs=512 count=2 seek=1 conv=notrunc
	bochs -f bochsrc

# MBR
boot.bin : boot.asm
	nasm -o boot.bin boot.asm

# Loader
loader.bin : loader.asm
	nasm -o loader.bin loader.asm 
