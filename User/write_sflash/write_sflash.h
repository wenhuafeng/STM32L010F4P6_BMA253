#ifndef WRITE_SFLASH_H
#define WRITE_SFLASH_H

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

void GsensorToSflash_Process(void);
void LP_TimeHandle(void);

void WriteSflash(void);

#endif
