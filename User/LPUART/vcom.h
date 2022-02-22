
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef VCOM_H
#define VCOM_H

#include <stdint.h>
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HW_UNLOCKED = 0x00U,
    HW_LOCKED = 0x01U,
} HW_LockTypeDef;

#define HW_LOCK(__HANDLE__)                    \
    do {                                       \
        if ((__HANDLE__)->Lock == HW_LOCKED) { \
            return;                            \
        } else {                               \
            (__HANDLE__)->Lock = HW_LOCKED;    \
        }                                      \
    } while (0)

#define HW_UNLOCK(__HANDLE__)             \
    do {                                  \
        (__HANDLE__)->Lock = HW_UNLOCKED; \
    } while (0)

#define AT_ERROR_RX_CHAR 0x01
#define DISABLE_IRQ() __disable_irq()
#define ENABLE_IRQ() __enable_irq()

void vcom_Init(void);
void vcom_DMA_Init(void);
void vcom_DeInit(void);
void vcom_IoInit(void);
void vcom_ReceiveInit(void);
void vcom_IoDeInit(void);
void vcom_Send(const char *format, ...);
FlagStatus IsNewCharReceived(void);
uint8_t GetNewChar(void);
void vcom_IRQHandler(void);

#if 1
#define PRINTF(...) vcom_Send(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#ifdef __cplusplus
}
#endif

#endif