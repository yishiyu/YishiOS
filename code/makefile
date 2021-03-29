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

# 分析函数调用关系时传入的文件名
export CALL_TREE_FILE = ./kernel/kernel.c

# 命令中断指令和option的宏定义
ASM		= nasm
DASM	= ndisasm
CC		= gcc
LD		= ld
INCLUDE_PATH = -I include/base/ -I include/display -I include/interupt -I include/process \
								 -I include/syscall -I include/process/terminal -I include/process/filesystem\
								 -I include/process/output -I include/process/input -I include/process/disk \
								 -I include/process/memory
ASM_BOOT_FLAGS	= -I boot/include/
ASM_KERNEL_FLAGS	=$(INCLUDE_PATH) -f elf
C_FLAGS		= $(INCLUDE_PATH) -c -fno-builtin -m32 -fno-stack-protector -g
LD_FLAGS		= -s -Ttext $(ENTRYPOINT) -m elf_i386
DASM_FLAGS	= -u -o $(ENTRYPOINT) -e $(ENTRYOFFSET) -g
LD_GDB_FLAGS = -Ttext $(ENTRYPOINT) -m elf_i386


# 生成目标文件的宏定义
# OBJS: 中间文件,需要通过LD命令进一步链接成最终的目标文件
YISHI_BOOT	= boot/boot.bin boot/loader.bin
YISHI_KERNEL	= kernel/kernel.bin
GDB_KERNEL = kernel_gdb.bin
OBJS		= objs/kernel.o objs/global.o objs/c_kernel.o \
						objs/proc.o objs/mem.o objs/display.o objs/port.o objs/func.o objs/irqhandler.o \
						objs/initirq.o objs/syscall.o objs/syscallasm.o \
						objs/keymap.o objs/terminal.o \
						objs/terminallib.o \
						objs/ipc.o objs/syscalllib.o objs/input_keyboard.o \
						objs/kerneltask.o objs/output.o objs/input.o \
						objs/disk.o objs/disklib.o objs/string.o \
						objs/filesystem.o objs/memory.o
DASM_OUTPUT	= kernel.bin.asm
CALL_TREE = calltree.png

# 生成贪吃蛇游戏
SNAKE_GAME = snake/snake.bin
LD_SNAKE_FLAGS = -s -m elf_i386
 
# 声明所有支持的命令
.PHONY : run everything clean realclean disasm final buildimg image calltree

# 默认启动点,有调试模式
run_debug	:	image 
	bochs -f bochsrc_debug

run : image
	bochs -f bochsrc

gdb : 
	gdb $(GDB_KERNEL)

# 生成函数调用关系图
calltree : 
	cflow $(INCLUDE_PATH)  -T $(CALL_TREE_FILE) | tee temp | tree2dotx | dot -Tpng -o $(CALL_TREE)
	cat ./temp
	rm ./temp

# 引导文件 内核文件
everything : $(YISHI_BOOT) $(YISHI_KERNEL) $(GDB_KERNEL) $(SNAKE_GAME)

# 删除所有中间文件
clean :
	rm -f $(OBJS) $(CALL_TREE)

# 清楚所有文件(包括中间文件和最终目标文件)
realclean :
	rm -f $(OBJS) $(YISHI_BOOT) $(YISHI_KERNEL) $(CALL_TREE) $(GDB_KERNEL)

# 把内核反汇编
disasm :
	$(DASM) $(DASM_FLAGS) $(YISHI_KERNEL) > $(DASM_OUTPUT)


# 仅生成最终的目标文件
final : everything 


# 把生成的文件写入 复制进硬盘
buildimg :
	# 把MBR写入首扇区(不截断式写入)
	dd if=boot/boot.bin of=c.img bs=512 count=1 conv=notrunc
	# 把内核文件复制进硬盘
	sudo mount ./c.img /mnt/bean
	sudo cp ./boot/loader.bin /mnt/bean
	sudo cp ./kernel/kernel.bin /mnt/bean
	sudo cp ./snake/snake.bin /mnt/bean
	sudo umount /mnt/bean

# 生成 目标镜像
image : final buildimg

