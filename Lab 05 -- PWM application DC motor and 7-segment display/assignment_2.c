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

/* For 1000ms update event */
#define TIMx_PSC            32000 
#define TIMx_ARR            1000

void SystemClock_Config(void);
void TIM_BASE_Config(void);
void TIM_OC_GPIO_Config(void);
void TIM_OC_Config(void);
void L293D_Config(void);

int main()
{
    SystemClock_Config();
    L293D_Config();
    
    // Reset state
    LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_4);
    LL_TIM_SetCounter(TIM3, RESET);
    // EN
    LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_7);

    while(1);
}

void TIM_BASE_Config(void)
{
    LL_TIM_InitTypeDef TIMBASE_InitStruct;
    
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);
    
    TIMBASE_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV2;
    TIMBASE_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
    TIMBASE_InitStruct.Autoreload = TIMx_ARR - 1;
    TIMBASE_InitStruct.Prescaler =  TIMx_PSC - 1;
    
    LL_TIM_Init(TIM3, &TIMBASE_InitStruct);
}

void TIM_OC_GPIO_Config(void)
{
    LL_GPIO_InitTypeDef GPIO_InitStruct;
    
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pin = LL_GPIO_PIN_5;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void TIM_OC_Config(void)
{
    LL_TIM_OC_InitTypeDef TIM_OC_InitStruct;
    
    TIM_BASE_Config();
    TIM_OC_GPIO_Config();
    
    TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
    TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
    TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
//  TIM_OC_InitStruct.CompareValue = LL_TIM_GetAutoReload(TIM3) * 0.2; //  20% duty
//  TIM_OC_InitStruct.CompareValue = LL_TIM_GetAutoReload(TIM3) * 0.4; //  40% duty
//  TIM_OC_InitStruct.CompareValue = LL_TIM_GetAutoReload(TIM3) * 0.6; //  60% duty
//  TIM_OC_InitStruct.CompareValue = LL_TIM_GetAutoReload(TIM3) * 0.8; //  80% duty
    TIM_OC_InitStruct.CompareValue = LL_TIM_GetAutoReload(TIM3);       // 100% duty
    LL_TIM_OC_Init(TIM3, LL_TIM_CHANNEL_CH2, &TIM_OC_InitStruct);
    /*Interrupt Configure*/
    NVIC_SetPriority(TIM3_IRQn, 1);
    NVIC_EnableIRQ(TIM3_IRQn);
    LL_TIM_EnableIT_CC2(TIM3);
    
    /*Start Output Compare in PWM Mode*/
    LL_TIM_CC_EnableChannel(TIM3, LL_TIM_CHANNEL_CH2);
    LL_TIM_EnableCounter(TIM3);
}

void L293D_Config(void)
{
    LL_GPIO_InitTypeDef l293d_InitStruct;

    TIM_OC_Config();
    
    // Configure l293d
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    
    l293d_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    l293d_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    l293d_InitStruct.Pull = LL_GPIO_PULL_NO;
    l293d_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    l293d_InitStruct.Pin = LL_GPIO_PIN_4 | LL_GPIO_PIN_7;
    LL_GPIO_Init(GPIOB, &l293d_InitStruct);

    l293d_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    l293d_InitStruct.Alternate = LL_GPIO_AF_2;
    l293d_InitStruct.Pin = LL_GPIO_PIN_5;
    LL_GPIO_Init(GPIOB, &l293d_InitStruct);
}

void TIM3_IRQHandler(void)
{
    if (LL_TIM_IsActiveFlag_CC2(TIM3) == SET) {
        LL_TIM_ClearFlag_CC2(TIM3);
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
