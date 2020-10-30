
/**
  ******************************************************************************
  * @file    system_lowpower.c 
  * @authors Pablo Fuentes and Joseph Peñafiel
	* @version V1.0.1
  * @date    2019
  * @brief   System Low Power Functions
  ******************************************************************************
*/

//#include "System.h"
#include "main.h"

// Fix MACRO definition
#undef LL_PWR_MODE_STOP0
#undef LL_PWR_MODE_STOP1
#define LL_PWR_MODE_STOP0 (0U)
#define LL_PWR_MODE_STOP1 (PWR_CR1_LPMS_0)

/** 
 ===============================================================================
              ##### Public Function System Clock #####
 ===============================================================================
 */
static void nothing(void) {}

static voidFuncPtr cur_clock = nothing;

void clock_init(void (*clockFunc)(void))
{
  clockFunc();
  cur_clock = clockFunc;
}

/** 
 ===============================================================================
              ##### Public functions #####
 ===============================================================================
 */

// ==========================================
// ==== Sleep Functions
// ==========================================

void system_sleepSeconds(uint32_t seconds)
{
  __disable_irq();
  LL_SYSTICK_DisableIT();
  NVIC_EnableIRQ(RTC_TAMP_IRQn);
  rtc_setAlarmBAfter(seconds);
  LL_LPM_EnableSleep();
  __WFI();
  __enable_irq();
  LL_SYSTICK_EnableIT();
}

void system_sleepMillis(uint32_t milliseconds)
{
  __disable_irq();
  LL_SYSTICK_DisableIT();
  NVIC_EnableIRQ(RTC_TAMP_IRQn);
  rtc_setWKUPMillis(milliseconds);
  LL_LPM_EnableSleep();
  __WFI();
  __enable_irq();
  LL_SYSTICK_EnableIT();
  rtc_setWKUPMillis(0); //disable rtc interrupt
}

// ==========================================
// ==== Low power Sleep Functions
// ==========================================

void system_sleepLPSeconds(uint32_t seconds)
{
  LL_FLASH_DisablePrefetch();
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  LL_PWR_EnableFlashPowerDownInLPSleep();
  LL_RCC_DeInit();
  CLOCK_HSI_2MHZ();
  __disable_irq();
  LL_SYSTICK_DisableIT();
  NVIC_EnableIRQ(RTC_TAMP_IRQn);
  rtc_setAlarmBAfter(seconds);
  LL_PWR_EnableLowPowerRunMode();
  LL_LPM_EnableSleep();
  __WFI();
  // Exitting low power modes
  LL_PWR_DisableLowPowerRunMode();
  LL_PWR_DisableFlashPowerDownInLPSleep();
  while (LL_PWR_IsEnabledLowPowerRunMode() == 1)
    ;
  LL_RCC_DeInit();
  cur_clock();
  __enable_irq();
  LL_SYSTICK_EnableIT();
}

void system_sleepLPMillis(uint32_t milliseconds)
{
  LL_FLASH_DisablePrefetch();
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  LL_PWR_EnableFlashPowerDownInLPSleep();
  LL_RCC_DeInit();
  CLOCK_HSI_2MHZ();
  __disable_irq();
  LL_SYSTICK_DisableIT();
  NVIC_EnableIRQ(RTC_TAMP_IRQn);
  rtc_setWKUPMillis(milliseconds);
  LL_PWR_EnableLowPowerRunMode();
  LL_LPM_EnableSleep();
  __WFI();
  // Exitting low power modes
  LL_PWR_DisableLowPowerRunMode();
  LL_PWR_DisableFlashPowerDownInLPSleep();
  while (LL_PWR_IsEnabledLowPowerRunMode() == 1);
  LL_RCC_DeInit();
  cur_clock();
  __enable_irq();
  LL_SYSTICK_EnableIT();
  rtc_setWKUPMillis(0);
}

// ==========================================
// ==== Stop 0 Functions
// ==========================================

void system_stop0Seconds(uint32_t seconds)
{
  CLOCK_HSI_2MHZ();
  __disable_irq();
  LL_SYSTICK_DisableIT();
  NVIC_EnableIRQ(RTC_TAMP_IRQn);
  rtc_setAlarmBAfter(seconds);
  LL_PWR_SetPowerMode(LL_PWR_MODE_STOP0);
  LL_LPM_EnableDeepSleep();
  __WFI();
  LL_LPM_EnableSleep();
  cur_clock();
  __enable_irq();
}

