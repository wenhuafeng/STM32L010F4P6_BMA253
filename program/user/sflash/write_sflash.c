#include "write_sflash.h"
#include <stdint.h>
#include <string.h>
#include "main.h"
#include "w25qxx.h"
#include "array_queue.h"
#include "bma253.h"
#include "common.h"

#if defined(GSENSOR_TO_SPI_FLASH) && GSENSOR_TO_SPI_FLASH

bool f_queueFull;
bool f_sflashFull;

bool f_eraseChip;
bool f_getData;
bool f_lowBatt;

uint8_t Buffer[ARRAY_QUEUE_MAXSIZE];
uint32_t Address_start;
uint32_t Address_current = 0x00;
#define _FLASH_SIZE_ (16 * 1024 * 1024)

void DataEnterQueue(uint8_t Data)
{
    if (QUEUE_Enable(Data) == false) {
        QUEUE_ToBuffer(Buffer);
        QUEUE_Init();
        f_queueFull = true;
    }
}

void AccelerometerDataGet(void)
{
    int ret;
    int16_t x, y, z;
    uint8_t data[6];

    if (Get32HzFlag() == true) {
        Set32HzFlag(false);
        //TEST
        //LED_BLINK();
        //TEST
        if (f_eraseChip)
            return;

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
    if (f_sflashFull == false) {
        if (f_queueFull) {
            f_queueFull = false;
            if (Address_current < _FLASH_SIZE_) {
                W25QXX_WriteNoCheck(&Buffer[0], Address_current, ARRAY_QUEUE_MAXSIZE);
                Address_current += ARRAY_QUEUE_MAXSIZE;
            } else {
                f_sflashFull = true;
            }
        }
    } else {
        if (Get250msFlag() == true) {
            Set250msFlag(false);
            LED_BLINK();
        }
    }
}

void AT_command_process(char *cRxBuf)
{
    char *buf;
    uint8_t *buffer;

    buf = "AT+RST";
    if (strstr(cRxBuf, buf) != NULL) {
        W25QXX_EraseChip();
        f_eraseChip = 1;

        uint8_t i = 10 * 4;
        do {
            if (Get250msFlag() == true) {
                Set250msFlag(false);
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
        SetTxCompleteFlag(true);
        do {
            if (Get32HzTwoFlag() == true) {
                Set32HzTwoFlag(false);
                LED_BLINK();
            }

            buffer = GetTxBuffer();
            if (GetTxCompleteFlag() == true) {
                SetTxCompleteFlag(false);
                W25QXX_Read(buffer, Address_start, ARRAY_QUEUE_MAXSIZE);
                LL_LPUART_EnableDMAReq_TX(LPUART1);
                Address_start += ARRAY_QUEUE_MAXSIZE;
            }
        } while (Address_start < Address_current);
        f_getData = 1;
        LED_OFF();
    }
}

void LPUART_DMA_Send_Test(void)
{
    uint8_t i;
    uint8_t *buf;

    buf = GetTxBuffer();
    if (Get2sFlag() == true) {
        Set2sFlag(false);

        i = 0;
        do {
            buf[i] = i;
            i++;
        } while (i < (strlen((char *)buf) - 1));
        buf[i] = i;

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
        f_lowBatt = 1;
    }
}

void WriteSflash(void)
{
    uint8_t *buf;

    buf = GetTxBuffer();
    if (GetRxCompleteFlag() == true) {
        SetRxCompleteFlag(false);
        AT_command_process((char *)buf);
    }

    LowPowerDetect();
    AccelerometerDataGet();
    WriteDataToSflash();
}

#endif