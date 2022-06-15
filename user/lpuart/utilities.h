#ifndef UTILITIES_H
#define UTILITIES_H

#include <stdint.h>
#include "main.h"

#define BACKUP_PRIMASK() uint32_t primask_bit = __get_PRIMASK()
#define DISABLE_IRQ() __disable_irq()
#define ENABLE_IRQ() __enable_irq()
#define RESTORE_PRIMASK() __set_PRIMASK(primask_bit)

#define ALIGN(n) __attribute__((aligned(n)))

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define POW2(n) (1 << n)

__STATIC_INLINE uint8_t __ffs(uint32_t value)
{
    return (uint32_t)(32 - __CLZ(value & (-value)));
}

void srand1(uint32_t seed);
int32_t randr(int32_t min, int32_t max);
void memcpy1(uint8_t *dst, const uint8_t *src, uint16_t size);
void memcpyr(uint8_t *dst, const uint8_t *src, uint16_t size);
void memset1(uint8_t *dst, uint8_t value, uint16_t size);
int8_t Nibble2HexChar(uint8_t a);

#endif
