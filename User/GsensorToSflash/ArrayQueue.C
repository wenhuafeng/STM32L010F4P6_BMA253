
#include <stdio.h>
#include "TypeDefine.h"
#include "ArrayQueue.h"
#include "main.h"

#if (_GSENSOR_TO_SPI_FLASH_)

//------------------------------------------------------------------------------
Queue queue;

//------------------------------------------------------------------------------
// init queue
void InitQueue(void)
{
    queue.front = 0;
    queue.rear = 0;
}

// Empty? front == rear
BOOLEAN isEmpty(void)
{
  if (queue.front == queue.rear) {
    return TRUE;
  }
  return FALSE;
}

// Full? (rear+1) % MAXSIZE == front
BOOLEAN isFull(void)
{
  if (queue.front == (queue.rear+1) % ARRAY_QUEUE_MAXSIZE) {
    return TRUE;
  }
  return FALSE;
}

// Into queue
BOOLEAN EnQueue(ElemType val)
{
  if (isFull()) {
    queue.data[queue.rear] = val;
    __asm("NOP");
    return FALSE;
  }
  queue.data[queue.rear] = val;
  queue.rear = (queue.rear + 1) % ARRAY_QUEUE_MAXSIZE;
  
  return TRUE;
}

// Out queue
BOOLEAN DeQueue(ElemType val)
{
  if (isEmpty()) {
    __asm("NOP");
    return FALSE;
  }
  val = queue.data[queue.front];
  queue.front = (queue.front + 1) % ARRAY_QUEUE_MAXSIZE;
  
  return TRUE;
}

// Get queue length, (rear - front) % MAXSIZE
int getLen(void)
{
  return (queue.rear - queue.front) % ARRAY_QUEUE_MAXSIZE;
}

//queue data to buffer
void QueueToBuffer(u8 *Buf)
{
  u8 i;
  
  for (i=0; i<(ARRAY_QUEUE_MAXSIZE-1); i++)
  {
    *(Buf+i) = queue.data[i];
  }
  *(Buf+i) = queue.data[i];
}

#endif

