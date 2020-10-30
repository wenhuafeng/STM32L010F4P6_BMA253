

#include <stdint.h>
#include "BMA253.h"

#include "i2c.h"

#define _SOFTWARE_IIC_                  0
#define _HARDWARE_IIC_                  1
#define _IIC_TYPE_                      _SOFTWARE_IIC_

#define BMA253_I2C                      &hi2c1

#define DATA_READY_BIT_MASK             (0x01)

//static uint8_t BMA253_NewAccelDataRdyX(void);
//static uint8_t BMA253_NewAccelDataRdyY(void);
//static uint8_t BMA253_NewAccelDataRdyZ(void);

//------------------------------------------------------------------------------
#if (_IIC_TYPE_ == _HARDWARE_IIC_)

void BMA253_Initialize(void)
{
  uint8_t i;
  
  i = BMA253_NORMAL_MODE;
  HAL_I2C_Mem_Write(BMA253_I2C, BMA253_ADDR, BMA253_PWR_MODE_ADDR, I2C_MEMADD_SIZE_8BIT, &i, 1, 100);
  i = BMA253_BW_7_81HZ;
  HAL_I2C_Mem_Write(BMA253_I2C, BMA253_ADDR, BMA253_BW_SELECT_ADDR, I2C_MEMADD_SIZE_8BIT, &i, 1, 100);
  i = BMA253_2G_RANGE;
  HAL_I2C_Mem_Write(BMA253_I2C, BMA253_ADDR, BMA253_RANGE_ADDR, I2C_MEMADD_SIZE_8BIT, &i, 1, 100);
}

void BMA253_GetAccelDataX(int16_t *xAccelData)
{
  uint8_t x[2];
  
  while(!BMA253_NewAccelDataRdyX())
  {
    // Do Nothing until we have new data ready in the register
  }
  
  HAL_I2C_Mem_Read(BMA253_I2C, BMA253_ADDR, BMA253_X_LSB_ADDR, I2C_MEMADD_SIZE_8BIT, x, 2, 1000);
  
  *xAccelData = ((uint16_t)x[0] | ((uint16_t)x[1] << 8));
  
}

void BMA253_GetAccelDataY(int16_t *yAccelData)
{
  uint8_t y[2];
  
  while(!BMA253_NewAccelDataRdyY())
  {
    // Do Nothing until we have new data ready in the register
  }
  
  
  HAL_I2C_Mem_Read(BMA253_I2C, BMA253_ADDR, BMA253_Y_LSB_ADDR, I2C_MEMADD_SIZE_8BIT, y, 2, 1000);
  
  *yAccelData = ((uint16_t)y[0] | ((uint16_t)y[1] << 8));
  
}

void BMA253_GetAccelDataZ(int16_t *zAccelData)
{
  uint8_t z[2];
  
  while(!BMA253_NewAccelDataRdyZ())
  {
    // Do Nothing until we have new data ready in the register
  }
  
  
  HAL_I2C_Mem_Read(BMA253_I2C, BMA253_ADDR, BMA253_Z_LSB_ADDR, I2C_MEMADD_SIZE_8BIT, z, 2, 1000);
  
  *zAccelData = ((uint16_t)z[0] | ((uint16_t)z[1] << 8));
  
}

/*
void BMA253_GetAccelDataXYZ(BMA253_ACCEL_DATA_t *accelData)
{
  
  while (!BMA253_NewAccelDataRdyX())
  {
    // Do Nothing until we have new data ready in the register
  }
  
  accelData->x = (((int16_t)i2c_read1ByteRegister(BMA253_ADDR, BMA253_X_LSB_ADDR))
                | ((int16_t)i2c_read1ByteRegister(BMA253_ADDR, BMA253_X_MSB_ADDR) << 8) >> 4);
  
  while (!BMA253_NewAccelDataRdyY())
  {
    // Do Nothing until we have new data ready in the register
  }
  
  accelData->y = (((int16_t)i2c_read1ByteRegister(BMA253_ADDR, BMA253_Y_LSB_ADDR))
                | ((int16_t)i2c_read1ByteRegister(BMA253_ADDR, BMA253_Y_MSB_ADDR) << 8) >> 4);
  
  while (!BMA253_NewAccelDataRdyZ())
  {
    // Do Nothing until we have new data ready in the register
  }
  
  accelData->z = (((int16_t)i2c_read1ByteRegister(BMA253_ADDR, BMA253_Z_LSB_ADDR))
                | ((int16_t)i2c_read1ByteRegister(BMA253_ADDR, BMA253_Z_MSB_ADDR) << 8) >> 4);
  
}*/

uint8_t BMA253_GetAccelChipId(void)
{
  uint8_t i;
  
  HAL_I2C_Mem_Read(BMA253_I2C, BMA253_ADDR, BMA253_CHIP_ID_ADDR, I2C_MEMADD_SIZE_8BIT, &i, 1, 1000);
  
  return i;
}

static uint8_t BMA253_NewAccelDataRdyX(void)
{
  uint8_t xNewData;
  
  //xNewData = (i2c_read1ByteRegister(BMA253_ADDR, BMA253_X_LSB_ADDR) & DATA_READY_BIT_MASK); //Is new data ready?
  HAL_I2C_Mem_Read(BMA253_I2C, BMA253_ADDR, BMA253_X_LSB_ADDR, I2C_MEMADD_SIZE_8BIT, &xNewData, 1, 1000);
  xNewData &= DATA_READY_BIT_MASK;
  
  return xNewData;
}

static uint8_t BMA253_NewAccelDataRdyY(void)
{
  uint8_t yNewData;
  
  //yNewData = (i2c_read1ByteRegister(BMA253_ADDR, BMA253_Y_LSB_ADDR) & DATA_READY_BIT_MASK); //Is new data ready?
  HAL_I2C_Mem_Read(BMA253_I2C, BMA253_ADDR, BMA253_Y_LSB_ADDR, I2C_MEMADD_SIZE_8BIT, &yNewData, 1, 1000);
  
  return yNewData;
}

static uint8_t BMA253_NewAccelDataRdyZ(void)
{
  uint8_t zNewData;
  
  //zNewData = (i2c_read1ByteRegister(BMA253_ADDR, BMA253_Z_LSB_ADDR) & DATA_READY_BIT_MASK); //Is new data ready?
  HAL_I2C_Mem_Read(BMA253_I2C, BMA253_ADDR, BMA253_Z_LSB_ADDR, I2C_MEMADD_SIZE_8BIT, &zNewData, 1, 1000);
  
  return zNewData;
}

#elif (_IIC_TYPE_ == _SOFTWARE_IIC_)



#endif

