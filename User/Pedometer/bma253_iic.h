
#ifndef _BMA253_IIC_H_
#define _BMA253_IIC_H_

//------------------------------------------------------------------------------
#include "TypeDefine.h"

//------------------------------------------------------------------------------
#define Write                   0
#define Read                    1

#define HIGH                    1
#define LOW                     0

#define TRUE                    1
#define FALSE                   0

#define true                    1
#define false                   0

#define ACK                     1
#define NOACK                   0

#define _BMA253_I2C_DELAY_      2

#define SlaveAddr               0x80
#define resolution              0

//------------------------------------------------------------------------------
#define BMA253_SCL_Pin LL_GPIO_PIN_4
#define BMA253_SCL_GPIO_Port GPIOA
#define BMA253_SDA_Pin LL_GPIO_PIN_10
#define BMA253_SDA_GPIO_Port GPIOA

typedef enum {
  _BMA253_SDA_OUTPUT_,
  _BMA253_SDA_INPUT_,
}SDA_IO_Type;

#define BMA253_SDAIN()          do{BMA253_SDA_SET(_BMA253_SDA_INPUT_);}while(0)
#define BMA253_SDAOUT()         do{BMA253_SDA_SET(_BMA253_SDA_OUTPUT_);}while(0)
#define BMA253_SCL_HIGH()       do{LL_GPIO_SetOutputPin(BMA253_SCL_GPIO_Port, BMA253_SCL_Pin);}while(0)
#define BMA253_SCL_LOW()        do{LL_GPIO_ResetOutputPin(BMA253_SCL_GPIO_Port, BMA253_SCL_Pin);}while(0)
#define BMA253_SDA_HIGH()       do{LL_GPIO_SetOutputPin(BMA253_SDA_GPIO_Port, BMA253_SDA_Pin);}while(0)
#define BMA253_SDA_LOW()        do{LL_GPIO_ResetOutputPin(BMA253_SDA_GPIO_Port, BMA253_SDA_Pin);}while(0)

//------------------------------------------------------------------------------
void BMA253_I2C_Init(void);
u8 BMA253_WriteByte(u8 I2C_addr,u8 Reg_addr,u8 data);
u8 BMA253_ReadByte(u8 I2C_addr, u8 Reg_addr, u8 *rxByte);
u8 BMA253_Write_LenBytes(u8 I2C_addr, u8 Reg_addr, u8 *buff, u8 len);
u8 BMA253_Read_LenBytes(u8 I2C_addr, u8 Reg_addr, u8 *buff, u8 len);

#endif