void system_stop0Millis(uint32_t milliseconds)
{
  CLOCK_HSI_2MHZ();
  __disable_irq();
  LL_SYSTICK_DisableIT();
  NVIC_EnableIRQ(RTC_TAMP_IRQn);
  rtc_setWKUPMillis(milliseconds);
  LL_PWR_SetPowerMode(LL_PWR_MODE_STOP0);
  LL_LPM_EnableDeepSleep();
  __WFI();
  LL_LPM_EnableSleep();
  cur_clock();
  __enable_irq();
  rtc_setWKUPMillis(0);
}

void system_stop0UntilInterrupt(void)
{
  CLOCK_HSI_2MHZ();
  LL_SYSTICK_DisableIT();
  LL_PWR_SetPowerMode(LL_PWR_MODE_STOP0);
  LL_LPM_EnableDeepSleep();
  __WFI();
  LL_LPM_EnableSleep();
  cur_clock();
}

// ==========================================
// ==== Stop 1 Functions
// ==========================================

void system_stop1Seconds(uint32_t seconds)
{
  CLOCK_HSI_2MHZ();
  __disable_irq();
  LL_SYSTICK_DisableIT();
  NVIC_EnableIRQ(RTC_TAMP_IRQn);
  rtc_setAlarmBAfter(seconds);
  LL_PWR_SetPowerMode(LL_PWR_MODE_STOP1);
  LL_LPM_EnableDeepSleep();
  __WFI();
  LL_LPM_EnableSleep();
  cur_clock();
  __enable_irq();
}

void system_stop1Millis(uint32_t milliseconds)
{
  CLOCK_HSI_2MHZ();
  __disable_irq();
  LL_SYSTICK_DisableIT();
  NVIC_EnableIRQ(RTC_TAMP_IRQn);
  rtc_setWKUPMillis(milliseconds);
  LL_PWR_SetPowerMode(LL_PWR_MODE_STOP1);
  LL_LPM_EnableDeepSleep();
  __WFI();
  LL_LPM_EnableSleep();
  cur_clock();
  __enable_irq();
  rtc_setWKUPMillis(0);
}

void system_stop1UntilInterrupt(void)
{
  CLOCK_HSI_2MHZ();
  LL_SYSTICK_DisableIT();
  LL_PWR_SetPowerMode(LL_PWR_MODE_STOP1);
  LL_LPM_EnableDeepSleep();
  __WFI();
  LL_LPM_EnableSleep();
  cur_clock();
}

// ==========================================
// ==== Standby Functions
// ==========================================

void system_standby(void)
{
  if (LL_PWR_IsActiveFlag_SB() != 0)
  {
    LL_PWR_ClearFlag_SB();
  }
  LL_SYSTICK_DisableIT();
  LL_PWR_ClearFlag_WU();
  #if defined(PWR_CR3_RRS)
  LL_PWR_DisableSRAMRetention();
  #endif
  LL_PWR_SetPowerMode(LL_PWR_MODE_STANDBY);
  LL_LPM_EnableDeepSleep();
  #if defined(__CC_ARM)
  __force_stores();
  #endif
  __WFI();
}

void system_standbySeconds(uint32_t seconds)
{
  if (LL_PWR_IsActiveFlag_SB() != 0)
  {
    LL_PWR_ClearFlag_SB();
  }
  __disable_irq();
  LL_SYSTICK_DisableIT();
  LL_PWR_ClearFlag_WU();
  NVIC_EnableIRQ(RTC_TAMP_IRQn);
  rtc_setAlarmBAfter(seconds);
  #if defined(PWR_CR3_RRS)
  LL_PWR_DisableSRAMRetention();
  #endif
  LL_PWR_SetPowerMode(LL_PWR_MODE_STANDBY);
  LL_LPM_EnableDeepSleep();
  #if defined(__CC_ARM)
  __force_stores();
  #endif
  __WFI();
}

