#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "main.h"
#include "gpio.h"
#include "i2c.h"
#include "usart.h"
#include "lptim.h"
#include "rtc.h"
#include "vcom.h"
#include "common.h"

#define RX_BUFF_SIZE 20
#define TX_BUFF_SIZE 100

bool F_rxComplete;
bool F_txComplete;
char g_usart1RxBuffer[RX_BUFF_SIZE];
uint8_t g_usart1TxBuffer[TX_BUFF_SIZE];

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
        LL_DMA_ClearFlag_TC4(DMA1);

        //LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_4, (uint32_t)(&LPUART1->TDR));
        LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_4, (uint32_t)g_usart1TxBuffer);
        LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, TX_BUFF_SIZE);
        //LL_DMA_ClearFlag_TC4(DMA1);
        //LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_4);
        LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);
        F_txComplete = true;
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
    LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_3, (uint32_t)(&LPUART1->RDR));
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_3, (uint32_t)g_usart1RxBuffer);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, RX_BUFF_SIZE);
    //LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_3);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);
    LL_LPUART_EnableDMAReq_RX(LPUART1);
    //LL_LPUART_ClearFlag_IDLE(LPUART1);
    LL_LPUART_EnableIT_IDLE(LPUART1);

    //TX
    LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_4, (uint32_t)(&LPUART1->TDR));
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_4, (uint32_t)g_usart1TxBuffer);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, TX_BUFF_SIZE);
    LL_DMA_ClearFlag_TC4(DMA1);
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_4);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);
    //LL_LPUART_EnableDMAReq_TX(LPUART1);

    LL_LPUART_Enable(LPUART1);
    while (!LL_LPUART_IsActiveFlag_TEACK(LPUART1) || !LL_LPUART_IsActiveFlag_REACK(LPUART1)) {
    }
}

/*
void LPUART_RxIdleCallback(void)
{
    uint8_t cnt;

    if (LL_LPUART_IsEnabledIT_IDLE(LPUART1) && LL_LPUART_IsActiveFlag_IDLE(LPUART1)) {
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);
        cnt = LL_DMA_GetDataLength(DMA1, LL_DMA_CHANNEL_3);
        LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, RX_BUFF_SIZE);
        LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);
        LL_LPUART_ClearFlag_IDLE(LPUART1);
        F_rxComplete = true;
    }
}

void LPUART_TxCompleteCallback(void)
{
  if (LL_LPUART_IsActiveFlag_TC(LPUART1))
  {
    //LL_LPUART_DisableIT_TC(LPUART1);
    //LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4);
    LL_LPUART_ClearFlag_TC(LPUART1);
    F_txComplete = true;
  }
}*/

static bool F_32HZ;
static bool F_250MS;
static bool F_500MS;
static bool F_1S;
static bool F_2S;

void LP_TimeHandle(void)
{
    static uint8_t ctr0 = 0x00;
    static uint8_t ctr1 = 0x00;
    static uint8_t ctr2 = 0x00;
    static uint8_t ctr3 = 0x00;

    F_32HZ = true;

    ctr0++;
    if (ctr0 > 7) {
        ctr0    = 0x00;
        F_250MS = 1;
    }
    ctr1++;
    if (ctr1 > 15) {
        ctr1    = 0x00;
        F_500MS = 1;
        LED_BLINK();
    }
    ctr2++;
    if (ctr2 > 31) {
        ctr2 = 0x00;
        F_1S = 1;
    }
    ctr3++;
    if (ctr3 > 63) {
        ctr3 = 0x00;
        F_2S = 1;
    }
}

bool Get32HzFlag(void)
{
    return F_32HZ;
}

void Set32HzFlag(bool flag)
{
    F_32HZ = flag;
}

bool GetOneSecondFlag(void)
{
    return F_1S;
}

void SetOneSecondFlag(bool flag)
{
    F_1S = flag;
}

void SysInit(void)
{
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_LPUART1_UART_Init();
    MX_RTC_Init();
    MX_LPTIM1_Init();
    LPTIM1_CounterStartIT();
    vcom_Init();
    vcom_ReceiveInit();
}

//#define LPTIM1_INT_TIME (32768-1) //32768/32768 = 1S
//#define LPTIM1_INT_TIME (8192-1) //8192/32768 = 0.25S
#define LPTIM1_INT_TIME (1024 - 1) //1024/32768 = 0.03125S

void LPTIM1_CounterStartIT(void)
{
    /* Enable the Autoreload match Interrupt */
    LL_LPTIM_EnableIT_ARRM(LPTIM1);
    /* Enable the LPTIM1 counter */
    LL_LPTIM_Enable(LPTIM1);

    /* Set the Autoreload value */
    LL_LPTIM_SetAutoReload(LPTIM1, LPTIM1_INT_TIME);

    /* Start the LPTIM counter in continuous mode */
    LL_LPTIM_StartCounter(LPTIM1, LL_LPTIM_OPERATING_MODE_CONTINUOUS);
}

void EnterStopMode(void)
{
    LL_PWR_EnableUltraLowPower();
    LL_PWR_EnableFastWakeUp();
    LL_PWR_SetPowerMode(LL_PWR_MODE_STOP);
    LL_PWR_SetRegulModeLP(LL_PWR_REGU_LPMODES_LOW_POWER);
    LL_PWR_ClearFlag_WU();
    LL_LPM_EnableDeepSleep();

    __WFI();
}

int fputc(int ch, FILE *stream)
{
    while (!LL_LPUART_IsActiveFlag_TXE(LPUART1));
    LL_LPUART_TransmitData8(LPUART1, (uint8_t)ch);
    while (!LL_LPUART_IsActiveFlag_TC(LPUART1));

    return ch;
}
