/* Base registers address header file */
#include "stm32l1xx.h"
/* Library related header files */
#include "stm32l1xx_ll_system.h"
#include "stm32l1xx_ll_bus.h"
#include "stm32l1xx_ll_gpio.h"
#include "stm32l1xx_ll_pwr.h"
#include "stm32l1xx_ll_rcc.h"
#include "stm32l1xx_ll_utils.h"
/* EXTI driver included */
#include "stm32l1xx_ll_exti.h"

void SystemClock_Config(void);

int main()
{
    /* Declare struct for GPIO config */
    LL_GPIO_InitTypeDef GPIO_InitStruct;

    SystemClock_Config();               // Max-performance configure
    
    /* Enable GPIOA and GPIOB clock */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    
    /* Enable System configuration controller clock */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);

    /* Config User-Botton GPIOA PA0 as input */
    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Pin = LL_GPIO_PIN_0;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);                  // Write configure value to registers
    
    /* Config LED4 (PB6) and LED3 (PB7) as input */
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);                  // Write configure value to registers

    GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);                  // Write configure value to registers
    
    /* EXTI Line 0 (EXTI0) for PA0 */
    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE0);
    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_0);
    LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_0);
    
    /* NVIC */
    NVIC_EnableIRQ((IRQn_Type) 6);
    NVIC_SetPriority((IRQn_Type) 6, 0);
    
    /* Reset start-up LED state */
    LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_6);
    LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_7);
        
    while(1);
}

void EXTI0_IRQHandler(void) {
    // Check if EXTI0 bit is seted by interrupt
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_0)) {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_0);             // Clear pending bit by writing 1
        LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_6);
        LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_7);
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
