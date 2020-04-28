/* Documentation : http://www.scoberlin.de/content/media/http/informatik/gcc_docs/ld_3.html */

ENTRY(_start)

SECTIONS
{
	/* Higher-half kernel : dans la mémoire virtuelle, le kernel est mappé à 3GiB + 1MiB. 
	Il se trouve dans la mémoire physique à partir de 1MiB*/
	
	. = 0xC0100000;
	_kernel_start = .;

	.text ALIGN(4K) : AT (ADDR (.text) - 0xC0000000)
	{
		*(.multiboot)
		*(.text)
	}

	.rodata ALIGN(4K) : AT (ADDR (.rodata) - 0xC0000000)
	{
		*(.rodata)
	}

	.data ALIGN(4K) : AT (ADDR (.data) - 0xC0000000)
	{
		*(.data)
	}

	.bss ALIGN(4K) : AT (ADDR (.bss) - 0xC0000000)
	{
		*(COMMON)
		*(.bss)
	}

	_kernel_end = .;
	_4M_aligned_after_kernel_end = 0xC0400000;
}
