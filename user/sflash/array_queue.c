#include "array_queue.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "main.h"

#if defined(GSENSOR_TO_SPI_FLASH) && GSENSOR_TO_SPI_FLASH

struct Queue queue;

// init queue
void InitQueue(void)
{
    queue.front = 0;
    queue.rear = 0;
}

// Empty? front == rear
bool isEmpty(void)
{
    if (queue.front == queue.rear) {
        return true;
    }
    return false;
}

// Full? (rear+1) % MAXSIZE == front
bool isFull(void)
{
    if (queue.front == (queue.rear + 1) % ARRAY_QUEUE_MAXSIZE) {
        return true;
    }
    return false;
}

// Into queue
bool EnQueue(uint8_t val)
{
    if (isFull()) {
        queue.data[queue.rear] = val;
        __asm("NOP");
        return false;
    }
    queue.data[queue.rear] = val;
    queue.rear = (queue.rear + 1) % ARRAY_QUEUE_MAXSIZE;

    return true;
}

// Out queue
bool DeQueue(uint8_t val)
{
    (void)val;
    if (isEmpty()) {
        __asm("NOP");
        return false;
    }
    val = queue.data[queue.front];
    queue.front = (queue.front + 1) % ARRAY_QUEUE_MAXSIZE;

    return true;
}

// Get queue length, (rear - front) % MAXSIZE
int getLen(void)
{
    return (queue.rear - queue.front) % ARRAY_QUEUE_MAXSIZE;
}

// queue data to buffer
void QueueToBuffer(uint8_t *Buf)
{
    uint8_t i;

    for (i = 0; i < (ARRAY_QUEUE_MAXSIZE - 1); i++) {
        *(Buf + i) = queue.data[i];
    }
    *(Buf + i) = queue.data[i];
}

#endif
