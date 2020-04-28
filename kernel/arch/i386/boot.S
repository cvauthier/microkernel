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
.align 16
.skip 16384 # 16 KiB
stack_top:

.align 4096
boot_page_directory:
.skip 4096
boot_page_table1:
.skip 4096

.section .text
.global _start
.type _start, @function
_start:
	movl $(stack_top - 0xC0000000), %esp
	push %eax
	push %ebx

# Mise en place du paging - code pris de l'article "Higher Half Bare Bones sur OSDev"
	mov $(boot_page_table1 - 0xC0000000), %edi
	mov $0, %esi
	mov $1023, %ecx

.loop:
	cmp $(_kernel_start - 0xC0000000), %esi
	jl .loop_inc
	cmp $(_kernel_end - 0xC0000000), %esi
	jge .loop_end

	mov %esi, %edx
	or $0x003, %edx
	mov %edx, (%edi)

.loop_inc:
	add $4096, %esi
	add $4, %edi
	loop .loop

.loop_end:
	movl $(0x000B8000 | 0x003), boot_page_table1 - 0xC0000000 + 1023 * 4

	movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 0
	movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 768 * 4
	movl $(boot_page_directory - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 1023 * 4

	mov $(boot_page_directory - 0xC0000000), %ecx
	mov %ecx, %cr3

	mov %cr0, %ecx
	or $0x80010000, %ecx
	mov %ecx, %cr0

	lea 1f, %ecx
	jmp *%ecx
1:
	
	movl $0, boot_page_directory + 0

	mov %cr3, %ecx
	mov %ecx, %cr3
	
	add $0xC0000000, %esp

	call _init
	call kernel_main

	cli
2:hlt
	jmp 2b

.size _start, . - _start
