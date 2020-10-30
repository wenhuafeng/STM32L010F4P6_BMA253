

#include <stdint.h>
#include <string.h>
#include "main.h"
#include "TypeDefine.h"
#include "accelerometer.h"
#include "W25Q128.h"
#include "ArrayQueue.h"

#include "accelerometer_bma253_iic.h"
#include "GsensorToSflash_Process.h"

#if (_GSENSOR_TO_SPI_FLASH_)

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#define USART1_RXBUFF_SIZE      20
#define USART1_TXBUFF_SIZE      256

BOOLEAN F_RxComplete;
BOOLEAN F_TxComplete;
char Usart1_RxBuff[USART1_RXBUFF_SIZE];
uint8_t Usart1_TxBuff[USART1_TXBUFF_SIZE];

/**
  * @brief  DMA TX ISR handler
  * @param  NONE
  * @retval NONE
  */
void DMA_ISR_Callback(void)
{
  if (LL_DMA_IsEnabledIT_TC(DMA1, LL_DMA_CHANNEL_4) && LL_DMA_IsActiveFlag_TC4(DMA1)) {
    LL_LPUART_DisableDMAReq_TX(LPUART1);
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4);
    LL_DMA_ClearFlag_TC4(DMA1);             /* Clear transfer complete flag */
  
    //LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_4, (uint32_t)(&LPUART1->TDR));// LL_USART_DMA_GetRegAddr(USART1->DR));
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_4, (uint32_t)Usart1_TxBuff);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, USART1_TXBUFF_SIZE);
    //LL_DMA_ClearFlag_TC4(DMA1);
    //LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_4);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);
    F_TxComplete = TRUE;
    
  }
}

/**
  * @brief  DMA config
  * @param  NONE
  * @retval NONE
  */
void LPUART_DMA_CONFIG(void)
{
  //RX
  LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_3, (uint32_t)(&LPUART1->RDR));// LL_USART_DMA_GetRegAddr(USART1->DR));
  LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_3, (uint32_t)Usart1_RxBuff);
  LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, USART1_RXBUFF_SIZE);
  //LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_3);
  LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);
  LL_LPUART_EnableDMAReq_RX(LPUART1);
  //LL_LPUART_ClearFlag_IDLE(LPUART1);
  LL_LPUART_EnableIT_IDLE(LPUART1);
  //TX
  LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_4, (uint32_t)(&LPUART1->TDR));// LL_USART_DMA_GetRegAddr(USART1->DR));
  LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_4, (uint32_t)Usart1_TxBuff);
  LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, USART1_TXBUFF_SIZE);
  LL_DMA_ClearFlag_TC4(DMA1);
  LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_4);
  LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);
  //LL_LPUART_EnableDMAReq_TX(LPUART1);
  
  LL_LPUART_Enable(LPUART1);
  while (!LL_LPUART_IsActiveFlag_TEACK(LPUART1) || !LL_LPUART_IsActiveFlag_REACK(LPUART1)) {}
}

void LPUART_RxIdleCallback(void)
{
  u8 cnt;
  
  if (LL_LPUART_IsEnabledIT_IDLE(LPUART1) && LL_LPUART_IsActiveFlag_IDLE(LPUART1)) {
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3); //¹Ø±ÕDMA
    cnt = LL_DMA_GetDataLength(DMA1,LL_DMA_CHANNEL_3);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, USART1_RXBUFF_SIZE);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);
    LL_LPUART_ClearFlag_IDLE(LPUART1);
    F_RxComplete = TRUE;
  }
}

/*
void LPUART_TxCompleteCallback(void)
{
  if (LL_LPUART_IsActiveFlag_TC(LPUART1))
  {
    //LL_LPUART_DisableIT_TC(LPUART1);
    //LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4);
    LL_LPUART_ClearFlag_TC(LPUART1);
    F_TxComplete = TRUE;
  }
}*/

//------------------------------------------------------------------------------

BOOLEAN F_32HZ;
BOOLEAN F_32HZ_1;
BOOLEAN F_QueueFull;
BOOLEAN F_Sflash_Full;

BOOLEAN F_250MS;
BOOLEAN F_500MS;
BOOLEAN F_1S;
BOOLEAN F_2S;

BOOLEAN F_EraseChip;
BOOLEAN F_GetData;
BOOLEAN F_LowBatt;

u8 Buffer[ARRAY_QUEUE_MAXSIZE];
u32 Address_start;
u32 Address_current = 0x00;
#define _FLASH_SIZE_    (16*1024*1024)

//------------------------------------------------------------------------------
/**
  * @brief  Low power timer interrupt (31.25ms)
  * @param  device address
  * @param  register address
  * @param  register data
  * @retval status true/false
  */
