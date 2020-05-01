#include <kernel/utility.h>

uint32_t read_bigendian_int(uint8_t *ptr)
{
	return (ptr[0]<<24) + (ptr[1]<<16) + (ptr[2]<<8) + ptr[3];
}

void write_bigendian_int(uint8_t *ptr, uint32_t n)
{
	for (int i = 3 ; i >= 0 ; i--)
	{
		ptr[i] = n&0xFF;
		n >>= 8;
	}
}