# 根据中间文件链接生成最终文件
$(YISHI_KERNEL) : $(OBJS)
	$(LD) $(LD_FLAGS) -o $(YISHI_KERNEL) $(OBJS)

$(GDB_KERNEL) : $(OBJS)
	$(LD) $(LD_GDB_FLAGS) -o $(GDB_KERNEL) $(OBJS)

$(SNAKE_GAME) : $(OBJS)
	$(LD) $(LD_SNAKE_FLAGS) -o $(SNAKE_GAME) $(OBJS)

boot/boot.bin : boot/boot.asm boot/include/boot.inc boot/include/Ext2.inc
	$(ASM) $(ASM_BOOT_FLAGS) -o $@ $<

boot/loader.bin : boot/loader.asm boot/include/loader.inc boot/include/pm.inc
	$(ASM) $(ASM_BOOT_FLAGS) -o $@ $<

objs/kernel.o : kernel/kernel.asm
	$(ASM) $(ASM_KERNEL_FLAGS) -o $@ $<

objs/c_kernel.o : kernel/kernel.c
	$(CC) $(C_FLAGS) -o $@ $<

objs/global.o : kernel/global.c
	$(CC) $(C_FLAGS) -o $@ $<

objs/func.o : lib/base/func.c
	$(CC) $(C_FLAGS) -o $@ $<

# 内存操作函数
objs/mem.o : lib/base/memory.asm
	$(ASM) $(ASM_KERNEL_FLAGS) -o $@ $<

objs/port.o : lib/base/port.asm
	$(ASM) $(ASM_KERNEL_FLAGS) -o $@ $<

objs/proc.o : lib/process/proc.c
	$(CC) $(C_FLAGS) -o $@ $<

objs/display.o : lib/display/display.asm
	$(ASM) $(ASM_KERNEL_FLAGS) -o $@ $<

objs/irqhandler.o : lib/interupt/irqhandler.c
	$(CC) $(C_FLAGS) -o $@ $<
	
objs/initirq.o : lib/interupt/initirq.c
	$(CC) $(C_FLAGS) -o $@ $<

objs/syscall.o : lib/syscall/syscall.c
	$(CC) $(C_FLAGS) -o $@ $<

objs/syscallasm.o : lib/syscall/syscall.asm
	$(ASM) $(ASM_KERNEL_FLAGS) -o $@ $<

objs/terminal.o : lib/process/terminal/terminal.c
	$(CC) $(C_FLAGS) -o $@ $<
	
objs/keymap.o : lib/process/input/keymap.c
	$(CC) $(C_FLAGS) -o $@ $<

objs/terminallib.o : lib/process/terminal/terminallib.c
	$(CC) $(C_FLAGS) -o $@ $<

objs/ipc.o : lib/syscall/ipc.c
	$(CC) $(C_FLAGS) -o $@ $<

objs/syscalllib.o : lib/syscall/syscalllib.c
	$(CC) $(C_FLAGS) -o $@ $<

objs/kerneltask.o : lib/process/kerneltask.c
	$(CC) $(C_FLAGS) -o $@ $<

objs/output.o : lib/process/output/output.c
	$(CC) $(C_FLAGS) -o $@ $<

objs/input.o : lib/process/input/input.c
	$(CC) $(C_FLAGS) -o $@ $<

objs/input_keyboard.o : lib/process/input/input_keyboard.c
	$(CC) $(C_FLAGS) -o $@ $<

objs/disk.o : lib/process/disk/disk.c
	$(CC) $(C_FLAGS) -o $@ $<

objs/disklib.o : lib/process/disk/disk.asm
	$(ASM) $(ASM_KERNEL_FLAGS) -o $@ $<

objs/string.o : lib/base/string.c
	$(CC) $(C_FLAGS) -o $@ $<

objs/filesystem.o : lib/process/filesystem/filesystem.c
	$(CC) $(C_FLAGS) -o $@ $<

# 内存管理模块
objs/memory.o : lib/process/memory/memory.c
	$(CC) $(C_FLAGS) -o $@ $<

snake/snake.o : snake/snake.c  
	$(CC) $(C_FLAGS) -o $@ $<