void LP_TimeHandle(void)
{
  static u8 Ctr,Ctr1,Ctr2;
  static u8 Ctr3;
  
  F_32HZ = 1;
  F_32HZ_1 = 1;
  
  Ctr++;
  if (Ctr > 7) {
    Ctr = 0x00;
    F_250MS = 1;
  }
  Ctr1++;
  if (Ctr1 > 15) {
    Ctr1 = 0x00;
    F_500MS = 1;
  }
  Ctr2++;
  if (Ctr2 > 31) {
    Ctr2 = 0x00;
    F_1S = 1;
  }
  Ctr3++;
  if (Ctr3 > 63) {
    Ctr3 = 0x00;
    F_2S = 1;
  }
}

//------------------------------------------------------------------------------
void DataEnterQueue(u8 Data)
{
  if (EnQueue(Data) == FALSE)
  {
    //Queue if full
    QueueToBuffer(Buffer);
    InitQueue();
    F_QueueFull = TRUE;
  }
}

//------------------------------------------------------------------------------
void AccelerometerDataGet(void)
{
  int ret;
  int16_t x, y, z;
  uint8_t data[6];

  if (F_32HZ)
  {
    F_32HZ = FALSE;
    //TEST
    //LED_Blink();
    //TEST
    if (F_EraseChip) return;
    //if (F_GetData) return;
    
    ret = accelerometer_accel_get(&x, &y, &z);
    if (ret == true)
    {
      // Convert to network order
      data[0] = (uint8_t)(x >> 8);
      DataEnterQueue(data[0]);
      
      data[1] = (uint8_t)(x & 0xff);
      DataEnterQueue(data[1]);
      
      data[2] = (uint8_t)(y >> 8);
      DataEnterQueue(data[2]);
      
      data[3] = (uint8_t)(y & 0xff);
      DataEnterQueue(data[3]);
      
      data[4] = (uint8_t)(z >> 8);
      DataEnterQueue(data[4]);
      
      data[5] = (uint8_t)(z & 0xff);
      DataEnterQueue(data[5]);
    }
  }
}

void WriteDataToSflash(void)
{
  if (F_Sflash_Full == FALSE) {
    if (F_QueueFull) {
      F_QueueFull = FALSE;
      if (Address_current < _FLASH_SIZE_) {
        W25QXX_Write_NoCheck(&Buffer[0], Address_current, ARRAY_QUEUE_MAXSIZE);
        Address_current += ARRAY_QUEUE_MAXSIZE;
      } else {
        F_Sflash_Full = TRUE;
      }
    }
  } else {
    if (F_250MS) {
      F_250MS = FALSE;
      LED_Blink();
    }
  }
}

void AT_command_process(char *cRxBuf)
{
  char *buf;
  
  buf = "AT+RST";
  if (strstr(cRxBuf,buf) != NULL) {
    W25QXX_Erase_Chip();
    F_EraseChip = 1;
    
    u8 i=10*4;
    do
    {
      if (F_250MS) {
        F_250MS = 0;
        LED_Blink();
        i--;
        if (i == 0) break;
      }
    } while (1);
    LED_OFF();
  }
  
  buf = "AT+GET";
  if (strstr(cRxBuf,buf) != NULL) {
    Address_start = 0x00;
    F_TxComplete = TRUE;
    do
    {
      if (F_32HZ_1) {
        F_32HZ_1 = 0;
        LED_Blink();
      }
      if (F_TxComplete == TRUE) {
        F_TxComplete = FALSE;
        W25QXX_Read(Usart1_TxBuff, Address_start, ARRAY_QUEUE_MAXSIZE);
        LL_LPUART_EnableDMAReq_TX(LPUART1);
        Address_start += ARRAY_QUEUE_MAXSIZE;
      }
    } while (Address_start < Address_current);
    F_GetData = 1;
    LED_OFF();
  }
}

void LPUART_DMA_Send_Test(void)
{
  u8 i;
  
  if (F_2S) {
    F_2S = 0;
    
    i = 0;
    do
    {
      Usart1_TxBuff[i] = i;
      i++;
    } while (i < (USART1_TXBUFF_SIZE-1));
    Usart1_TxBuff[i] = i;
    
    //for (i=0; i<USART1_TXBUFF_SIZE; i++)
    //{
    //  Usart1_TxBuff[i] = i;
    //}
    //Usart1_TxBuff[i] = i;
    
    LL_LPUART_EnableDMAReq_TX(LPUART1);
  }
}

void LowPowerDetect(void)
{
  u8 i=0;
  u8 j=0;
  
  do
  {
    if (LL_GPIO_IsInputPinSet(LB_detect_GPIO_Port,LB_detect_Pin) == 0) {
      i++;
    }
    j++;
  } while (j < 30);
  
  if (i > 25) {
    F_LowBatt = 1;
  }
}

void GsensorToSflash_Process(void)
{
  if (F_RxComplete) {
    F_RxComplete = FALSE;
    AT_command_process(Usart1_RxBuff);
  }
  
  LowPowerDetect();
  //if (F_LowBatt == 0) {
    AccelerometerDataGet();
    WriteDataToSflash();
  //} else {
  //  if (F_500MS) {
  //    F_500MS = 0;
  //    LED_Blink();
  //  }
  //}
  //LPUART_DMA_Send_Test();
}

#endif

//==============================================================================
//==============================================================================
