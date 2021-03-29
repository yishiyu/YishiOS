;=====================
;   用来记一些BOOT用到的参数
;=====================


; 栈底,从这个位置向低地址生长,BIOS ROM使用的最高地址为500h,只要不覆盖到那里就不会有问题 
Stack_Base	equ	07c00h			

 ; Loader 目标内存地址
Loader_Base	equ 90000h		 
Loader_Offset equ 00h

;超级块 目标内存地址
SuperBlock_Base	equ  070h		
SuperBlock_Offset	equ 00h

;GroupDescriptors 目标内存地址(超级块本身不到512字节,不会冲突)
GroupDescriptors_Base   equ 090h
GroupDescriptors_Offset equ 00h

;inode table 目标内存地址(60m的硬盘GroupDescriptors只有一个block)
InodeTable_Base	equ	0d0h		
InodeTable_Offset	equ 00h


