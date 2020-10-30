
/******************************************************************************
 * @file    vcom.c
 * @author  MCD Application Team
 * @version V1.1.2
 * @date    08-September-2017
 * @brief   manages virtual com port
 ******************************************************************************
 */

#include <stdarg.h>
#include "vcom.h"
#include "main.h"
#include "TypeDefine.h"
//#include "hw_gpio.h"
//#include <stdarg.h>
#include "tiny_vsnprintf.h"
//#include "low_power.h"
//#include "command.h"

/* Force include of hal uart in order to inherite HAL_UART_StateTypeDef definition */
//#include "stm32l0xx_hal_dma.h"
//#include "stm32l0xx_hal_uart.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* based on UART_HandleTypeDef */
static struct {
  char buffTx[256];                   /**< buffer to transmit */
  char buffRx[256];                   /**< Circular buffer of received chars */
  int rx_idx_free;                    /**< 1st free index in BuffRx */
  int rx_idx_toread;                  /**< next char to read in buffRx, when not rx_idx_free */
  HW_LockTypeDef Lock;                /**< Locking object */

  //__IO HAL_UART_StateTypeDef gState;  /**< UART state information related to global Handle management
  //                                         and also related to Tx operations. */
  //__IO HAL_UART_StateTypeDef RxState; /**< UART state information related to Rx operations. */
} uart_context;

/* Private function prototypes -----------------------------------------------*/
/**
 * @brief  Transmit uart_context.buffTx from start to len - 1
 * @param  1st index to transmit
 * @param  Last index not to transmit
 * @return Last index not transmitted
 */
static int buffer_transmit(int start, int len);

/**
 * @brief  Takes one character that has been received and save it in uart_context.buffRx
 * @param  received character
 */
static void receive(char rx);


/* Functions Definition ------------------------------------------------------*/
//#define USART1_RXBUFF_SIZE      20
//#define USART1_TXBUFF_SIZE      256

//BOOLEAN F_RxComplete;
//BOOLEAN F_TxComplete;
//char Usart1_RxBuff[USART1_RXBUFF_SIZE];
//uint8_t Usart1_TxBuff[USART1_TXBUFF_SIZE];



/**
  * @brief  DMA TX ISR handler
  * @param  NONE
  * @retval NONE
  */
//void DMA_ISR_Callback(void)
//{
//  if (LL_DMA_IsEnabledIT_TC(DMA1, LL_DMA_CHANNEL_4) && LL_DMA_IsActiveFlag_TC4(DMA1)) {
//    LL_LPUART_DisableDMAReq_TX(LPUART1);
//    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4);
//    LL_DMA_ClearFlag_TC4(DMA1);             /* Clear transfer complete flag */
//  
//    //LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_4, (uint32_t)(&LPUART1->TDR));// LL_USART_DMA_GetRegAddr(USART1->DR));
//    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_4, (uint32_t)Usart1_TxBuff);
//    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, USART1_TXBUFF_SIZE);
//    //LL_DMA_ClearFlag_TC4(DMA1);
//    //LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_4);
//    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);
//    F_TxComplete = TRUE;
//    
//  }
//}

//u8 cnt;
//void LPUART_RxIdleCallback(void)
//{
//  if (LL_LPUART_IsEnabledIT_IDLE(LPUART1) && LL_LPUART_IsActiveFlag_IDLE(LPUART1)) {
//    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3); //¹Ø±ÕDMA
//    cnt = LL_DMA_GetDataLength(DMA1,LL_DMA_CHANNEL_3);
//    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, USART1_RXBUFF_SIZE);
//    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);
//    LL_LPUART_ClearFlag_IDLE(LPUART1);
//    F_RxComplete = TRUE;
//  }
//  else if (LL_LPUART_IsEnabledIT_WKUP(LPUART1) && LL_LPUART_IsActiveFlag_WKUP(LPUART1)) {
//    LL_LPUART_ClearFlag_WKUP(LPUART1);
//    F_LPUART1_WKUP = TRUE;
//    TaskStep_WKUPDelayCtr = _TASK_STEP_LPUART_WKUP_DELAY_;
//    if (!LL_LPTIM_IsEnabled(LPTIM1)) {
//      MX_LPTIM1_Init();
//      LPTIM1_Counter_Start_IT();
//      __ASM("nop");
//    }
//  }
//}

