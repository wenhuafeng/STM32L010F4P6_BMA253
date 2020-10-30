
/*******************************************************************************
* Copytight 2020 raycohk Tech.Co., Ltd. All rights reserved                    *
*                                                                              *
* Filename      : BMA253.c                                                     *
* Author        : wenhuafeng                                                   *
* Version       : 1.0                                                          *
*                                                                              *
* Decription    : BMA GSensor Init                                             *
*                                                                              *
* Created       : 2020-05-13                                                   *
* Last modified : 2020.06.06                                                   *
*******************************************************************************/

#include "bma2x2.h"
#include "bma253.h"

#include "Task_Step.h"
#include "TypeDefine.h"
#include "bma253_iic.h"
#include "main.h"


#if (_PEDOMETER_ && !_BMA253_NEW_DRIVE_)

//------------------------------------------------------------------------------
struct bma2x2_t bma2x2;

//#define BMA253_I2C_SLAVE_ADDRESS 0x18
//#define BMA253_I2C_SLAVE_ADDRESS 0x19

// Left Shit 1 bit
#define BMA253_I2C_SLAVE_ADDRESS  0x30
//#define BMA253_I2C_SLAVE_ADDRESS 0x32

// b'1111'1010
#define BMA253_CHIP_ID            (0xFA)

#define BMA2x2_ACCEL_INTR1_LEVEL  0
#define BMA2x2_ACCEL_INTR2_LEVEL  1



//#define CS_BMA_ENABLE()   nrf_gpio_pin_clear(BMA_SPI_CS_PIN)
//#define CS_BMA_DISABLE()  nrf_gpio_pin_set(BMA_SPI_CS_PIN)

//#define BMA_POWER_DOWN()  nrf_gpio_pin_clear(BMA_POWER_PIN)
//#define BMA_POWER_ON()    nrf_gpio_pin_set(BMA_POWER_PIN)

//------------------------------------------------------------------------------
/**
 * void init_spi_master(void);
 * Set the SPI pin SCLK, MISO, MOSI And
 * Initialize the SPI Master. 
 */
void BMA_init_spi_pin(void)
{
  
}

/**
 * Brief : bma interrupt init
 * param : none
 * return: none
*/
void bma_int_init(void)
{
  
}

/**
 * Brief : The delay routine
 * param : delay in ms
 * return: none
*/
void BMA2x2_delay_msek(u8 msek)
{
  /*Here you can write your own delay routine*/

  LL_mDelay(msek);
  
  //int t = 1000*msek;
  //
  //while (t-- > 0)
  //{
  //  __asm("nop");
  //}
}

/**
 * Brief : bma253 deep sleep
 * param : =0 disable, =1 enable.
 * return: none
*/
void make_bma250_to_deep_sleep(FlagStatus enable)
{
  u8 cnt = 0;
  
  if (enable) {
    u8 powerMode = 0;
    do { 
      bma2x2_set_power_mode(BMA2x2_MODE_DEEP_SUSPEND);
      bma2x2_get_power_mode(&powerMode);
    }while(powerMode != BMA2x2_MODE_DEEP_SUSPEND && (++cnt<32));
  } else {
    bma253_init();
  }
}


