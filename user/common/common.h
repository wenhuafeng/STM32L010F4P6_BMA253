#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdbool.h>

#define LED_BLINK()                                \
    do {                                           \
        LL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin); \
    } while (0)
#define LED_ON()                                        \
    do {                                                \
        LL_GPIO_ResetOutputPin(LED_GPIO_Port, LED_Pin); \
    } while (0)
#define LED_OFF()                                     \
    do {                                              \
        LL_GPIO_SetOutputPin(LED_GPIO_Port, LED_Pin); \
    } while (0)

extern void DMA_ISR_Callback(void);
extern void LPUART_DMA_CONFIG(void);
extern void LPUART_RxIdleCallback(void);
extern void LPUART_TxCompleteCallback(void);

extern void LPTIM1_CounterStartIT(void);
extern void LPTIM1_IsrHandle(void);

extern bool Get32HzFlag(void);
extern void Set32HzFlag(bool flag);
extern bool GetOneSecondFlag(void);
extern void SetOneSecondFlag(bool flag);
extern void SysInit(void);
extern void EnterStopMode(void);
extern int fputc(int ch, FILE *stream);

#endif
