#ifndef UTILITY_H
#define UTILITY_H

#include <stdint.h>

uint32_t read_bigendian_int(uint8_t *ptr);
void write_bigendian_int(uint8_t *ptr, uint32_t n);

#endif
