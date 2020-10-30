

/************************************************************/
//文件名:BMA253.c
//主程序利用的函数有：
//UCB0IICInit 初始化
/************************************************************/
#include <stdio.h>
#include "main.h"
#include "TypeDefine.h"

#if (_PEDOMETER_)

#if (_IIC_TYPE_ == _SOFTWARE_IIC_)
#include "bma253_iic.h"

//==============================================================================
//* Function Name  : IIC Delay
//* Description    : delay
//* Input          : None
//* Output         : None
//* Return         : None
//* Attention      : None
//==============================================================================
static void BMA253_I2C_Delay(u16 j)
{
  u16 i = j;

  while(i--);
}

//==============================================================================
//* Function Name  : BMA253_SDA_SET
//* Description    : SDA input / SDA output
//* Input          : None
//* Output         : None
//* Return         : None
//* Attention      : None
//==============================================================================
static void BMA253_SDA_SET(SDA_IO_Type ioset)
{
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  if (ioset == _BMA253_SDA_OUTPUT_) {
    GPIO_InitStruct.Pin = I2C1_SDA_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    LL_GPIO_Init(BMA253_SDA_GPIO_Port, &GPIO_InitStruct);
  } else {
    GPIO_InitStruct.Pin = I2C1_SDA_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;//LL_GPIO_PULL_UP;
    LL_GPIO_Init(BMA253_SDA_GPIO_Port, &GPIO_InitStruct);
  }
}

