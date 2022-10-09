#include "print.h"
#include <stdint.h>
#include <stdbool.h>
#include "main.h"
#include "common.h"
#include "vcom.h"
#include "bma253.h"

#if defined(ACC_PRINTF) && ACC_PRINTF

void ACC_PRINTF_GetData(void)
{
    bool ret;
    int16_t x, y, z;

    if (Get1sFlag() == false) {
        return;
    }
    Set1sFlag(false);

    ret = accelerometer_accel_get(&x, &y, &z);
    if (ret == true) {
        PRINTF("x:%d, y:%d, z:%d\r\n", x, y, z);
    } else {
        PRINTF("bma253 read false!\r\n");
    }
}

#else

void ACC_PRINTF_GetData(void)
{
}

#endif
