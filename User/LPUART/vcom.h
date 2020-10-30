
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __VCOM_H__
#define __VCOM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
//#include "hw_conf.h"
#include "main.h"
#include "TypeDefine.h"

typedef enum
{
  HW_UNLOCKED = 0x00U,
  HW_LOCKED   = 0x01U
} HW_LockTypeDef;

#define HW_LOCK(__HANDLE__)               \
  do {                                    \
    if ((__HANDLE__)->Lock == HW_LOCKED)  \
    {                                     \
      return;                             \
    }                                     \
    else                                  \
    {                                     \
      (__HANDLE__)->Lock = HW_LOCKED;     \
    }                                     \
  } while (0)

#define HW_UNLOCK(__HANDLE__)             \
  do {                                    \
    (__HANDLE__)->Lock = HW_UNLOCKED;     \
  } while (0)

#define AT_ERROR_RX_CHAR    0x01
#define DISABLE_IRQ()       __disable_irq()
#define ENABLE_IRQ()        __enable_irq()

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */

/**
 * @brief  Init the VCOM.
 * @param  None
 * @retval None
 */
void vcom_Init(void);
void vcom_DMA_Init(void);

/**
 * @brief  DeInit the VCOM.
 * @param  None
 * @retval None
 */
void vcom_DeInit(void);

/**
 * @brief  Init the VCOM IOs.
 * @param  None
 * @retval None
 */
void vcom_IoInit(void);

/**
 * @brief  Init the VCOM RX
 * @param  None
 * @retval None
 */
void vcom_ReceiveInit(void);

/**
 * @brief  DeInit the VCOM IOs.
 * @param  None
 * @retval None
 */
void vcom_IoDeInit(void);

/**
 * @brief  Sends string on com port
 * @param  String
 * @retval None
 */
void vcom_Send(const char *format, ...);

/**
 * @brief  Checks if a new character has been received on com port
 * @param  None
 * @retval Returns SET if new character has been received on com port, RESET otherwise
 */
FlagStatus IsNewCharReceived(void);

/**
 * @brief  Gets new received characters on com port
 * @param  None
 * @retval Returns the character
 */
u8 GetNewChar(void);

/**
 * @brief  vcom IRQ Handler
 * @param  None
 * @retval None
 */
void vcom_IRQHandler(void);

/* Exported macros -----------------------------------------------------------*/
#if 1
#define PRINTF(...)     vcom_Send(__VA_ARGS__)
#else
#define PRINTF(...)
#endif


#ifdef __cplusplus
}
#endif

#endif /* __VCOM_H__*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
