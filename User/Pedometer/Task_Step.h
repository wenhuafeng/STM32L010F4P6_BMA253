
#ifndef __TASKSTEP_H__
#define __TASKSTEP_H__

#include "TypeDefine.h"


#define TRUE                    1
#define FALSE                   0

#define true                    1
#define false                   0

#define LED_Blink()     do{LL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);}while(0)
#define LED_ON()        do{LL_GPIO_ResetOutputPin(LED_GPIO_Port, LED_Pin);}while(0)
#define LED_OFF()       do{LL_GPIO_SetOutputPin(LED_GPIO_Port, LED_Pin);}while(0)

//------------------------------------------------------------------------------
typedef  struct
{
  u32 counter;
  //u32 distance;
  //u32 calorie;
  //u32 goal;
  //u8 goal_per;
}PedometerInfo_Typedef;

extern PedometerInfo_Typedef pedometer, pedometer_last;

extern u32 taskStepTaskId;

extern BOOLEAN F_GsensorINT;

//------------------------------------------------------------------------------
extern void LP_TimeHandle(void);

extern void LPUART_DMA_CONFIG(void);
extern void DMA_ISR_Callback(void);
extern void LPUART_RxIdleCallback(void);
extern void AT_command_process(void);

extern void EnterStopMode(void);

extern void load_pedometer(u32 last_pedometer);
extern void taskStepInit(u32 task_id);
extern void unit_update(void);
extern void TaskStepClear_Countdown(void);
extern void TaskStepClearAllCounter(void);

extern void LPUART1_WakeUpDelayStopMode_Countdown(void);

extern u32 PedometerGet(void);
extern void taskStep(void);

#endif
