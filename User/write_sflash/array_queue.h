#ifndef ARRAY_QUEUE_H
#define ARRAY_QUEUE_H

#include <stdint.h>
#include <stdbool.h>

//#define MAXSIZE_X       501
//#define MAXSIZE_Y       14
#define ARRAY_QUEUE_MAXSIZE   256

struct Queue {
    uint8_t data[ARRAY_QUEUE_MAXSIZE]; //[MAXSIZE_X][MAXSIZE_Y];
    uint16_t front;
    uint16_t rear;
};

void InitQueue(void);
bool isEmpty(void);
bool isFull(void);
bool EnQueue(uint8_t val);
bool DeQueue(uint8_t val);
int getLen(void);
void QueueToBuffer(uint8_t *Buf);
//int ArrayQueueProcess(void);

#endif