void system_standbyUntilWakeUpPin(uint32_t WAKEUP_PIN_x)
{
  if (LL_PWR_IsActiveFlag_SB() != 0)
  {
    LL_PWR_ClearFlag_SB();
  }
  __disable_irq();
  LL_SYSTICK_DisableIT();
  LL_PWR_DisableWakeUpPin(WAKEUP_PIN_x);
  LL_PWR_ClearFlag_WU();
  //if (polarity)
  //{
  //  LL_PWR_SetWakeUpPinPolarityHigh(WAKEUP_PIN_x);
  //}
  //else
  //{
  //  LL_PWR_SetWakeUpPinPolarityLow(WAKEUP_PIN_x);
  //}
  LL_PWR_EnableWakeUpPin(WAKEUP_PIN_x);
  LL_PWR_ClearFlag_WU();
  #if defined(PWR_CR3_RRS)
  LL_PWR_DisableSRAMRetention();
  #endif
  LL_RCC_DisableRTC();
  LL_RCC_LSI_Disable();
  LL_RCC_LSE_Disable();
  LL_PWR_SetPowerMode(LL_PWR_MODE_STANDBY);
  LL_LPM_EnableDeepSleep();
  #if defined(__CC_ARM)
  __force_stores();
  #endif
  __WFI();
}

// ==========================================
// ==== Standby SRAM Functions
// ==========================================

#if defined(PWR_CR3_RRS)
void system_standbySRAM(void)
{
  if (LL_PWR_IsActiveFlag_SB() != 0)
  {
    LL_PWR_ClearFlag_SB();
  }
  LL_SYSTICK_DisableIT();
  LL_PWR_ClearFlag_WU();
  LL_PWR_EnableSRAMRetention();
  LL_PWR_SetPowerMode(LL_PWR_MODE_STANDBY);
  LL_LPM_EnableDeepSleep();
  #if defined(__CC_ARM)
  __force_stores();
  #endif
  __WFI();
}

void system_standbySRAMSeconds(uint32_t seconds)
{
  if (LL_PWR_IsActiveFlag_SB() != 0)
  {
    LL_PWR_ClearFlag_SB();
  }
  __disable_irq();
  LL_SYSTICK_DisableIT();
  LL_PWR_ClearFlag_WU();
  NVIC_EnableIRQ(RTC_TAMP_IRQn);
  rtc_setAlarmBAfter(seconds);
  LL_PWR_EnableSRAMRetention();
  LL_PWR_SetPowerMode(LL_PWR_MODE_STANDBY);
  LL_LPM_EnableDeepSleep();
  #if defined(__CC_ARM)
  __force_stores();
  #endif
  __WFI();
}

void system_standbySRAMUntilWakeUpPin(uint32_t WAKEUP_PIN_x, uint8_t polarity)
{
  if (LL_PWR_IsActiveFlag_SB() != 0)
  {
    LL_PWR_ClearFlag_SB();
  }
  __disable_irq();
  LL_SYSTICK_DisableIT();
  LL_PWR_DisableWakeUpPin(WAKEUP_PIN_x);
  LL_PWR_ClearFlag_WU();
  if (polarity)
  {
    LL_PWR_SetWakeUpPinPolarityHigh(WAKEUP_PIN_x);
  }
  else
  {
    LL_PWR_SetWakeUpPinPolarityLow(WAKEUP_PIN_x);
  }
  LL_PWR_EnableWakeUpPin(WAKEUP_PIN_x);
  LL_PWR_ClearFlag_WU();
  LL_PWR_EnableSRAMRetention();
  LL_RCC_DisableRTC();
  LL_RCC_LSI_Disable();
  LL_RCC_LSE_Disable();
  LL_PWR_SetPowerMode(LL_PWR_MODE_STANDBY);
  LL_LPM_EnableDeepSleep();
  #if defined(__CC_ARM)
  __force_stores();
  #endif
  __WFI();
}
#endif

// ==========================================
// ==== Shutdown Functions
// ==========================================

#if defined(PWR_CR1_LPMS_2)

void system_shutdown(void)
{
  if (LL_PWR_IsActiveFlag_SB() != 0)
  {
    LL_PWR_ClearFlag_SB();
  }
  LL_SYSTICK_DisableIT();
  LL_PWR_ClearFlag_WU();
  LL_PWR_SetPowerMode(LL_PWR_MODE_SHUTDOWN);
  LL_LPM_EnableDeepSleep();
  #if defined(__CC_ARM)
  __force_stores();
  #endif
  __WFI();
}

