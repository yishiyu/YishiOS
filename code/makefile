##################################################
# Makefile
##################################################

.PHONY : all

all : run clean

run : boot.bin loader.bin kernel.bin
	# 写入MBR
	dd if=boot.bin of=c.img bs=512 count=1 conv=notrunc
	# 写入Boot Sector
	sudo mount ./c.img /mnt/bean
	sudo cp ./loader.bin /mnt/bean
	sudo cp ./kernel.bin /mnt/bean
	sudo umount /mnt/bean
	bochs -f bochsrc

# MBR
boot.bin : boot.asm
	nasm -o boot.bin boot.asm

# Loader
loader.bin : loader.asm
	nasm -o loader.bin loader.asm 

# Kernel
kernel.bin : kernel.asm
	nasm -f elf -o kernel.o kernel.asm
	# 设置ld采用32位模式进行链接
	ld -s -Ttext 0x1000 -m elf_i386  -o kernel.bin kernel.o

clean : 
	rm ./boot.bin ./loader.bin
