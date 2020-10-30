

#ifndef BMA253_ACCEL_H
#define BMA253_ACCEL_H

#include <stdint.h>

/* BMA253 I2C ADDRESS */
#define BMA253_ADDR                         (0x19)

/* DATA REGISTER ADDRESSES */
#define BMA253_CHIP_ID_ADDR                 (0x00)
#define BMA253_X_LSB_ADDR                   (0x02)
#define BMA253_X_MSB_ADDR                   (0x03)
#define BMA253_Y_LSB_ADDR                   (0x04)
#define BMA253_Y_MSB_ADDR                   (0x05)
#define BMA253_Z_LSB_ADDR                   (0x06)
#define BMA253_Z_MSB_ADDR                   (0x07)
#define BMA253_BW_SELECT_ADDR               (0x10)
#define BMA253_PWR_MODE_ADDR                (0x11)
#define BMA253_RANGE_ADDR                   (0x0F)

/* INTERRUPT REGISTER ADRESSES */
#define BMA253_INT_EN_1_ADDR                (0x17) //bit 4 data ready interrupt '0'-> disabled '1'-> enabled
#define BMA253_INT1_PIN_ADDR                (0x19)
#define BMA253_INT_DATA_ADDR                (0x1A) //bit 0 data ready to int1 pin '0'-> disabled '1'-> enabled

/* POWER MODE DEFINITONS */
#define BMA253_NORMAL_MODE                  (0x00) //default power mode

/* BANDWITH SETTINGS */
#define BMA253_BW_7_81HZ                    (0x08)
#define BMA253_BW_15_63HZ                   (0x09)
#define BMA253_BW_31_25HZ                   (0x0A)
#define BMA253_BW_62_5HZ                    (0x0B)
#define BMA253_BW_125HZ                     (0x0C)
#define BMA253_BW_250HZ                     (0x0D)
#define BMA253_BW_500HZ                     (0x0E)
#define BMA253_BW_1KHZ                      (0x0F)

/* ACCEL G-RANGE SETTINGS */
#define BMA253_2G_RANGE                     (0x03) // default range
#define BMA253_4G_RANGE                     (0x05)
#define BMA253_8G_RANGE                     (0x08)
#define BMA253_16G_RANGE                    (0x0C)

typedef struct
{
  int16_t x;
  int16_t y;
  int16_t z;
}BMA253_ACCEL_DATA_t;

//------------------------------------------------------------------------------
void BMA253_Initialize(void);
void BMA253_GetAccelDataX(int16_t *xAccelData);
void BMA253_GetAccelDataY(int16_t *yAccelData);
void BMA253_GetAccelDataZ(int16_t *zAccelData);
void BMA253_GetAccelDataXYZ(BMA253_ACCEL_DATA_t *accelData);
uint8_t BMA253_GetAccelChipId(void);

#endif  /* BMA253_ACCEL_H */

