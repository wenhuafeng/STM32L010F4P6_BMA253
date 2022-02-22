#ifndef AT_H
#define AT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum eATEerror
{
  AT_OK = 0,
  AT_ERROR,
  AT_PARAM_ERROR,
  AT_BUSY_ERROR,
  AT_TEST_PARAM_OVERFLOW,
  AT_NO_NET_JOINED,
  AT_RX_ERROR,
  AT_MAX,
} ATEerror_t;

#define AT_PRINTF(...)     vcom_Send(__VA_ARGS__)

/* AT Command strings. Commands start with AT */
#define AT_RESET      "+REBOOT"
#define AT_GSTEP      "+GSTEP"

//void set_at_receive(uint8_t AppPort, uint8_t* Buff, uint8_t BuffSize);
//ATEerror_t at_return_ok(const char *param);
ATEerror_t at_return_error(const char *param);
ATEerror_t at_reset(const char *param);
ATEerror_t AT_GetPedometer(const char *param);

#ifdef __cplusplus
}
#endif

#endif /* __AT_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
