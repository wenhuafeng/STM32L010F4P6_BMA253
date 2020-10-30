
#ifndef _ARRAY_QUEUE_H_
#define _ARRAY_QUEUE_H_

#include "TypeDefine.h"

//#define OK              1
//#define ERROR           0

#define TRUE            1
#define FALSE           0

typedef u8              BOOLEAN;
typedef u8              ElemType;

//#define MAXSIZE_X       501
//#define MAXSIZE_Y       14
#define ARRAY_QUEUE_MAXSIZE   256

// Queue struct
typedef struct {
    ElemType data[ARRAY_QUEUE_MAXSIZE]; //[MAXSIZE_X][MAXSIZE_Y];
    u16 front;                          // Queue front
    u16 rear;                           // Queue rear
}Queue; 

//------------------------------------------------------------------------------
void InitQueue(void);
BOOLEAN isEmpty(void);
BOOLEAN isFull(void);
BOOLEAN EnQueue(ElemType val);
BOOLEAN DeQueue(ElemType val);
int getLen(void);
void QueueToBuffer(u8 *Buf);
//int ArrayQueueProcess(void);

#endif
