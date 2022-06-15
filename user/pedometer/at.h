#ifndef AT_H
#define AT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* AT Command strings. Commands start with AT */
#define AT_RESET "+REBOOT"
#define AT_GSTEP "+GSTEP"

typedef enum eATEerror {
    AT_OK = 0,
    AT_ERROR,
    AT_PARAM_ERROR,
    AT_BUSY_ERROR,
    AT_TEST_PARAM_OVERFLOW,
    AT_NO_NET_JOINED,
    AT_RX_ERROR,
    AT_MAX,
} ATEerror_t;

extern ATEerror_t AT_ReturnError(const char *param);
extern ATEerror_t AT_Reset(const char *param);
extern ATEerror_t AT_GetPedometer(const char *param);

#ifdef __cplusplus
}
#endif

#endif /* __AT_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
