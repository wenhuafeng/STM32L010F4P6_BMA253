
/*******************************************************************************
* Copytight 2020 raycohk Tech.Co., Ltd. All rights reserved                    *
*                                                                              *
* Filename      : Task_Step.c                                                  *
* Author        : wenhuafeng                                                   *
* Version       : 1.0                                                          *
*                                                                              *
* Decription    : step task                                                    *
*                                                                              *
* Created       : 2020-05-13                                                   *
* Last modified : 2020.06.06                                                   *
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "lptim.h"
#include "TypeDefine.h"
#include "Task_Step.h"
#include "step_process.h"
#include "vcom.h"

#if (_PEDOMETER_)

#if (_BMA253_NEW_DRIVE_)
  #include "bma253_1.h"
#else
  #include "bma253.h"
#endif

#define _TASK_STEP_CLEAR_TIME_1_        (1500/31.25)
#define _TASK_STEP_CLEAR_TIME_2_        (2200/31.25)

//------------------------------------------------------------------------------
PedometerInfo_Typedef pedometer,pedometer_last;
struct bma2x2_accel_data_fifo accel_fifo[FIFO_DEPTH];

BOOLEAN F_LPUART1_WKUP;
static u8 TaskStepClearCtr;

static BOOLEAN stepStatusMachine;
static u32 osal_systemClock;
static u32 stepInterruptTime;

//------------------------------------------------------------------------------
BOOLEAN F_GsensorINT;

//------------------------------------------------------------------------------
BOOLEAN F_32HZ;
BOOLEAN F_32HZ_1;

BOOLEAN F_250MS;
BOOLEAN F_500MS;
BOOLEAN F_1S;

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
  
}

#define _TEST_    1
void EnterStopMode(void)
{
  #if (_TEST_)
    LED_OFF();
    __ASM("nop");
    //printf("IN\n");
  #endif
  
  // Enable ultra low power mode by switching off VREFINT during STOP mode
  LL_PWR_EnableUltraLowPower();
  // Enable fast wake up from ultra low power mode
  LL_PWR_EnableFastWakeUp();
  // Clear PDDS bit to enter STOP mode when the CPU enters Deepsleep
  LL_PWR_SetPowerMode(LL_PWR_MODE_STOP);
  // Set LPSDSR bit to switch regulator to low-power mode when the CPU enters Deepsleep
  LL_PWR_SetRegulModeLP(LL_PWR_REGU_LPMODES_LOW_POWER);
  // Clear the WUF flag (after 2 clock cycles)
  LL_PWR_ClearFlag_WU();
  // Set SLEEPDEEP bit of Cortex System Control Register
  LL_LPM_EnableDeepSleep();

  // Put core into STOP mode until an interrupt occurs
  __WFI();
  
  #if (_TEST_)
    __ASM("nop");
    LED_ON();
    //printf("OUT\n");
  #endif
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/**
  * New pedometer not large than 10 and no more step generate
  * we clear the flag and counter set to previous vlaues. 
*/
void TaskStepClear_Countdown(void)
{
  u8 i;
  
  if (F_32HZ) {
    F_32HZ = FALSE;
    if (TaskStepClearCtr) {
      TaskStepClearCtr--;
      if (TaskStepClearCtr == 0x00) {
        accelDevice.allCounter  = accelDevice.oCounter;
        for (i=0; i<3; i++)
        {
          accelDevice.Counter[i]  = accelDevice.oCounter;
          accelDevice.iCounter[i] = accelDevice.oCounter;
        }
        
        stepStatusMachine = 0;
        LL_LPTIM_Disable(LPTIM1);
      }
    }
  }
}

/**
  * clear all counter anytime is needed.
  *
*/
void TaskStepClearAllCounter(void)
{
  u8 i;
  
  accelDevice.allCounter = accelDevice.oCounter = 0;
  
  for (i=0; i<3; i++)
  {
    accelDevice.Counter[i]  = 0;
    accelDevice.iCounter[i] = 0;
  }
  
  pedometer.counter  = 0;
}

