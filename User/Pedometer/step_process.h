#ifndef STEP_PROCESS_H
#define STEP_PROCESS_H

#include <stdint.h>
#include <stdbool.h>

void SetGsensorIntFlag(bool flag);
uint32_t GetPedometer(void);
void TaskStepInit(void);
void TaskStep(void);

#endif
