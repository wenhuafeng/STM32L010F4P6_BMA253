#include "w25qxx.h"
#include <stdint.h>
#include "main.h"

#if defined(GSENSOR_TO_SPI_FLASH) && GSENSOR_TO_SPI_FLASH

#define W25Q80  0XEF13
#define W25Q16  0XEF14
#define W25Q32  0XEF15
#define W25Q64  0XEF16
#define W25Q128 0XEF17

#define W25QXX_CS_LOW()                                                    \
    do {                                                                   \
        HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET); \
    } while (0)
#define W25QXX_CS_HIGH()                                                 \
    do {                                                                 \
        HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET); \
    } while (0)

#define W25X_WriteEnable      0x06
#define W25X_WriteDisable     0x04
#define W25X_ReadStatusReg    0x05
#define W25X_WriteStatusReg   0x01
#define W25X_ReadData         0x03
#define W25X_FastReadData     0x0B
#define W25X_FastReadDual     0x3B
#define W25X_PageProgram      0x02
#define W25X_BlockErase       0xD8
#define W25X_SectorErase      0x20
#define W25X_ChipErase        0xC7
#define W25X_PowerDown        0xB9
#define W25X_ReleasePowerDown 0xAB
#define W25X_DeviceID         0xAB
#define W25X_ManufactDeviceID 0x90
#define W25X_JedecDeviceID    0x9F

#define W25Q_WRITE_EN                0x06
#define W25Q_VOLATILE_SR_WRITE_EN    0x50
#define W25Q_WRITE_DEN               0x04
#define W25Q_READ_REG_1              0x05
#define W25Q_WRITE_REG_1             0x01
#define W25Q_READ_REG_2              0x35
#define W25Q_WRITE_REG_2             0x31
#define W25Q_READ_REG_3              0x15
#define W25Q_WRITE_REG_3             0x11
#define W25Q_CHIP_ERASE              0xC7
#define W25Q_ERASE_PROG_SUSPEND      0x75
#define W25Q_ERASE_PROG_RESUME       0x7A
#define W25Q_POWER_DOWN              0xB9
#define W25Q_RELASE_POWER_DOWN       0xAB
#define W25Q_MFTR_DEVICE_ID          0x90
#define W25Q_JEDEC_ID                0x9F
#define W25Q_GLB_BLK_LOCK            0x7E
#define W25Q_GLB_BLK_UNLOCK          0x98
#define W25Q_ENTER_QPI_MODE          0x38
#define W25Q_EANBLE_RESET            0x66
#define W25Q_RESET_DEVICE            0x99
#define W25Q_UNIQUE_ID               0x4B
#define W25Q_PAGE_PROG               0x02
#define W25Q_QUAD_PAGE_PROG          0x32
#define W25Q_SECTOR_ERASE            0x20
#define W25Q_BLK32_ERASE             0x52
#define W25Q_BLK64_ERASE             0xD8
#define W25Q_READ_DATA               0x03
#define W25Q_FAST_READ               0x0B
#define W25Q_FAST_READ_DUAL_OUTPUT   0x3B
#define W25Q_FAST_READ_QUAD_OUTPUT   0x6B
#define W25Q_READ_SFDF_REG           0x5A
#define W25Q_ERASE_SECURITY_REG      0x44
#define W25Q_PROG_SECURITY_REG       0x42
#define W25Q_READ_SECURITY_REG       0x48
#define W25Q_INDIVIDUAL_BLK_LOCK     0x36
#define W25Q_INDIVIDUAL_BLK_UNLOCK   0x39
#define W25Q_READ_BLK_LOCK           0x3D
#define W25Q_FAST_READ_DUAL_IO       0xBB
#define W25Q_MFTR_DEVICE_ID_DUAL_IO  0x92
#define W25Q_SET_BURST_WITH_WRAP     0x77
#define W25Q_FAST_READ_QUAD_IO       0xEB
#define W25Q_WORD_READ_QUAD_IO       0xE7
#define W25Q_OCTAL_WORD_READ_QUAD_IO 0xE3
#define W25Q_MFTR_DEVICE_ID_QUAD_IO  0x94

uint16_t W25QXX_TYPE = W25Q128;
uint8_t g_buffer[512];

void W25QXX_CS(uint8_t cs)
{
    cs ? LL_GPIO_SetOutputPin(SPI1_CS_GPIO_Port, SPI1_CS_Pin) : LL_GPIO_ResetOutputPin(SPI1_CS_GPIO_Port, SPI1_CS_Pin);
}

uint8_t SPI1_ReadWriteByte(uint8_t dat)
{
    uint8_t r;

    while (LL_SPI_IsActiveFlag_TXE(SPI1) == RESET);

    LL_SPI_TransmitData8(SPI1, dat);

    while (LL_SPI_IsActiveFlag_RXNE(SPI1) == RESET);
    while (LL_SPI_IsActiveFlag_BSY(SPI1) == SET);

    r = LL_SPI_ReceiveData8(SPI1);

    return (r);
}

void W25QXX_Init(void)
{
    LL_SPI_Enable(SPI1);
    SPI1_ReadWriteByte(0xFF);
    W25QXX_TYPE = W25QXX_ReadID();
    if (W25QXX_TYPE != W25Q128) {
        __NOP();
    }
}

uint8_t W25QXX_ReadSR(void)
{
    uint8_t byte = 0;

    W25QXX_CS(0);
    SPI1_ReadWriteByte(W25X_ReadStatusReg);
    byte = SPI1_ReadWriteByte(0Xff);
    W25QXX_CS(1);

    return byte;
}

