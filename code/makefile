##################################################
# Makefile
##################################################


.PHONY : run

run : yishi.com
	sudo mount -o loop ./yishi.img /mnt/floppy
	sudo rm -rf /mnt/floppy/*
	sudo cp ./yishi.com /mnt/floppy
	sudo umount /mnt/floppy
	bochs -f bochsrc

yishi.com : boot.asm
	nasm -o yishi.com boot.asm

