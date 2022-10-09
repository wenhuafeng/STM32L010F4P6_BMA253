#ifndef STEP_PROCESS_H
#define STEP_PROCESS_H

#include <stdint.h>
#include <stdbool.h>

void STEP_SetGsensorIntFlag(bool flag);
uint32_t STEP_GetPedometer(void);
void STEP_TaskStepInit(void);
void STEP_TaskStep(void);

#endif