void W25QXX_WriteSR(uint8_t sr)
{
    W25QXX_CS(0);
    SPI1_ReadWriteByte(W25X_WriteStatusReg);
    SPI1_ReadWriteByte(sr);
    W25QXX_CS(1);
}

void W25QXX_WriteEnable(void)
{
    W25QXX_CS(0);
    SPI1_ReadWriteByte(W25X_WriteEnable);
    W25QXX_CS(1);
}

void W25QXX_WriteDisable(void)
{
    W25QXX_CS(0);
    SPI1_ReadWriteByte(W25X_WriteDisable);
    W25QXX_CS(1);
}

uint16_t W25QXX_ReadID(void)
{
    uint16_t Temp = 0;

    W25QXX_CS(0);
    SPI1_ReadWriteByte(0x90);
    SPI1_ReadWriteByte(0x00);
    SPI1_ReadWriteByte(0x00);
    SPI1_ReadWriteByte(0x00);
    Temp |= (SPI1_ReadWriteByte(0xFF) << 8);
    Temp |= SPI1_ReadWriteByte(0xFF);
    W25QXX_CS(1);

    return Temp;
}

void W25QXX_Read(uint8_t *buffer, uint32_t addr, uint16_t len)
{
    uint16_t i;

    W25QXX_CS(0);
    SPI1_ReadWriteByte(W25X_ReadData);
    SPI1_ReadWriteByte((uint8_t)((addr) >> 16));
    SPI1_ReadWriteByte((uint8_t)((addr) >> 8));
    SPI1_ReadWriteByte((uint8_t)addr);
    for (i = 0; i < len; i++) {
        buffer[i] = SPI1_ReadWriteByte(0XFF);
    }
    W25QXX_CS(1);
}

void W25QXX_WritePage(uint8_t *buffer, uint32_t addr, uint16_t len)
{
    uint16_t i;

    W25QXX_WriteEnable();
    W25QXX_CS(0);
    SPI1_ReadWriteByte(W25X_PageProgram);
    SPI1_ReadWriteByte((uint8_t)((addr) >> 16));
    SPI1_ReadWriteByte((uint8_t)((addr) >> 8));
    SPI1_ReadWriteByte((uint8_t)addr);
    for (i = 0; i < len; i++) {
        SPI1_ReadWriteByte(buffer[i]);
    }
    W25QXX_CS(1);
    W25QXX_WaitBusy();
}

void W25QXX_WriteNoCheck(uint8_t *buffer, uint32_t addr, uint16_t len)
{
    uint16_t pageremain;

    pageremain = (256 - addr % 256);
    if (len <= pageremain) {
        pageremain = len;
    }

    while (1) {
        W25QXX_WritePage(buffer, addr, pageremain);
        if (len == pageremain) {
            break;
        } else {
            buffer += pageremain;
            addr += pageremain;

            len -= pageremain;
            if (len > 256) {
                pageremain = 256;
            } else {
                pageremain = len;
            }
        }
    };
}

void W25QXX_Write(uint8_t *buffer, uint32_t addr, uint16_t len)
{
    uint32_t secpos;
    uint16_t secoff;
    uint16_t secremain;
    uint16_t i;
    uint8_t *buff;

    buff      = g_buffer;
    secpos    = (addr / 4096);
    secoff    = (addr % 4096);
    secremain = (4096 - secoff);
    //printf("ad:%X,nb:%X\r\n",addr,len);

    if (len <= secremain) {
        secremain = len;
    }
    while (1) {
        W25QXX_Read(buff, secpos * 4096, 4096);
        for (i = 0; i < secremain; i++) {
            if (buff[secoff + i] != 0xFF) {
                break;
            }
        }
        if (i < secremain) {
            W25QXX_EraseSector(secpos);
            for (i = 0; i < secremain; i++) {
                buff[i + secoff] = buffer[i];
            }
            W25QXX_WriteNoCheck(buff, secpos * 4096, 4096);
        } else {
            W25QXX_WriteNoCheck(buffer, addr, secremain);
        }

        if (len == secremain) {
            break;
        } else {
            secpos++;
            secoff = 0;

            buffer += secremain;
            addr += secremain;
            len -= secremain;
            if (len > 4096) {
                secremain = 4096;
            } else {
                secremain = len;
            }
        }
    };
}

void W25QXX_EraseChip(void)
{
    W25QXX_WriteEnable();
    W25QXX_WaitBusy();
    W25QXX_CS(0);
    SPI1_ReadWriteByte(W25X_ChipErase);
    W25QXX_CS(1);
    W25QXX_WaitBusy();
}

void W25QXX_EraseSector(uint32_t addr)
{
    //printf("fe:%x\r\n",addr);
    addr *= 4096;
    W25QXX_WriteEnable();
    W25QXX_WaitBusy();
    W25QXX_CS(0);
    SPI1_ReadWriteByte(W25X_SectorErase);
    SPI1_ReadWriteByte((uint8_t)((addr) >> 16));
    SPI1_ReadWriteByte((uint8_t)((addr) >> 8));
    SPI1_ReadWriteByte((uint8_t)addr);
    W25QXX_CS(1);
    W25QXX_WaitBusy();
}

void W25QXX_WaitBusy(void)
{
    while ((W25QXX_ReadSR() & 0x01) == 0x01);
}

void W25QXX_PowerDown(void)
{
    W25QXX_CS(0);
    SPI1_ReadWriteByte(W25X_PowerDown);
    W25QXX_CS(1);
    LL_mDelay(1);
}

void W25QXX_Wakeup(void)
{
    W25QXX_CS(0);
    SPI1_ReadWriteByte(W25X_ReleasePowerDown);
    W25QXX_CS(1);
    LL_mDelay(1);
}

#endif
