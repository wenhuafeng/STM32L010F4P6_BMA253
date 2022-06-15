#include "at.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "utilities.h"
#include "vcom.h"
#include "tiny_sscanf.h"

#if defined(PEDOMETER) && PEDOMETER

#define MAX_RECEIVED_DATA 255

#define CHECK_STATUS(status)                             \
    do {                                                 \
        ATEerror_t at_status = translate_status(status); \
        if (at_status != AT_OK) {                        \
            return at_status;                            \
        }                                                \
    } while (0)

static char g_receivedData[MAX_RECEIVED_DATA];
static unsigned g_receivedDataSize = 0;
static uint8_t g_receivedDataPort;

void AT_SetReceive(uint8_t AppPort, uint8_t *Buff, uint8_t BuffSize)
{
    if (MAX_RECEIVED_DATA <= BuffSize) {
        BuffSize = MAX_RECEIVED_DATA;
    }

    memcpy1((uint8_t *)g_receivedData, Buff, BuffSize);
    g_receivedDataSize = BuffSize;
    g_receivedDataPort = AppPort;
}

ATEerror_t AT_ReturnOk(const char *param)
{
    return AT_OK;
}

ATEerror_t AT_ReturnError(const char *param)
{
    return AT_ERROR;
}

ATEerror_t AT_Reset(const char *param)
{
    NVIC_SystemReset();
    return AT_OK;
}

ATEerror_t AT_GetPedometer(const char *param)
{
    PRINTF("+OK=");
    return AT_OK;
}

#endif
