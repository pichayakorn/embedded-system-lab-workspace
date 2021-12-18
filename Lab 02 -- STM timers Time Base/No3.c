/* Base registers address header file */
#include "stm32l1xx.h"
/* Library related header files */
#include "stm32l1xx_ll_system.h"
#include "stm32l1xx_ll_bus.h"
#include "stm32l1xx_ll_gpio.h"
#include "stm32l1xx_ll_pwr.h"
#include "stm32l1xx_ll_rcc.h"
#include "stm32l1xx_ll_utils.h"
/* Timer peripheral driver included */
#include "stm32l1xx_ll_tim.h"
/* LCD peripheral driver included */
#include <stdio.h>
#include "stm32l1xx_ll_lcd.h"
#include "stm32l152_glass_lcd.h"

void SystemClock_Config(void);
void TIMBase_Config(void);
void GPIO_Config(void);
void LED_ON(void);

uint16_t cnt = 0;
uint8_t cnt_down = 30;
char disp_str[7];

int main()
{
    SystemClock_Config();
    LCD_GLASS_Init();                   // LCD low-level init
    GPIO_Config();
    TIMBase_Config();
    
    sprintf(disp_str, "%d", cnt_down);
    LCD_GLASS_DisplayString((uint8_t*)disp_str);
    
    while(1) {
        cnt = LL_TIM_GetCounter(TIM2);
    }
}

void TIMBase_Config(void) {
    LL_TIM_InitTypeDef TIM_InitStruct;
    
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
    
    TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.Autoreload = 1000 - 1;
    TIM_InitStruct.Prescaler = 32000 - 1;
    
    LL_TIM_Init(TIM2, &TIM_InitStruct);
    
    LL_TIM_ClearFlag_UPDATE(TIM2);
    LL_TIM_EnableIT_UPDATE(TIM2);
    
    NVIC_SetPriority(TIM2_IRQn, 0);
    NVIC_EnableIRQ(TIM2_IRQn);

    LL_TIM_EnableCounter(TIM2);
}

void GPIO_Config(void) {
    /* Declare struct for GPIO config */
    LL_GPIO_InitTypeDef GPIO_InitStruct;
    /* Enable GPIOB clock */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    
    /* Config LED4 (PB6) and LED3 (PB7) as input */
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    
    GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);                  // Write configure value to registers

    GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);                  // Write configure value to registers
}

void LED_ON(void) {
    LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_6);
    LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_7);
}

void TIM2_IRQHandler(void) {
    if (LL_TIM_IsActiveFlag_UPDATE(TIM2) == SET) {
        LL_TIM_ClearFlag_UPDATE(TIM2);
        if (cnt_down != 0) {
            LL_TIM_SetCounter(TIM2, 0);
            LCD_GLASS_Clear();
            cnt_down--;
            sprintf(disp_str, "%d", cnt_down);
            LCD_GLASS_DisplayString((uint8_t*)disp_str);
            if (cnt_down == 0) {
                LL_TIM_DisableCounter(TIM2);
                LED_ON();
            }
        }
    }
}

void SystemClock_Config(void)
{
    /* Enable ACC64 access and set FLASH latency */ 
    LL_FLASH_Enable64bitAccess();; 
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

    /* Set Voltage scale1 as MCU will run at 32MHz */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);

    /* Poll VOSF bit of in PWR_CSR. Wait until it is reset to 0 */
    while (LL_PWR_IsActiveFlag_VOSF() != 0)
    {
    };

    /* Enable HSI if not already activated*/
    if (LL_RCC_HSI_IsReady() == 0)
    {
        /* HSI configuration and activation */
        LL_RCC_HSI_Enable();
        while(LL_RCC_HSI_IsReady() != 1)
        {
        };
    }
  
    /* Main PLL configuration and activation */
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLL_MUL_6, LL_RCC_PLL_DIV_3);

    LL_RCC_PLL_Enable();
    while(LL_RCC_PLL_IsReady() != 1)
    {
    };

    /* Sysclk activation on the main PLL */
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
    {
    };

    /* Set APB1 & APB2 prescaler*/
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

    /* Set systick to 1ms in using frequency set to 32MHz                             */
    /* This frequency can be calculated through LL RCC macro                          */
    /* ex: __LL_RCC_CALC_PLLCLK_FREQ (HSI_VALUE, LL_RCC_PLL_MUL_6, LL_RCC_PLL_DIV_3); */
    LL_Init1msTick(32000000);

    /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
    LL_SetSystemCoreClock(32000000);
}