//==============================================================================
//* Function Name  : BMA253 SDA/SCL pin init
//* Description    : BMA253 SDA/SCL output high
//* Input          : None
//* Output         : None
//* Return         : None
//* Attention      : None
//==============================================================================
void BMA253_I2C_Init(void)
{
  LL_GPIO_InitTypeDef  GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin = I2C1_SDA_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(I2C1_SDA_GPIO_Port, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin = I2C1_SCL_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(I2C1_SCL_GPIO_Port, &GPIO_InitStruct);
  
  BMA253_SDA_LOW();
  BMA253_SCL_LOW();
  BMA253_I2C_Delay(_BMA253_I2C_DELAY_);
  BMA253_I2C_Delay(_BMA253_I2C_DELAY_);
  BMA253_SCL_HIGH();
  BMA253_I2C_Delay(_BMA253_I2C_DELAY_);
  BMA253_SDA_HIGH();
  BMA253_I2C_Delay(_BMA253_I2C_DELAY_);
}

//==============================================================================
//* Function Name  : IIC start
//* Description    : IIC start
//* Input          : None
//* Output         : None
//* Return         : None
//* Attention      : None
//==============================================================================
static void BMA253_I2C_Start(void)
{
  BMA253_SDA_HIGH();
  BMA253_I2C_Delay(_BMA253_I2C_DELAY_);
  BMA253_SCL_HIGH();
  BMA253_I2C_Delay(_BMA253_I2C_DELAY_);
  BMA253_SDA_LOW();
  BMA253_I2C_Delay(_BMA253_I2C_DELAY_);
  BMA253_SCL_LOW();
  BMA253_I2C_Delay(_BMA253_I2C_DELAY_);
}

//==============================================================================
//* Function Name  : IIC stop
//* Description    : IIC stop
//* Input          : None
//* Output         : None
//* Return         : None
//* Attention      : None
//==============================================================================
static void BMA253_I2C_Stop(void)
{
  BMA253_SDA_LOW();
  BMA253_I2C_Delay(_BMA253_I2C_DELAY_);
  BMA253_SCL_HIGH();
  BMA253_I2C_Delay(_BMA253_I2C_DELAY_);
  BMA253_SDA_HIGH();
  BMA253_I2C_Delay(_BMA253_I2C_DELAY_);
}

//==============================================================================
//* Function Name  : IIC ack
//* Description    : IIC ack
//* Input          : None
//* Output         : None
//* Return         : None
//* Attention      : None
//==============================================================================
static void BMA253_I2C_Ack(void)
{
  BMA253_SDA_LOW();
  BMA253_I2C_Delay(_BMA253_I2C_DELAY_);
  BMA253_SCL_LOW();
  BMA253_I2C_Delay(_BMA253_I2C_DELAY_);
  BMA253_SCL_HIGH();
  BMA253_I2C_Delay(_BMA253_I2C_DELAY_);
  BMA253_SCL_LOW();
  BMA253_I2C_Delay(_BMA253_I2C_DELAY_);
  BMA253_SDA_HIGH();
  BMA253_I2C_Delay(_BMA253_I2C_DELAY_);
}

//==============================================================================
//* Function Name  : IIC Nack
//* Description    : IIC Nack
//* Input          : None
//* Output         : None
//* Return         : None
//* Attention      : None
//==============================================================================
static void BMA253_I2C_NoAck(void)
{
  BMA253_SDA_HIGH();
  BMA253_I2C_Delay(_BMA253_I2C_DELAY_);
  BMA253_SCL_HIGH();
  BMA253_I2C_Delay(_BMA253_I2C_DELAY_);
  BMA253_SCL_LOW();
}

//==============================================================================
//* Function Name  : IIC WriteByte
//* Description    : IIC WriteByte
//* Input          : send_data
//* Output         : None
//* Return         : ACK
//* Attention      : None
//==============================================================================
u8 BMA253_I2C_WriteByte(u8 send_data)
{
  u8 bit_cnt;
  u8 b_ack = true;
  u8 i;

  for(bit_cnt=0; bit_cnt<8; bit_cnt++)
  {
    BMA253_SCL_LOW();
    if ((send_data << bit_cnt) & 0x80) {
      BMA253_SDA_HIGH();
    } else {
      BMA253_SDA_LOW();
    }
    BMA253_I2C_Delay(_BMA253_I2C_DELAY_);
    BMA253_SCL_HIGH();
    BMA253_I2C_Delay(_BMA253_I2C_DELAY_);
  }

  BMA253_SCL_LOW();
  BMA253_I2C_Delay(_BMA253_I2C_DELAY_);
  BMA253_SDA_HIGH();
  BMA253_I2C_Delay(_BMA253_I2C_DELAY_);

  //BMA253_I2C_Delay(_BMA253_I2C_DELAY_);
  BMA253_SCL_HIGH();
  BMA253_I2C_Delay(_BMA253_I2C_DELAY_);

  BMA253_SDAIN();
  i = 200;
  while (--i)
  {
    BMA253_I2C_Delay(_BMA253_I2C_DELAY_);
    if (LL_GPIO_IsInputPinSet(BMA253_SDA_GPIO_Port,BMA253_SDA_Pin) == 0) {
      break;
    }
  }
  BMA253_SDAOUT();

  if (i == 0) {
    b_ack = false;  //error
  }

  BMA253_SCL_LOW();
  BMA253_I2C_Delay(_BMA253_I2C_DELAY_);

  return b_ack;
}

//==============================================================================
//* Function Name  : IIC ReadByte
//* Description    : IIC ReadByte
//* Input          : None
//* Output         : None
//* Return         : Read data
//* Attention      : None
//==============================================================================
u8 BMA253_I2C_ReadByte(u8 ack)
{
  u8 read_value = 0;
  u8 bit_cnt;

  BMA253_SDAIN();
  for (bit_cnt=0; bit_cnt<8; bit_cnt++)
  {
    BMA253_SCL_HIGH();
    BMA253_I2C_Delay(_BMA253_I2C_DELAY_);
    read_value <<= 1;
    if (LL_GPIO_IsInputPinSet(BMA253_SDA_GPIO_Port,BMA253_SDA_Pin) == 1) {
      read_value += 1;
    }
    BMA253_SCL_LOW();
  }
  BMA253_SDAOUT();
  BMA253_SCL_LOW();
  BMA253_I2C_Delay(_BMA253_I2C_DELAY_);
  if (!ack) {
    BMA253_I2C_NoAck();
  } else {
    BMA253_I2C_Ack();
  }
  
  return (read_value);
}

/**
  * @brief  BMA253 Write byte
  * @param  device address
  * @param  register address
  * @param  register data
  * @retval status true/false
  */
u8 BMA253_WriteByte(u8 I2C_addr,u8 Reg_addr,u8 data)
{
    u8 status;
    
    BMA253_I2C_Start();
    status = BMA253_I2C_WriteByte(I2C_addr | 0X00);     //WRITE i2c
    if (status==false)
    {
        goto err;
    }
    
    status = BMA253_I2C_WriteByte(Reg_addr);
    if (status==false)
    {
        goto err;
    }
    
    status = BMA253_I2C_WriteByte(data);
err:
    BMA253_I2C_Stop();
    return status;
}

/**
  * @brief  BMA253 read xbyte
  * @param  device address
  * @param  register address
  * @param  register data
  * @retval status true/false
  */
u8 BMA253_ReadByte(u8 I2C_addr, u8 Reg_addr, u8 *rxByte)
{
    u8 status;
    *rxByte = 0x00;
    
    BMA253_I2C_Start();
    status = BMA253_I2C_WriteByte(I2C_addr | 0X00);        //WRITE i2c
    if (status==false)
    {
        goto err;
    }
    //delay_10ms();
    
    status = BMA253_I2C_WriteByte(Reg_addr);
    if (status==false)
    {
        goto err;
    }
    //delay_10ms();
    //stop();
    
    BMA253_I2C_Start();
    status = BMA253_I2C_WriteByte(I2C_addr | 0X01);        //READ
    if (status==false)
    {
        goto err;
    }
    //delay_10ms();
    
    *rxByte = BMA253_I2C_ReadByte(NOACK);
    
err:
    BMA253_I2C_Stop();
    
    return status;
}

/**
  * @brief  BMA253 write nbyte
  * @param  device address
  * @param  register address
  * @param  register data
  * @param  register length
  * @retval status true/false
  */
u8 BMA253_Write_LenBytes(u8 I2C_addr, u8 Reg_addr, u8 *buff, u8 len)
{
    u8 status;
    
    BMA253_I2C_Start();
    status = BMA253_I2C_WriteByte(I2C_addr | 0X00);        //WRITE i2c
    if (status==false)
    {
        goto err;
    }
    
    status = BMA253_I2C_WriteByte(Reg_addr);
    if (status==false)
    {
        goto err;
    }
    
    while(len)
    {
        status = BMA253_I2C_WriteByte(*buff);
        if (status==false) {
          goto err;
        }
        len--;
        buff++;
    }
    
err:
    BMA253_I2C_Stop();
    
    return status;
}

/**
  * @brief  BMA253 read nbyte
  * @param  device address
  * @param  register address
  * @param  register data
  * @param  register length
  * @retval status true/false
  */
u8 BMA253_Read_LenBytes(u8 I2C_addr, u8 Reg_addr, u8 *buff, u8 len)
{
    u8 status;
    
    BMA253_I2C_Start();
    status = BMA253_I2C_WriteByte(I2C_addr | 0X00);        //WRITE i2c
    if (status==false)
    {
        goto err;
    }
    
    status = BMA253_I2C_WriteByte(Reg_addr);
    if (status==false)
    {
        goto err;
    }
    //stop();
    
    BMA253_I2C_Start();
    status = BMA253_I2C_WriteByte(I2C_addr | 0X01);        //READ
    if (status == false)
    {
        goto err;
    }
    
    while(len)
    {
        if (len == 1)
        {
          *buff = BMA253_I2C_ReadByte(NOACK); //读数据,发送nACK
        }
        else
        {
          *buff = BMA253_I2C_ReadByte(ACK);   //读数据,发送ACK
        }
        len--;
        buff++;
    }
    
err:
    BMA253_I2C_Stop();
    
    return status;
}

#elif (_IIC_TYPE_ == _HARDWARE_IIC_)

#include "i2c.h"
//------------------------------------------------------------------------------
#define LIB_I2C_TIMEOUT         10000

void BMA253_I2C_Init(void)
{
  MX_I2C1_Init();
  LL_I2C_Disable(I2C1);   // Disable until needed
  LL_I2C_GenerateStopCondition(I2C1); // STOP
}

u8 BMA253_WriteByte(u8 I2C_addr,u8 Reg_addr,u8 data)
{
  u16 n;
  BOOLEAN status=true;
  
  LL_I2C_Enable(I2C1);    // Enable
  LL_I2C_SetSlaveAddr(I2C1, (I2C_addr << 0));
  LL_I2C_SetTransferRequest(I2C1, LL_I2C_REQUEST_WRITE);
  LL_I2C_SetTransferSize(I2C1, 2);    // Transfer of <ADDR><DATA> 
  LL_I2C_GenerateStartCondition(I2C1);    // START
  //LIB_I2C_POLLING(!(LL_I2C_IsActiveFlag_TXE(I2C1)));
  n = LIB_I2C_TIMEOUT;
  while (!(LL_I2C_IsActiveFlag_TXE(I2C1))) {
    if (--n == 0) {
      __nop();
      status = false;
    }
  }
  LL_I2C_TransmitData8(I2C1, Reg_addr);
  //LIB_I2C_POLLING(!(LL_I2C_IsActiveFlag_TXE(I2C1)));
  n = LIB_I2C_TIMEOUT;
  while (!(LL_I2C_IsActiveFlag_TXE(I2C1))) {
    if (--n == 0) {
      __nop();
      status = false;
    }
  }
  LL_I2C_TransmitData8(I2C1, data);
  //LIB_I2C_POLLING(!(LL_I2C_IsActiveFlag_TC(I2C1)));
  n = LIB_I2C_TIMEOUT;
  while ((LL_I2C_IsActiveFlag_TC(I2C1))) {
    if (--n == 0) {
      __nop();
      status = false;
    }
  }
  LL_I2C_GenerateStopCondition(I2C1); // STOP
  LL_I2C_Disable(I2C1);
  
  return status;
}

u8 BMA253_ReadByte(u8 I2C_addr, u8 Reg_addr, u8 *rxByte)
{
  u16 n;
  BOOLEAN status=true;
  
  LL_I2C_Enable(I2C1);    // Enable
  LL_I2C_SetSlaveAddr(I2C1, (I2C_addr << 0));
  LL_I2C_SetTransferRequest(I2C1, LL_I2C_REQUEST_WRITE);
  LL_I2C_SetTransferSize(I2C1, 1);    // First check target register
  LL_I2C_GenerateStartCondition(I2C1);    // START
  //LIB_I2C_POLLING(!(LL_I2C_IsActiveFlag_TXE(I2C1)));
  n = LIB_I2C_TIMEOUT;
  while (!(LL_I2C_IsActiveFlag_TXE(I2C1))) {
    if (--n == 0) {
      __nop();
      status = false;
    }
  }
  LL_I2C_TransmitData8(I2C1, Reg_addr);
  //LIB_I2C_POLLING(!(LL_I2C_IsActiveFlag_TC(I2C1)));
  n = LIB_I2C_TIMEOUT;
  while ((LL_I2C_IsActiveFlag_TC(I2C1))) {
    if (--n == 0) {
      __nop();
      status = false;
    }
  }
  LL_I2C_GenerateStopCondition(I2C1); // STOP
  LL_I2C_SetTransferRequest(I2C1, LL_I2C_REQUEST_READ);
  LL_I2C_SetTransferSize(I2C1, 1);    // Now request read of single byte from register
  LL_I2C_GenerateStartCondition(I2C1);    // START
  //LIB_I2C_POLLING(!(LL_I2C_IsActiveFlag_RXNE(I2C1)));
  n = LIB_I2C_TIMEOUT;
  while ((LL_I2C_IsActiveFlag_RXNE(I2C1))) {
    if (--n == 0) {
      __nop();
      status = false;
    }
  }
  *rxByte = LL_I2C_ReceiveData8(I2C1);
  //LIB_I2C_POLLING(!(LL_I2C_IsActiveFlag_TC(I2C1)));
  n = LIB_I2C_TIMEOUT;
  while ((LL_I2C_IsActiveFlag_TC(I2C1))) {
    if (--n == 0) {
      __nop();
      status = false;
    }
  }
  LL_I2C_GenerateStopCondition(I2C1); // STOP
  LL_I2C_Disable(I2C1);
  
  return status;
}

u8 BMA253_Write_LenBytes(u8 I2C_addr, u8 Reg_addr, u8 *buff, u8 len)
{
  u16 n;
  
  LL_I2C_Enable(I2C1); // Enable
  LL_I2C_SetSlaveAddr(I2C1, (I2C_addr << 0));
  LL_I2C_SetTransferRequest(I2C1, LL_I2C_REQUEST_WRITE);
  LL_I2C_SetTransferSize(I2C1, ++len); // Transfer of data_n bytes + addr byte
  LL_I2C_GenerateStartCondition(I2C1); // START
  //LIB_I2C_POLLING(!(LL_I2C_IsActiveFlag_TXE(I2C1)));
  n = LIB_I2C_TIMEOUT;
  while (!(LL_I2C_IsActiveFlag_TXE(I2C1))) {
    if (--n == 0) {
      return false;
    }
  }
  LL_I2C_TransmitData8(I2C1, Reg_addr);   // Send address
  //LIB_I2C_POLLING(!(LL_I2C_IsActiveFlag_TXE(I2C1)));
  n = LIB_I2C_TIMEOUT;
  while (!(LL_I2C_IsActiveFlag_TXE(I2C1))) {
    if (--n == 0) {
      return false;
    }
  }
  for(u8 i=1; i<len; i++)
  {
    LL_I2C_TransmitData8(I2C1, *buff++); // Push data buffer to slave device
    //LIB_I2C_POLLING(!(LL_I2C_IsActiveFlag_TXE(I2C1)));
    u16 n = LIB_I2C_TIMEOUT;
    while (!(LL_I2C_IsActiveFlag_TXE(I2C1))) {
      if (--n == 0) {
        return false;
      }
    }
  }
  //LIB_I2C_POLLING(!(LL_I2C_IsActiveFlag_TC(I2C1)));
  n = LIB_I2C_TIMEOUT;
  while (!(LL_I2C_IsActiveFlag_TC(I2C1))) {
    if (--n == 0) {
      return false;
    }
  }
  LL_I2C_GenerateStopCondition(I2C1); // STOP
  LL_I2C_Disable(I2C1);
  return true;
}

u8 BMA253_Read_LenBytes(u8 I2C_addr, u8 Reg_addr, u8 *buff, u8 len)
{
  u16 n;
  
  LL_I2C_Enable(I2C1);    // Enable
  LL_I2C_SetSlaveAddr(I2C1, (I2C_addr << 0));
  LL_I2C_SetTransferRequest(I2C1, LL_I2C_REQUEST_WRITE);
  LL_I2C_SetTransferSize(I2C1, 1);    // Request check target register
  LL_I2C_GenerateStartCondition(I2C1);    // START
  //LIB_I2C_POLLING(!(LL_I2C_IsActiveFlag_TXE(I2C1)));
  n = LIB_I2C_TIMEOUT;
  while (!(LL_I2C_IsActiveFlag_TXE(I2C1))) {
    if (--n == 0) {
      return false;
    }
  }
  LL_I2C_TransmitData8(I2C1, Reg_addr);   // Send slave register address
  //LIB_I2C_POLLING(!(LL_I2C_IsActiveFlag_TC(I2C1)));
  n = LIB_I2C_TIMEOUT;
  while (!(LL_I2C_IsActiveFlag_TC(I2C1))) {
    if (--n == 0) {
      return false;
    }
  }
  LL_I2C_GenerateStopCondition(I2C1); // STOP
  LL_I2C_SetTransferRequest(I2C1, LL_I2C_REQUEST_READ);
  LL_I2C_SetTransferSize(I2C1, len); // Now request reading of data_n bytes
  LL_I2C_GenerateStartCondition(I2C1);    // START
  for(u8 i=len; i>0; i--)
  {
      //LIB_I2C_POLLING(!(LL_I2C_IsActiveFlag_RXNE(I2C1)));
    u16 n = LIB_I2C_TIMEOUT;
    while (!(LL_I2C_IsActiveFlag_RXNE(I2C1))) {
      if (--n == 0) {
        return false;
      }
    }
    *buff++ = LL_I2C_ReceiveData8(I2C1);   // Pop data from slave device
  }
  //LIB_I2C_POLLING(!(LL_I2C_IsActiveFlag_TC(I2C1)));
  n = LIB_I2C_TIMEOUT;
  while (!(LL_I2C_IsActiveFlag_TC(I2C1))) {
    if (--n == 0) {
      return false;
    }
  }
  LL_I2C_GenerateStopCondition(I2C1); // STOP
  LL_I2C_Disable(I2C1);
  return true;
}


/*
void BMA253_I2C_Init(void)
{
  MX_I2C1_Init();
  LL_I2C_Disable(I2C1);   // Disable until needed
  LL_I2C_GenerateStopCondition(I2C1); // STOP
}

u8 BMA253_WriteByte(u8 I2C_addr,u8 Reg_addr,u8 data)
{
  if(LL_I2C_IsActiveFlag_BUSY(I2C1))return 0;

  //Generate Start Signal, and wait for it complete
  LL_I2C_GenerateStartCondition(I2C1);
  while(!LL_I2C_IsActiveFlag_SB(I2C1))continue;

  LL_I2C_TransmitData8(I2C1, (I2C_addr << 0) & I2C_REQUEST_WRITE);
  while(!LL_I2C_IsActiveFlag_ADDR(I2C1))continue;
  LL_I2C_ClearFlag_ADDR(I2C1);

  while(!LL_I2C_IsActiveFlag_TXE(I2C1))continue;
  LL_I2C_TransmitData8(I2C1, data);

  //wait until the last transfer complete
  while(!LL_I2C_IsActiveFlag_BTF(I2C1))continue;

  LL_I2C_GenerateStopCondition(I2C1);
  //Wait until bus release
  while(LL_I2C_IsActiveFlag_BUSY(I2C1))continue;

  return data;
}

u8 BMA253_ReadByte(u8 I2C_addr, u8 Reg_addr, u8 *rxByte)
{
  if(LL_I2C_IsActiveFlag_BUSY(I2C1))return 0;

  //Generate Start Signal, and wait for it complete
  LL_I2C_GenerateStartCondition(I2C1);
  while(!LL_I2C_IsActiveFlag_SB(I2C1))continue;

  //Transfer device Addr for read, and wait for it complete
  LL_I2C_TransmitData8(I2C1, (I2C_addr << 0) | I2C_REQUEST_READ);
  while(!LL_I2C_IsActiveFlag_ADDR(I2C1))continue;
  LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_NACK);
  LL_I2C_ClearFlag_ADDR(I2C1);

  LL_I2C_GenerateStopCondition(I2C1);
  while(!LL_I2C_IsActiveFlag_RXNE(I2C1))continue;
  uint8_t data = LL_I2C_ReceiveData8(I2C1);

  //Wait until bus release
  while(LL_I2C_IsActiveFlag_BUSY(I2C1))continue;

  return data;
}*/

#endif

#endif
