ENTRY(_start)

SECTIONS
{
	. = 0x8048000;

	.text ALIGN(4K) : AT (ADDR (.text) - 0x8000000)
	{
		*(.text)
	}

	.rodata ALIGN(4K) : AT (ADDR (.rodata) - 0x8000000)
	{
		*(.rodata)
	}

	.data ALIGN(4K) : AT (ADDR (.data) - 0x8000000)
	{
		*(.data)
	}

	.bss ALIGN(4K) : AT (ADDR (.bss) - 0x8000000)
	{
		*(.bss)
	}
}
