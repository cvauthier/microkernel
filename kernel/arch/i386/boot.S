# Multiboot header (cf Bare Bones sur OSDev)
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

# Declare a header as in the Multiboot Standard.
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .bss, "aw", @nobits
# Pile initiale
.global tss_stack_top
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:
.skip 16384
tss_stack_top:

.align 4096
boot_page_directory:
.skip 4096
boot_page_table1:
.skip 4096

.section .text
.global _start
.type _start, @function
_start:

# Mise en place du paging - code pris de l'article "Higher Half Bare Bones sur OSDev"
	movl $(boot_page_table1 - 0xC0000000), %edi
	movl $0, %esi
	movl $1023, %ecx

.loop:
	cmpl $(_kernel_start - 0xC0000000), %esi
	jl .loop_inc
	cmpl $(_kernel_end - 0xC0000000), %esi
	jge .loop_end

	movl %esi, %edx
	orl $0x003, %edx
	movl %edx, (%edi)

.loop_inc:
	addl $4096, %esi
	addl $4, %edi
	loop .loop

.loop_end:
	movl $(0x000B8000 | 0x003), boot_page_table1 - 0xC0000000 + 1023 * 4

	movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 0
	movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 768 * 4
	movl $(boot_page_directory - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 1023 * 4

	movl $(boot_page_directory - 0xC0000000), %ecx
	movl %ecx, %cr3

	movl %cr0, %ecx
	orl $0x80010000, %ecx
	movl %ecx, %cr0

	lea 1f, %ecx
	jmp *%ecx
1:
	
	movl $0, boot_page_directory + 0

	movl %cr3, %ecx
	movl %ecx, %cr3
	
	movl $stack_top, %esp

	call _init
	call kernel_main

	cli
2:hlt
	jmp 2b

.size _start, . - _start
