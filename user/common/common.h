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

extern void Set32HzFlag(bool flag);
extern bool Get32HzFlag(void);
extern void Set250msFlag(bool flag);
extern bool Get250msFlag(void);
extern void Set500msFlag(bool flag);
extern bool Get500msFlag(void);
extern void Set1sFlag(bool flag);
extern bool Get1sFlag(void);
extern void Set2sFlag(bool flag);
extern bool Get2sFlag(void);

extern void SysInit(void);
extern void EnterStopMode(void);
extern int fputc(int ch, FILE *stream);

#endif
