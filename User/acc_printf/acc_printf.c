#include "acc_printf.h"
#include <stdint.h>
#include <stdbool.h>
#include "sys_func.h"

#if (ACC_PRINTF)

void ACC_PRINTF_Init(void)
{

}

void ACC_PRINTF_GetData(void)
{
    bool ret;
    int16_t x, y, z;

    if (GetOneSecondFlag() == false) {
        return;
    }
    SetOneSecondFlag(false);

    ret = accelerometer_accel_get(&x, &y, &z);
    if (ret == true) {
        PRINTF("x:%d, y:%d, z:%d\r\n", x, y, z);
    } else {
        PRINTF("bma253 read false!\r\n");
    }
}

#else

void ACC_PRINTF_Init(void)
{
}
void ACC_PRINTF_GetData(void)
{
}

#endif
