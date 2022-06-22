#ifndef ARRAY_QUEUE_H
#define ARRAY_QUEUE_H

#include <stdint.h>
#include <stdbool.h>

#define ARRAY_QUEUE_MAXSIZE 256

struct Queue {
    uint8_t data[ARRAY_QUEUE_MAXSIZE];
    uint16_t front;
    uint16_t rear;
};

void QUEUE_Init(void);
bool QUEUE_IsEmpty(void);
bool QUEUE_IsEull(void);
bool QUEUE_Enable(uint8_t val);
bool QUEUE_Disable(uint8_t val);
int QUEUE_GetLen(void);
void QUEUE_ToBuffer(uint8_t *Buf);

#endif
