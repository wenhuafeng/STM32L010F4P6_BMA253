#include "bma253_iic.h"
#include <stdbool.h>
#include "main.h"
#include "i2c.h"
#include "vcom.h"

#if defined(PEDOMETER) && PEDOMETER

#define BMA253_I2C_SLAVE_ADDRESS 0x18

#if 0

void BMA253_I2cInit(void)
{
    MX_I2C1_Init();
    LL_I2C_Enable(I2C1);
    //LL_I2C_Disable(I2C1);
    //LL_I2C_GenerateStopCondition(I2C1);
}

void i2c_write(uint8_t addr, uint8_t* data, uint8_t size, uint8_t stop)
{
    LL_I2C_SetSlaveAddr(I2C1, addr << 1);
    LL_I2C_SetTransferSize(I2C1, size);
    LL_I2C_SetTransferRequest(I2C1, LL_I2C_REQUEST_WRITE);
    LL_I2C_GenerateStartCondition(I2C1);

    while(!LL_I2C_IsActiveFlag_TC(I2C1)) {
        if(LL_I2C_IsActiveFlag_NACK(I2C1)) {
            return;
        }
        if(LL_I2C_IsActiveFlag_TXIS(I2C1)) {
            LL_I2C_TransmitData8(I2C1, *data++);
        }
    }

    if(stop) {
        LL_I2C_GenerateStopCondition(I2C1);
        while(!LL_I2C_IsActiveFlag_STOP(I2C1));
    }

}

void i2c_read(uint8_t addr, uint8_t *data, uint8_t size)
{
    LL_I2C_SetSlaveAddr(I2C1, addr << 1);
    LL_I2C_SetTransferSize(I2C1, size);
    LL_I2C_SetTransferRequest(I2C1, LL_I2C_REQUEST_READ);
    LL_I2C_GenerateStartCondition(I2C1);
    while (size) {
        if(LL_I2C_IsActiveFlag_RXNE(I2C1)) {
            *data++ = LL_I2C_ReceiveData8(I2C1);
            size--;
        }
    }
    LL_I2C_GenerateStopCondition(I2C1);
    while(!LL_I2C_IsActiveFlag_STOP(I2C1));
}

bool BMA253_WriteByte(uint8_t reg, uint8_t data)
{
    //delay(10);
    uint8_t dataSend[2];
    dataSend[0] = reg;
    dataSend[1] = data;
    i2c_write(BMA253_I2C_SLAVE_ADDRESS, dataSend, 2, 1);

    return true;
}

bool BMA253_ReadByte(uint8_t reg, uint8_t *data)
{
    //delay(10);
    data[0] = reg;
    i2c_write(BMA253_I2C_SLAVE_ADDRESS, data, 1, 0);
    i2c_read(BMA253_I2C_SLAVE_ADDRESS, data, 1);

    return true;
}

bool BMA253_ReadNBytes(uint8_t reg, uint8_t *data, uint16_t size)
{
    data[0] = reg;
    i2c_write(BMA253_I2C_SLAVE_ADDRESS, data, 1, 0);
    i2c_read(BMA253_I2C_SLAVE_ADDRESS, data, size);

    return true;
}

#else

#define LIB_I2C_TIMEOUT 100
#define LIB_I2C_POLLING(condition)    \
    do {                              \
        uint16_t n = LIB_I2C_TIMEOUT; \
        while (condition) {           \
            if (--n == 0) {           \
                goto I2C_ERROR;       \
            }                         \
            LL_mDelay(2);             \
        }                             \
    } while (0)

void BMA253_I2cInit(void)
{
    MX_I2C1_Init();
    LL_I2C_Enable(I2C1);
    LL_I2C_EnableAutoEndMode(I2C1);
    //LL_I2C_Disable(I2C1);
    //LL_I2C_GenerateStopCondition(I2C1);
}

bool BMA253_WriteByte(uint8_t reg, uint8_t data)
{
    while (LL_I2C_IsActiveFlag_BUSY(I2C1));

    //LL_I2C_Enable(I2C1);
    LL_I2C_SetSlaveAddr(I2C1, (BMA253_I2C_SLAVE_ADDRESS << 1));
    LL_I2C_SetTransferRequest(I2C1, LL_I2C_REQUEST_WRITE);
    LL_I2C_SetTransferSize(I2C1, 2);     // Transfer of <reg><data>
    LL_I2C_GenerateStartCondition(I2C1);
    LIB_I2C_POLLING(!(LL_I2C_IsActiveFlag_TXE(I2C1)));
    //while(!(LL_I2C_IsActiveFlag_TXE(I2C1)));
    LL_I2C_TransmitData8(I2C1, reg);
    LIB_I2C_POLLING(!(LL_I2C_IsActiveFlag_TXE(I2C1)));
    LL_I2C_TransmitData8(I2C1, data);
    LIB_I2C_POLLING(!(LL_I2C_IsActiveFlag_TXE(I2C1)));
    //LL_I2C_GenerateStopCondition(I2C1);
    LIB_I2C_POLLING(LL_I2C_IsActiveFlag_BUSY(I2C1));
    //LL_I2C_Disable(I2C1);
    return true;

I2C_ERROR:
    __asm("nop");
    return false;
}

