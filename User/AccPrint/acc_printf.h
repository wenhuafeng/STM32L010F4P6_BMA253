
#ifndef _GSENSOR_TO_SFLASH_PROCESS_H_
#define _GSENSOR_TO_SFLASH_PROCESS_H_

#include "TypeDefine.h"

//------------------------------------------------------------------------------
#define LED_Blink()     do{LL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);}while(0)
#define LED_ON()        do{LL_GPIO_ResetOutputPin(LED_GPIO_Port, LED_Pin);}while(0)
#define LED_OFF()       do{LL_GPIO_SetOutputPin(LED_GPIO_Port, LED_Pin);}while(0)

//------------------------------------------------------------------------------
void GsensorToSflash_Process(void);
void LP_TimeHandle(void);

void LPUART_DMA_CONFIG(void);
void LPUART_RxIdleCallback(void);
void LPUART_TxCompleteCallback(void);

void DMA_ISR_Callback(void);

void Acc_DataGet(void);

#endif