void system_shutdownSeconds(uint32_t seconds)
{
  if (LL_PWR_IsActiveFlag_SB() != 0)
  {
    LL_PWR_ClearFlag_SB();
  }
  __disable_irq();
  LL_SYSTICK_DisableIT();
  LL_PWR_ClearFlag_WU();
  NVIC_EnableIRQ(RTC_TAMP_IRQn);
  rtc_setAlarmBAfter(seconds);
  LL_PWR_SetPowerMode(LL_PWR_MODE_SHUTDOWN);
  LL_LPM_EnableDeepSleep();
  #if defined(__CC_ARM)
  __force_stores();
  #endif
  __WFI();
}

void system_shutdownUntilWakeUpPin(uint32_t WAKEUP_PIN_x, uint8_t polarity)
{
  if (LL_PWR_IsActiveFlag_SB() != 0)
  {
    LL_PWR_ClearFlag_SB();
  }
  __disable_irq();
  LL_SYSTICK_DisableIT();
  LL_PWR_DisableWakeUpPin(WAKEUP_PIN_x);
  LL_PWR_ClearFlag_WU();
  if (polarity)
  {
    LL_PWR_SetWakeUpPinPolarityHigh(WAKEUP_PIN_x);
  }
  else
  {
    LL_PWR_SetWakeUpPinPolarityLow(WAKEUP_PIN_x);
  }
  LL_PWR_EnableWakeUpPin(WAKEUP_PIN_x);
  LL_PWR_ClearFlag_WU();
  LL_RCC_DisableRTC();
  LL_RCC_LSI_Disable();
  LL_RCC_LSE_Disable();
  LL_PWR_SetPowerMode(LL_PWR_MODE_SHUTDOWN);
  LL_LPM_EnableDeepSleep();
  #if defined(__CC_ARM)
  __force_stores();
  #endif
  __WFI();
}

#endif

//==============================================================================
//==== Other Functions
//==============================================================================
void system_turnOffAllGPIOs(void)
{
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_ALL);
#ifdef GPIOA
  GPIOA->MODER = 0xFFFFFFFF;
  GPIOA->OTYPER = 0x00000000;
  GPIOA->OSPEEDR = 0x00000000;
  GPIOA->PUPDR = 0x00000000;
  GPIOA->ODR = 0x00000000;
  GPIOA->BSRR = 0x00000000;
  GPIOA->BRR = 0x00000000;
#endif
#ifdef GPIOB
  GPIOB->MODER = 0xFFFFFFFF;
  GPIOB->OTYPER = 0x00000000;
  GPIOB->OSPEEDR = 0x00000000;
  GPIOB->PUPDR = 0x00000000;
  GPIOB->ODR = 0x00000000;
  GPIOB->BSRR = 0x00000000;
  GPIOB->BRR = 0x00000000;
#endif
#ifdef GPIOC
  GPIOC->MODER = 0xFFFFFFFF;
  GPIOC->OTYPER = 0x00000000;
  GPIOC->OSPEEDR = 0x00000000;
  GPIOC->PUPDR = 0x00000000;
  GPIOC->ODR = 0x00000000;
  GPIOC->BSRR = 0x00000000;
  GPIOC->BRR = 0x00000000;
#endif
#ifdef GPIOD
  GPIOD->MODER = 0xFFFFFFFF;
  GPIOD->OTYPER = 0x00000000;
  GPIOD->OSPEEDR = 0x00000000;
  GPIOD->PUPDR = 0x00000000;
  GPIOD->ODR = 0x00000000;
  GPIOD->BSRR = 0x00000000;
  GPIOD->BRR = 0x00000000;
#endif
#ifdef GPIOF
  GPIOF->MODER = 0xFFFFFFFF;
  GPIOF->OTYPER = 0x00000000;
  GPIOF->OSPEEDR = 0x00000000;
  GPIOF->PUPDR = 0x00000000;
  GPIOF->ODR = 0x00000000;
  GPIOF->BSRR = 0x00000000;
  GPIOF->BRR = 0x00000000;
#endif
  LL_IOP_GRP1_DisableClock(LL_IOP_GRP1_PERIPH_ALL);
}

