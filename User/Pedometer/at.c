#include "at.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "utilities.h"
#include "vcom.h"
#include "tiny_sscanf.h"

#if (PEDOMETER)

#define MAX_RECEIVED_DATA 255

#define CHECK_STATUS(status)                             \
    do {                                                 \
        ATEerror_t at_status = translate_status(status); \
        if (at_status != AT_OK) {                        \
            return at_status;                            \
        }                                                \
    } while (0)

static char ReceivedData[MAX_RECEIVED_DATA];
static unsigned ReceivedDataSize = 0;
static uint8_t ReceivedDataPort;

void set_at_receive(uint8_t AppPort, uint8_t *Buff, uint8_t BuffSize)
{
    if (MAX_RECEIVED_DATA <= BuffSize)
        BuffSize = MAX_RECEIVED_DATA;
    memcpy1((uint8_t *)ReceivedData, Buff, BuffSize);
    ReceivedDataSize = BuffSize;
    ReceivedDataPort = AppPort;
}

ATEerror_t at_return_ok(const char *param)
{
    return AT_OK;
}

ATEerror_t at_return_error(const char *param)
{
    return AT_ERROR;
}

ATEerror_t at_reset(const char *param)
{
    NVIC_SystemReset();
    return AT_OK;
}

ATEerror_t AT_GetPedometer(const char *param)
{
    AT_PRINTF("+OK=");
//#if (PEDOMETER)
//    print_d(PedometerGet());
//#endif
    return AT_OK;
}

#if 0
static int sscanf_16_hhx(const char *from, uint8_t *pt)
{
    return tiny_sscanf(
            from, "%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
            &pt[0], &pt[1], &pt[2], &pt[3], &pt[4], &pt[5], &pt[6], &pt[7], &pt[8], &pt[9], &pt[10], &pt[11], &pt[12],
            &pt[13], &pt[14], &pt[15]);
}

static void print_16_02x(uint8_t *pt)
{
    AT_PRINTF("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r", pt[0], pt[1], pt[2], pt[3], pt[4],
              pt[5], pt[6], pt[7], pt[8], pt[9], pt[10], pt[11], pt[12], pt[13], pt[14], pt[15]);
}

static int sscanf_uint32_as_hhx(const char *from, uint32_t *value)
{
    return tiny_sscanf(from, "%02hhx%02hhx%02hhx%02hhx", &((unsigned char *)(value))[3], &((unsigned char *)(value))[2],
                       &((unsigned char *)(value))[1], &((unsigned char *)(value))[0]);
}

static void print_uint32_as_02x(uint32_t value)
{
    AT_PRINTF("%02x%02x%02x%02x\r", (unsigned)((unsigned char *)(&value))[3], (unsigned)((unsigned char *)(&value))[2],
              (unsigned)((unsigned char *)(&value))[1], (unsigned)((unsigned char *)(&value))[0]);
}

static void print_8_02x(uint8_t *pt)
{
    AT_PRINTF("%02x%02x%02x%02x%02x%02x%02x%02x\r", pt[0], pt[1], pt[2], pt[3], pt[4], pt[5], pt[6], pt[7]);
}

static void print_d(int value)
{
    AT_PRINTF("%d\r", value);
}

static void print_u(unsigned int value)
{
    AT_PRINTF("%u\r", value);
}

#endif
#endif
