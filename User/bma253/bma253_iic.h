#ifndef BMA253_IIC_H
#define BMA253_IIC_H

#include <stdint.h>
#include <stdbool.h>

extern void BMA253_I2cInit(void);
extern bool BMA253_WriteByte(uint8_t reg, uint8_t data);
extern bool BMA253_ReadByte(uint8_t reg, uint8_t *data);
extern bool BMA253_WriteNBytes(uint8_t reg, uint8_t *data, uint16_t size);
extern bool BMA253_ReadNBytes(uint8_t reg, uint8_t *data, uint16_t size);

#endif
