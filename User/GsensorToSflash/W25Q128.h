

#ifndef __W25QXX_H__
#define __W25QXX_H__

#include "TypeDefine.h"
#include "stm32l0xx_ll_gpio.h"
#include "main.h"

//////////////////////////////////////////////////////////////////////////////////
//W25X系列/Q系列芯片列表
//W25Q80  ID  0XEF13
//W25Q16  ID  0XEF14
//W25Q32  ID  0XEF15
//W25Q64  ID  0XEF16
//W25Q128 ID  0XEF17
#define W25Q80    0XEF13
#define W25Q16    0XEF14
#define W25Q32    0XEF15
#define W25Q64    0XEF16
#define W25Q128   0XEF17

extern u16 W25QXX_TYPE;       //定义W25QXX芯片型号

//#define W25QXX_CS(x)      do{\
//                            if (x==0) {\
//                              HAL_GPIO_WritePin(SPI1_CS_GPIO_Port,SPI1_CS_Pin,GPIO_PIN_RESET);\
//                            } else {\
//                              HAL_GPIO_WritePin(SPI1_CS_GPIO_Port,SPI1_CS_Pin,GPIO_PIN_SET);\
//                            }\
//                          }while(0)

#define W25QXX_CS_LOW()   do{HAL_GPIO_WritePin(SPI1_CS_GPIO_Port,SPI1_CS_Pin,GPIO_PIN_RESET);}while(0)
#define W25QXX_CS_HIGH()  do{HAL_GPIO_WritePin(SPI1_CS_GPIO_Port,SPI1_CS_Pin,GPIO_PIN_SET);}while(0)

//////////////////////////////////////////////////////////////////////////////////
//指令表
#define W25X_WriteEnable        0x06
#define W25X_WriteDisable       0x04
#define W25X_ReadStatusReg      0x05
#define W25X_WriteStatusReg     0x01
#define W25X_ReadData           0x03
#define W25X_FastReadData       0x0B
#define W25X_FastReadDual       0x3B
#define W25X_PageProgram        0x02
#define W25X_BlockErase         0xD8
#define W25X_SectorErase        0x20
#define W25X_ChipErase          0xC7
#define W25X_PowerDown          0xB9
#define W25X_ReleasePowerDown   0xAB
#define W25X_DeviceID           0xAB
#define W25X_ManufactDeviceID   0x90
#define W25X_JedecDeviceID      0x9F

//instruction set
#define W25Q_WRITE_EN                 0x06
#define W25Q_VOLATILE_SR_WRITE_EN     0x50
#define W25Q_WRITE_DEN                0x04
#define W25Q_READ_REG_1               0x05
#define W25Q_WRITE_REG_1              0x01
#define W25Q_READ_REG_2               0x35
#define W25Q_WRITE_REG_2              0x31
#define W25Q_READ_REG_3               0x15
#define W25Q_WRITE_REG_3              0x11
#define W25Q_CHIP_ERASE               0xC7  //0X60
#define W25Q_ERASE_PROG_SUSPEND       0x75
#define W25Q_ERASE_PROG_RESUME        0x7A
#define W25Q_POWER_DOWN               0xB9
#define W25Q_RELASE_POWER_DOWN        0xAB
#define W25Q_MFTR_DEVICE_ID           0x90
#define W25Q_JEDEC_ID                 0x9F
#define W25Q_GLB_BLK_LOCK             0x7E
#define W25Q_GLB_BLK_UNLOCK           0x98
#define W25Q_ENTER_QPI_MODE           0x38
#define W25Q_EANBLE_RESET             0x66
#define W25Q_RESET_DEVICE             0x99
#define W25Q_UNIQUE_ID                0x4B
#define W25Q_PAGE_PROG                0x02
#define W25Q_QUAD_PAGE_PROG           0x32
#define W25Q_SECTOR_ERASE             0x20
#define W25Q_BLK32_ERASE              0x52
#define W25Q_BLK64_ERASE              0xD8
#define W25Q_READ_DATA                0x03
#define W25Q_FAST_READ                0x0B
#define W25Q_FAST_READ_DUAL_OUTPUT    0x3B
#define W25Q_FAST_READ_QUAD_OUTPUT    0x6B
#define W25Q_READ_SFDF_REG            0x5A
#define W25Q_ERASE_SECURITY_REG       0x44
#define W25Q_PROG_SECURITY_REG        0x42
#define W25Q_READ_SECURITY_REG        0x48
#define W25Q_INDIVIDUAL_BLK_LOCK      0x36
#define W25Q_INDIVIDUAL_BLK_UNLOCK    0x39
#define W25Q_READ_BLK_LOCK            0x3D
#define W25Q_FAST_READ_DUAL_IO        0xBB
#define W25Q_MFTR_DEVICE_ID_DUAL_IO   0x92
#define W25Q_SET_BURST_WITH_WRAP      0x77
#define W25Q_FAST_READ_QUAD_IO        0xEB
#define W25Q_WORD_READ_QUAD_IO        0xE7
#define W25Q_OCTAL_WORD_READ_QUAD_IO  0xE3
#define W25Q_MFTR_DEVICE_ID_QUAD_IO   0x94

//////////////////////////////////////////////////////////////////////////////////
void W25QXX_Init(void);
u16  W25QXX_ReadID(void);               //读取FLASH ID
u8   W25QXX_ReadSR(void);               //读取状态寄存器 
void W25QXX_Write_SR(u8 sr);            //写状态寄存器
void W25QXX_Write_Enable(void);         //写使能 
void W25QXX_Write_Disable(void);        //写保护
void W25QXX_Write_NoCheck(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite);
void W25QXX_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead);   //读取flash
void W25QXX_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite);//写入flash
void W25QXX_Write_Page(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite);
void W25QXX_Erase_Chip(void);           //整片擦除
void W25QXX_Erase_Sector(u32 Dst_Addr); //扇区擦除
void W25QXX_Wait_Busy(void);            //等待空闲
void W25QXX_PowerDown(void);            //进入掉电模式
void W25QXX_WAKEUP(void);               //唤醒

#endif
