bool BMA253_ReadByte(uint8_t reg, uint8_t *rxByte)
{
    while (LL_I2C_IsActiveFlag_BUSY(I2C1));

    //LL_I2C_Enable(I2C1);
    LL_I2C_SetSlaveAddr(I2C1, (BMA253_I2C_SLAVE_ADDRESS << 1));
    LL_I2C_SetTransferRequest(I2C1, LL_I2C_REQUEST_WRITE);
    LL_I2C_SetTransferSize(I2C1, 1);
    LL_I2C_GenerateStartCondition(I2C1);
    LIB_I2C_POLLING(!(LL_I2C_IsActiveFlag_TXE(I2C1)));

    LL_I2C_TransmitData8(I2C1, reg);
    LIB_I2C_POLLING(LL_I2C_IsActiveFlag_BUSY(I2C1));
    //LL_I2C_GenerateStopCondition(I2C1); // automatically generate STOP signal.

    LL_I2C_SetSlaveAddr(I2C1, (BMA253_I2C_SLAVE_ADDRESS << 1));
    LL_I2C_SetTransferRequest(I2C1, LL_I2C_REQUEST_READ);
    LL_I2C_SetTransferSize(I2C1, 1);     // Now request read of single byte from register
    LL_I2C_GenerateStartCondition(I2C1); // START
    LIB_I2C_POLLING(!(LL_I2C_IsActiveFlag_RXNE(I2C1)));
    *rxByte = LL_I2C_ReceiveData8(I2C1);
    //LL_I2C_GenerateStopCondition(I2C1);
    LIB_I2C_POLLING(LL_I2C_IsActiveFlag_BUSY(I2C1));
    //LL_I2C_Disable(I2C1);
    return true;

I2C_ERROR:
    __asm("nop");
    return false;
}

//bool BMA253_WriteNBytes(uint8_t reg, uint8_t *data, uint16_t size)
//{
//    while (LL_I2C_IsActiveFlag_BUSY(I2C1));
//
//    //LL_I2C_Enable(I2C1);
//    LL_I2C_SetSlaveAddr(I2C1, (BMA253_I2C_SLAVE_ADDRESS << 1));
//    LL_I2C_SetTransferRequest(I2C1, LL_I2C_REQUEST_WRITE);
//    LL_I2C_SetTransferSize(I2C1, 1); // Transfer of data_n bytes + addr byte
//    LL_I2C_GenerateStartCondition(I2C1); // START
//    LIB_I2C_POLLING(!(LL_I2C_IsActiveFlag_TXE(I2C1)));
//
//    LL_I2C_TransmitData8(I2C1, reg); // Send address
//    LIB_I2C_POLLING(!(LL_I2C_IsActiveFlag_TXE(I2C1)));
//
//    for (uint8_t i = 1; i < size; i++) {
//        LL_I2C_TransmitData8(I2C1, *data++); // Push data buffer to slave device
//        LIB_I2C_POLLING(!(LL_I2C_IsActiveFlag_TXE(I2C1)));
//    }
//    LIB_I2C_POLLING(!(LL_I2C_IsActiveFlag_TC(I2C1)));
//
//    LL_I2C_GenerateStopCondition(I2C1);
//    //LL_I2C_Disable(I2C1);
//    return true;
//
//I2C_ERROR:
//    __asm("nop");
//    return false;
//}

bool BMA253_ReadNBytes(uint8_t reg, uint8_t *data, uint16_t size)
{
    uint16_t i = 0xffff;

    while (LL_I2C_IsActiveFlag_BUSY(I2C1));

    //LL_I2C_Enable(I2C1);
    LL_I2C_SetSlaveAddr(I2C1, (BMA253_I2C_SLAVE_ADDRESS << 1));
    LL_I2C_SetTransferRequest(I2C1, LL_I2C_REQUEST_WRITE);
    LL_I2C_SetTransferSize(I2C1, 1);
    LL_I2C_GenerateStartCondition(I2C1);
    LIB_I2C_POLLING(!(LL_I2C_IsActiveFlag_TXE(I2C1)));

    LL_I2C_TransmitData8(I2C1, reg);
    LIB_I2C_POLLING(LL_I2C_IsActiveFlag_BUSY(I2C1));

    LL_I2C_SetSlaveAddr(I2C1, (BMA253_I2C_SLAVE_ADDRESS << 1));
    LL_I2C_SetTransferRequest(I2C1, LL_I2C_REQUEST_READ);
    LL_I2C_SetTransferSize(I2C1, size);
    LL_I2C_GenerateStartCondition(I2C1);
    //LIB_I2C_POLLING(!(LL_I2C_IsActiveFlag_TXE(I2C1)));

    for (i = 0; i < size; i++) {
        LIB_I2C_POLLING(!(LL_I2C_IsActiveFlag_RXNE(I2C1)));
        *data++ = LL_I2C_ReceiveData8(I2C1);
    }
    LIB_I2C_POLLING(LL_I2C_IsActiveFlag_BUSY(I2C1));
    //LL_I2C_GenerateStopCondition(I2C1);
    //LL_I2C_Disable(I2C1);
    return true;

I2C_ERROR:
    PRINTF("i:%d\r\n", i);
    __asm("nop");
    return false;
}

#endif
#endif
