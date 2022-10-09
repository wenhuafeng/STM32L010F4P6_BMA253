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

#define SOFTWARE_VERSION "V101"

#define RX_BUFF_SIZE 20
#define TX_BUFF_SIZE 100

bool f_rxComplete;
bool f_txComplete;
char g_usart1RxBuffer[RX_BUFF_SIZE];
uint8_t g_usart1TxBuffer[TX_BUFF_SIZE];

uint8_t *GetTxBuffer(void)
{
    return g_usart1TxBuffer;
}

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
        f_txComplete = true;
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
        f_rxComplete = true;
    }
}

void LPUART_TxCompleteCallback(void)
{
  if (LL_LPUART_IsActiveFlag_TC(LPUART1))
  {
    //LL_LPUART_DisableIT_TC(LPUART1);
    //LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4);
    LL_LPUART_ClearFlag_TC(LPUART1);
    f_txComplete = true;
  }
}*/

#define TIME_250MS 7
#define TIME_500MS 15
#define TIME_1S 31
#define TIME_2S 63

static bool f_32hz;
static bool f_32hz_1;
static bool f_250ms;
static bool f_500ms;
static bool f_1s;
static bool f_2s;

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

void LPTIM1_IsrHandle(void)
{
    static uint8_t ctr0 = 0x00;
    static uint8_t ctr1 = 0x00;
    static uint8_t ctr2 = 0x00;
    static uint8_t ctr3 = 0x00;

    if (LL_LPTIM_IsActiveFlag_ARRM(LPTIM1) == false) {
        return;
    }
    LL_LPTIM_ClearFLAG_ARRM(LPTIM1);

    f_32hz = true;
    f_32hz_1 = true;

    ctr0++;
    if (ctr0 > TIME_250MS) {
        ctr0    = 0x00;
        f_250ms = true;
    }
    ctr1++;
    if (ctr1 > TIME_500MS) {
        ctr1    = 0x00;
        f_500ms = true;
        LED_BLINK();
    }
    ctr2++;
    if (ctr2 > TIME_1S) {
        ctr2 = 0x00;
        f_1s = true;
    }
    ctr3++;
    if (ctr3 > TIME_2S) {
        ctr3 = 0x00;
        f_2s = true;
    }
}

bool Get32HzFlag(void)
{
    return f_32hz;
}

void Set32HzFlag(bool flag)
{
    f_32hz = flag;
}

bool Get32HzTwoFlag(void)
{
    return f_32hz_1;
}

void Set32HzTwoFlag(bool flag)
{
    f_32hz_1 = flag;
}

void Set250msFlag(bool flag)
{
    f_250ms = flag;
}

bool Get250msFlag(void)
{
    return f_250ms;
}

void Set500msFlag(bool flag)
{
    f_500ms = flag;
}

bool Get500msFlag(void)
{
    return f_500ms;
}

void Set1sFlag(bool flag)
{
    f_1s = flag;
}

bool Get1sFlag(void)
{
    return f_1s;
}

void Set2sFlag(bool flag)
{
    f_2s = flag;
}

bool Get2sFlag(void)
{
    return f_2s;
}

void SetTxCompleteFlag(bool flag)
{
    f_txComplete = flag;
}

bool GetTxCompleteFlag(void)
{
    return f_txComplete;
}

void SetRxCompleteFlag(bool flag)
{
    f_rxComplete = flag;
}

bool GetRxCompleteFlag(void)
{
    return f_rxComplete;
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

    PRINTF("%s, %s, %s\r\n", SOFTWARE_VERSION, __TIME__, __DATE__);
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
