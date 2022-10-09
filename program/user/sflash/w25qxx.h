#ifndef W25Qxx_H
#define W25Qxx_H

#include <stdint.h>

void W25QXX_Init(void);
void W25QXX_WriteSR(uint8_t sr);
void W25QXX_WriteDisable(void);
void W25QXX_Read(uint8_t *buffer, uint32_t addr, uint16_t len);
void W25QXX_Write(uint8_t *buffer, uint32_t addr, uint16_t len);
void W25QXX_WriteNoCheck(uint8_t *buffer, uint32_t addr, uint16_t len);
void W25QXX_EraseChip(void);
void W25QXX_PowerDown(void);
void W25QXX_Wakeup(void);

#endif
