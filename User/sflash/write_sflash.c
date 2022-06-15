#include "write_sflash.h"
#include <stdint.h>
#include <string.h>
#include "main.h"
#include "w25qxx.h"
#include "array_queue.h"

#if (_GSENSOR_TO_SPI_FLASH_)

bool F_QueueFull;
bool F_Sflash_Full;

bool F_EraseChip;
bool F_GetData;
bool F_LowBatt;

uint8_t Buffer[ARRAY_QUEUE_MAXSIZE];
uint32_t Address_start;
uint32_t Address_current = 0x00;
#define _FLASH_SIZE_ (16 * 1024 * 1024)

void DataEnterQueue(uint8_t Data)
{
    if (EnQueue(Data) == false) {
        QueueToBuffer(Buffer);
        InitQueue();
        F_QueueFull = true;
    }
}

void AccelerometerDataGet(void)
{
    int ret;
    int16_t x, y, z;
    uint8_t data[6];

    if (F_32HZ) {
        F_32HZ = false;
        //TEST
        //LED_BLINK();
        //TEST
        if (F_EraseChip)
            return;
        //if (F_GetData) return;

        ret = accelerometer_accel_get(&x, &y, &z);
        if (ret == true) {
            // Convert to network order
            data[0] = (uint8_t)(x >> 8);
            DataEnterQueue(data[0]);

            data[1] = (uint8_t)(x & 0xff);
            DataEnterQueue(data[1]);

            data[2] = (uint8_t)(y >> 8);
            DataEnterQueue(data[2]);

            data[3] = (uint8_t)(y & 0xff);
            DataEnterQueue(data[3]);

            data[4] = (uint8_t)(z >> 8);
            DataEnterQueue(data[4]);

            data[5] = (uint8_t)(z & 0xff);
            DataEnterQueue(data[5]);
        }
    }
}

void WriteDataToSflash(void)
{
    if (F_Sflash_Full == false) {
        if (F_QueueFull) {
            F_QueueFull = false;
            if (Address_current < _FLASH_SIZE_) {
                W25QXX_Write_NoCheck(&Buffer[0], Address_current, ARRAY_QUEUE_MAXSIZE);
                Address_current += ARRAY_QUEUE_MAXSIZE;
            } else {
                F_Sflash_Full = true;
            }
        }
    } else {
        if (F_250MS) {
            F_250MS = false;
            LED_BLINK();
        }
    }
}

void AT_command_process(char *cRxBuf)
{
    char *buf;

    buf = "AT+RST";
    if (strstr(cRxBuf, buf) != NULL) {
        W25QXX_Erase_Chip();
        F_EraseChip = 1;

        uint8_t i = 10 * 4;
        do {
            if (F_250MS) {
                F_250MS = 0;
                LED_BLINK();
                i--;
                if (i == 0)
                    break;
            }
        } while (1);
        LED_OFF();
    }

    buf = "AT+GET";
    if (strstr(cRxBuf, buf) != NULL) {
        Address_start = 0x00;
        F_txComplete = true;
        do {
            if (F_32HZ_1) {
                F_32HZ_1 = 0;
                LED_BLINK();
            }
            if (F_txComplete == true) {
                F_txComplete = false;
                W25QXX_Read(g_usart1TxBuffer, Address_start, ARRAY_QUEUE_MAXSIZE);
                LL_LPUART_EnableDMAReq_TX(LPUART1);
                Address_start += ARRAY_QUEUE_MAXSIZE;
            }
        } while (Address_start < Address_current);
        F_GetData = 1;
        LED_OFF();
    }
}

void LPUART_DMA_Send_Test(void)
{
    uint8_t i;

    if (F_2S) {
        F_2S = 0;

        i = 0;
        do {
            g_usart1TxBuffer[i] = i;
            i++;
        } while (i < (USART1_TXBUFF_SIZE - 1));
        g_usart1TxBuffer[i] = i;

        LL_LPUART_EnableDMAReq_TX(LPUART1);
    }
}

void LowPowerDetect(void)
{
    uint8_t i = 0;
    uint8_t j = 0;

    do {
        if (LL_GPIO_IsInputPinSet(LB_detect_GPIO_Port, LB_detect_Pin) == 0) {
            i++;
        }
        j++;
    } while (j < 30);

    if (i > 25) {
        F_LowBatt = 1;
    }
}

void WriteSflash(void)
{
    if (F_rxComplete) {
        F_rxComplete = false;
        AT_command_process(g_usart1RxBuffer);
    }

    LowPowerDetect();
    AccelerometerDataGet();
    WriteDataToSflash();
}

#endif
