##################################################
# Yishi OS 's Makefile
##################################################

# Yishi OS 的entry point 
# 用于指定生成的elf格式的kernel.bin中的代码入口
# 需要和loader.inc中的Kernel_Enter_point保持一致
ENTRYPOINT	= 	0x1000

# Yishi OS 的entry point在kernel.bin中的偏移
# 在对kernel.bin进行反汇编的时候会用到
# 取决于 ENTRYPOINT
ENTRYOFFSET	=   0x400

# 命令中断指令和option的宏定义
ASM		= nasm
DASM	= ndisasm
CC		= gcc
LD		= ld
ASM_BOOT_FLAGS	= -I boot/include/
ASM_KERNEL_FLAGS	= -I include/ -f elf
LD_FLAGS		= -s -Ttext $(ENTRYPOINT) -m elf_i386
DASM_FLAGS	= -u -o $(ENTRYPOINT) -e $(ENTRYOFFSET)

# 生成目标文件的宏定义
# OBJS: 中间文件,需要通过LD命令进一步链接成最终的目标文件
YISHI_BOOT	= boot/boot.bin boot/loader.bin
YISHI_KERNEL	= kernel/kernel.bin
OBJS		= kernel/kernel.o
DASM_OUTPUT	= kernel.bin.asm


# 声明所有支持的命令
.PHONY : run everything clean realclean disasm final buildimg image

# 默认启动点
run	:	image 
	bochs -f bochsrc

# 引导文件 内核文件
everything : $(YISHI_BOOT) $(YISHI_KERNEL)

# 删除所有中间文件
clean :
	rm -f $(OBJS)

# 清楚所有文件(包括中间文件和最终目标文件)
realclean :
	rm -f $(OBJS) $(YISHI_BOOT) $(YISHI_KERNEL)

# 把内核反汇编
disasm :
	$(DASM) $(DASM_FLAGS) $(YISHI_KERNEL) > $(DASM_OUTPUT)


# 仅生成最终的目标文件
final : everything clean


# 把生成的文件写入 复制进硬盘
buildimg :
	# 把MBR写入首扇区(不截断式写入)
	dd if=boot/boot.bin of=c.img bs=512 count=1 conv=notrunc
	# 把内核文件复制进硬盘
	sudo mount ./c.img /mnt/bean
	sudo cp ./boot/loader.bin /mnt/bean
	sudo cp ./kernel/kernel.bin /mnt/bean
	sudo umount /mnt/bean

# 生成 目标镜像
image : final buildimg

# 根据中间文件链接生成最终文件
$(YISHI_KERNEL) : $(OBJS)
	$(LD) $(LD_FLAGS) -o $(YISHI_KERNEL) $(OBJS)


# 生成boot.bin
boot/boot.bin : boot/boot.asm boot/include/boot.inc boot/include/Ext2.inc
	$(ASM) $(ASM_BOOT_FLAGS) -o $@ $<

# 生成loader.bin
boot/loader.bin : boot/loader.asm boot/include/loader.inc boot/include/pm.inc
	$(ASM) $(ASM_BOOT_FLAGS) -o $@ $<

# 生成kernel.bin
kernel/kernel.o : kernel/kernel.asm
	$(ASM) $(ASM_KERNEL_FLAGS) -o $@ $<

