##################################################
# Yishi OS 's Makefile
##################################################

# 这个makefile专门生成贪吃蛇游戏
# 同时复制进磁盘映像依然由上级总makefile负责  

# snake.bin 的entry point 
# 用于指定生成的elf格式的snake.bin中的代码入口
ENTRYPOINT	= 	0x1000

# 目标文件
SNAKE_GAME = snake.bin
SNAKE_GDB = snake_gdb.bin

# 引用路径
INCLUDE_PATH = -I sys/

# 工具和标志位
ASM		= nasm
CC		= gcc
LD		= ld
LD_FLAGS = -s -Ttext $(ENTRYPOINT) -m elf_i386
ASM_FLAGS	=$(INCLUDE_PATH) -f elf
C_FLAGS		= $(INCLUDE_PATH) -c -fno-builtin -m32 -fno-stack-protector -g
LD_GDB_FLAGS = -Ttext $(ENTRYPOINT) -m elf_i386

# 中间文件
OBJS = objs/snake.o objs/syscall.o  objs/syscallasm.o objs/string.o

.PHONY : build clean realclean

# 默认启动点
build_snake : $(SNAKE_GAME) $(SNAKE_GDB)

clean : 
	rm -f $(OBJS)

realclean :
	rm -f $(OBJS) $(SNAKE_GAME) 

$(SNAKE_GAME) : $(OBJS)  
	$(LD) $(LD_FLAGS) -o $(SNAKE_GAME) $(OBJS)

$(SNAKE_GDB) : $(OBJS)  
	$(LD) $(LD_GDB_FLAGS) -o $(SNAKE_GDB) $(OBJS)

objs/snake.o : snake.c 
	$(CC) $(C_FLAGS) -o $@ $<

objs/syscall.o : sys/syscall.c
	$(CC) $(C_FLAGS) -o $@ $<

objs/syscallasm.o : sys/syscall.asm
	$(ASM) $(ASM_FLAGS) -o $@ $<

objs/string.o: sys/string.c
	$(CC) $(C_FLAGS) -o $@ $<