u8 error_cnt; //TEST
/**
 * Brief : bma253 init
 * param : =0 disable, =1 enable.
 * return: none
*/
ErrorStatus bma253_init(void)
{
  u8 ret, cnt = 0;
  u8 BackRead = 0;
  //u8 error_cnt;
  
  error_cnt = 0;
  BMA_init_spi_pin();
  bma_int_init();
  //spi_configure(NRF_SPI0, BMA_SPI_SCL_PIN, BMA_SPI_MOSI_PIN, BMA_SPI_MISO_PIN);
  bma2x2.chip_id = BMA253_CHIP_ID;
  bma2x2.dev_addr = BMA253_I2C_SLAVE_ADDRESS;
  bma2x2.bus_write  = BMA253_Write_LenBytes;//BMA2x2_SPI_bus_write;
  bma2x2.bus_read   = BMA253_Read_LenBytes;//BMA2x2_SPI_bus_read;
  bma2x2.delay_msec = BMA2x2_delay_msek;

  bma2x2_init(&bma2x2);
  
  bma2x2_soft_rst();
  BMA2x2_delay_msek(10);
  
  bma2x2_soft_rst();
  BMA2x2_delay_msek(10);
  
  cnt = 0;
  /* Set PowerMode to BMA2x2_MODE_LOWPOWER1 */
  do { 
    bma2x2_set_power_mode(BMA2x2_MODE_NORMAL);
    bma2x2_get_power_mode( &BackRead );
  }while( BackRead != BMA2x2_MODE_NORMAL && (++cnt<32));
  if (cnt >= 32) {
    __asm("nop");
    error_cnt++;
  }
  
  BMA2x2_delay_msek(10);
    
  /* software reset (ALL Register realod default values )*/
  bma2x2_soft_rst(); 
  BMA2x2_delay_msek(10);

  /* software reset (ALL Register realod default values )*/
  bma2x2_soft_rst(); 
  BMA2x2_delay_msek(10);
  
  /* Configure the BMA250E work at FIFO mode */
  cnt = 0;
  do{
    bma2x2_set_bw(BMA2x2_BW_500HZ);
    bma2x2_get_bw( &BackRead );
  }while( BackRead != BMA2x2_BW_500HZ && (++cnt<32));
  if (cnt >= 32) {
    __asm("nop");
    error_cnt++;
  }

  /*中断信号为推挽输出模式 */
  cnt = 0;
  do{
    bma2x2_set_intr_output_type(BMA2x2_ACCEL_INTR1_LEVEL, 0);//PUSH_PULL
    bma2x2_get_intr_output_type(BMA2x2_ACCEL_INTR1_LEVEL, &BackRead);
  }while( BackRead != 0  && (++cnt<32));
  if (cnt >= 32) {
    __asm("nop");
    error_cnt++;
  }

   /* 中断信号为高电平 */
  cnt = 0;
  do{
    bma2x2_set_intr_level(BMA2x2_ACCEL_INTR1_LEVEL, ACTIVE_HIGH);
    bma2x2_get_intr_level(BMA2x2_ACCEL_INTR1_LEVEL, &BackRead);
  }while( BackRead != ACTIVE_HIGH && (++cnt<32));
  if (cnt >= 32) {
    __asm("nop");
    error_cnt++;
  }
  
  /* Clear any latch interrupt */
  cnt = 0;
  do{
    ret = bma2x2_rst_intr(0x1);  
  }while( ret != true && (++cnt<32)); 
  if (cnt >= 32) {
    __asm("nop");
    error_cnt++;
  }
  
  /** int signal latch 1ms */
  cnt = 0;
  do{
    bma2x2_set_latch_intr(BMA2x2_LATCH_DURN_1MS);
    bma2x2_get_latch_intr( &BackRead );
  }while( BackRead != BMA2x2_LATCH_DURN_1MS && (++cnt<32));
  if (cnt >= 32) {
    __asm("nop");
    error_cnt++;
  }

  cnt = 0;
  do{
    bma2x2_set_range(BMA2x2_RANGE_2G);
    bma2x2_get_range( &BackRead );
  }while( BackRead != BMA2x2_RANGE_2G && (++cnt<32));
  if (cnt >= 32) {
    __asm("nop");
    error_cnt++;
  }
  
  /* FIFO accel data selected : XYZ in FIFO */
  cnt = 0;
  do{
    bma2x2_set_fifo_data_select(0x0);
    bma2x2_get_fifo_data_select( &BackRead );
  }while( BackRead != 0x0 && (++cnt<32)); 
  if (cnt >= 32) {
    __asm("nop");
    error_cnt++;
  }
  
   /* FIFO mode : stream mode  */
  cnt = 0;
  do{
    bma2x2_set_fifo_mode(2); 
    bma2x2_get_fifo_mode( &BackRead );
  }while( BackRead != 2 && (++cnt<32));
  if (cnt >= 32) {
    __asm("nop");
    error_cnt++;
  }
  
  //// water marker level 20 ///////////////////
  cnt = 0;
  do{
    bma2x2_set_fifo_wml_trig(FIFO_DEPTH);
    bma2x2_get_fifo_wml_trig( &BackRead );
  }while( BackRead != FIFO_DEPTH && (++cnt<32));
  if (cnt >= 32) {
    __asm("nop");
    error_cnt++;
  }
  
  cnt = 0;
  do{
    bma2x2_set_intr_fifo_wm(INTR_ENABLE);
    bma2x2_get_intr_fifo_wm( &BackRead );
  }while( BackRead != INTR_ENABLE && (++cnt<32));
  if (cnt >= 32) {
    __asm("nop");
    error_cnt++;
  }
  
  cnt = 0;
  do{
    bma2x2_set_intr1_fifo_wm(INTR_ENABLE);
    bma2x2_get_intr1_fifo_wm( &BackRead );
  }while( BackRead != INTR_ENABLE && (++cnt<32)); 
  if (cnt >= 32) {
    __asm("nop");
    error_cnt++;
  }

  cnt = 0;
  do{
    bma2x2_set_sleep_durn(BMA2x2_SLEEP_DURN_25MS);
    bma2x2_get_sleep_durn( &BackRead );
  }while( BackRead != BMA2x2_SLEEP_DURN_25MS && (++cnt<32));
  if (cnt >= 32) {
    __asm("nop");
    error_cnt++;
  }
  
  cnt = 0;
  do{
    bma2x2_set_sleep_timer_mode(1);  /* EST mode */
    bma2x2_get_sleep_timer_mode( &BackRead );
  }while( BackRead != 1 && (++cnt<32)); 
  if (cnt >= 32) {
    __asm("nop");
    error_cnt++;
  }
  
  /* Set PowerMode to BMA2x2_MODE_LOWPOWER1 */
  cnt = 0;
  do { 
    bma2x2_set_power_mode(BMA2x2_MODE_LOWPOWER1); 
    bma2x2_get_power_mode(&BackRead);
  }while( BackRead != BMA2x2_MODE_LOWPOWER1 && (++cnt<32));
  if (cnt >= 32) {
    __asm("nop");
    error_cnt++;
  }
  
  return (error_cnt != 0)?(ERROR):(SUCCESS);
}

#endif

//==============================================================================
//==============================================================================