//void vcom_DMA_Init(void)
//{
//  //RX
//  LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_3, (uint32_t)(&LPUART1->RDR));
//  LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_3, (uint32_t)Usart1_RxBuff);
//  LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, USART1_RXBUFF_SIZE);
//  //LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_3);
//  LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);
//  LL_LPUART_EnableDMAReq_RX(LPUART1);
//  LL_LPUART_ClearFlag_IDLE(LPUART1);
//  LL_LPUART_EnableIT_IDLE(LPUART1);
//  
//  /* Configuring the LPUART specific LP feature - the wakeup from STOP */
//  LL_LPUART_EnableInStopMode(LPUART1);
//  /* USART1 interrupt Init */
//  LL_LPUART_SetWKUPType(LPUART1, LL_LPUART_WAKEUP_ON_STARTBIT);
//  //LL_LPUART_SetWKUPType(LPUART1, LL_LPUART_WAKEUP_ON_RXNE);
//  //LL_LPUART_EnableIT_RXNE(LPUART1);
//  LL_LPUART_EnableIT_WKUP(LPUART1);
//  
//  //TX
//  LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_4, (uint32_t)(&LPUART1->TDR));
//  LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_4, (uint32_t)Usart1_TxBuff);
//  LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, USART1_TXBUFF_SIZE);
//  LL_DMA_ClearFlag_TC4(DMA1);
//  LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_4);
//  LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);
//  //LL_LPUART_EnableDMAReq_TX(LPUART1);
//  
//  LL_LPUART_Enable(LPUART1);
//  while (!LL_LPUART_IsActiveFlag_TEACK(LPUART1) || !LL_LPUART_IsActiveFlag_REACK(LPUART1)) {}
//}

void vcom_Init(void)
{
  /* Configuring the LPUART specific LP feature - the wakeup from STOP */
  LL_LPUART_EnableInStopMode(LPUART1);

  /* WakeUp from stop mode on start bit detection*/
  LL_LPUART_SetWKUPType(LPUART1, LL_LPUART_WAKEUP_ON_STARTBIT);
  //LL_LPUART_SetWKUPType(UARTX, LL_LPUART_WAKEUP_ON_RXNE);
  //LL_LPUART_EnableIT_RXNE(LPUART1);
  LL_LPUART_EnableIT_WKUP(LPUART1);

  LL_LPUART_Enable(LPUART1);
  while (LL_LPUART_IsActiveFlag_TEACK(LPUART1) == RESET)
  {
    ;
  }
  while (LL_LPUART_IsActiveFlag_REACK(LPUART1) == RESET)
  {
    ;
  }
  
  //uart_context.gState = HAL_UART_STATE_READY;
  //uart_context.RxState = HAL_UART_STATE_READY;
}

void vcom_ReceiveInit(void)
{
  //if (uart_context.RxState != HAL_UART_STATE_READY)
  //{
  //  return;
  //}

  /* Process Locked */
  HW_LOCK(&uart_context);

  //uart_context.RxState = HAL_UART_STATE_BUSY_RX;

  /* Enable the UART Parity Error Interrupt */
  LL_LPUART_EnableIT_PE(LPUART1);

  /* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
  LL_LPUART_EnableIT_ERROR(LPUART1);

  /* Process Unlocked */
  HW_UNLOCK(&uart_context);
}

void vcom_Send(const char *format, ...)
{
  va_list args;
  static __IO uint16_t len = 0;
  uint16_t current_len;
  int start;
  int stop;

  va_start(args, format);

  //BACKUP_PRIMASK();
  DISABLE_IRQ();
  if (len != 0)
  {
    if (len != sizeof(uart_context.buffTx))
    {
      current_len = len; /* use current_len instead of volatile len in below computation */
      len = current_len + tiny_vsnprintf_like(uart_context.buffTx + current_len, \
                                              sizeof(uart_context.buffTx) - current_len, format, args);
    }
    ENABLE_IRQ();
    //RESTORE_PRIMASK();
    va_end(args);
    return;
  }
  else
  {
    len = tiny_vsnprintf_like(uart_context.buffTx, sizeof(uart_context.buffTx), format, args);
  }

  current_len = len;
  ENABLE_IRQ();
  //RESTORE_PRIMASK();

  start = 0;
  
  do
  {
    stop = buffer_transmit(start, current_len);

    {
      //BACKUP_PRIMASK();
      DISABLE_IRQ();
      if (len == stop)
      {
        len = 0;
      }
      else
      {
        start = stop;
        current_len = len;
      }
      ENABLE_IRQ();
      //RESTORE_PRIMASK();
    }
  } while (current_len != stop);

  va_end(args);
}

