##################################################
# Makefile
##################################################

.PHONY : all

all : run clean

run : boot.bin loader.bin
	# 写入MBR
	dd if=boot.bin of=c.img bs=512 count=1 conv=notrunc
	# 写入Boot Sector
	sudo mount ./c.img /mnt/bean
	sudo cp ./loader.bin /mnt/bean
	sudo umount /mnt/bean
	bochs -f bochsrc

# MBR
boot.bin : boot.asm
	nasm -o boot.bin boot.asm

# Loader
loader.bin : loader.asm
	nasm -o loader.bin loader.asm 

clean : 
	rm ./boot.bin ./loader.bin 
