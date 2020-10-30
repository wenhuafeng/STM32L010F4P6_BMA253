

#include <stdint.h>
#include <string.h>
#include "main.h"
#include "TypeDefine.h"
#include "acc_bma253.h"
#include "acc_bma253_iic.h"
#include "acc_printf.h"
#include "vcom.h"

#if (_ACC_PRINTF_)

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
BOOLEAN F_32HZ;
BOOLEAN F_32HZ_1;

BOOLEAN F_250MS;
BOOLEAN F_500MS;
BOOLEAN F_1S;
BOOLEAN F_2S;

//------------------------------------------------------------------------------
/**
  * @brief  Low power timer interrupt (31.25ms)
  * @param  device address
  * @param  register address
  * @param  register data
  * @retval status true/false
  */
void LP_TimeHandle(void)
{
  static u8 Ctr,Ctr1,Ctr2;
  static u8 Ctr3;
  
  F_32HZ = 1;
  F_32HZ_1 = 1;
  
  Ctr++;
  if (Ctr > 7) {
    Ctr = 0x00;
    F_250MS = 1;
  }
  Ctr1++;
  if (Ctr1 > 15) {
    Ctr1 = 0x00;
    F_500MS = 1;
  }
  Ctr2++;
  if (Ctr2 > 31) {
    Ctr2 = 0x00;
    F_1S = 1;
  }
  Ctr3++;
  if (Ctr3 > 63) {
    Ctr3 = 0x00;
    F_2S = 1;
  }
}


//------------------------------------------------------------------------------
void Acc_DataGet(void)
{
  int ret;
  int16_t x, y, z;

  //if (F_32HZ) {
  //  F_32HZ = FALSE;
  if (F_1S) {
    F_1S = FALSE;
    //TEST
    //LED_Blink();
    //TEST
    
    ret = accelerometer_accel_get(&x, &y, &z);
    if (ret == true)
    {
      PRINTF("X:%d,Y:%d,Z:%d\r\n",x,y,z);
    }
  }
}

#endif

//==============================================================================
//==============================================================================