void load_pedometer(u32 last_pedometer)
{
  u8 i;
  
  accelDevice.allCounter = accelDevice.oCounter = last_pedometer;

  for (i=0; i<3; i++)
  {
    accelDevice.iCounter[i]       = last_pedometer;
    accelDevice.Counter[i]        = last_pedometer;
    accelDevice.MaxThreshold[i]   = last_pedometer;
    accelDevice.MinThreshold[i]   = last_pedometer;
    accelDevice.MidleThreshold[i] = last_pedometer;
  }

  pedometer.counter = accelDevice.oCounter;
}

void TaskStepUploadPedometer(void)
{
  static u32 ooCounter = 0;
  u8 val = 9;

  if (stepStatusMachine == 0) {
    /* 至少2秒内要走出一步，且连续十步有效  */
    if ((accelDevice.allCounter - accelDevice.oCounter) > val) {
      stepStatusMachine = 1;
      TaskStepClearCtr = 0x00;
      goto exit;
    }

    if (ooCounter != accelDevice.allCounter) {
      ooCounter = accelDevice.allCounter;
      
      TaskStepClearCtr = _TASK_STEP_CLEAR_TIME_1_;
      if (!LL_LPTIM_IsEnabled(LPTIM1)) {
        MX_LPTIM1_Init();
        LPTIM1_Counter_Start_IT();
      }
    }
    goto exit;
  }
  
  if (accelDevice.oCounter != accelDevice.allCounter) {
    accelDevice.oCounter = accelDevice.allCounter;
    pedometer.counter = accelDevice.oCounter;     //update pedometer
    //printf("Pedometer:%d\n",pedometer.counter);
    
    TaskStepClearCtr = _TASK_STEP_CLEAR_TIME_2_;
    if (!LL_LPTIM_IsEnabled(LPTIM1)) {
      MX_LPTIM1_Init();
      LPTIM1_Counter_Start_IT();
    }
  }
  
exit:
  __asm("NOP");
}

u32 PedometerGet(void)
{
  return pedometer.counter;
}

void taskStep(void)
{
  u8 i;
  u8 d4time;
  const char *pbuf;
  
  /***************************************************************************
   *                                                                         *
   * 读取GSensor数据                                                         *
   *                                                                         *
   **************************************************************************/
  if (F_GsensorINT) {
    F_GsensorINT = 0;
    
    //LED_OFF();
    
    /** Read the 3accel data from fifo buffer */
    BMA2X2_READ_FIFO_BUFFER(accel_fifo, 6 * FIFO_DEPTH);
    
    /** 
     * Less than 8*50=400ms indicate a error ocurr, we
     * need to re-configure the gsensor.
     */
    d4time = osal_systemClock - stepInterruptTime ;
    
    //stepInterruptTime = osal_systemClock;
    
    if ((d4time<(8-1)) || (d4time>10)) /* 8*system tick(50ms) = 400ms */
    {
    //  osal_set_event( task_id, TASK_STEP_BMA_RE_INIT_EVT );
    //  
    //  return ( events ^ STEP_TASK_DECT_EVT );
    }
    
    for (i=0; i<FIFO_DEPTH; i++)
    {
      pbuf = (char*)&accel_fifo[i];
      
      accel_fifo[i].x = (s16)((((s32)((s8)pbuf[1]))\
        << C_BMA2x2_EIGHT_U8X) | (pbuf[0] & BMA2x2_10_BIT_SHIFT));
      accel_fifo[i].x = accel_fifo[i].x >> C_BMA2x2_SIX_U8X;
      
      accel_fifo[i].y = (s16)((((s32)((s8)pbuf[3]))\
        << C_BMA2x2_EIGHT_U8X) | (pbuf[2] & BMA2x2_10_BIT_SHIFT));
      accel_fifo[i].y = accel_fifo[i].y >> C_BMA2x2_SIX_U8X;
      
      accel_fifo[i].z = (s16)((((s32)((s8)pbuf[5]))\
        << C_BMA2x2_EIGHT_U8X) | (pbuf[4] & BMA2x2_10_BIT_SHIFT));
      accel_fifo[i].z = accel_fifo[i].z >> C_BMA2x2_SIX_U8X;
    }
    
    {
      AccelHandle();
      TaskStepUploadPedometer();
    }
    
    #if (_BMA253_NEW_DRIVE_)
      accelerometer_rst_intr();
    #else
      bma2x2_rst_intr(0x1);
    #endif
    
    //LED_ON();
  }
}

//==============================================================================
//==============================================================================


#endif

