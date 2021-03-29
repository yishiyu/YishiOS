;=======================================
;   loader 中会用到的常量定义
;=======================================


; Loader加载地址
Loader_Base	equ 09000h		 
Loader_Offset equ 00h
Loader_Phy_Address equ Loader_Base * 0x10 + Loader_Offset

; Kernel.bin 直接加载地址
; kernel直接加载在 0x70000 ~ 0x90000
; 重新放置后分配的空间为 0x10000 ~ 0x70000 
Kernel_File_Base		equ	0x7000
Kernel_File_Offset		equ	0
Kernel_File_Phy_Addr	equ	Kernel_File_Base * 0x10
Kernel_Enter_point	equ	0x1000

; 超级块 目标内存地址
SuperBlock_Base	equ  070h		
SuperBlock_Offset	equ 00h

; GroupDescriptors 目标内存地址(超级块本身不到512字节,不会冲突)
GroupDescriptors_Base   equ 090h
GroupDescriptors_Offset equ 00h

; inode table 目标内存地址(60m的硬盘GroupDescriptors只有一个block)
InodeTable_Base	equ	0d0h		
InodeTable_Offset	equ 00h

;根目录目标内存地址(inode table目前我们只需要第一个block就行了,留下1k空间足够了)
RootDir_Base    equ 0110h
RootDir_Offset  equ 00h

;一级索引临时block存放区
First_Index_Block_Base    equ   0150h
First_Index_Block_Offset    equ 00h

;inode相关的数据结构
;长度为128字节
;记录数据block号的起始偏移为40
Inode_Length    equ 128
Inode_Blocks    equ 28
Inode_Block equ 40

; 记录目录项中的数据结构偏移
Inode_Number_Offset    equ 0
Record_Length_Offset   equ 4
Name_Len_Offset    equ 6
File_Type_Offset   equ 7
File_Name_Offset   equ 8

; 分页机制用到的常量
Page_Dir_Base		equ	0x100000
Page_Table_BASE		equ	0x101000

; ELF文件相关的常量
ELF_Header_e_phnum      equ 0x2c
ELF_Header_e_phoff      equ 0x1c
ELF_Program_Header_p_size   equ 0x10
ELF_Program_Header_p_offset equ 0x4
ELF_Program_Header_p_vaddr  equ 0x8