FlagStatus IsNewCharReceived(void)
{
  FlagStatus status;
  
  //BACKUP_PRIMASK();
  DISABLE_IRQ();
  
  status = ((uart_context.rx_idx_toread == uart_context.rx_idx_free) ? RESET : SET);
  
  ENABLE_IRQ();
  //RESTORE_PRIMASK();
  
  return status;
}

uint8_t GetNewChar(void)
{
  uint8_t NewChar;

  //BACKUP_PRIMASK();
  DISABLE_IRQ();

  NewChar = uart_context.buffRx[uart_context.rx_idx_toread];
  uart_context.rx_idx_toread = (uart_context.rx_idx_toread + 1) % sizeof(uart_context.buffRx);

  ENABLE_IRQ();
  //RESTORE_PRIMASK();
  
  return NewChar;
}

void vcom_IRQHandler(void)
{
  int rx_ready = 0;
  char rx;
  
  /* UART Wake Up interrupt occured ------------------------------------------*/
  if (LL_LPUART_IsActiveFlag_WKUP(LPUART1) && (LL_LPUART_IsEnabledIT_WKUP(LPUART1) != RESET))
  {
    LL_LPUART_ClearFlag_WKUP(LPUART1);

    /* forbid stop mode */
    //LowPower_Disable(e_LOW_POWER_UART);

    /* Enable the UART Data Register not empty Interrupt */
    LL_LPUART_EnableIT_RXNE(LPUART1);
  }

  if (LL_LPUART_IsActiveFlag_RXNE(LPUART1) && (LL_LPUART_IsEnabledIT_RXNE(LPUART1) != RESET))
  {
    /* no need to clear the RXNE flag because it is auto cleared by reading the data*/
    rx = LL_LPUART_ReceiveData8(LPUART1);
    rx_ready = 1;
    
    /* allow stop mode*/
    //LowPower_Enable(e_LOW_POWER_UART);
  }

  if (LL_LPUART_IsActiveFlag_PE(LPUART1) || LL_LPUART_IsActiveFlag_FE(LPUART1) || LL_LPUART_IsActiveFlag_ORE(LPUART1) || LL_LPUART_IsActiveFlag_NE(LPUART1))
  {
    PRINTF("Error when receiving\n");
    /* clear error IT */
    LL_LPUART_ClearFlag_PE(LPUART1);
    LL_LPUART_ClearFlag_FE(LPUART1);
    LL_LPUART_ClearFlag_ORE(LPUART1);
    LL_LPUART_ClearFlag_NE(LPUART1);
    
    rx = AT_ERROR_RX_CHAR;
    rx_ready = 1;
  }
  
  if (rx_ready == 1)
  {
    receive(rx);
  }
}

/* Private functions Definition ------------------------------------------------------*/

static int buffer_transmit(int start, int len)
{
  int i;
  
  for (i = start; i < len; i++)
  {
    LL_LPUART_ClearFlag_TC(LPUART1);
    LL_LPUART_TransmitData8(LPUART1, uart_context.buffTx[i]);

    while (LL_LPUART_IsActiveFlag_TC(LPUART1) != SET)
    {
      ;
    }
  }
  LL_LPUART_ClearFlag_TC(LPUART1);
  return len;
}

static void receive(char rx)
{
  int next_free;

  /** no need to clear the RXNE flag because it is auto cleared by reading the data*/
  uart_context.buffRx[uart_context.rx_idx_free] = rx;
  next_free = (uart_context.rx_idx_free + 1) % sizeof(uart_context.buffRx);
  if (next_free != uart_context.rx_idx_toread) {
    /* this is ok to read as there is no buffer overflow in input */
    uart_context.rx_idx_free = next_free;
  } else {
    /* force the end of a command in case of overflow so that we can process it */
    uart_context.buffRx[uart_context.rx_idx_free] = '\r';
    PRINTF("uart_context.buffRx buffer overflow %d\r\n");
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
